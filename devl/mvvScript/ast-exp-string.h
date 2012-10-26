#ifndef MVV_PARSER_AST_STRING_H_
# define MVV_PARSER_AST_STRING_H_

# include "ast-exp.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstString : public AstExp
   {
   public:
      typedef std::string    value_type;

      AstString( const YYLTYPE& location, value_type value ) : AstExp( location ), _value( value )
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