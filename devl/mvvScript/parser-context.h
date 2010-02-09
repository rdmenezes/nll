#ifndef MVV_PARSER_PARSER_CONTEXT_H_
# define MVV_PARSER_PARSER_CONTEXT_H_

# include <stack>
# include "location.h"
# include "parser.tab.hh"

// configuring...
# define YY_NO_UNISTD_H
# define isatty(x) false

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

// Announce to Flex the prototype we want for lexing function
# define YY_DECL  int yylex (YYSTYPE* yylval, yy::location* yylloc, mvv::parser::ParserContext& context)

// Announce to Bison the lexing function it must use.
YY_DECL;

namespace mvv
{
namespace parser
{
   class ParserContext
   {
   public:
      friend YY_DECL;

   private:
      // open the scanner, defined in lexer.ll
      void scanOpen();

      // close the scanner, defined in lexer.ll
      void scanClose();

   private:
      // save the string we have to parse, if parsing a string
      std::string    _input;

      // save the file we have to parse if parsing a file
      std::string    _filename;

      // Stack of scanning states, used to have some (weak) reentrancy.
      std::stack <yy_buffer_state*> _states;

      // Verbose scanning?
      bool _scan_trace_p;

      // Verbose parsing?
      bool _parse_trace_p;
   };
}
}

#endif