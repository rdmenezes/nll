
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
     AND = 263,
     ASSIGN = 264,
     COMA = 265,
     DIVIDE = 266,
     DOT = 267,
     EQ = 268,
     GE = 269,
     GT = 270,
     DCOLON = 271,
     IF = 272,
     ELSE = 273,
     LBRACE = 274,
     LBRACK = 275,
     LE = 276,
     LPAREN = 277,
     LT = 278,
     MINUS = 279,
     NE = 280,
     OR = 281,
     PLUS = 282,
     RBRACE = 283,
     RBRACK = 284,
     RPAREN = 285,
     SEMI = 286,
     TIMES = 287,
     OPERATORPARENT = 288,
     OPERATORBRACKET = 289,
     FOR = 290,
     IN = 291,
     VAR = 292,
     CLASS = 293,
     VOID = 294,
     NIL = 295,
     RETURN = 296,
     INT_T = 297,
     FLOAT_T = 298,
     STRING_T = 299,
     IMPORT = 300,
     INCLUDE = 301,
     IFX = 302,
     UMINUS = 303
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 43 "parser.yy"

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



/* Line 1676 of yacc.c  */
#line 119 "parser.tab.hh"
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



