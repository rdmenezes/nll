#ifndef MVV_PARSER_AST_VAR2_H_
# define MVV_PARSER_AST_VAR2_H_

# include "typable.h"
# include "ast-exp.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstVar : public AstExp
   {
   public:
      AstVar( const YYLTYPE& location ) : AstExp( location ), _decl( 0 )
      {
      }

      void setReference( AstDecl* decl )
      {
         _decl = decl;
      }

      AstDecl* getReference() const
      {
         return _decl;
      }

   private:
      AstDecl*    _decl;
   };
}
}

#endif