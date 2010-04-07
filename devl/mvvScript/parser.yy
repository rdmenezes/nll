%{								// -*- C++ -*-

   //
   // scanner/parser generator:
   //   FLEX 2.5.35
   //   BISON 2.4.1
   //
   // command:
   //   flex -o lex.yy.cpp lexer.ll
   //   bison parser.yy -d -v
   //
   
   /**
    Notes:
    
    - Test()			is a rvalue
    - val fn(args);		so we can import a function
    - var[] array;		can contain any time of classes, type is directly known...
    - typename Test::Test2(); we need to be less accurate on the type so we can Test::test.a
    - namespace, static, public: namespace = class, default is public, static, can be in several files
    - a name of function/variable/class must be unique, else confusing
    - class declaration are global, function can only be declared in global scope but may have several functions with same name for overloading
    - operator +, = -, /, *, [], () have special meanings if not a primitive...
    - prmitive int, float, string
    - delc: int a[][][] = { {}{}{}... } not handled ->int a[] = {x, x, x...}, but not int a[5] = {...} => we should not give size, quite difficult to parse, not very useful...
    - function name & class can't have the same name: class Test{} int Test(); print( Test() );
    - Test Test; int Test(); => Test()
    - type find rule: -check in current class (if any)
                      -check if name of the class
                      -check declared in the scope outside the class has been declared and so on until global scope
    - construtor: checked when type checking, must be the same name than class
    
    -check function decl inside function
    - int a[ 5 ]; float b[ 5 ]; a = b; => we copy the content of b in a and cast if necessary
    - int[] fun(){} : we can't return an array (but they have the correct semantic! TODO should be added in the grammar)
    - TODO: specific operators on string, float, int...
    
    
    principles:
    - int, float, string are primitive types. the are copied by value
    - String class is "boxing" string 
    - structures are copied by reference and are automatically allocated/deallocated using ref counting
    - operator= can't be overloaded: just refcounting. To create a new instance, just recreate another object...
    - operator== checks the address for types... and not the semantic
    */
   
   #include <string>
   #include <iostream>
   #include <list>
   #include <sstream>
   #include <nll/nll.h>
   #include "parser-context.h"
   #include "ast-files.h"
   
 inline void linkFunctionToClass( mvv::parser::AstDeclClass& c )
 {
  typedef mvv::parser::AstDecls::Decls	Container;

  for ( Container::iterator it = c.getDeclarations().getDecls().begin(); it != c.getDeclarations().getDecls().end(); ++it )
  {
   mvv::parser::AstDeclFun* decl = dynamic_cast<mvv::parser::AstDeclFun*>( *it );
   if ( decl )
   {
    decl->setMemberOfClass( &c );
   }
  }
 }
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
   mvv::parser::AstNil*       nil;
   std::string*			      str;
   const mvv::Symbol*         symbol;
   mvv::parser::AstExp*	      astExp;
   mvv::parser::Ast*          ast;
   mvv::parser::AstStatements*astStatements;
   mvv::parser::AstVar*	      astVar;
   mvv::parser::AstTypeT*     astTypeT;
   mvv::parser::AstDeclVar*   astDeclVar;
   mvv::parser::AstDecls*     astDecls;
   mvv::parser::AstDeclVars*  astDeclVars;
   mvv::parser::AstArgs*      astArgs;
   std::vector<mvv::parser::AstExp*>*      arrayDim;
}

%token <str>    STRING "string"
%token <symbol> ID     "identifier"
%token <ival>   INT    "integer"
%token <fval>   FLOAT  "float"
%token <nil>    NIL    "NULL"

%type<ast>              statement
%type<astStatements>    statements program
%type<astExp>           rvalue
%type<astVar>           lvalue thislvalue
%type<astTypeT>         type type_simple type_field
%type<astDeclVar>       var_dec_simple
%type<astDecls>         var_decs_class
%type<astDeclVars>      fn_var_dec_add
%type<astDeclVars>      fn_var_dec
%type<astArgs>          args_add
%type<astArgs>          args
%type<arrayDim>         array_decl
%type<symbol>           operator_def

%destructor { delete $$; }                   "string"
%destructor { delete $$.symbol; }            "symbol"

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
%token THIS         "this"

%token OPERATORPARENT   "operator()"
%token OPERATORBRACKET  "operator[]"

