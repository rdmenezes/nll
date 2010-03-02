
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
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
     INT = 260,
     FLOAT = 261,
     NIL = 262,
     AND = 264,
     ASSIGN = 265,
     COMA = 266,
     DIVIDE = 267,
     DOT = 268,
     EQ = 269,
     GE = 270,
     GT = 271,
     DCOLON = 272,
     IF = 273,
     ELSE = 274,
     LBRACE = 275,
     LBRACK = 276,
     LE = 277,
     LPAREN = 278,
     LT = 279,
     MINUS = 280,
     NE = 281,
     OR = 282,
     PLUS = 283,
     RBRACE = 284,
     RBRACK = 285,
     RPAREN = 286,
     SEMI = 287,
     TIMES = 288,
     OPERATORPARENT = 289,
     OPERATORBRACKET = 290,
     OPERATOR_PLUS = 291,
     OPERATOR_MINUS = 292,
     OPERATOR_TIMES = 293,
     OPERATOR_DIVIDE = 294,
     OPERATOR_LT = 295,
     OPERATOR_GT = 296,
     OPERATOR_LE = 297,
     OPERATOR_GE = 298,
     OPERATOR_EQ = 299,
     OPERATOR_NE = 300,
     OPERATOR_AND = 301,
     OPERATOR_OR = 302,
     FOR = 303,
     IN = 304,
     VAR = 305,
     CLASS = 306,
     VOID = 307,
     RETURN = 308,
     TYPENAME = 309,
     INT_T = 310,
     FLOAT_T = 311,
     STRING_T = 312,
     IMPORT = 313,
     INCLUDE = 314,
     IFX = 315,
     UMINUS = 316
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 89 "parser.yy"

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



/* Line 1676 of yacc.c  */
#line 134 "parser.tab.hh"
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



