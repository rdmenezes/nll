#ifndef MVV_PARSER_AST_DECL_H_
# define MVV_PARSER_AST_DECL_H_

# include "typable.h"
# include "ast.h"


namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstDecl : public Ast, public Typable
   {
   public:
      AstDecl( const YYLTYPE& location ) : Ast( location )
      {
      }
   };
}
}

#endif