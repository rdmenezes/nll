#ifndef MVV_VISITOR_TRANSLATE_H_
# define MVV_VISITOR_TRANSLATE_H_

# include <mvvScript/visitor.h>
# include <mvvScript/ast-files.h>
# include <mvvScript/indent.h>

namespace mvv
{
namespace parser
{
   namespace impl
   {
      inline std::string toString( AstOpBin::Op op )
      {
         switch ( op )
         {
         case AstOpBin::GE:
            return " >= ";
         case AstOpBin::LE:
            return " <= ";
         case AstOpBin::NE:
            return " != ";
         case AstOpBin::MINUS:
            return " - ";
         case AstOpBin::PLUS:
            return " + ";
         case AstOpBin::TIMES:
            return " * ";
         case AstOpBin::DIVIDE:
            return " / ";
         case AstOpBin::EQ:
            return " == ";
         case AstOpBin::GT:
            return " > ";
         case AstOpBin::LT:
            return " < ";
         case AstOpBin::AND:
            return " && ";
         case AstOpBin::OR:
            return " || ";
         };
         ensure( 0, "unreachable, or missing a case..." );
      }
   }

   /**
    @brief visitor that translate the language to a closer form of C++ so that it can be parsed by doxygen 
    */
   class VisitorTranslate : public ConstVisitor
   {
   public:
      VisitorTranslate( std::ostream& o ) : _o( o ), _statementDepth( 0 )
      {
         _line = 1;
      }

      virtual void operator()( const AstThis& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }
         _o << "this";
      }

      virtual void operator()( const AstInt& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }
         _o << e.getValue();
      }

