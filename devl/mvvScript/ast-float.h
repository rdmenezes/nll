#ifndef MVV_PARSER_AST_FLOAT_H_
# define MVV_PARSER_AST_FLOAT_H_

# include "ast.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstFloat : public Ast
   {
   public:
      typedef float    value_type;

      AstFloat( const YYLTYPE& location, value_type value ) : Ast( location )
      {
      }

      const value_type& getValue() const
      {
         return _value;
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
      value_type   _value;
   };
}
}

#endif