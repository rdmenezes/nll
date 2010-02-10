#ifndef MVV_PARSER_FORWARD_H_
# define MVV_PARSER_FORWARD_H_

namespace yy
{
  // From parsetiger.yy.
  class parser;
  class location;
}

// From scantiger.cc.
union YYSTYPE;

// From scantiger.cc.
struct yy_buffer_state;

namespace mvv
{
namespace parser
{
   class ParserContext;
}
}

#endif
