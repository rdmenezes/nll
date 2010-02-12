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
   #include <nll/nll.h>
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
   static int mvvParserInputNumber = 0;
   @$.filename = mvv::Symbol::create( tp._filename == "" ? "(input" + nll::core::val2str( mvvParserInputNumber++ ) + ")" : tp._filename );
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

%destructor { delete $$; }  		                  "string"
%destructor { delete $$.symbol; }  	               "symbol"

%token AND          "&&"
%token ASSIGN       "="
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
%token OR           "||"
%token PLUS         "+"
%token RBRACE       "}"
%token RBRACK       "]"
%token RPAREN       ")"
%token SEMI         ";"
%token TIMES        "*"

%token OPERATORPARENT      "operator()"
%token OPERATORBRACKET     "operator[]"
%token FOR                 "for"
%token IN                  "in"
%token VAR                 "var"
%token CLASS               "class"
%token VOID                "void"
%token NIL                 "NULL"

%token YYEOF   0    "end of file"

%left ID

%left OR
%left AND
%nonassoc GE LE EQ NE LT GT
%left PLUS MINUS
%left TIMES DIVIDE
%nonassoc UMINUS


%start program

%%
program:
      rvalue               { tp._root = new mvv::parser::Ast()}
      
  
     
rvalue :
      INT                  { std::cout << "INT" << std::endl;}
     |FLOAT                { std::cout << "FLOAT" << std::endl;}
     |rvalue PLUS rvalue   { std::cout << "+" << std::endl;}
     |rvalue MINUS rvalue  { std::cout << "-" << std::endl;}
     |rvalue TIMES rvalue  { std::cout << "*" << std::endl;}
     |rvalue DIVIDE rvalue { std::cout << "/" << std::endl;}
     |MINUS rvalue %prec UMINUS{ std::cout << "UMINUS" << std::endl;}
     |LPAREN rvalue RPAREN { std::cout << "()" << std::endl;}
     |STRING
     |lvalue

     
lvalue :
     |ID
     |ID LBRACK rvalue RBRACK
     |lvalue LBRACK rvalue RBRACK
     |lvalue DOT ID
%%
