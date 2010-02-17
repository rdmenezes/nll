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
	yydebug = tp._parse_trace_p;
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
   mvv::parser::AstExp*		   astExp;
   mvv::parser::Ast*		      ast;
   mvv::parser::AstStatements*astStatements;
   mvv::parser::AstVar*		   astVar;
   mvv::parser::AstType*      astType;
   mvv::parser::AstDeclVar*   astDeclVar;
   mvv::parser::AstDecls*     astDecls;
   mvv::parser::AstDeclVars*  astDeclVars;
   mvv::parser::AstArgs*      astArgs;
}

%token <str>    STRING "string"
%token <symbol> ID     "identifier"
%token <ival>   INT    "integer"
%token <fval>   FLOAT  "float"

%type<ast>				   statement
%type<astStatements>	   statements program
%type<astExp>			   rvalue
%type<astVar>			   lvalue
%type<astType>          type type_simple
%type<astDeclVar>       var_dec_simple
%type<astDecls>         var_decs_class
%type<astDeclVars>      fn_var_dec_add
%type<astDeclVars>      fn_var_dec
%type<astArgs>          args_add
%type<astArgs>          args

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
%token DCOLON       "::"

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

%token <symbol> OPERATORPARENT   "operator()"
%token <symbol> OPERATORBRACKET  "operator[]"
%token FOR              "for"
%token IN               "in"
%token VAR              "var"
%token CLASS            "class"
%token VOID             "void"
%token NIL              "NULL"
%token RETURN			"return"

%token INT_T			"int type"
%token FLOAT_T			"float type"
%token STRING_T			"string type"
%token ARRAY_T          "[]"

%token IMPORT           "import"
%token INCLUDE          "include"

%token YYEOF   0        "end of file"


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

statements: /* empty */								{ $$ = new mvv::parser::AstStatements( @$ ); }		
			|statement statements					{ $$ = $2; $2->insert( $1 ); }

statement: IF LPAREN rvalue RPAREN LBRACE statements RBRACE %prec IFX   { $$ = new mvv::parser::AstIf( @$, $3, $6, 0 ); }
          |IF LPAREN rvalue RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE		{ $$ = new mvv::parser::AstIf( @$, $3, $6, $10 ); }
          |CLASS ID LBRACE var_decs_class RBRACE						{ $$ = new mvv::parser::AstDeclClass( @$, *$2, $4 ); }


          /**
           Test a = Test() -> type ID = rvalue, Test() is a rvalue!
           */
          |type ID SEMI													{ $$ = new mvv::parser::AstDeclVar( @$, $1, *$2 ); }
          |type ID ASSIGN rvalue SEMI									{ $$ = new mvv::parser::AstDeclVar( @$, $1, *$2, $4 ); }
          |type ID ASSIGN LBRACE args RBRACE SEMI						{  }
          |type ID LPAREN fn_var_dec RPAREN SEMI						{ $$ = new mvv::parser::AstDeclFun( @$, $1, *$2, $4 ); }
          |type ID LPAREN fn_var_dec RPAREN LBRACE statements RBRACE	{ $$ = new mvv::parser::AstDeclFun( @$, $1, *$2, $4, $7 ); }
          
          |type ID LBRACK rvalue RBRACK SEMI		{ /*$$ = new mvv::parser::AstVarArray( @$, new mvv::parser::AstVarSimple( @$, *$1, true ), true, $3 );*/ }
          |type ID LBRACK RBRACK ASSIGN LBRACE args RBRACE SEMI
          
          /*|type ID LPAREN args RPAREN SEMI*/
          /*|lvalue ASSIGN type LPAREN args RPAREN SEMI*/
          
          
          |lvalue ASSIGN rvalue SEMI { $$ = new mvv::parser::AstExpAssign( @$, $1, $3 ); }
          |lvalue LPAREN args RPAREN SEMI  { $$ = new mvv::parser::AstExpCall( @$, $1, $3 ); }
          
          
          
          |RETURN rvalue SEMI											{ $$ = new mvv::parser::AstReturn( @$, $2 ); }
          |RETURN SEMI													{ $$ = new mvv::parser::AstReturn( @$ ); }
          |LBRACE statements RBRACE										{ $$ = $2; }
          |IMPORT STRING												{ $$ = new mvv::parser::AstImport( @$, *$2 ); }
          |INCLUDE STRING												{ $$ = new mvv::parser::AstInclude( @$, *$2 ); }
     
rvalue : INT                  { $$ = new mvv::parser::AstInt( @$, $1 ); }
        |FLOAT                { $$ = new mvv::parser::AstFloat( @$, $1 ); }
        |rvalue PLUS rvalue   { $$ = new mvv::parser::AstOpBin( @$, $1, $3, mvv::parser::AstOpBin::PLUS ); }
        |rvalue MINUS rvalue  { $$ = new mvv::parser::AstOpBin( @$, $1, $3, mvv::parser::AstOpBin::MINUS ); }
        |rvalue TIMES rvalue  { $$ = new mvv::parser::AstOpBin( @$, $1, $3, mvv::parser::AstOpBin::TIMES ); }
        |rvalue DIVIDE rvalue { $$ = new mvv::parser::AstOpBin( @$, $1, $3, mvv::parser::AstOpBin::DIVIDE ); }
        |rvalue AND rvalue	  { $$ = new mvv::parser::AstOpBin( @$, $1, $3, mvv::parser::AstOpBin::AND ); }
        |rvalue OR rvalue	  { $$ = new mvv::parser::AstOpBin( @$, $1, $3, mvv::parser::AstOpBin::OR ); }
        |rvalue LT rvalue
        |rvalue LE rvalue
        |rvalue GT rvalue
        |rvalue GE rvalue
        |rvalue NE rvalue
        |MINUS rvalue %prec UMINUS{ $$ = new mvv::parser::AstOpBin( @$, new mvv::parser::AstInt( @$, 0 ) , $2, mvv::parser::AstOpBin::MINUS ); }
        |LPAREN rvalue RPAREN { $$ = $2; }
        |STRING				  { $$ = new mvv::parser::AstString( @$, *$1 ); }
        |lvalue ASSIGN LBRACE args RBRACE { /* TODO*/ }
        |lvalue				  { $$ = $1; }
        |lvalue ASSIGN rvalue { $$ = new mvv::parser::AstExpAssign( @$, $1, $3 ); }
        |lvalue LPAREN args RPAREN { $$ = new mvv::parser::AstExpCall( @$, $1, $3 ); }

     
