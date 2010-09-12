/* A Bison parser, made by GNU Bison 2.4.2.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2006, 2009-2010 Free Software
   Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     YYEOF = 0,
     STRING = 258,
     ID = 259,
     CMP_INT = 260,
     CMP_FLOAT = 261,
     NIL = 262,
     TILDE = 264,
     AND = 265,
     ASSIGN = 266,
     COMA = 267,
     DIVIDE = 268,
     DOT = 269,
     EQ = 270,
     GE = 271,
     GT = 272,
     DCOLON = 273,
     WHILE = 274,
     BREAK = 275,
     IF = 276,
     ELSE = 277,
     LBRACE = 278,
     LBRACK = 279,
     LE = 280,
     LPAREN = 281,
     LT = 282,
     MINUS = 283,
     NE = 284,
     OR = 285,
     PLUS = 286,
     RBRACE = 287,
     RBRACK = 288,
     RPAREN = 289,
     SEMI = 290,
     TIMES = 291,
     THIS = 292,
     REF = 293,
     STREAMO = 294,
     STREAMI = 295,
     OPERATORPARENT = 296,
     OPERATORBRACKET = 297,
     OPERATOR_PLUS = 298,
     OPERATOR_MINUS = 299,
     OPERATOR_TIMES = 300,
     OPERATOR_DIVIDE = 301,
     OPERATOR_LT = 302,
     OPERATOR_GT = 303,
     OPERATOR_LE = 304,
     OPERATOR_GE = 305,
     OPERATOR_EQ = 306,
     OPERATOR_NE = 307,
     OPERATOR_AND = 308,
     OPERATOR_OR = 309,
     OPERATOR_STREAMO = 310,
     OPERATOR_STREAMI = 311,
     FOR = 312,
     IN = 313,
     VAR = 314,
     CLASS = 315,
     VOID = 316,
     RETURN = 317,
     TYPENAME = 318,
     TYPEDEF = 319,
     INT_T = 320,
     FLOAT_T = 321,
     STRING_T = 322,
     IMPORT = 323,
     INCLUDE = 324,
     IFX = 325,
     UMINUS = 326
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1685 of yacc.c  */
#line 127 "parser.yy"

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



/* Line 1685 of yacc.c  */
#line 143 "parser.tab.hh"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif



#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



