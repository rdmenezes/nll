#ifndef MVV_PARSE_PRINT_VISITOR_SIMPLE_H_
# define MVV_PARSE_PRINT_VISITOR_SIMPLE_H_

# include "visitor.h"
# include "ast-files.h"
# include "indent.h"
# include "visitor-print.h"

namespace mvv
{
namespace parser
{
   /**
    @brief Simple visitor that is used for the completion: It will print only nodes derived from AstDecl*
    */
   class VisitorPrintSimple : public ConstVisitor
   {
   public:
      VisitorPrintSimple( std::ostream& o ) : _o( o )
      {}

      virtual void operator()( const AstThis& )
      {
         _o << "this";
      }

      virtual void operator()( const AstInt& e )
      {
         _o << e.getValue();
      }

      virtual void operator()( const AstFloat& e )
      {
         _o << e.getValue();
      }

      virtual void operator()( const AstNil& )
      {
         _o << "NULL";
      }

      virtual void operator()( const AstString& e )
      {
         _o << "\"" << e.getValue() << "\"";
      }

      virtual void operator()( const AstOpBin& e )
      {
         operator()( e.getLeft() );
         _o << " " << impl::toString( e.getOp() ) << " ";
         operator()( e.getRight() );
      }

      virtual void operator()( const AstIf& )
      {
         // do nothing
      }

      virtual void operator()( const AstStatements& )
      {
         // do nothing
      }

      virtual void operator()( const AstExpAssign& )
      {
         // do nothing
      }

      virtual void operator()( const AstVarSimple& e )
      {
         _o << e.getName();
      }

      virtual void operator()( const AstVarArray& e )
      {
         operator()( e.getName() );
         _o << "[ ";
         operator()( e.getIndex() );
         _o << " ]";
      }

      virtual void operator()( const AstVarField& e )
      {
         operator()( e.getField() );
         _o << "." << e.getName();
      }

      virtual void operator()( const AstType& e )
      {
         if ( e.getSymbol() )
            _o << *e.getSymbol();
         else
         {
            switch ( e.getType() )
            {
            case AstType::CMP_INT:
               _o << "int";
               break;
            case AstType::CMP_FLOAT:
               _o << "float";
               break;
            case AstType::STRING:
               _o << "string";
               break;
            case AstType::VOID:
               _o << "void";
               break;
            case AstType::VAR:
               _o << "var";
               break;
            default:
               ensure( 0, "runtime error: unknown type" );
            }
         }

         if ( e.isAReference() )
         {
            _o << "&";
         }
      }

      virtual void operator()( const AstDecls& )
      {
         // do nothing
      }

      virtual void operator()( const AstDeclVars& e )
      {
         ui32 n = 0;
         for ( AstDeclVars::Decls::const_iterator it = e.getVars().begin(); it != e.getVars().end(); ++it, ++n )
         {
            operator()( **it );
            if ( n != e.getVars().size() - 1 )
               _o << ", ";
         }
      }

      virtual void operator()( const AstDeclClass& e )
      {
         _o << "class " << e.getName() << iendl;
      }

      virtual void operator()( const AstDeclFun& e ) 
      {
         if ( !e.getBody() )
            _o << "import ";
         if ( e.getType() )
         {
            operator()( *e.getType() );
            _o << " ";
         }
         _o << e.getName();
         if ( e.getVars().getVars().size() )
         {
            _o << "( ";
            operator()( e.getVars() );
            _o << " )";
         } else {
            _o << "()";
         }
         _o <<iendl;
      }

      virtual void operator()( const AstArgs& e )
      {
         unsigned n = 0;
         for ( AstArgs::Args::const_iterator it = e.getArgs().begin(); it != e.getArgs().end(); ++it, ++n )
         {
            operator()( **it );
            if ( n + 1 != e.getArgs().size() )
            {
               _o << ", ";
            }
         }
      }

      virtual void operator()( const AstReturn& )
      {
         // do nothing
      }

      virtual void operator()( const AstImport& )
      {
         // do nothing
      }

      virtual void operator()( const AstInclude& )
      {
         // do nothing
      }

      virtual void operator()( const AstExpCall& e )
      {
         operator()( e.getName() );
         if ( e.getArgs().getArgs().size() )
         {
            _o << "( ";
            operator()( e.getArgs() );
            _o << " )";
         } else {
            _o << "()";
         }
      }

      virtual void operator()( const AstDeclVar& e )
      {
         operator()( e.getType() );
         _o << " " << e.getName();

         // if object initialization, cant make array!
         if ( e.getObjectInitialization() )
         {
            // don't print more if size == 0
            if ( e.getObjectInitialization()->getArgs().size() )
            {
               _o << "( ";
               operator()( *e.getObjectInitialization() );
               _o << " )";
            }
         } else {
            if ( e.getType().isArray() )
            {
               if ( e.getType().getSize() && e.getType().getSize()->size() > 0 )
               {
                  for ( size_t n = 0; n < e.getType().getSize()->size(); ++n )
                  {
                     _o << "[ ";
                     operator()( *( (*e.getType().getSize())[ n ] ) );
                     _o << " ]";
                  }
               } else {
                  _o << "[]";
               }
            }
         }

         if ( !e.getIsInFunctionPrototype() )
         {
            _o << iendl;
         }
      }

      virtual void operator()( const AstBreak& )
      {
         // do nothing
      }

      virtual void operator()( const AstWhile& )
      {
         // do nothing
      }

      virtual void operator()( const AstExpSeq& )
      {
         // do nothing
      }

      virtual void operator()( const AstTypeField& e )
      {
         _o << e.getName() << "::";
         operator()( e.getField() );
      }

      virtual void operator()( const AstExpTypename& e )
      {
         _o << " typename ";
         operator()( e.getType() );
         if ( e.getArgs().getArgs().size() )
         {
            _o << "( ";
            operator()( e.getArgs() );
            _o << " )";
         } else {
            _o << "()";
         }
      }

      virtual void operator()( const AstTypedef& e )
      {
         _o << "typedef ";
         operator()( e.getType() );
         _o << " " << e.getName().getName();

         _o << iendl;
      }

      virtual void operator()( const AstFunctionType& e )
      {
         operator()( e.getType() );
         _o << "(";
         operator()( e.getArgs() );
         _o << ")";
      }

      virtual void operator()( const Ast& e )
      {
         e.accept( *this );
      }

   private:
      // disabled
      VisitorPrintSimple& operator=( const VisitorPrintSimple& );
      VisitorPrintSimple( const VisitorPrintSimple& );

   private:
      std::ostream&     _o;
   };
}
}

#endif