%token <symbol> OPERATOR_PLUS    "operator+"
%token <symbol> OPERATOR_MINUS   "operator-"
%token <symbol> OPERATOR_TIMES   "operator*"
%token <symbol> OPERATOR_DIVIDE  "operator/"
%token <symbol> OPERATOR_LT      "operator<"
%token <symbol> OPERATOR_GT      "operator>"
%token <symbol> OPERATOR_LE      "operator<="
%token <symbol> OPERATOR_GE      "operator>="
%token <symbol> OPERATOR_EQ      "operator=="
%token <symbol> OPERATOR_NE      "operator!="
%token <symbol> OPERATOR_AND     "operator&&"
%token <symbol> OPERATOR_OR      "operator||"

%token FOR              "for"
%token IN               "in"
%token VAR              "var"
%token CLASS            "class"
%token VOID             "void"
%token RETURN           "return"
%token TYPENAME         "typename"

%token INT_T            "int type"
%token FLOAT_T          "float type"
%token STRING_T         "string type"

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
%nonassoc TYPENAME


%start program

%%
program: statements									{ tp._root = $1; }        

statements: /* empty */								{ $$ = new mvv::parser::AstStatements( @$ ); }		
			|statement statements					{ $$ = $2; $2->insert( $1 ); }

statement: IF LPAREN rvalue RPAREN LBRACE statements RBRACE %prec IFX                     { $$ = new mvv::parser::AstIf( @$, $3, $6, 0 ); }
          |IF LPAREN rvalue RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE { $$ = new mvv::parser::AstIf( @$, $3, $6, $10 ); }
          |CLASS ID LBRACE var_decs_class RBRACE                                          { mvv::parser::AstDeclClass* decl = new mvv::parser::AstDeclClass( @$, *$2, $4 ); $$ = decl; linkFunctionToClass( *decl ); }


          /**
           Test a = Test() -> type ID = rvalue, Test() is a rvalue!
           */
          |type ID ASSIGN rvalue SEMI                                { $$ = new mvv::parser::AstDeclVar( @$, $1, *$2, $4 ); }
          |type ID ASSIGN LBRACE args RBRACE SEMI                    { $$ = new mvv::parser::AstDeclVar( @$, $1, *$2, 0, $5 ); }
          |type ID LPAREN fn_var_dec RPAREN SEMI                     { $$ = new mvv::parser::AstDeclFun( @$, $1, *$2, $4 ); }
          |type ID LPAREN fn_var_dec RPAREN LBRACE statements RBRACE { $$ = new mvv::parser::AstDeclFun( @$, $1, *$2, $4, $7 ); }
          |type ID array_decl SEMI                                   { $$ = new mvv::parser::AstDeclVar( @$, $1, *$2 );
                                                                       if ( $3->size() )
                                                                       {
                                                                          $1->setArray( true );
                                                                          $1->setSize( $3 );
                                                                       } 
                                                                     }
                                                                     
          |type ID LBRACK RBRACK ASSIGN LBRACE args RBRACE SEMI       { $$ = new mvv::parser::AstDeclVar( @$, $1, *$2, 0, $7 ); $1->setArray( true ); /* we don't handle several dimensions*/ }
          |lvalue ASSIGN rvalue SEMI                                 { $$ = new mvv::parser::AstExpAssign( @$, $1, $3 ); }
          |lvalue LPAREN args RPAREN SEMI                            { $$ = new mvv::parser::AstExpCall( @$, $1, $3 ); }
          |RETURN rvalue SEMI                                        { $$ = new mvv::parser::AstReturn( @$, $2 ); }
          |RETURN SEMI                                               { $$ = new mvv::parser::AstReturn( @$ ); }
          |LBRACE statements RBRACE                                  { $$ = $2; }
          |IMPORT STRING                                             { $$ = new mvv::parser::AstImport( @$, *$2 ); }
          |INCLUDE STRING                                            { $$ = new mvv::parser::AstInclude( @$, *$2 ); }
          
          /* operator overloading*/
          |type operator_def LPAREN fn_var_dec RPAREN LBRACE statements RBRACE { $$ = new mvv::parser::AstDeclFun( @$, $1, *$2, $4, $7 ); }
          |type operator_def LPAREN fn_var_dec RPAREN SEMI                     { $$ = new mvv::parser::AstDeclFun( @$, $1, *$2, $4 ); }
          

			
array_decl: /* empty */                                              { $$ = new std::vector<mvv::parser::AstExp*>(); }
           |LBRACK rvalue RBRACK array_decl                          { $$ = $4; $$->push_back( $2 ); }
           
