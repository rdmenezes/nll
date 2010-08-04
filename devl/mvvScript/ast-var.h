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

      // i.e int a; int b; =>b<= = 3, for b, it will return the index on the stack frame of b
      virtual ui32 getRuntimeIndex()
      {
         return 0;
      }


   private:
      AstDecl*    _decl;
   };
}
}

#endif