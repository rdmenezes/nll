#ifndef MVV_PARSER_PARSER_CONTEXT_H_
# define MVV_PARSER_PARSER_CONTEXT_H_

# include <stack>
# include "ast.h"
# include "forward.h"
# include "parser.tab.hh"
# include "mvvScript.h"

// Announce to Flex the prototype we want for lexing function
# define YY_DECL_BODY yylex (YYSTYPE* yylval, YYLTYPE* yylloc, mvv::parser::ParserContext& context)
# define YY_DECL  int ::YY_DECL_BODY

// Announce to Bison the lexing function it must use.
int YY_DECL_BODY;

// declare the parser function to declare it as a friend
#define YYPARSE_DECL int MVVSCRIPT_API yyparse (mvv::parser::ParserContext& tp);
YYPARSE_DECL;

// 
inline void yyerror( YYLTYPE* yylloc, mvv::parser::ParserContext& context, char* msg )
{
   std::cerr << yylloc->filename.getName() << ": L"  << yylloc->first_line << "." << yylloc->first_column
                                           << "-L"   << yylloc->last_line  << "." << yylloc->last_column
                                           << " " << msg    << std::endl;
}



namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API ParserContext
   {
   public:
      friend int ::yyparse (mvv::parser::ParserContext& tp);
      
      ParserContext()
      {
         _root = 0;
      }

      Ast* parseFile( const std::string& file );

      Ast* parseString( const std::string& string );

   private:
      // open the scanner, defined in lexer.ll
      void scanOpen();

      // close the scanner, defined in lexer.ll
      void scanClose();

      // parse
      void parse()
      {
         _root = 0;
         scanOpen();
         int result = yyparse( *this );
         scanClose();

         // if parsing fails, deallocate the current AST if any
         // we won't need it...
         if ( result )
         {
            delete _root;
            _root = 0;
         }
      }

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
   };
}
}

#endif