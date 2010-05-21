#ifndef MVV_PARSER_AST_EXP2_H_
# define MVV_PARSER_AST_EXP2_H_

# include "typable.h"


namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstExp : public Ast, public Typable
   {
   public:
      AstExp( const YYLTYPE& location, bool deallocate = true ) : Ast( location ), Typable( deallocate )
      {
      }

      virtual ~AstExp()
      {
      }
   };
}
}

#endif