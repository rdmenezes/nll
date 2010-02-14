#ifndef MVV_PARSER_AST_EXP_H_
# define MVV_PARSER_AST_EXP_H_

# include "ast.h"
# include "typable.h"


namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstExp : public Ast, public Typable
   {
   public:
      AstExp( const YYLTYPE& location ) : Ast( location )
      {
      }
   };
}
}

#endif