#ifndef MVV_PARSER_AST_ASSIGN_H_
# define MVV_PARSER_AST_ASSIGN_H_

# include "ast-exp.h"
# include "ast-var.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstExpAssign : public AstExp
   {
   public:
      AstExpAssign( const YYLTYPE& location, AstVar* lvalue,  AstExp* value ) : AstExp( location ), _value( value ), _lvalue( lvalue )
      {
         ensure( lvalue && value, "can't be null" );
      }

      ~AstExpAssign()
      {
         delete _lvalue;
         delete _value;
      }

      const AstExp& getValue() const
      {
         return *_value;
      }

      AstExp& getValue()
      {
         return *_value;
      }

      const AstVar& getLValue() const
      {
         return *_lvalue;
      }

      AstVar& getLValue()
      {
         return *_lvalue;
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
      AstVar*   _lvalue;
      AstExp*   _value;
   };
}
}

#endif