#ifndef MVV_PARSE_PRINT_VISITOR_H_
# define MVV_PARSE_PRINT_VISITOR_H_

# include "visitor.h"
# include "ast-files.h"

namespace mvv
{
namespace parser
{
   class VisitorPrint : public ConstVisitor
   {
   public:
      VisitorPrint( std::ostream& o ) : _o( o )
      {}

      virtual void operator()( const AstInt& e )
      {
         _o << e.getValue();
      }
/*
      virtual void operator()( nll::core::Constify<AstFloat>::type& e )
      {
         o << e.getValue();
      }

      virtual void operator()( nll::core::Constify<AstString>::type& e )
      {
         o << e.getValue();
      }
*/
   private:
      std::ostream&     _o;
   };
}
}

#endif