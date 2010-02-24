#ifndef MVV_PARSE_VISITOR_TYPE_H_
# define MVV_PARSE_VISITOR_TYPE_H_

# include "visitor.h"
# include "ast-files.h"
# include "type.h"

namespace mvv
{
namespace parser
{
   /**
    @brief Defines a visitor visiting all the nodes but doing nothing
    @note this is usefull if a visitor is only handling a limited number of nodes
    */
   class VisitorType : public Visitor
   {
   public:
      VisitorType( ParserContext& context,
                   const SymbolTableVars& vars,
                   const SymbolTableFuncs& funcs,
                   const SymbolTableClasses& classes ) : _context( context ), _vars( vars ), _funcs( funcs ), _classes( classes )
      {
      }

      virtual void operator()( AstInt& e )
      {
         e.setNodeType( new TypeInt() );
      }

      virtual void operator()( AstFloat& e )
      {
         e.setNodeType( new TypeFloat() );
      }

      virtual void operator()( AstString& e )
      {
         e.setNodeType( new TypeString() );
      }

      virtual void operator()( AstOpBin& e )
      {
         // first evaluate the operand type
         operator()( e.getLeft() );
         operator()( e.getRight() );

         if ( !e.getLeft().getNodeType() )
         {
            e.setNodeType( new TypeVoid() );
            impl::reportTypeError( e.getLeft().getLocation(), _context, "expression cannot be typed");
            return;
         }

         if ( !e.getRight().getNodeType() )
         {
            e.setNodeType( new TypeVoid() );
            impl::reportTypeError( e.getRight().getLocation(), _context, "expression cannot be typed");
            return;
         }

         // types must be compatible
         if ( !e.getRight().getNodeType()->isCompatibleWith( *e.getLeft().getNodeType() ) )
         {
            impl::reportTypeError( e.getLocation(), _context, "incompatible types");
         }

         // TODO: promotion?
         e.setNodeType( e.getLeft().getNodeType() );

         // restrict type
         //
         // TODO operator handling, require function overloading resolution
         //
      }

      virtual void operator()( AstIf& e )
      {
         // TODO check type condition
         operator()( e.getCondition() );

         operator()( e.getThen() );

         if ( e.getElse() )
         {
            operator()( *e.getElse() );
         }
      }

      virtual void operator()( AstStatements& e )
      {
         for ( AstStatements::Statements::const_iterator it = e.getStatements().begin();
               it != e.getStatements().end();
               ++it )
         {
            operator()( **it );
         }
      }

      virtual void operator()( AstExpAssign& e )
      {
         operator()( e.getValue() );
         operator()( e.getLValue() );
         e.setNodeType( e.getValue().getNodeType() );

         ensure( e.getLValue().getNodeType(), "compiler error: cannot evaluate expression type" );
         if ( e.getValue().getNodeType()->isCompatibleWith( *e.getLValue().getNodeType() ) )
         {
            impl::reportTypeError( e.getLocation(), _context, "incompatible types");
         }
      }

      virtual void operator()( AstVarSimple& e )
      {
         if ( e.isFunctionCall() )
         {
            // TODO resolve overloading
         } else {
            ensure( e.getReference(), "compiler error: cannot find reference" );
            e.setNodeType( e.getReference()->getNodeType() );
         }
      }

      virtual void operator()( AstVarArray& e )
      {
         operator()( e.getName() );
         operator()( e.getIndex() );
      }

      virtual void operator()( AstVarField& e )
      {
         operator()( e.getField() );
      }

      virtual void operator()( AstType& e )
      {
         switch ( e.getType() )
         {
         case AstType::FLOAT:
            e.setNodeType( new TypeFloat() );
            break;

         case AstType::INT:
            e.setNodeType( new TypeInt() );
            break;

         case AstType::STRING:
            e.setNodeType( new TypeString() );
            break;

         case AstType::VOID:
            e.setNodeType( new TypeVoid() );
            break;

         case AstType::SYMBOL:
            ensure( e.getReference(), "compiler error: can't find a link on a symbol" );
            e.setNodeType( e.getReference()->getNodeType() );
            break;

         default:
            ensure( 0, "compiler error: primitive type not handled" );
         }
      }

