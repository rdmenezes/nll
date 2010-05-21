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
      AstVar( const YYLTYPE& location, bool deallocate = true ) : AstExp( location, deallocate ), _decl( 0 )
      {
      }

      virtual ~AstVar()
      {}

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