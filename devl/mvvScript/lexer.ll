%option noyywrap
%option nounput
%option debug
%option batch
%option stack

/**
 Declare the different states
 */
%x SC_COMMENT SC_STRING

%{
   #define YYDEBUG 1
   #define YY_USER_ACTION yylloc->columns (yyleng);
   #include <string.h>
	#include <iostream>
	#include <stdexcept>
	#include <sstream>
	#include <string>
	
	#include "parser-context.h"
	#include "parser.h"
	
	std::string	add_location (yy::location& l, const char* msg)
   {
     std::ostringstream	os;

     os << l << ": " << msg;
     return os.str ();
   }
%}

/* any character, including newline */
ANY       (.|"\n")

/* starting character in a name */
LETTER    [a-zA-Z]

/* starting character in a numeric literal */
DIGIT     [0-9]

/* double-quote */
DQUOTE    "\""

/* non-newline whitespace */
HWHITE    [ \t\f\v]

/* newline Characters */
NEWLINE ("\n\r"|"\r\n"|"\n"|"\r")

/* Definition for C-String character */
STRCHR	[A-Za-z_]


%%

"/""*" {
  /* C-style comments */
  yy_push_state (SC_COMMENT);
}

<SC_COMMENT>{
  "*/" {
    yy_pop_state ();
  }
  "/*" {
    yy_push_state (SC_COMMENT);
  }

  . { /*skip*/ }

  "\n"+ {
    yylloc->lines (yyleng);
    yylloc->step ();
  }

   <<EOF>> {
    //    misc::fatal (misc::exit_scan, add_location (*yylloc, "Unterminated Comment."));
    exit(1);
   }
}

 {DQUOTE} {
  yylval->str = new std::string ();
  yy_push_state (SC_STRING);
}

<SC_STRING>{
  "\"" {
    yy_top_state ();
    yy_pop_state ();
    return STRING;
  }

  "\n"+ {
    yylloc->lines (yyleng);
    yylloc->step ();
  }

  . {
    yylval->str->append(yytext);
  }

  <<EOF>> {
    exit(1);
  }
}

"if"		return IF;


"."		return DOT;
";"		return SEMI;
":"		return COLON;
","		return COMA;
"]"		return RBRACK;
"["		return LBRACK;
"("		return LPAREN;
")"		return RPAREN;
"{"		return LBRACE;
"}"		return RBRACE;
 
">="	   return GE;
"<="	   return LE;
"!="	   return NE;
"-"		return MINUS;
"+"		return PLUS;
"*"		return TIMES;
"/"		return DIVIDE;
"="		return EQ;
">"		return GT;
"<"		return LT;
"&"		return AND;
"|"		return OR;

{DIGIT}+ {
  std::istringstream iss (yytext);
  iss >> yylval->ival;
  return INT;
}

{LETTER}({LETTER}|{DIGIT}|"_")* {
  yylval->symbol = &(symbol::Symbol::create (yytext));
  return ID;
}

{HWHITE}    yylloc->step ();
{NEWLINE}+  yylloc->lines (yyleng); yylloc->step ();
<<EOF>> yyterminate ();
.           {
	exit( 1 ) /* invalid character */
}


%%

namespace mvv
{
namespace parser
{
   // Keep this scan_open valid for multiple calls (i.e., do use
   // yyrestart) so that for instance using a SWIG interpreter we may
   // load several files.
   void
   ParserContext::scanOpen ()
   {
      static bool first = true;
      if ( first )
      {
         first = false;
         // Reclaim all the memory allocated by Flex.
         std::atexit ((void (*) ()) yylex_destroy);
      }

      yy_flex_debug = _scan_trace_p;

      // Save the current state.
      _states.push (YY_CURRENT_BUFFER);

      if ( filename_ != "" )
      {
         yyin = _filename == "-" ? stdin : fopen (_filename.c_str (), "r");

         /*
         if (!yyin)
         error_ << misc::Error::failure
         << program_name
         << ": cannot open `" << filename_ << "': "
         << strerror (errno) << std::endl
         << &misc::Error::exit;
         */
         
         yy_switch_to_buffer (yy_create_buffer (yyin, YY_BUF_SIZE));
      }
      else
      {
         yyin = 0;
         yy_switch_to_buffer (yy_scan_string (input_.c_str ()));
      }
   }
   
   void
   destroy_stack ()
   {
      delete yy_start_stack;
      yy_start_stack = 0;
   }

   void
   ParserContext::scanClose ()
   {
      if (yyin)
         fclose (yyin);
         
      // Restore the current scanning state.
      yy_delete_buffer (YY_CURRENT_BUFFER);
      yy_switch_to_buffer (_states.top ());
      _states.pop ();
      std::atexit ((void (*) ()) destroy_stack);
   }
}
}