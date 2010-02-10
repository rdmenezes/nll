%{								// -*- C++ -*-

   //
   // scanner/parser generator:
   //   FLEX 2.5.35
   //   BISON 2.4.1
   //
   // command:
   //   flex -o lex.yy.cpp lexer.ll
   //   bison parser.yy -d
   //
   
   #include <string>
   #include <iostream>
   #include <list>
   #include <sstream>
   #include "parser-context.h"
%}

%locations

%error-verbose

%parse-param { mvv::parser::ParserContext& tp }
%lex-param { mvv::parser::ParserContext& tp }

%debug
%pure_parser

%initial-action
{
   /**
    setup the filename each time before parsing
    */
   @$.filename = mvv::Symbol::create( tp._filename == "" ? "(input)" : tp._filename );
}

%union
{
   // Tokens.
   int                        ival;
   float                      fval;
   std::string*			      str;
   const mvv::Symbol*		   symbol;
}

%token <str>    STRING "string"
%token <symbol> ID     "identifier"
%token <ival>   INT    "integer"
%token <fval>   FLOAT  "float"

%destructor { delete $$; }  		            "string"
%destructor { delete $$.symbol; }  	         "symbol"

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

%start program

%%
program:
      exp               { tp._root = new mvv::parser::Ast()}

exp :
      INT INT			        { std::cout << "int" << std::endl; }
      
%%
