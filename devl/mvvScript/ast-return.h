#ifndef MVV_PARSER_AST_RETURN_H_
# define MVV_PARSER_AST_RETURN_H_

# include "ast.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstReturn : public Ast
   {
   public:
      AstReturn( const YYLTYPE& location, AstExp* retVal = 0 ) : Ast( location ), _retVal( retVal )
      {
      }

      const AstExp* getReturnValue() const
      {
         return _retVal;
      }

      AstExp* getReturnValue()
      {
         return _retVal;
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

   private:
      AstExp* _retVal;
   };
}
}

#endif