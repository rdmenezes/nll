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
	#include <iostream>
	#include <stdexcept>
	#include <sstream>
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

/**
 * ----------- Comments ----------
 */
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

/**
 * ----------- Strings ----------
 */
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

/**
 * ----------- Keywords ----------
 */
"if"		return IF;

/**
 * ----------- Symbols ----------
 */
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
 
/**
 * ----------- Operators ----------
 */
">="	return GE;
"<="	return LE;
"<>"	return NE;
"-"		return MINUS;
"+"		return PLUS;
"*"		return TIMES;
"/"		return DIVIDE;
"="		return EQ;
">"		return GT;
"<"		return LT;
"&"		return AND;
"|"		return OR;

/**
 * ----------- Numeric ----------
 */
{DIGIT}+ {
  std::istringstream iss (yytext);
  iss >> yylval->ival;
  return INT;
}

/**
 * ----------- Identifier ----------
 */
{LETTER}({LETTER}|{DIGIT}|"_")* {
  yylval->symbol = &(symbol::Symbol::create (yytext));
  return ID;
}

/**
 * ----------- STEP ----------
 */
{HWHITE}    yylloc->step ();
{NEWLINE}+  yylloc->lines (yyleng); yylloc->step ();
<<EOF>> yyterminate ();
.           {
	exit( 1 ) /* invalid character */
}
 