operator_def: OPERATOR_PLUS                                          { $$ = new mvv::Symbol( mvv::Symbol::create ( "operator+" ) ); }
          |OPERATOR_MINUS                                            { $$ = new mvv::Symbol( mvv::Symbol::create ( "operator-" ) ); }
          |OPERATOR_TIMES                                            { $$ = new mvv::Symbol( mvv::Symbol::create ( "operator*" ) ); }
          |OPERATOR_DIVIDE                                           { $$ = new mvv::Symbol( mvv::Symbol::create ( "operator/" ) ); }
          |OPERATOR_LT                                               { $$ = new mvv::Symbol( mvv::Symbol::create ( "operator<" ) ); }
          |OPERATOR_GT                                               { $$ = new mvv::Symbol( mvv::Symbol::create ( "operator>" ) ); }
          |OPERATOR_LE                                               { $$ = new mvv::Symbol( mvv::Symbol::create ( "operator<=" ) ); }
          |OPERATOR_GE                                               { $$ = new mvv::Symbol( mvv::Symbol::create ( "operator>=" ) ); }
          |OPERATOR_EQ                                               { $$ = new mvv::Symbol( mvv::Symbol::create ( "operator==" ) ); }
          |OPERATOR_NE                                               { $$ = new mvv::Symbol( mvv::Symbol::create ( "operator!=" ) ); }
          |OPERATOR_AND                                              { $$ = new mvv::Symbol( mvv::Symbol::create ( "operator&&" ) ); }
          |OPERATOR_OR                                               { $$ = new mvv::Symbol( mvv::Symbol::create ( "operator||" ) ); }
           

     
rvalue : INT                  { $$ = new mvv::parser::AstInt( @$, $1 ); }
        |FLOAT                { $$ = new mvv::parser::AstFloat( @$, $1 ); }
        |NIL                  { $$ = new mvv::parser::AstNil( @$ ); }
        |rvalue PLUS rvalue   { $$ = new mvv::parser::AstOpBin( @$, $1, $3, mvv::parser::AstOpBin::PLUS ); }
        |rvalue MINUS rvalue  { $$ = new mvv::parser::AstOpBin( @$, $1, $3, mvv::parser::AstOpBin::MINUS ); }
        |rvalue TIMES rvalue  { $$ = new mvv::parser::AstOpBin( @$, $1, $3, mvv::parser::AstOpBin::TIMES ); }
        |rvalue DIVIDE rvalue { $$ = new mvv::parser::AstOpBin( @$, $1, $3, mvv::parser::AstOpBin::DIVIDE ); }
        |rvalue AND rvalue    { $$ = new mvv::parser::AstOpBin( @$, $1, $3, mvv::parser::AstOpBin::AND ); }
        |rvalue OR rvalue     { $$ = new mvv::parser::AstOpBin( @$, $1, $3, mvv::parser::AstOpBin::OR ); }
        |rvalue LT rvalue     { $$ = new mvv::parser::AstOpBin( @$, $1, $3, mvv::parser::AstOpBin::LT ); }
        |rvalue LE rvalue     { $$ = new mvv::parser::AstOpBin( @$, $1, $3, mvv::parser::AstOpBin::LE ); }
        |rvalue GT rvalue     { $$ = new mvv::parser::AstOpBin( @$, $1, $3, mvv::parser::AstOpBin::GT ); }
        |rvalue GE rvalue     { $$ = new mvv::parser::AstOpBin( @$, $1, $3, mvv::parser::AstOpBin::GE ); }
        |rvalue NE rvalue     { $$ = new mvv::parser::AstOpBin( @$, $1, $3, mvv::parser::AstOpBin::NE ); }
        |MINUS rvalue %prec UMINUS        { $$ = new mvv::parser::AstOpBin( @$, new mvv::parser::AstInt( @$, 0 ) , $2, mvv::parser::AstOpBin::MINUS ); }
        |LPAREN rvalue RPAREN             { $$ = new mvv::parser::AstExpSeq( @$, $2 ); }
        |STRING                           { $$ = new mvv::parser::AstString( @$, *$1 ); }
        |lvalue                           { $$ = $1; }
        |lvalue ASSIGN rvalue             { $$ = new mvv::parser::AstExpAssign( @$, $1, $3 ); }
        |TYPENAME type LPAREN args RPAREN { $$ = new mvv::parser::AstExpTypename( @$, $2, $4 );}

/* note that nothing prevent from this.this.varname, however as "this" is a token, there will be a binding error: you can't declare a variable named 'this' */
thislvalue : THIS                            { $$ = new mvv::parser::AstThis( @$ ); }
           | lvalue DOT THIS                 { $$ = new mvv::parser::AstVarField( @$, $1, mvv::Symbol::create( "this" ) ); }