      virtual void operator()( const AstFloat& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }
         _o << e.getValue();
      }

      virtual void operator()( const AstNil& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }
         _o << "NULL";
      }

      virtual void operator()( const AstString& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }
         _o << "\"" << e.getValue() << "\"";
      }

      virtual void operator()( const AstOpBin& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

         operator()( e.getLeft() );
         _o << " " << impl::toString( e.getOp() ) << " ";
         operator()( e.getRight() );
      }

      virtual void operator()( const AstIf& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

         _o << "if ( ";
         operator()( e.getCondition() );
         _o << " )"  << iendl;
         ++_line;
         operator()( e.getThen() );

         if ( e.getElse() )
         {
            _o << " else ";
         operator()( *e.getElse() );
         } else {
            _o << iendl;
            ++_line;
         }
      }

      virtual void operator()( const AstStatements& e )
      {  /*
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }*/

         unsigned n = 0;
         if ( _statementDepth > 0 )
         {
            _o << "{";
            if ( e.getStatements().size() )
            {
               _o << incendl;
               ++_line;
            }
         }

         ++_statementDepth;

         for ( AstStatements::Statements::const_iterator it = e.getStatements().begin();
               it != e.getStatements().end();
               ++it, ++n )
         {
            operator()( **it );

            if ( !dynamic_cast<AstIf*>( *it ) &&
                 !dynamic_cast<AstDeclClass*>( *it ) && 
                 !dynamic_cast<AstImport*>( *it )    &&
                 !dynamic_cast<AstInclude*>( *it )   &&
                 !dynamic_cast<AstStatements*>( *it ) &&
                 !dynamic_cast<AstWhile*>( *it ) )
            {
               AstDeclFun* f = dynamic_cast<AstDeclFun*>( *it );
               if ( !f || f && !f->getBody() )
               {
                  // if not a function or a function and not body
                  _o << ";";
               }
            }
            if ( n + 1 != e.getStatements().size() )
            {
               _o << iendl;
               ++_line;
            }
         }

         --_statementDepth;

         if ( _statementDepth )
         {
            if ( e.getStatements().size()  )
            {
               _o << decendl << "}";
               ++_line;
            } else {
               _o << iendl << "}";
               ++_line;
            }
         }
      }

      virtual void operator()( const AstExpAssign& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

         operator()( e.getLValue() );
         _o << " = ";
         operator()( e.getValue() );
      }

      virtual void operator()( const AstVarSimple& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

         _o << e.getName();
      }

      virtual void operator()( const AstVarArray& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

         operator()( e.getName() );
         _o << "[ ";
         operator()( e.getIndex() );
         _o << " ]";
      }

      virtual void operator()( const AstVarField& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

         operator()( e.getField() );
         _o << "." << e.getName();
      }

      virtual void operator()( const AstType& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

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

         /*
         // The array will be displayed in the declaration...
         if ( e.isArray() )
         {
            _o << "[]";
         }
         */
      }

      virtual void operator()( const AstDecls& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

         unsigned n = 0;
         for ( AstDecls::Decls::const_iterator it = e.getDecls().begin(); it != e.getDecls().end(); ++it, ++n )
         {
            operator()( **it );

            const AstDeclFun* fun = dynamic_cast<const AstDeclFun*>( *it );
            const AstDeclVar* var = dynamic_cast<const AstDeclVar*>( *it );
            if ( ( fun && !fun->getBody() ) || var )
            {
               _o << ";";
            }
            if ( n + 1 != e.getDecls().size() )
            {
               _o << iendl;
               ++_line;
            }
         }
      }

      virtual void operator()( const AstDeclVars& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

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
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

         _o << "class " << e.getName() << iendl << "{";
         ++_line;
         if ( e.getDeclarations().getDecls().size() )
         {
            _o << incendl;
            ++_line;
         }
         operator()( e.getDeclarations() );
         if ( e.getDeclarations().getDecls().size() )
         {
            _o << decendl << "};";
            ++_line;
         } else {
            _o << iendl << "};";
            ++_line;
         }
      }

      virtual void operator()( const AstDeclFun& e ) 
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

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
         
         if ( e.getBody() )
         {
            _o << iendl;
            ++_line;
            operator()( *e.getBody() );
         } else {
        //    _o << ";";
         }
      }

      virtual void operator()( const AstArgs& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

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

      virtual void operator()( const AstReturn& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

         _o << "return";
         if ( e.getReturnValue() )
         {
            _o << " ";
            operator()( *e.getReturnValue() );
         }
      }

      virtual void operator()( const AstImport& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

         _o << "include \"" << e.getStr() << ".ludo\"";
      }

      virtual void operator()( const AstInclude& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

         _o << "include \"" << e.getStr() << ".ludo\"";
      }

      virtual void operator()( const AstExpCall& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

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
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

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

            if ( e.getInit() )
            {
               _o << " = ";
               operator()( *e.getInit() );
            } else if ( e.getDeclarationList() )
            {
               _o << " = { ";
               operator()( *e.getDeclarationList() );
               _o << " }";
            }
         }
      }

      virtual void operator()( const AstBreak& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

         _o << "break";
      }

      virtual void operator()( const AstWhile& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

         _o << "while ( ";
         operator()( e.getCondition() );
         _o << " )" << iendl << "{" << incendl;
         ++_line;
         operator()( e.getStatements() );
         _o << decendl << "}" << iendl;
         ++_line;
      }

      virtual void operator()( const AstExpSeq& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

         _o << "( ";
         operator()( e.getExp() );
         _o << " )";
      }

      virtual void operator()( const AstTypeField& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

         _o << e.getName() << "::";
         operator()( e.getField() );
      }

      virtual void operator()( const AstExpTypename& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

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
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

         _o << "typedef ";
         operator()( e.getType() );
         _o << " " << e.getName().getName();
      }

      virtual void operator()( const AstFunctionType& e )
      {
         while ( _line < e.getLocation().first_line )
         {
            ++_line;
            std::cout << std::endl;
         }

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
      VisitorTranslate& operator=( const VisitorTranslate& );
      VisitorTranslate( const VisitorTranslate& );

   private:
      std::ostream&     _o;
      int               _statementDepth;
      int               _line;
   };
}
}

#endif