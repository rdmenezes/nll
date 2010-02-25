#ifndef MVV_PARSE_VISITOR_TYPE_H_
# define MVV_PARSE_VISITOR_TYPE_H_

# include "visitor.h"
# include "ast-files.h"
# include "type.h"

//
// TODO node type must all be cloned when copied! else memory problems...
//
//

namespace mvv
{
namespace parser
{
   /**
    @brief Defines a visitor visiting all the nodes but doing nothing
    @note this is usefull if a visitor is only handling a limited number of nodes
    */
   class VisitorType : public VisitorDefault
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
            impl::reportTypeError( e.getLeft().getLocation(), _context, "expression cannot be typed");
            e.setNodeType( new TypeError() );
            return;
         }

         if ( !e.getRight().getNodeType() )
         {
            impl::reportTypeError( e.getRight().getLocation(), _context, "expression cannot be typed");
            e.setNodeType( new TypeError() );
            return;
         }

         // types must be compatible
         if ( !e.getRight().getNodeType()->isCompatibleWith( *e.getLeft().getNodeType() ) )
         {
            impl::reportTypeError( e.getLocation(), _context, "incompatible types");
            e.setNodeType( new TypeError() );
            return;
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
         // TODO handle "var" here
         e.setNodeType( e.getValue().getNodeType() );

         ensure( e.getLValue().getNodeType(), "compiler error: cannot evaluate expression type" );
         if ( !e.getValue().getNodeType()->isCompatibleWith( *e.getLValue().getNodeType() ) )
         {
            impl::reportTypeError( e.getLocation(), _context, "incompatible types");
            e.setNodeType( new TypeError() );
         }
      }

      virtual void operator()( AstVarSimple& e )
      {
         // TODO constructor overloading...

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
         // TODO set the type
         operator()( e.getName() );
         operator()( e.getIndex() );
      }

      virtual void operator()( AstVarField& e )
      {
         // TODO set the type
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
         e.setNodeType( new TypeNamed( &e ) );
         operator()( e.getDeclarations() );
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
                  e.setNodeType( new TypeError() );
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
            ensure( e.getNodeType(), "error: a function doesn't have a return type" );
            _returnType.push_back( e.getNodeType() );
            operator()( *e.getBody() );
            _returnType.pop_back();

            // check return has been called, or void type
            if ( !e.getExpectedFunctionType() && !TypeVoid().isCompatibleWith( *e.getNodeType() )  )
            {
               impl::reportTypeError( e.getLocation(), _context, "return type statement has not been called in a function returning a value" );
               return;
            }

            // return should have checked incorrect types... so don't check it again
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
         ensure( e.getFunction(), "unknown error" );
         if ( e.getReturnValue() )
         {
            operator()( *e.getReturnValue() );
            e.setNodeType( e.getReturnValue()->getNodeType() );
         } else {
            e.setNodeType( new TypeVoid() );
         }
         e.getFunction()->setExpectedFunctionType( e.getNodeType() );

         ensure( _returnType.size(), "error: return allowed outside of a function..." );
         if ( !e.getNodeType()->isCompatibleWith( **_returnType.rbegin() ) )
         {
            impl::reportTypeError( e.getLocation(), _context, "function return type and actual type are incompatible" );
            e.setNodeType( new TypeError() );
         }
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

         // we first must visite the type!
         operator()( e.getType() );

         if ( e.getType().isArray() )
         {
            ensure( e.getType().getSize()->size(), "must have a dimensionality >= 1" );
            if ( e.getType().getSize() && e.getType().getSize()->size() > 0 )
            {
               for ( size_t n = 0; n < e.getType().getSize()->size(); ++n )
               {
                  // TODO must be an int exp
                  operator()( *( (*e.getType().getSize())[ n ] ) );
               }
            }
            ensure( e.getType().getNodeType(), "can't type properly a tree" );
            e.setNodeType( new TypeArray( e.getType().getSize()->size(), *e.getType().getNodeType() ) );
         } else {
            e.setNodeType( e.getType().getNodeType() );
         }

         if ( e.getInit() )
         {
            // TODO check type
            operator()( *e.getInit() );
         } else if ( e.getDeclarationList() )
         {
            // TODO check type // ARRAY
            operator()( *e.getDeclarationList() );
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
      std::vector<const Type*>   _returnType;


      ParserContext&      _context;
      SymbolTableVars     _vars;
      SymbolTableFuncs    _funcs;
      SymbolTableClasses  _classes;
   };
}
}

#endif