lvalue : ID                               { $$ = new mvv::parser::AstVarSimple( @$, *$1, true ); }
        |ID LBRACK rvalue RBRACK          { $$ = new mvv::parser::AstVarArray( @$, new mvv::parser::AstVarSimple( @$, *$1, true ), $3 ); }
        |lvalue DOT ID                    { $$ = new mvv::parser::AstVarField( @$, $1, *$3 ); }
        |lvalue LBRACK rvalue RBRACK      { $$ = new mvv::parser::AstVarArray( @$, $1, $3 ); }
        |lvalue LPAREN args RPAREN        { $$ = new mvv::parser::AstExpCall( @$, $1, $3 ); }
        |thislvalue                       { $$ = $1; }


	  
var_decs_class: /* empty */				                                                     { $$ = new mvv::parser::AstDecls( @$ ); }
      |var_dec_simple SEMI var_decs_class                                                    { $$ = $3; $$->insert( $1 ); }
      |type ID LBRACK RBRACK ASSIGN LBRACE args RBRACE SEMI var_decs_class                   { $$ = $10; mvv::parser::AstDeclVar* var = new mvv::parser::AstDeclVar( @$, $1, *$2, 0, $7 ); $1->setArray( true ); $$->insert( var ); }
      |CLASS ID LBRACE var_decs_class RBRACE var_decs_class                                  { $$ = $6; mvv::parser::AstDeclClass* decl = new mvv::parser::AstDeclClass( @$, *$2, $4 ); $$->insert( decl ); linkFunctionToClass( *decl ); }
      |type ID LPAREN fn_var_dec RPAREN LBRACE statements RBRACE var_decs_class              { $$ = $9; $$->insert( new mvv::parser::AstDeclFun( @$, $1, *$2, $4, $7 ) ); }	
      |type ID LPAREN fn_var_dec RPAREN SEMI var_decs_class                                  { $$ = $7; $$->insert( new mvv::parser::AstDeclFun( @$, $1, *$2, $4 ) ); }
      |type OPERATORBRACKET LPAREN fn_var_dec RPAREN LBRACE statements RBRACE var_decs_class { $$ = $9; $$->insert( new mvv::parser::AstDeclFun( @$, $1, mvv::Symbol::create("operator[]"), $4, $7 ) ); }	
      |type OPERATORBRACKET LPAREN fn_var_dec RPAREN SEMI var_decs_class                     { $$ = $7; $$->insert( new mvv::parser::AstDeclFun( @$, $1, mvv::Symbol::create("operator[]"), $4 ) ); }
      |type OPERATORPARENT LPAREN fn_var_dec RPAREN LBRACE statements RBRACE var_decs_class  { $$ = $9; $$->insert( new mvv::parser::AstDeclFun( @$, $1, mvv::Symbol::create("operator()"), $4, $7 ) ); }	
      |type OPERATORPARENT LPAREN fn_var_dec RPAREN SEMI var_decs_class                      { $$ = $7; $$->insert( new mvv::parser::AstDeclFun( @$, $1, mvv::Symbol::create("operator()"), $4 ) ); }
      |ID LPAREN fn_var_dec RPAREN LBRACE statements RBRACE var_decs_class                   { $$ = $8; $$->insert( new mvv::parser::AstDeclFun( @$, 0, *$1, $3, $6 ) ); }
      |ID LPAREN fn_var_dec RPAREN SEMI var_decs_class                                       { $$ = $6; $$->insert( new mvv::parser::AstDeclFun( @$, 0, *$1, $3 ) ); }
      
      |type OPERATOR_PLUS LPAREN fn_var_dec RPAREN LBRACE statements RBRACE var_decs_class   { $$ = $9; $$->insert( new mvv::parser::AstDeclFun( @$, $1, mvv::Symbol::create("operator+"), $4, $7 ) ); }
      |type OPERATOR_MINUS LPAREN fn_var_dec RPAREN LBRACE statements RBRACE var_decs_class  { $$ = $9; $$->insert( new mvv::parser::AstDeclFun( @$, $1, mvv::Symbol::create("operator-"), $4, $7 ) ); }
      |type OPERATOR_TIMES LPAREN fn_var_dec RPAREN LBRACE statements RBRACE var_decs_class  { $$ = $9; $$->insert( new mvv::parser::AstDeclFun( @$, $1, mvv::Symbol::create("operator*"), $4, $7 ) ); }
      |type OPERATOR_DIVIDE LPAREN fn_var_dec RPAREN LBRACE statements RBRACE var_decs_class { $$ = $9; $$->insert( new mvv::parser::AstDeclFun( @$, $1, mvv::Symbol::create("operator/"), $4, $7 ) ); }
      |type OPERATOR_LT LPAREN fn_var_dec RPAREN LBRACE statements RBRACE var_decs_class     { $$ = $9; $$->insert( new mvv::parser::AstDeclFun( @$, $1, mvv::Symbol::create("operator<"), $4, $7 ) ); }
      |type OPERATOR_GT LPAREN fn_var_dec RPAREN LBRACE statements RBRACE var_decs_class     { $$ = $9; $$->insert( new mvv::parser::AstDeclFun( @$, $1, mvv::Symbol::create("operator>"), $4, $7 ) ); }
      |type OPERATOR_LE LPAREN fn_var_dec RPAREN LBRACE statements RBRACE var_decs_class     { $$ = $9; $$->insert( new mvv::parser::AstDeclFun( @$, $1, mvv::Symbol::create("operator<="), $4, $7 ) ); }
      |type OPERATOR_GE LPAREN fn_var_dec RPAREN LBRACE statements RBRACE var_decs_class     { $$ = $9; $$->insert( new mvv::parser::AstDeclFun( @$, $1, mvv::Symbol::create("operator>="), $4, $7 ) ); }
      |type OPERATOR_EQ LPAREN fn_var_dec RPAREN LBRACE statements RBRACE var_decs_class     { $$ = $9; $$->insert( new mvv::parser::AstDeclFun( @$, $1, mvv::Symbol::create("operator=="), $4, $7 ) ); }
      |type OPERATOR_NE LPAREN fn_var_dec RPAREN LBRACE statements RBRACE var_decs_class     { $$ = $9; $$->insert( new mvv::parser::AstDeclFun( @$, $1, mvv::Symbol::create("operator!="), $4, $7 ) ); }
      |type OPERATOR_AND LPAREN fn_var_dec RPAREN LBRACE statements RBRACE var_decs_class    { $$ = $9; $$->insert( new mvv::parser::AstDeclFun( @$, $1, mvv::Symbol::create("operator&&"), $4, $7 ) ); }
      |type OPERATOR_OR LPAREN fn_var_dec RPAREN LBRACE statements RBRACE var_decs_class     { $$ = $9; $$->insert( new mvv::parser::AstDeclFun( @$, $1, mvv::Symbol::create("operator||"), $4, $7 ) ); }
          
  
 type_simple: VAR                     { $$ = new mvv::parser::AstType( @$, mvv::parser::AstType::VAR ); }
             |INT_T                   { $$ = new mvv::parser::AstType( @$, mvv::parser::AstType::INT ); }
             |FLOAT_T                 { $$ = new mvv::parser::AstType( @$, mvv::parser::AstType::FLOAT );}
             |STRING_T                { $$ = new mvv::parser::AstType( @$, mvv::parser::AstType::STRING ); }
             |VOID                    { $$ = new mvv::parser::AstType( @$, mvv::parser::AstType::VOID );}
	         
