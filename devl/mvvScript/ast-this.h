#ifndef MVV_PARSER_AST_EXP_THIS_H_
# define MVV_PARSER_AST_EXP_THIS_H_

# include "ast-exp.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstThis : public AstVar
   {
   public:
      typedef std::string    value_type;

      AstThis( const YYLTYPE& location ) : AstVar( location )
      {
      }

      /// Accept a const visitor \a v.
      virtual void accept( ConstVisitor& v ) const
      {
         v( *this );
      }

      /// Accept a non-const visitor \a v.
      virtual void accept( Visitor& v )
      {
         v( *this );
      }
   };
}
}

#endif