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
   
   #define YY_USER_ACTION  yylloc->last_column += yyleng;
   
   /*
   #define YY_USER_ACTION yylloc->first_line = yylloc->last_line = yylineno; \
    yylloc->first_column = yylloc->current_column; yylloc->last_column = yylloc->current_column+yyleng-1; \
    yylloc->current_column += yyleng;
   */
   
	#include <iostream>
	#include <stdexcept>
	#include <sstream>
	#include <string>
	
	#include "parser-context.h"
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
    // update location
    yylloc->last_column = 0;
    yylloc->last_line += yyleng;
    yylloc->first_column = yylloc->last_column;
    yylloc->first_line = yylloc->last_line;
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
    yylloc->last_column = 0;
    yylloc->last_line += yyleng;
    yylloc->first_column = yylloc->last_column;
    yylloc->first_line = yylloc->last_line;
  }

  . {
    yylval->str->append(yytext);
  }

  <<EOF>> {
    exit(1);
  }
}

"if"		return IF;
"="		return ASSIGN;


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
"=="		return EQ;
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
    yylval->symbol = &(mvv::Symbol::create (yytext));
    return ID;
}

{HWHITE}    {
    yylloc->first_column = yylloc->last_column;
    yylloc->first_line = yylloc->last_line;
}

{NEWLINE}+  {
    yylloc->last_column = 0;
    yylloc->last_line += yyleng;
    yylloc->first_column = yylloc->last_column;
    yylloc->first_line = yylloc->last_line;
}

<<EOF>> yyterminate ();
. {
    exit( 1 ); /* invalid character */
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

      if ( _filename != "" )
      {
         yyin = _filename == "-" ? stdin : fopen (_filename.c_str (), "r");
         std::cout << "filename opened=" << _filename << std::endl;

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
         yy_switch_to_buffer (yy_scan_string (_input.c_str ()));
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