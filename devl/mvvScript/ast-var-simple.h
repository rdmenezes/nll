#ifndef MVV_PARSER_AST_VAR_SIMPLE_H_
# define MVV_PARSER_AST_VAR_SIMPLE_H_

# include "ast-var.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstVarSimple : public AstVar
   {
   public:
      AstVarSimple( const YYLTYPE& location, mvv::Symbol name, bool isDeduced ) : AstVar( location ), _name( name ), _isDeduced ( isDeduced )
      {
      }

      const Symbol& getName() const
      {
         return _name;
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
      mvv::Symbol   _name;
      bool          _isDeduced;
   };
}
}

#endif