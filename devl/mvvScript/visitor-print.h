#ifndef MVV_PARSE_PRINT_VISITOR_H_
# define MVV_PARSE_PRINT_VISITOR_H_

# include "visitor.h"
# include "ast-files.h"
# include "indent.h"

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
         case AstOpBin::Op::GE:
            return " >= ";
         case AstOpBin::Op::LE:
            return " <= ";
         case AstOpBin::Op::NE:
            return " != ";
         case AstOpBin::Op::MINUS:
            return " - ";
         case AstOpBin::Op::PLUS:
            return " + ";
         case AstOpBin::Op::TIMES:
            return " * ";
         case AstOpBin::Op::DIVIDE:
            return " / ";
         case AstOpBin::Op::EQ:
            return " == ";
         case AstOpBin::Op::GT:
            return " > ";
         case AstOpBin::Op::LT:
            return " < ";
         case AstOpBin::Op::AND:
            return " && ";
         case AstOpBin::Op::OR:
            return " || ";
         };
         ensure( 0, "unreachable, or missing a case..." );
      }
   }

   class VisitorPrint : public ConstVisitor
   {
   public:
      VisitorPrint( std::ostream& o ) : _o( o )
      {}

      virtual void operator()( const AstInt& e )
      {
         _o << e.getValue();
      }

      virtual void operator()( const AstFloat& e )
      {
         _o << e.getValue();
      }

      virtual void operator()( const AstString& e )
      {
         _o << e.getValue();
      }

      virtual void operator()( const AstOpBin& e )
      {
         operator()( e.getLeft() );
         _o << " " << impl::toString( e.getOp() ) << " ";
         operator()( e.getRight() );
      }

      virtual void operator()( const AstIf& e )
      {
         _o << "if ( ";
         operator()( e.getCondition() );
         _o << " )"  << iendl
            << "{"   << incendl;
         operator()( e.getThen() );
         _o << decendl
            << "}";

         if ( e.getElse() )
         {
            _o << " else {"   << incendl;
         operator()( *e.getElse() );
         _o << decendl
            << "}";
         } else {
            _o << iendl;
         }
      }

      virtual void operator()( const AstStatements& e )
      {
         for ( AstStatements::Statements::const_iterator it = e.getStatements().begin();
               it != e.getStatements().end();
               ++it )
         {
            operator()( **it );

            if ( !dynamic_cast<AstIf*>( *it ) )
            {
               _o << ";";
            }
         }
      }

      virtual void operator()( const AstExpAssign& e )
      {
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

      virtual void operator()( const Ast& e )
      {
         e.accept( *this );
      }

   private:
      std::ostream&     _o;
   };
}
}

#endif