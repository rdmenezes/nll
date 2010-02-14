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
   #include "ast-files.h"
%}

%locations

%error-verbose

%parse-param { mvv::parser::ParserContext& tp }
%lex-param { mvv::parser::ParserContext& tp }

%debug
%pure_parser

%initial-action
{
	yydebug = 0;
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
   const mvv::Symbol*		  symbol;
   mvv::parser::AstExp*		  astExp;
   mvv::parser::Ast*		  ast;
   mvv::parser::AstStatements*astStatements;
}

%token <str>    STRING "string"
%token <symbol> ID     "identifier"
%token <ival>   INT    "integer"
%token <fval>   FLOAT  "float"

%type<ast>				statement
%type<astStatements>	statements program
%type<astExp>			rvalue lvalue

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
%token ELSE         "else"

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

/* TODO CHECK*/
/*%left SEMI*/

%left ID
%left LBRACK

%left IFX
%left ELSE
%left ASSIGN

%left OR
%left AND
%nonassoc GE LE EQ NE LT GT
%left PLUS MINUS
%left TIMES DIVIDE
%nonassoc UMINUS


%start program

%%
program: statements									{ tp._root = $1; }        

statements: /* empty */								{ $$ = new mvv::parser::AstStatements( @$ ); std::cout << "create statements" << $$ << std::endl }		
			|statement statements					{}
			|rvalue SEMI statements					{}

statement: IF LPAREN rvalue RPAREN LBRACE statements RBRACE %prec IFX			{ $$ = new mvv::parser::AstIf( @$, $3, $6, 0 ); }
          |IF LPAREN rvalue RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE		{ $$ = new mvv::parser::AstIf( @$, $3, $6, $10 ); }
     
rvalue : INT                  { std::cout << "INT" << std::endl;}
        |FLOAT                { std::cout << "FLOAT" << std::endl;}
        |rvalue PLUS rvalue   { std::cout << "+" << std::endl;}
        |rvalue MINUS rvalue  { std::cout << "-" << std::endl;}
        |rvalue TIMES rvalue  { std::cout << "*" << std::endl;}
        |rvalue DIVIDE rvalue { std::cout << "/" << std::endl;}
        |rvalue AND rvalue
        |rvalue OR rvalue
        /*
        |MINUS rvalue %prec UMINUS{ std::cout << "UMINUS" << std::endl;}
        */
        |LPAREN rvalue RPAREN { std::cout << "()" << std::endl;}
        |STRING
        |lvalue
        |lvalue ASSIGN rvalue

     
lvalue : ID
        |ID LBRACK rvalue RBRACK
        |lvalue LBRACK rvalue RBRACK
        |lvalue DOT ID
%%