type_field: ID                        { $$ = new mvv::parser::AstType( @$, mvv::parser::AstType::SYMBOL, $1 ); }
           |ID DCOLON type_field      { $$ = new mvv::parser::AstTypeField( @$, $3, *$1 ); }
          
type: type_field                      { $$ = $1; }
     |type_simple                     { $$ = $1; }
     
	  
var_dec_simple: type ID ASSIGN rvalue { $$ = new mvv::parser::AstDeclVar( @$, $1, *$2, $4 ); }
               |type ID array_decl    { $$ = new mvv::parser::AstDeclVar( @$, $1, *$2 );
                                        if ( $3->size() )
                                        {
                                           $1->setArray( true );
                                           $1->setSize( $3 );
                                        } 
                                      }                                                                     
	   		    

args_add: /* empty */                 { $$ = new mvv::parser::AstArgs( @$ ); }
          |COMA rvalue args_add       { $$ = $3; $$->insert( $2 ); }
		  	  
args: /* empty */                     { $$ = new mvv::parser::AstArgs( @$ ); }
      |rvalue args_add                { $$ = $2; $$->insert( $1 ); }
	  
fn_var_dec_add: /* empty */						{ $$ = new mvv::parser::AstDeclVars( @$ ); }
		  |COMA var_dec_simple fn_var_dec_add	{ $$ = $3; $$->insert( $2 ); }
		  	  
fn_var_dec: /* empty */							{ $$ = new mvv::parser::AstDeclVars( @$ ); }
	  |var_dec_simple fn_var_dec_add			{ $$ = $2; $$->insert( $1 ); }
	 
%%
