%option noyywrap
%option nounput
%option debug
%option batch
%option stack



/**
 Declare the different states
 */
%x SC_COMMENT SC_STRING SC_COMMENT_LINE

%{
   #define YYDEBUG 1
   
   #define YY_USER_ACTION  yylloc->last_column += yyleng;
   
	#include <iostream>
	#include <stdexcept>
	#include <sstream>
	#include <string>
	
	#include "parser-context.h"
	#include "error.h"
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
    /* reset the state: else if other calls are made, we are in a wrong state */
    BEGIN(0);
   
    std::stringstream msg;
    msg << *yylloc << "unterminated comment." << std::endl;
    tp._error << mvv::parser::Error::SCAN << msg.str();
    yyterminate();
   }
}

 {DQUOTE} {
  yylval->str = new std::string ();
  std::cout << "CREATTE STR=" << yylval->str << std::endl;
  yy_push_state (SC_STRING);
}

<SC_STRING>{
  "\"" {
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
    /* reset the state: else if other calls are made, we are in a wrong state */
    BEGIN(0);
    
    std::stringstream msg;
    msg << *yylloc << "unterminated string." << std::endl;
    tp._error << mvv::parser::Error::SCAN << msg.str();
    yyterminate();
  }
}

"//" {
  /* one line comment */
  yy_push_state (SC_COMMENT_LINE);
}

<SC_COMMENT_LINE>{
 <<EOF>>          { yy_pop_state (); }
 {NEWLINE}        { yy_pop_state (); }
 .                {}
}

{DIGIT}+"."{DIGIT}+ {
  std::istringstream iss (yytext);
  iss >> yylval->fval;
  return FLOAT;
}

"operator[]" return OPERATORBRACKET;
"operator()" return OPERATORPARENT;
"for"    return FOR;
"if"		return IF;
"var"    return VAR;
"class"  return CLASS;
"void"   return VOID;
"in"     return IN;
"NULL"   return NIL;


"="		return ASSIGN;
"."		return DOT;
";"		return SEMI;
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
"&&"		return AND;
"||"		return OR;

{DIGIT}+ {
  std::istringstream iss (yytext);
  iss >> yylval->ival;
  return INT;
}

{LETTER}({LETTER}|{DIGIT}|"_")* {
    yylval->symbol = ( new mvv::Symbol( mvv::Symbol::create ( yytext ) ) );
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
    std::stringstream msg;
    msg << *yylloc << "invalid character." << std::endl;
    tp._error << mvv::parser::Error::SCAN << msg.str();
    
    ++yylloc->first_column;
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
   ParserContext::_scanOpen ()
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
		 if ( _filename == "-" )
		 {
			yyin = stdin;
		 } else {
			FILE* f = fopen (_filename.c_str (), "r");
			if ( f )
			{
				yyrestart( f );
			} else {
				std::stringstream msg;
				msg << "cannot open '" << _filename << "': "
					<< strerror (errno) << std::endl;
	                
				_error << mvv::parser::Error::FAILURE << msg.str();
				return;
			}
		 }         
         
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
   ParserContext::_scanClose ()
   {
      if (yyin)
         fclose (yyin);
         
      // Restore the current scanning state.
      yy_delete_buffer (YY_CURRENT_BUFFER);
      _states.pop ();
      
      if ( _states.size() )
      {
		yy_switch_to_buffer (_states.top ());
	  }
      std::atexit ((void (*) ()) destroy_stack);
   }
}
}