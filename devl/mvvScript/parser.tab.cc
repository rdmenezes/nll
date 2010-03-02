
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 1



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "parser.yy"
								// -*- C++ -*-

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


/* Line 189 of yacc.c  */
#line 141 "parser.tab.cc"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


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

/* Line 214 of yacc.c  */
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



/* Line 214 of yacc.c  */
#line 259 "parser.tab.cc"
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


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 284 "parser.tab.cc"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  38
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   748

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  62
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  16
/* YYNRULES -- Number of rules.  */
#define YYNRULES  102
/* YYNRULES -- Number of states.  */
#define YYNSTATES  385

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   316

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     6,     9,    17,    29,    35,    41,
      49,    56,    65,    70,    80,    85,    91,    95,    98,   102,
     105,   108,   117,   126,   135,   144,   153,   162,   171,   180,
     189,   198,   207,   216,   217,   222,   224,   226,   228,   232,
     236,   240,   244,   248,   252,   256,   260,   264,   268,   272,
     275,   279,   281,   283,   287,   293,   295,   300,   304,   309,
     314,   315,   319,   330,   337,   347,   355,   365,   373,   383,
     391,   400,   407,   417,   427,   437,   447,   457,   467,   477,
     487,   497,   507,   517,   527,   529,   531,   533,   535,   537,
     539,   543,   545,   547,   552,   556,   557,   561,   562,   565,
     566,   570,   571
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      63,     0,    -1,    64,    -1,    -1,    65,    64,    -1,    18,
      23,    67,    31,    20,    64,    29,    -1,    18,    23,    67,
      31,    20,    64,    29,    19,    20,    64,    29,    -1,    51,
       4,    20,    69,    29,    -1,    72,     4,    10,    67,    32,
      -1,    72,     4,    10,    20,    75,    29,    32,    -1,    72,
       4,    23,    77,    31,    32,    -1,    72,     4,    23,    77,
      31,    20,    64,    29,    -1,    72,     4,    66,    32,    -1,
      72,     4,    21,    30,    10,    20,    75,    29,    32,    -1,
      68,    10,    67,    32,    -1,    68,    23,    75,    31,    32,
      -1,    53,    67,    32,    -1,    53,    32,    -1,    20,    64,
      29,    -1,    58,     3,    -1,    59,     3,    -1,    72,    36,
      23,    77,    31,    20,    64,    29,    -1,    72,    37,    23,
      77,    31,    20,    64,    29,    -1,    72,    38,    23,    77,
      31,    20,    64,    29,    -1,    72,    39,    23,    77,    31,
      20,    64,    29,    -1,    72,    40,    23,    77,    31,    20,
      64,    29,    -1,    72,    41,    23,    77,    31,    20,    64,
      29,    -1,    72,    42,    23,    77,    31,    20,    64,    29,
      -1,    72,    43,    23,    77,    31,    20,    64,    29,    -1,
      72,    44,    23,    77,    31,    20,    64,    29,    -1,    72,
      45,    23,    77,    31,    20,    64,    29,    -1,    72,    46,
      23,    77,    31,    20,    64,    29,    -1,    72,    47,    23,
      77,    31,    20,    64,    29,    -1,    -1,    21,    67,    30,
      66,    -1,     5,    -1,     6,    -1,     7,    -1,    67,    28,
      67,    -1,    67,    25,    67,    -1,    67,    33,    67,    -1,
      67,    12,    67,    -1,    67,     9,    67,    -1,    67,    27,
      67,    -1,    67,    24,    67,    -1,    67,    22,    67,    -1,
      67,    16,    67,    -1,    67,    15,    67,    -1,    67,    26,
      67,    -1,    25,    67,    -1,    23,    67,    31,    -1,     3,
      -1,    68,    -1,    68,    10,    67,    -1,    54,    72,    23,
      75,    31,    -1,     4,    -1,     4,    21,    67,    30,    -1,
      68,    13,     4,    -1,    68,    21,    67,    30,    -1,    68,
      23,    75,    31,    -1,    -1,    73,    32,    69,    -1,    72,
       4,    21,    30,    10,    20,    75,    29,    32,    69,    -1,
      51,     4,    20,    69,    29,    69,    -1,    72,     4,    23,
      77,    31,    20,    64,    29,    69,    -1,    72,     4,    23,
      77,    31,    32,    69,    -1,    72,    35,    23,    77,    31,
      20,    64,    29,    69,    -1,    72,    35,    23,    77,    31,
      32,    69,    -1,    72,    34,    23,    77,    31,    20,    64,
      29,    69,    -1,    72,    34,    23,    77,    31,    32,    69,
      -1,     4,    23,    77,    31,    20,    64,    29,    69,    -1,
       4,    23,    77,    31,    32,    69,    -1,    72,    36,    23,
      77,    31,    20,    64,    29,    69,    -1,    72,    37,    23,
      77,    31,    20,    64,    29,    69,    -1,    72,    38,    23,
      77,    31,    20,    64,    29,    69,    -1,    72,    39,    23,
      77,    31,    20,    64,    29,    69,    -1,    72,    40,    23,
      77,    31,    20,    64,    29,    69,    -1,    72,    41,    23,
      77,    31,    20,    64,    29,    69,    -1,    72,    42,    23,
      77,    31,    20,    64,    29,    69,    -1,    72,    43,    23,
      77,    31,    20,    64,    29,    69,    -1,    72,    44,    23,
      77,    31,    20,    64,    29,    69,    -1,    72,    45,    23,
      77,    31,    20,    64,    29,    69,    -1,    72,    46,    23,
      77,    31,    20,    64,    29,    69,    -1,    72,    47,    23,
      77,    31,    20,    64,    29,    69,    -1,    50,    -1,    55,
      -1,    56,    -1,    57,    -1,    52,    -1,     4,    -1,     4,
      17,    71,    -1,    71,    -1,    70,    -1,    72,     4,    10,
      67,    -1,    72,     4,    66,    -1,    -1,    11,    67,    74,
      -1,    -1,    67,    74,    -1,    -1,    11,    73,    76,    -1,
      -1,    73,    76,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   210,   210,   212,   213,   215,   216,   217,   223,   224,
     225,   226,   227,   235,   236,   237,   238,   239,   240,   241,
     242,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   259,   260,   264,   265,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   287,   288,   289,   290,   291,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   322,   323,   324,   325,   326,   328,
     329,   331,   332,   335,   336,   345,   346,   348,   349,   351,
     352,   354,   355
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "$undefined", "\"string\"",
  "\"identifier\"", "\"integer\"", "\"float\"", "\"NULL\"", "\"symbol\"",
  "\"&&\"", "\"=\"", "\",\"", "\"/\"", "\".\"", "\"==\"", "\">=\"",
  "\">\"", "\"::\"", "\"if\"", "\"else\"", "\"{\"", "\"[\"", "\"<=\"",
  "\"(\"", "\"<\"", "\"-\"", "\"!=\"", "\"||\"", "\"+\"", "\"}\"", "\"]\"",
  "\")\"", "\";\"", "\"*\"", "\"operator()\"", "\"operator[]\"",
  "\"operator+\"", "\"operator-\"", "\"operator*\"", "\"operator/\"",
  "\"operator<\"", "\"operator>\"", "\"operator<=\"", "\"operator>=\"",
  "\"operator==\"", "\"operator!=\"", "\"operator&&\"", "\"operator||\"",
  "\"for\"", "\"in\"", "\"var\"", "\"class\"", "\"void\"", "\"return\"",
  "\"typename\"", "\"int type\"", "\"float type\"", "\"string type\"",
  "\"import\"", "\"include\"", "IFX", "UMINUS", "$accept", "program",
  "statements", "statement", "array_decl", "rvalue", "lvalue",
  "var_decs_class", "type_simple", "type_field", "type", "var_dec_simple",
  "args_add", "args", "fn_var_dec_add", "fn_var_dec", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    62,    63,    64,    64,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    66,    66,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    68,    68,    68,    68,    68,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    70,    70,    70,    70,    70,    71,
      71,    72,    72,    73,    73,    74,    74,    75,    75,    76,
      76,    77,    77
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     0,     2,     7,    11,     5,     5,     7,
       6,     8,     4,     9,     4,     5,     3,     2,     3,     2,
       2,     8,     8,     8,     8,     8,     8,     8,     8,     8,
       8,     8,     8,     0,     4,     1,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       3,     1,     1,     3,     5,     1,     4,     3,     4,     4,
       0,     3,    10,     6,     9,     7,     9,     7,     9,     7,
       8,     6,     9,     9,     9,     9,     9,     9,     9,     9,
       9,     9,     9,     9,     1,     1,     1,     1,     1,     1,
       3,     1,     1,     4,     3,     0,     3,     0,     2,     0,
       3,     0,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,    89,     0,     3,    84,     0,    88,     0,    85,    86,
      87,     0,     0,     0,     2,     3,     0,    92,    91,     0,
       0,     0,     0,     0,     0,    51,    55,    35,    36,    37,
       0,     0,    17,     0,     0,    52,    19,    20,     1,     4,
       0,     0,     0,    97,    33,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    89,    90,     0,
       0,    18,    60,     0,    49,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    16,     0,     0,    97,
       0,    57,     0,    95,     0,     0,     0,   101,     0,   101,
     101,   101,   101,   101,   101,   101,   101,   101,   101,   101,
     101,    56,     0,    89,     0,     0,     0,     0,    50,    97,
      42,    41,    47,    46,    45,    44,    39,    48,    43,    38,
      40,    53,     0,    14,    58,     0,    98,    59,    97,     0,
       0,     0,     0,    99,     0,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     3,   101,
       0,     7,    33,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    60,     0,    59,
      95,    15,     0,     8,     0,    33,    33,     0,   102,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    60,     0,     0,   101,    94,   101,
     101,   101,   101,   101,   101,   101,   101,   101,   101,   101,
     101,   101,   101,    61,    54,    96,     0,    97,     0,    34,
      99,     3,    10,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     5,     0,     0,    93,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     9,     0,   100,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     3,    60,    60,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,     3,     0,    71,    63,    97,
       3,    60,     3,    60,     3,    60,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,    13,     0,
      60,     0,     0,    65,     0,    69,     0,    67,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       6,    70,     0,    60,    60,    60,    60,    60,    60,    60,
      60,    60,    60,    60,    60,    60,    60,    60,    60,    64,
      68,    66,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    62
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    13,    14,    15,   198,    83,    16,   105,    17,    18,
      19,   133,   126,    84,   178,   134
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -79
static const yytype_int16 yypact[] =
{
     131,   115,   -12,   131,   -79,    23,   -79,   219,   -79,   -79,
     -79,    10,    49,    21,   -79,   131,    30,   -79,   -79,   361,
      51,   466,   466,    53,    58,   -79,    62,   -79,   -79,   -79,
     466,   466,   -79,   103,   529,   127,   -79,   -79,   -79,   -79,
     466,   102,   466,   466,    18,    81,    87,    89,    90,    91,
      97,    99,   101,   106,   108,   118,   119,   128,   -79,   551,
     571,   -79,    34,   591,   -79,   123,   466,   466,   466,   466,
     466,   466,   466,   466,   466,   466,   -79,   466,   466,   466,
     611,   -79,   633,   653,    92,   229,   265,   103,   125,   103,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   -79,   143,   -11,   161,   156,   470,   159,   -79,   466,
      -8,   -79,    93,    93,    93,    93,    -7,    93,   277,    -7,
     -79,   715,   116,   -79,   -79,   466,   -79,   160,   466,   673,
     183,   695,   190,   184,   165,   -79,   168,   170,   185,   186,
     187,   196,   197,   198,   199,   206,   207,   208,   131,   103,
     201,   -79,   133,   220,   222,   223,   224,   225,   227,   251,
     252,   253,   256,   258,   261,   262,   264,    34,   260,   -79,
     653,   -79,   267,   -79,   278,   279,     9,   103,   -79,   -17,
     286,   287,   288,   289,   292,   293,   294,   295,   296,   297,
     298,   318,   310,   309,    34,   466,   363,   103,   -79,   103,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   -79,   -79,   -79,   311,   466,   466,   -79,
     184,   131,   -79,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   322,   -10,   316,   715,   337,
     329,   331,   333,   340,   341,   346,   347,   348,   349,   350,
     351,   352,   356,   358,   359,   -79,   362,   -79,   366,   380,
     381,   383,   385,   387,   389,   390,   404,   405,   406,   407,
     408,   372,   131,    34,    34,   418,    14,    16,    17,   421,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   455,
     456,   445,   -79,   -79,   -79,   -79,   -79,   -79,   -79,   -79,
     -79,   -79,   -79,   -79,   -79,   131,   449,   -79,   -79,   466,
     131,    34,   131,    34,   131,    34,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   -79,   452,
      34,   454,   457,   -79,   458,   -79,   459,   -79,   461,   463,
     464,   465,   467,   468,   469,   471,   472,   473,   474,   489,
     -79,   -79,   453,    34,    34,    34,    34,    34,    34,    34,
      34,    34,    34,    34,    34,    34,    34,    34,    34,   -79,
     -79,   -79,   -79,   -79,   -79,   -79,   -79,   -79,   -79,   -79,
     -79,   -79,   -79,   -79,   -79
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -79,   -79,    32,   -79,   -42,   354,     2,   169,   -79,   475,
     -33,   100,   369,   -78,   299,     3
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -56
static const yytype_int16 yytable[] =
{
      65,   122,    88,   221,    67,    67,    20,    68,    69,    35,
     272,    22,   149,    36,    70,   222,    71,    72,    73,   195,
      75,    38,   273,    35,    35,    77,    77,    24,    85,   106,
     218,   168,    35,    35,   310,    23,   312,   314,   103,    86,
      40,    87,    35,    41,    35,    35,   311,    39,   313,   315,
     172,    42,    37,    43,   132,    57,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,    35,    35,
      35,    35,    35,    35,    35,    35,    35,    35,    62,    35,
      35,    35,    61,    21,     4,   104,     6,    35,    35,     8,
       9,    10,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,    89,    67,    81,    57,   -56,   -56,
      90,    35,    91,    92,    93,   -56,   132,   -56,    72,   -56,
      94,    75,    95,   127,    96,   -55,    77,    35,   -55,    97,
      35,    98,    20,   219,   106,     1,    21,    78,   -55,   256,
      41,    99,   100,   195,   132,    20,   109,   169,    42,     2,
      79,     3,   193,     4,   196,     6,   197,   135,     8,     9,
      10,   106,   107,   148,   132,   150,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,   132,   132,
     192,     4,     5,     6,     7,   151,     8,     9,    10,    11,
      12,   167,   171,   174,   176,   177,   179,    35,    35,   180,
     240,   181,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   182,   183,   184,    35,
      35,   194,    25,    26,    27,    28,    29,   185,   186,   187,
     188,   331,    25,    26,    27,    28,    29,   189,   190,   191,
     106,   106,    30,   199,    31,   200,   201,   202,   203,   128,
     204,    32,    30,   258,    31,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   107,    25,    26,
      27,    28,    29,    33,   205,   206,   207,   220,   106,   208,
     106,   209,   106,    33,   210,   211,    66,   212,    30,    67,
      31,   214,    68,    69,   107,   130,   216,   106,   217,    70,
     218,    71,    72,    73,   306,    75,   223,   224,   225,   226,
      77,    35,   227,   228,   229,   230,   231,   232,   233,    33,
     106,   106,   106,   106,   106,   106,   106,   106,   106,   106,
     106,   106,   106,   106,   106,   106,   213,   329,   234,   235,
     236,   271,   332,   255,   334,   274,   336,   275,   338,   339,
     340,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     276,    34,   277,   237,   278,    44,    25,    26,    27,    28,
      29,   279,   280,   107,   107,    59,    60,   281,   282,   283,
     284,   285,   286,   287,    63,    64,    30,   288,    31,   289,
     290,   291,   305,   239,    80,   292,    82,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,   293,
     294,   107,   295,   107,   296,   107,   297,    33,   298,   299,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     107,   120,   121,   300,   301,   302,   303,   304,   309,   129,
     131,   316,   307,   308,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,    25,
      26,    27,    28,    29,   152,   326,   327,   328,   330,   170,
     333,   350,   335,   352,   337,   368,   353,   354,   355,    30,
     356,    31,   357,   358,   359,    58,   360,   361,   362,   351,
     363,   364,   365,   366,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   367,   257,
      33,     0,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,    66,   215,
       0,    67,     0,     0,    68,    69,     0,     0,     0,   238,
     131,    70,     0,    71,    72,    73,    74,    75,     0,     0,
      66,    76,    77,    67,     0,     0,    68,    69,     0,     0,
       0,     0,   131,    70,     0,    71,    72,    73,    74,    75,
      66,   101,     0,    67,    77,     0,    68,    69,     0,     0,
       0,     0,     0,    70,     0,    71,    72,    73,    74,    75,
      66,     0,   102,    67,    77,     0,    68,    69,     0,     0,
       0,     0,     0,    70,     0,    71,    72,    73,    74,    75,
      66,     0,   108,    67,    77,     0,    68,    69,     0,     0,
       0,     0,     0,    70,     0,    71,    72,    73,    74,    75,
       0,     0,    66,   123,    77,    67,     0,     0,    68,    69,
       0,     0,     0,     0,     0,    70,     0,    71,    72,    73,
      74,    75,    66,   124,   125,    67,    77,     0,    68,    69,
       0,     0,     0,     0,     0,    70,     0,    71,    72,    73,
      74,    75,    66,     0,     0,    67,    77,     0,    68,    69,
       0,     0,     0,     0,     0,    70,     0,    71,    72,    73,
      74,    75,     0,     0,    66,   173,    77,    67,     0,     0,
      68,    69,     0,     0,     0,     0,     0,    70,     0,    71,
      72,    73,    74,    75,    66,   175,     0,    67,    77,     0,
      68,    69,     0,     0,     0,     0,     0,    70,     0,    71,
      72,    73,    74,    75,     0,     0,     0,     0,    77
};

static const yytype_int16 yycheck[] =
{
      33,    79,    44,    20,    12,    12,    17,    15,    16,     7,
      20,    23,    23,     3,    22,    32,    24,    25,    26,    10,
      28,     0,    32,    21,    22,    33,    33,     4,    10,    62,
      21,   109,    30,    31,    20,     3,    20,    20,     4,    21,
      10,    23,    40,    13,    42,    43,    32,    15,    32,    32,
     128,    21,     3,    23,    87,     4,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    20,    77,
      78,    79,    29,    21,    50,    51,    52,    85,    86,    55,
      56,    57,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,    23,    12,     4,     4,    15,    16,
      23,   109,    23,    23,    23,    22,   149,    24,    25,    26,
      23,    28,    23,    31,    23,    10,    33,   125,    13,    23,
     128,    23,    17,   175,   167,     4,    21,    10,    23,   217,
      13,    23,    23,    10,   177,    17,    23,    31,    21,    18,
      23,    20,   149,    50,    21,    52,    23,    32,    55,    56,
      57,   194,    62,    20,   197,     4,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     148,    50,    51,    52,    53,    29,    55,    56,    57,    58,
      59,    32,    32,    10,     4,    11,    31,   195,   196,    31,
     197,    31,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,    31,    31,    31,   217,
     218,    20,     3,     4,     5,     6,     7,    31,    31,    31,
      31,   309,     3,     4,     5,     6,     7,    31,    31,    31,
     273,   274,    23,    23,    25,    23,    23,    23,    23,    20,
      23,    32,    23,   221,    25,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   167,     3,     4,
       5,     6,     7,    54,    23,    23,    23,   177,   311,    23,
     313,    23,   315,    54,    23,    23,     9,    23,    23,    12,
      25,    31,    15,    16,   194,    30,    29,   330,    20,    22,
      21,    24,    25,    26,   272,    28,    20,    20,    20,    20,
      33,   309,    20,    20,    20,    20,    20,    20,    20,    54,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   167,   305,    20,    29,
      31,    19,   310,    32,   312,    29,   314,    10,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
      31,     7,    31,   194,    31,     4,     3,     4,     5,     6,
       7,    31,    31,   273,   274,    21,    22,    31,    31,    31,
      31,    31,    31,    31,    30,    31,    23,    31,    25,    31,
      31,    29,    20,    30,    40,    29,    42,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    29,
      29,   311,    29,   313,    29,   315,    29,    54,    29,    29,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
     330,    77,    78,    29,    29,    29,    29,    29,    20,    85,
      86,    20,   273,   274,    20,    20,    20,    20,    20,    20,
      20,    20,    20,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,     3,
       4,     5,     6,     7,     4,    20,    20,    32,    29,   125,
     311,    29,   313,    29,   315,    32,    29,    29,    29,    23,
      29,    25,    29,    29,    29,    20,    29,    29,    29,   330,
      29,    29,    29,    29,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    29,   220,
      54,    -1,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,     9,   170,
      -1,    12,    -1,    -1,    15,    16,    -1,    -1,    -1,   195,
     196,    22,    -1,    24,    25,    26,    27,    28,    -1,    -1,
       9,    32,    33,    12,    -1,    -1,    15,    16,    -1,    -1,
      -1,    -1,   218,    22,    -1,    24,    25,    26,    27,    28,
       9,    30,    -1,    12,    33,    -1,    15,    16,    -1,    -1,
      -1,    -1,    -1,    22,    -1,    24,    25,    26,    27,    28,
       9,    -1,    31,    12,    33,    -1,    15,    16,    -1,    -1,
      -1,    -1,    -1,    22,    -1,    24,    25,    26,    27,    28,
       9,    -1,    31,    12,    33,    -1,    15,    16,    -1,    -1,
      -1,    -1,    -1,    22,    -1,    24,    25,    26,    27,    28,
      -1,    -1,     9,    32,    33,    12,    -1,    -1,    15,    16,
      -1,    -1,    -1,    -1,    -1,    22,    -1,    24,    25,    26,
      27,    28,     9,    30,    11,    12,    33,    -1,    15,    16,
      -1,    -1,    -1,    -1,    -1,    22,    -1,    24,    25,    26,
      27,    28,     9,    -1,    -1,    12,    33,    -1,    15,    16,
      -1,    -1,    -1,    -1,    -1,    22,    -1,    24,    25,    26,
      27,    28,    -1,    -1,     9,    32,    33,    12,    -1,    -1,
      15,    16,    -1,    -1,    -1,    -1,    -1,    22,    -1,    24,
      25,    26,    27,    28,     9,    30,    -1,    12,    33,    -1,
      15,    16,    -1,    -1,    -1,    -1,    -1,    22,    -1,    24,
      25,    26,    27,    28,    -1,    -1,    -1,    -1,    33
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     4,    18,    20,    50,    51,    52,    53,    55,    56,
      57,    58,    59,    63,    64,    65,    68,    70,    71,    72,
      17,    21,    23,    64,     4,     3,     4,     5,     6,     7,
      23,    25,    32,    54,    67,    68,     3,     3,     0,    64,
      10,    13,    21,    23,     4,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,     4,    71,    67,
      67,    29,    20,    67,    67,    72,     9,    12,    15,    16,
      22,    24,    25,    26,    27,    28,    32,    33,    10,    23,
      67,     4,    67,    67,    75,    10,    21,    23,    66,    23,
      23,    23,    23,    23,    23,    23,    23,    23,    23,    23,
      23,    30,    31,     4,    51,    69,    72,    73,    31,    23,
      67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    75,    32,    30,    11,    74,    31,    20,    67,
      30,    67,    72,    73,    77,    32,    77,    77,    77,    77,
      77,    77,    77,    77,    77,    77,    77,    77,    20,    23,
       4,    29,     4,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    32,    75,    31,
      67,    32,    75,    32,    10,    30,     4,    11,    76,    31,
      31,    31,    31,    31,    31,    31,    31,    31,    31,    31,
      31,    31,    64,    77,    20,    10,    21,    23,    66,    23,
      23,    23,    23,    23,    23,    23,    23,    23,    23,    23,
      23,    23,    23,    69,    31,    74,    29,    20,    21,    66,
      73,    20,    32,    20,    20,    20,    20,    20,    20,    20,
      20,    20,    20,    20,    20,    29,    31,    69,    67,    30,
      77,    77,    77,    77,    77,    77,    77,    77,    77,    77,
      77,    77,    77,    77,    77,    32,    75,    76,    64,    64,
      64,    64,    64,    64,    64,    64,    64,    64,    64,    64,
      64,    19,    20,    32,    29,    10,    31,    31,    31,    31,
      31,    31,    31,    31,    31,    31,    31,    31,    31,    31,
      31,    29,    29,    29,    29,    29,    29,    29,    29,    29,
      29,    29,    29,    29,    29,    20,    64,    69,    69,    20,
      20,    32,    20,    32,    20,    32,    20,    20,    20,    20,
      20,    20,    20,    20,    20,    20,    20,    20,    32,    64,
      29,    75,    64,    69,    64,    69,    64,    69,    64,    64,
      64,    64,    64,    64,    64,    64,    64,    64,    64,    64,
      29,    69,    29,    29,    29,    29,    29,    29,    29,    29,
      29,    29,    29,    29,    29,    29,    29,    29,    32,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (&yylloc, tp, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, &yylloc, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, &yylloc, tp)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location, tp); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, mvv::parser::ParserContext& tp)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, tp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
    mvv::parser::ParserContext& tp;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
  YYUSE (tp);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, mvv::parser::ParserContext& tp)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp, tp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
    mvv::parser::ParserContext& tp;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, tp);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, mvv::parser::ParserContext& tp)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule, tp)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int yyrule;
    mvv::parser::ParserContext& tp;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       , tp);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule, tp); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, mvv::parser::ParserContext& tp)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp, tp)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
    mvv::parser::ParserContext& tp;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (tp);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {
      case 3: /* "\"string\"" */

/* Line 1000 of yacc.c  */
#line 127 "parser.yy"
	{ delete (yyvaluep->str); };

/* Line 1000 of yacc.c  */
#line 1567 "parser.tab.cc"
	break;
      case 8: /* "\"symbol\"" */

/* Line 1000 of yacc.c  */
#line 128 "parser.yy"
	{ delete (*yyvaluep).symbol; };

/* Line 1000 of yacc.c  */
#line 1576 "parser.tab.cc"
	break;

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (mvv::parser::ParserContext& tp);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */





/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (mvv::parser::ParserContext& tp)
#else
int
yyparse (tp)
    mvv::parser::ParserContext& tp;
#endif
#endif
{
/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Location data for the lookahead symbol.  */
YYLTYPE yylloc;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.
       `yyls': related to locations.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[2];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;

#if YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 1;
#endif

/* User initialization code.  */

/* Line 1242 of yacc.c  */
#line 79 "parser.yy"
{
	yydebug = tp._parse_trace_p;
   /**
    setup the filename each time before parsing
    */
   static int mvvParserInputNumber = 0;
   yylloc.filename = mvv::Symbol::create( tp._filename == "" ? "(input" + nll::core::val2str( mvvParserInputNumber++ ) + ")" : tp._filename );
}

/* Line 1242 of yacc.c  */
#line 1736 "parser.tab.cc"
  yylsp[0] = yylloc;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);

	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
	YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1455 of yacc.c  */
#line 210 "parser.yy"
    { tp._root = (yyvsp[(1) - (1)].astStatements); ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 212 "parser.yy"
    { (yyval.astStatements) = new mvv::parser::AstStatements( (yyloc) ); ;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 213 "parser.yy"
    { (yyval.astStatements) = (yyvsp[(2) - (2)].astStatements); (yyvsp[(2) - (2)].astStatements)->insert( (yyvsp[(1) - (2)].ast) ); ;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 215 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstIf( (yyloc), (yyvsp[(3) - (7)].astExp), (yyvsp[(6) - (7)].astStatements), 0 ); ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 216 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstIf( (yyloc), (yyvsp[(3) - (11)].astExp), (yyvsp[(6) - (11)].astStatements), (yyvsp[(10) - (11)].astStatements) ); ;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 217 "parser.yy"
    { mvv::parser::AstDeclClass* decl = new mvv::parser::AstDeclClass( (yyloc), *(yyvsp[(2) - (5)].symbol), (yyvsp[(4) - (5)].astDecls) ); (yyval.ast) = decl; linkFunctionToClass( *decl ); ;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 223 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (5)].astTypeT), *(yyvsp[(2) - (5)].symbol), (yyvsp[(4) - (5)].astExp) ); ;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 224 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (7)].astTypeT), *(yyvsp[(2) - (7)].symbol), 0, (yyvsp[(5) - (7)].astArgs) ); ;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 225 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (6)].astTypeT), *(yyvsp[(2) - (6)].symbol), (yyvsp[(4) - (6)].astDeclVars) ); ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 226 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (8)].astTypeT), *(yyvsp[(2) - (8)].symbol), (yyvsp[(4) - (8)].astDeclVars), (yyvsp[(7) - (8)].astStatements) ); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 227 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (4)].astTypeT), *(yyvsp[(2) - (4)].symbol) );
                                                                       if ( (yyvsp[(3) - (4)].arrayDim)->size() )
                                                                       {
                                                                          (yyvsp[(1) - (4)].astTypeT)->setArray( true );
                                                                          (yyvsp[(1) - (4)].astTypeT)->setSize( (yyvsp[(3) - (4)].arrayDim) );
                                                                       } 
                                                                     ;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 235 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (9)].astTypeT), *(yyvsp[(2) - (9)].symbol), 0, (yyvsp[(7) - (9)].astArgs) ); (yyvsp[(1) - (9)].astTypeT)->setArray( true ); /* we don't handle several dimensions*/ ;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 236 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstExpAssign( (yyloc), (yyvsp[(1) - (4)].astVar), (yyvsp[(3) - (4)].astExp) ); ;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 237 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstExpCall( (yyloc), (yyvsp[(1) - (5)].astVar), (yyvsp[(3) - (5)].astArgs) ); ;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 238 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstReturn( (yyloc), (yyvsp[(2) - (3)].astExp) ); ;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 239 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstReturn( (yyloc) ); ;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 240 "parser.yy"
    { (yyval.ast) = (yyvsp[(2) - (3)].astStatements); ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 241 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstImport( (yyloc), *(yyvsp[(2) - (2)].str) ); ;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 242 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstInclude( (yyloc), *(yyvsp[(2) - (2)].str) ); ;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 245 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (8)].astTypeT), mvv::Symbol::create("operator+"), (yyvsp[(4) - (8)].astDeclVars), (yyvsp[(7) - (8)].astStatements) ); ;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 246 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (8)].astTypeT), mvv::Symbol::create("operator-"), (yyvsp[(4) - (8)].astDeclVars), (yyvsp[(7) - (8)].astStatements) ); ;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 247 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (8)].astTypeT), mvv::Symbol::create("operator*"), (yyvsp[(4) - (8)].astDeclVars), (yyvsp[(7) - (8)].astStatements) ); ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 248 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (8)].astTypeT), mvv::Symbol::create("operator/"), (yyvsp[(4) - (8)].astDeclVars), (yyvsp[(7) - (8)].astStatements) ); ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 249 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (8)].astTypeT), mvv::Symbol::create("operator<"), (yyvsp[(4) - (8)].astDeclVars), (yyvsp[(7) - (8)].astStatements) ); ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 250 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (8)].astTypeT), mvv::Symbol::create("operator>"), (yyvsp[(4) - (8)].astDeclVars), (yyvsp[(7) - (8)].astStatements) ); ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 251 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (8)].astTypeT), mvv::Symbol::create("operator<="), (yyvsp[(4) - (8)].astDeclVars), (yyvsp[(7) - (8)].astStatements) ); ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 252 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (8)].astTypeT), mvv::Symbol::create("operator>="), (yyvsp[(4) - (8)].astDeclVars), (yyvsp[(7) - (8)].astStatements) ); ;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 253 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (8)].astTypeT), mvv::Symbol::create("operator=="), (yyvsp[(4) - (8)].astDeclVars), (yyvsp[(7) - (8)].astStatements) ); ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 254 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (8)].astTypeT), mvv::Symbol::create("operator!="), (yyvsp[(4) - (8)].astDeclVars), (yyvsp[(7) - (8)].astStatements) ); ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 255 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (8)].astTypeT), mvv::Symbol::create("operator&&"), (yyvsp[(4) - (8)].astDeclVars), (yyvsp[(7) - (8)].astStatements) ); ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 256 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (8)].astTypeT), mvv::Symbol::create("operator||"), (yyvsp[(4) - (8)].astDeclVars), (yyvsp[(7) - (8)].astStatements) ); ;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 259 "parser.yy"
    { (yyval.arrayDim) = new std::vector<mvv::parser::AstExp*>(); ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 260 "parser.yy"
    { (yyval.arrayDim) = (yyvsp[(4) - (4)].arrayDim); (yyval.arrayDim)->push_back( (yyvsp[(2) - (4)].astExp) ); ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 264 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstInt( (yyloc), (yyvsp[(1) - (1)].ival) ); ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 265 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstFloat( (yyloc), (yyvsp[(1) - (1)].fval) ); ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 266 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstNil( (yyloc) ); ;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 267 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::PLUS ); ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 268 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::MINUS ); ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 269 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::TIMES ); ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 270 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::DIVIDE ); ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 271 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::AND ); ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 272 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::OR ); ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 273 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::LT ); ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 274 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::LE ); ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 275 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::GT ); ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 276 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::GE ); ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 277 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::NE ); ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 278 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), new mvv::parser::AstInt( (yyloc), 0 ) , (yyvsp[(2) - (2)].astExp), mvv::parser::AstOpBin::MINUS ); ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 279 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstExpSeq( (yyloc), (yyvsp[(2) - (3)].astExp) ); ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 280 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstString( (yyloc), *(yyvsp[(1) - (1)].str) ); ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 281 "parser.yy"
    { (yyval.astExp) = (yyvsp[(1) - (1)].astVar); ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 282 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstExpAssign( (yyloc), (yyvsp[(1) - (3)].astVar), (yyvsp[(3) - (3)].astExp) ); ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 283 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstExpTypename( (yyloc), (yyvsp[(2) - (5)].astTypeT), (yyvsp[(4) - (5)].astArgs) );;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 287 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarSimple( (yyloc), *(yyvsp[(1) - (1)].symbol), true ); ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 288 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarArray( (yyloc), new mvv::parser::AstVarSimple( (yyloc), *(yyvsp[(1) - (4)].symbol), true ), (yyvsp[(3) - (4)].astExp) ); ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 289 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarField( (yyloc), (yyvsp[(1) - (3)].astVar), *(yyvsp[(3) - (3)].symbol) ); ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 290 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarArray( (yyloc), (yyvsp[(1) - (4)].astVar), (yyvsp[(3) - (4)].astExp) ); ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 291 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstExpCall( (yyloc), (yyvsp[(1) - (4)].astVar), (yyvsp[(3) - (4)].astArgs) ); ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 295 "parser.yy"
    { (yyval.astDecls) = new mvv::parser::AstDecls( (yyloc) ); ;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 296 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(3) - (3)].astDecls); (yyval.astDecls)->insert( (yyvsp[(1) - (3)].astDeclVar) ); ;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 297 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(10) - (10)].astDecls); mvv::parser::AstDeclVar* var = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (10)].astTypeT), *(yyvsp[(2) - (10)].symbol), 0, (yyvsp[(7) - (10)].astArgs) ); (yyvsp[(1) - (10)].astTypeT)->setArray( true ); (yyval.astDecls)->insert( var ); ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 298 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(6) - (6)].astDecls); mvv::parser::AstDeclClass* decl = new mvv::parser::AstDeclClass( (yyloc), *(yyvsp[(2) - (6)].symbol), (yyvsp[(4) - (6)].astDecls) ); (yyval.astDecls)->insert( decl ); linkFunctionToClass( *decl ); ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 299 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), *(yyvsp[(2) - (9)].symbol), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 300 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(7) - (7)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (7)].astTypeT), *(yyvsp[(2) - (7)].symbol), (yyvsp[(4) - (7)].astDeclVars) ) ); ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 301 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), mvv::Symbol::create("operator[]"), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 302 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(7) - (7)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (7)].astTypeT), mvv::Symbol::create("operator[]"), (yyvsp[(4) - (7)].astDeclVars) ) ); ;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 303 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), mvv::Symbol::create("operator()"), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 304 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(7) - (7)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (7)].astTypeT), mvv::Symbol::create("operator()"), (yyvsp[(4) - (7)].astDeclVars) ) ); ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 305 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(8) - (8)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), 0, *(yyvsp[(1) - (8)].symbol), (yyvsp[(3) - (8)].astDeclVars), (yyvsp[(6) - (8)].astStatements) ) ); ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 306 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(6) - (6)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), 0, *(yyvsp[(1) - (6)].symbol), (yyvsp[(3) - (6)].astDeclVars) ) ); ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 308 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), mvv::Symbol::create("operator+"), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 309 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), mvv::Symbol::create("operator-"), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 310 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), mvv::Symbol::create("operator*"), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 311 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), mvv::Symbol::create("operator/"), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 312 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), mvv::Symbol::create("operator<"), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 313 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), mvv::Symbol::create("operator>"), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 314 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), mvv::Symbol::create("operator<="), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 315 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), mvv::Symbol::create("operator>="), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 316 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), mvv::Symbol::create("operator=="), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 317 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), mvv::Symbol::create("operator!="), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 318 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), mvv::Symbol::create("operator&&"), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 319 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), mvv::Symbol::create("operator||"), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 322 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::VAR ); ;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 323 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::INT ); ;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 324 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::FLOAT );;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 325 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::STRING ); ;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 326 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::VOID );;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 328 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::SYMBOL, (yyvsp[(1) - (1)].symbol) ); ;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 329 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstTypeField( (yyloc), (yyvsp[(3) - (3)].astTypeT), *(yyvsp[(1) - (3)].symbol) ); ;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 331 "parser.yy"
    { (yyval.astTypeT) = (yyvsp[(1) - (1)].astTypeT); ;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 332 "parser.yy"
    { (yyval.astTypeT) = (yyvsp[(1) - (1)].astTypeT); ;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 335 "parser.yy"
    { (yyval.astDeclVar) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (4)].astTypeT), *(yyvsp[(2) - (4)].symbol), (yyvsp[(4) - (4)].astExp) ); ;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 336 "parser.yy"
    { (yyval.astDeclVar) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (3)].astTypeT), *(yyvsp[(2) - (3)].symbol) );
                                        if ( (yyvsp[(3) - (3)].arrayDim)->size() )
                                        {
                                           (yyvsp[(1) - (3)].astTypeT)->setArray( true );
                                           (yyvsp[(1) - (3)].astTypeT)->setSize( (yyvsp[(3) - (3)].arrayDim) );
                                        } 
                                      ;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 345 "parser.yy"
    { (yyval.astArgs) = new mvv::parser::AstArgs( (yyloc) ); ;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 346 "parser.yy"
    { (yyval.astArgs) = (yyvsp[(3) - (3)].astArgs); (yyval.astArgs)->insert( (yyvsp[(2) - (3)].astExp) ); ;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 348 "parser.yy"
    { (yyval.astArgs) = new mvv::parser::AstArgs( (yyloc) ); ;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 349 "parser.yy"
    { (yyval.astArgs) = (yyvsp[(2) - (2)].astArgs); (yyval.astArgs)->insert( (yyvsp[(1) - (2)].astExp) ); ;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 351 "parser.yy"
    { (yyval.astDeclVars) = new mvv::parser::AstDeclVars( (yyloc) ); ;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 352 "parser.yy"
    { (yyval.astDeclVars) = (yyvsp[(3) - (3)].astDeclVars); (yyval.astDeclVars)->insert( (yyvsp[(2) - (3)].astDeclVar) ); ;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 354 "parser.yy"
    { (yyval.astDeclVars) = new mvv::parser::AstDeclVars( (yyloc) ); ;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 355 "parser.yy"
    { (yyval.astDeclVars) = (yyvsp[(2) - (2)].astDeclVars); (yyval.astDeclVars)->insert( (yyvsp[(1) - (2)].astDeclVar) ); ;}
    break;



/* Line 1455 of yacc.c  */
#line 2643 "parser.tab.cc"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, tp, YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (&yylloc, tp, yymsg);
	  }
	else
	  {
	    yyerror (&yylloc, tp, YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }

  yyerror_range[0] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, &yylloc, tp);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[0] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[0] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp, tp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, tp, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, &yylloc, tp);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp, tp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 357 "parser.yy"


