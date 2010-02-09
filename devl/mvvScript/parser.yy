%{								// -*- C++ -*-
   #include <string>
   #include <iostream>
   #include <list>
   #include <sstream>
   #include "parser-context.h"
%}

%error-verbose
%define "filename_type" "const mvv::Symbol"
%locations
%parse-param { mvv::parser::ParserContext& tp }
%lex-param { mvv::parser::ParserContext& tp }
%debug

%union
{
   // Tokens.
   int                        ival;
   std::string*			      str;
   const mvv::Symbol*		   symbol;
}

%token <str>    STRING "string"
%token <symbol> ID     "identifier"
%token <ival>   INT    "integer"

%destructor { delete $$; }  		      "string"

%token AND          "&"
%token ASSIGN       "="
%token COLON        ":"
%token COMA         ","
%token DIVIDE       "/"
%token DOT          "."
%token EQ           "=="
%token GE           ">="
%token GT           ">"

%token IF           "if"
%token LBRACE       "{"
%token LBRACK       "["
%token LE           "<="
%token LPAREN       "("
%token LT           "<"
%token MINUS        "-"
%token NE           "!="
%token OR           "|"
%token PLUS         "+"
%token RBRACE       "}"
%token RBRACK       "]"
%token RPAREN       ")"
%token SEMI         ";"
%token TIMES        "*"
%token YYEOF   0    "end of file"

%%

exp :
      INT			        {  }
      
%%