lvalue : ID								{ $$ = new mvv::parser::AstVarSimple( @$, *$1, true ); }
        |ID LBRACK rvalue RBRACK		{ $$ = new mvv::parser::AstVarArray( @$, new mvv::parser::AstVarSimple( @$, *$1, true ), true, $3 ); }
        |lvalue DOT ID					{ $$ = new mvv::parser::AstVarField( @$, $1, *$3 ); }
        |lvalue LBRACK rvalue RBRACK	{ $$ = new mvv::parser::AstVarArray( @$, $1, true, $3 ); }


	  
var_decs_class: /* empty */				                                                     { $$ = new mvv::parser::AstDecls( @$ ); }
	  |var_dec_simple SEMI var_decs_class		                                             { $$ = $3; $$->insert( $1 ); }
	  |type ID LBRACK rvalue RBRACK SEMI var_decs_class	                        {/* TODO FIXME*/ $$ = $7 }
     |type ID LBRACK RBRACK ASSIGN LBRACE args RBRACE SEMI var_decs_class        {/* TODO FIXME*/ $$ = $10 }
     |CLASS ID LBRACE var_decs_class RBRACE	var_decs_class					         {/* TODO FIXME*/ $$ = $6 }
          
	  |type ID LPAREN fn_var_dec RPAREN LBRACE statements RBRACE var_decs_class	             { $$ = $9; $$->insert( new mvv::parser::AstDeclFun( @$, $1, *$2, $4, $7 ) ); }	
	  |type ID LPAREN fn_var_dec RPAREN SEMI var_decs_class                                  { $$ = $7; $$->insert( new mvv::parser::AstDeclFun( @$, $1, *$2, $4 ) ); }
	  |type OPERATORBRACKET LPAREN fn_var_dec RPAREN LBRACE statements RBRACE var_decs_class { $$ = $9; $$->insert( new mvv::parser::AstDeclFun( @$, $1, *$2, $4, $7 ) ); }	
	  |type OPERATORBRACKET LPAREN fn_var_dec RPAREN SEMI var_decs_class                     { $$ = $7; $$->insert( new mvv::parser::AstDeclFun( @$, $1, *$2, $4 ) ); }
	  |type OPERATORPARENT LPAREN fn_var_dec RPAREN LBRACE statements RBRACE var_decs_class	 { $$ = $9; $$->insert( new mvv::parser::AstDeclFun( @$, $1, *$2, $4, $7 ) ); }	
	  |type OPERATORPARENT LPAREN fn_var_dec RPAREN SEMI var_decs_class                      { $$ = $7; $$->insert( new mvv::parser::AstDeclFun( @$, $1, *$2, $4 ) ); }
	  
type_simple: VAR				{ $$ = new mvv::parser::AstType( @$, mvv::parser::AstType::VAR ); }
	         |INT_T			{ $$ = new mvv::parser::AstType( @$, mvv::parser::AstType::INT ); }
	         |FLOAT_T			{ $$ = new mvv::parser::AstType( @$, mvv::parser::AstType::FLOAT );}
	         |STRING_T		{ $$ = new mvv::parser::AstType( @$, mvv::parser::AstType::STRING ); }
	         |VOID				{ $$ = new mvv::parser::AstType( @$, mvv::parser::AstType::VOID );}
	  
type_field: ID
           |type_field DCOLON ID
          
type: type_field
     |type_simple
     
	  
var_dec_simple: type ID							{ $$ = new mvv::parser::AstDeclVar( @$, $1, *$2 ); }
               |type ID ASSIGN rvalue			{ $$ = new mvv::parser::AstDeclVar( @$, $1, *$2, $4 ); }
	   		    

args_add: /* empty */							{ $$ = new mvv::parser::AstArgs( @$ ); }
		  |COMA rvalue args_add			        { $$ = $3; $$->insert( $2 ); }
		  	  
args: /* empty */						        { $$ = new mvv::parser::AstArgs( @$ ); }
	  |rvalue args_add							{ $$ = $2; $$->insert( $1 ); }
	  
fn_var_dec_add: /* empty */						{ $$ = new mvv::parser::AstDeclVars( @$ ); }
		  |COMA var_dec_simple fn_var_dec_add	{ $$ = $3; $$->insert( $2 ); }
		  	  
fn_var_dec: /* empty */							{ $$ = new mvv::parser::AstDeclVars( @$ ); }
	  |var_dec_simple fn_var_dec_add			{ $$ = $2; $$->insert( $1 ); }
	 
%%
