#ifndef MVV_PARSER_AST_EXP_VAR_H_
# define MVV_PARSER_AST_EXP_VAR_H_

# include "ast-exp.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstExpVar : public AstExp
   {
   public:
      AstExpVar( const YYLTYPE& location, mvv::Symbol name, bool isDeduced ) : AstExp( location ), _name( name ), _isDeduced ( isDeduced )
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