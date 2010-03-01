#ifndef MVV_PARSER_AST_EXP_NIL_H_
# define MVV_PARSER_AST_EXP_NIL_H_

# include "ast-exp.h"


namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstNil : public AstExp
   {
   public:
      AstNil( const YYLTYPE& location ) : AstExp( location )
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