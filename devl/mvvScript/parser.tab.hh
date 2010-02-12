
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
     IF = 271,
     ELSE = 272,
     LBRACE = 273,
     LBRACK = 274,
     LE = 275,
     LPAREN = 276,
     LT = 277,
     MINUS = 278,
     NE = 279,
     OR = 280,
     PLUS = 281,
     RBRACE = 282,
     RBRACK = 283,
     RPAREN = 284,
     SEMI = 285,
     TIMES = 286,
     OPERATORPARENT = 287,
     OPERATORBRACKET = 288,
     FOR = 289,
     IN = 290,
     VAR = 291,
     CLASS = 292,
     VOID = 293,
     NIL = 294,
     IFX = 295,
     UMINUS = 296
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 41 "parser.yy"

   // Tokens.
   int                        ival;
   float                      fval;
   std::string*			      str;
   const mvv::Symbol*		   symbol;



/* Line 1676 of yacc.c  */
#line 103 "parser.tab.hh"
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