      virtual void operator()( AstDecls& e )
      {
         for ( AstDecls::Decls::const_iterator it = e.getDecls().begin(); it != e.getDecls().end(); ++it )
         {
            operator()( **it );
         }
      }

      virtual void operator()( AstDeclVars& e )
      {
         for ( AstDeclVars::Decls::const_iterator it = e.getVars().begin(); it != e.getVars().end(); ++it )
         {
            operator()( **it );
         }
      }

      virtual void operator()( AstDeclClass& e )
      {
         // TODO remove?
         _defaultClassPath.push_back( e.getName() );

         e.setNodeType( new TypeNamed( &e ) );
         operator()( e.getDeclarations() );

         _defaultClassPath.pop_back();
      }

      virtual void operator()( AstDeclFun& e ) 
      {
         if ( e.getType() )
         {
            operator()( *e.getType() );
            e.setNodeType( e.getType()->getNodeType() );
         } else {
            // if not in class, type can't be empty
            if ( !e.getMemberOfClass() )
            {
               e.setNodeType( new TypeVoid() );
               impl::reportTypeError( e.getLocation(), _context, "missing type in function prototype" );
               return;
            } else {
               // else it is a constructor and must have the same name than the class
               e.setNodeType( new TypeVoid() );

               if ( e.getName() != e.getMemberOfClass()->getName() )
               {
                  impl::reportTypeError( e.getLocation(), _context, "constructor must have the same name than class, or missing function type" );
                  return;
               }
            }
         }

         if ( e.getVars().getVars().size() )
         {
            operator()( e.getVars() );
         }
         
         if ( e.getBody() )
         {
            _returnType.push_back( e.getNodeType() );
            operator()( *e.getBody() );
            _returnType.pop_back();
         }
      }

      virtual void operator()( AstArgs& e )
      {
         for ( AstArgs::Args::const_iterator it = e.getArgs().begin(); it != e.getArgs().end(); ++it )
         {
            operator()( **it );
         }
      }

      virtual void operator()( AstReturn& e )
      {
         if ( e.getReturnValue() )
         {
            operator()( *e.getReturnValue() );
         }
         
         // TODO check type
      }

      virtual void operator()( AstExpCall& e )
      {
         // TODO overloading
         // TODO set type
         operator()( e.getArgs() );
         operator()( e.getName() );
      }

      virtual void operator()( AstDeclVar& e )
      {
         // TODO must not be void
         operator()( e.getType() );

         //take into account array
         //e.setNodeType( e.getType().getNodeType() );
         //

         if ( e.getInit() )
         {
            // TODO check type
            operator()( *e.getInit() );
         } else if ( e.getDeclarationList() )
         {
            // TODO check type // ARRAY
            operator()( *e.getDeclarationList() );
         }

         if ( e.getType().isArray() )
         {
            if ( e.getType().getSize() && e.getType().getSize()->size() > 0 )
            {
               for ( size_t n = 0; n < e.getType().getSize()->size(); ++n )
               {
                  // TODO must be an int exp
                  operator()( *( (*e.getType().getSize())[ n ] ) );
               }
            } 
         }
      }

      virtual void operator()( AstExpSeq& e )
      {
         operator()( e.getExp() );
      }

      virtual void operator()( AstTypeField& e )
      {
         operator()( e.getField() );
      }

      virtual void operator()( AstExpTypename& e )
      {
         operator()( e.getType() );
         if ( e.getArgs().getArgs().size() )
         {
            operator()( e.getArgs() );
         } 
      }

      virtual void operator()( Ast& e )
      {
         e.accept( *this );
      }

   private:
      // disabled
      VisitorType& operator=( VisitorType& );
      VisitorType( const VisitorType& );


   private:
      std::vector<mvv::Symbol>   _defaultClassPath;
      std::vector<const Type*>   _returnType;


      ParserContext&      _context;
      SymbolTableVars     _vars;
      SymbolTableFuncs    _funcs;
      SymbolTableClasses  _classes;
   };
}
}

#endif