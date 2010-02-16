#ifndef MVV_PARSER_PARSER_CONTEXT_H_
# define MVV_PARSER_PARSER_CONTEXT_H_

# include <stack>
# include "ast.h"
# include "forward.h"
# include "parser.tab.hh"
# include "mvvScript.h"
# include "error.h"

// Announce to Flex the prototype we want for lexing function
# define YY_DECL_BODY yylex (YYSTYPE* yylval, YYLTYPE* yylloc, mvv::parser::ParserContext& tp)
# define YY_DECL  int ::YY_DECL_BODY

// Announce to Bison the lexing function it must use.
int YY_DECL_BODY;

// declare the parser function to declare it as a friend
#define YYPARSE_DECL int MVVSCRIPT_API yyparse (mvv::parser::ParserContext& tp);
YYPARSE_DECL;

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API ParserContext
   {
   public:
      friend int ::yyparse (mvv::parser::ParserContext& tp);
      friend YY_DECL;
      
      ParserContext()
      {
         _root = 0;
         _parse_trace_p = false;
         _scan_trace_p = false;
      }

      Ast* parseFile( const std::string& file );

      Ast* parseString( const std::string& string );

      Error& getError()
      {
         return _error;
      }

   private:
      // open the scanner, defined in lexer.ll
      void _scanOpen();

      // close the scanner, defined in lexer.ll
      void _scanClose();

      // parse
      void _parse();

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

      // ast root
      Ast*  _root;

      Error _error;
   };
}
}

inline void yyerror( YYLTYPE* yylloc, mvv::parser::ParserContext& context, char* msg )
{
   std::stringstream txt;
   txt << *yylloc << msg << std::endl;
   context.getError() << txt.str() << mvv::parser::Error::PARSE;
}

#endif