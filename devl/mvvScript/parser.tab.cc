
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
   //   bison parser.yy -d
   //
   
   /**
    Notes:
    
    - Test()			is a rvalue
    - val fn(args);		so we can import a function
    - var[] array;		can contain any time of classes, type is directly known...
    - typename Test::Test2(); we need to be less accurate on the type so we can Test::test.a
    - namespace, static, public: namespace = class, default is public, static, can be in several files
    - a name of function/variable/class must be unique
    - operator +, = -, /, *, [], () have special meanings if not a primitive...
    - prmitive int, float, string
    - delc: int a[][][] = { {}{}{}... } not handled ->int a[] = {x, x, x...}, but not int a[5] = {...} => we should not give size, quite difficult to parse, not very useful...
    */
   
   #include <string>
   #include <iostream>
   #include <list>
   #include <sstream>
   #include <nll/nll.h>
   #include "parser-context.h"
   #include "ast-files.h"


/* Line 189 of yacc.c  */
#line 109 "parser.tab.cc"

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
     TYPENAME = 297,
     INT_T = 298,
     FLOAT_T = 299,
     STRING_T = 300,
     IMPORT = 301,
     INCLUDE = 302,
     IFX = 303,
     UMINUS = 304
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 57 "parser.yy"

   // Tokens.
   int                        ival;
   float                      fval;
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
#line 214 "parser.tab.cc"
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
#line 239 "parser.tab.cc"

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
#define YYFINAL  36
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   510

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  50
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  16
/* YYNRULES -- Number of rules.  */
#define YYNRULES  77
/* YYNRULES -- Number of states.  */
#define YYNSTATES  204

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   304

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
      45,    46,    47,    48,    49
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     6,     9,    17,    29,    35,    41,
      49,    56,    65,    70,    80,    85,    91,    95,    98,   102,
     105,   108,   109,   114,   116,   118,   122,   126,   130,   134,
     138,   142,   146,   150,   154,   158,   162,   165,   169,   171,
     173,   177,   183,   185,   190,   194,   199,   204,   205,   209,
     220,   227,   237,   245,   255,   263,   273,   281,   290,   297,
     299,   301,   303,   305,   307,   309,   313,   315,   317,   322,
     326,   327,   331,   332,   335,   336,   340,   341
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      51,     0,    -1,    52,    -1,    -1,    53,    52,    -1,    17,
      22,    55,    30,    19,    52,    28,    -1,    17,    22,    55,
      30,    19,    52,    28,    18,    19,    52,    28,    -1,    38,
       4,    19,    57,    28,    -1,    60,     4,     9,    55,    31,
      -1,    60,     4,     9,    19,    63,    28,    31,    -1,    60,
       4,    22,    65,    30,    31,    -1,    60,     4,    22,    65,
      30,    19,    52,    28,    -1,    60,     4,    54,    31,    -1,
      60,     4,    20,    29,     9,    19,    63,    28,    31,    -1,
      56,     9,    55,    31,    -1,    56,    22,    63,    30,    31,
      -1,    41,    55,    31,    -1,    41,    31,    -1,    19,    52,
      28,    -1,    46,     3,    -1,    47,     3,    -1,    -1,    20,
      55,    29,    54,    -1,     5,    -1,     6,    -1,    55,    27,
      55,    -1,    55,    24,    55,    -1,    55,    32,    55,    -1,
      55,    11,    55,    -1,    55,     8,    55,    -1,    55,    26,
      55,    -1,    55,    23,    55,    -1,    55,    21,    55,    -1,
      55,    15,    55,    -1,    55,    14,    55,    -1,    55,    25,
      55,    -1,    24,    55,    -1,    22,    55,    30,    -1,     3,
      -1,    56,    -1,    56,     9,    55,    -1,    42,    60,    22,
      63,    30,    -1,     4,    -1,     4,    20,    55,    29,    -1,
      56,    12,     4,    -1,    56,    20,    55,    29,    -1,    56,
      22,    63,    30,    -1,    -1,    61,    31,    57,    -1,    60,
       4,    20,    29,     9,    19,    63,    28,    31,    57,    -1,
      38,     4,    19,    57,    28,    57,    -1,    60,     4,    22,
      65,    30,    19,    52,    28,    57,    -1,    60,     4,    22,
      65,    30,    31,    57,    -1,    60,    34,    22,    65,    30,
      19,    52,    28,    57,    -1,    60,    34,    22,    65,    30,
      31,    57,    -1,    60,    33,    22,    65,    30,    19,    52,
      28,    57,    -1,    60,    33,    22,    65,    30,    31,    57,
      -1,     4,    22,    65,    30,    19,    52,    28,    57,    -1,
       4,    22,    65,    30,    31,    57,    -1,    37,    -1,    43,
      -1,    44,    -1,    45,    -1,    39,    -1,     4,    -1,    59,
      16,     4,    -1,    59,    -1,    58,    -1,    60,     4,     9,
      55,    -1,    60,     4,    54,    -1,    -1,    10,    55,    62,
      -1,    -1,    55,    62,    -1,    -1,    10,    61,    64,    -1,
      -1,    61,    64,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   163,   163,   165,   166,   168,   169,   170,   176,   177,
     178,   179,   180,   188,   189,   190,   191,   192,   193,   194,
     195,   198,   199,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   225,   226,   227,   228,   229,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   246,
     247,   248,   249,   250,   252,   253,   255,   256,   259,   260,
     269,   270,   272,   273,   275,   276,   278,   279
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "$undefined", "\"string\"",
  "\"identifier\"", "\"integer\"", "\"float\"", "\"symbol\"", "\"&&\"",
  "\"=\"", "\",\"", "\"/\"", "\".\"", "\"==\"", "\">=\"", "\">\"",
  "\"::\"", "\"if\"", "\"else\"", "\"{\"", "\"[\"", "\"<=\"", "\"(\"",
  "\"<\"", "\"-\"", "\"!=\"", "\"||\"", "\"+\"", "\"}\"", "\"]\"", "\")\"",
  "\";\"", "\"*\"", "\"operator()\"", "\"operator[]\"", "\"for\"",
  "\"in\"", "\"var\"", "\"class\"", "\"void\"", "\"NULL\"", "\"return\"",
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
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    50,    51,    52,    52,    53,    53,    53,    53,    53,
      53,    53,    53,    53,    53,    53,    53,    53,    53,    53,
      53,    54,    54,    55,    55,    55,    55,    55,    55,    55,
      55,    55,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    55,    56,    56,    56,    56,    56,    57,    57,    57,
      57,    57,    57,    57,    57,    57,    57,    57,    57,    58,
      58,    58,    58,    58,    59,    59,    60,    60,    61,    61,
      62,    62,    63,    63,    64,    64,    65,    65
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     0,     2,     7,    11,     5,     5,     7,
       6,     8,     4,     9,     4,     5,     3,     2,     3,     2,
       2,     0,     4,     1,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     3,     1,     1,
       3,     5,     1,     4,     3,     4,     4,     0,     3,    10,
       6,     9,     7,     9,     7,     9,     7,     8,     6,     1,
       1,     1,     1,     1,     1,     3,     1,     1,     4,     3,
       0,     3,     0,     2,     0,     3,     0,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,    42,     0,     3,    59,     0,    63,     0,    60,    61,
      62,     0,     0,     0,     2,     3,     0,    67,    66,     0,
       0,     0,     0,     0,    38,    42,    23,    24,     0,     0,
      17,     0,     0,    39,    19,    20,     1,     4,     0,     0,
       0,    72,     0,    21,     0,     0,    18,    47,     0,    36,
      64,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    16,     0,     0,    72,     0,    44,     0,    70,
       0,    65,     0,     0,    76,     0,    43,     0,    64,     0,
       0,     0,     0,    37,    72,    29,    28,    34,    33,    32,
      31,    26,    35,    30,    25,    27,    40,     0,    14,    45,
       0,    73,    46,    72,     0,     0,     0,     0,    74,     0,
      12,     3,    76,     0,     7,    21,     0,     0,    47,     0,
      46,    70,    15,     0,     8,     0,    21,    21,     0,    77,
       0,     0,     0,    47,     0,     0,    76,    69,    76,    76,
      48,    41,    71,     0,    72,     0,    22,    74,     3,    10,
       5,     0,     0,    68,     0,     0,     0,     0,     9,     0,
      75,     0,     0,     3,    47,    47,     0,     0,     0,     0,
       0,    11,     3,     0,    58,    50,    72,     3,    47,     3,
      47,     3,    47,    13,     0,    47,     0,     0,    52,     0,
      56,     0,    54,     6,    57,     0,    47,    47,    47,    47,
      51,    55,    53,    49
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    13,    14,    15,   137,    69,    33,    80,    17,    18,
      81,    82,   101,    70,   129,   109
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -90
static const yytype_int16 yypact[] =
{
     167,     7,   -12,   167,   -90,    12,   -90,   119,   -90,   -90,
     -90,    11,    35,    39,   -90,   167,   125,   -90,    30,    55,
     152,   152,    34,    44,   -90,    57,   -90,   -90,   152,   152,
     -90,    43,   242,   140,   -90,   -90,   -90,   -90,   152,    60,
     152,   152,    88,    59,   264,   284,   -90,    28,   304,   -90,
     -90,    76,   152,   152,   152,   152,   152,   152,   152,   152,
     152,   152,   -90,   152,   152,   152,   324,   -90,   346,   366,
      69,   -90,   212,   216,    43,    75,   -90,    94,    78,   110,
      87,     8,    89,   -90,   152,   463,   -90,   478,   478,   478,
     478,    -4,   478,   448,    -4,   -90,   428,    91,   -90,   -90,
     152,   -90,    95,   152,   386,   109,   408,   115,   117,    99,
     -90,   167,    43,   114,   -90,    81,   113,   118,    28,   116,
     -90,   366,   -90,   123,   -90,   134,   124,    10,    43,   -90,
     -18,   131,   135,    28,   152,   220,    43,   -90,    43,    43,
     -90,   -90,   -90,   133,   152,   152,   -90,   117,   167,   -90,
     150,   -11,   142,   428,   164,   145,   158,   159,   -90,   169,
     -90,   171,   161,   167,    28,    28,   176,    14,    17,    64,
     170,   -90,   167,   174,   -90,   -90,   152,   167,    28,   167,
      28,   167,    28,   -90,   175,    28,   179,   181,   -90,   199,
     -90,   200,   -90,   -90,   -90,   198,    28,    28,    28,    28,
     -90,   -90,   -90,   -90
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -90,   -90,    19,   -90,   -41,    -3,     6,   -89,   -90,   -90,
       0,   -34,   111,   -60,    83,    54
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -65
static const yytype_int16 yytable[] =
{
      19,   148,    75,    19,    32,    97,    16,    53,   163,    16,
      21,   -64,   115,   149,    34,    19,    23,    44,    45,   134,
     164,    16,    22,   -64,   119,    48,    49,    20,    63,   140,
     145,    51,    78,   177,    37,    66,   179,    68,    35,    36,
     108,   116,   117,   123,   152,   178,    42,    50,   180,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    43,
      95,    96,    46,    47,    67,     4,    79,     6,    72,   104,
     106,     8,     9,    10,   107,   174,   175,    20,   108,    73,
       4,    74,     6,   181,   159,   146,     8,     9,    10,   188,
     134,   190,    71,   192,   147,   182,   194,   121,    84,   102,
     112,   135,   108,   136,   108,   108,   110,   200,   201,   202,
     203,    19,   107,   111,   113,   114,   186,    16,   125,   127,
     118,   120,    24,    25,    26,    27,   122,   128,   107,   130,
     131,   153,   106,   133,    38,   138,   107,    39,   107,   107,
     139,    28,   106,    29,   145,    40,   141,    41,    19,    64,
      30,   143,    39,   144,    16,    24,    25,    26,    27,   150,
      40,    31,    65,    19,   158,   151,   132,   161,   162,    16,
     165,     1,    19,   166,    28,   167,    29,    19,    16,    19,
     172,    19,   173,    16,     2,    16,     3,    16,   168,   169,
     155,   184,   156,   157,    31,   176,   187,   170,   189,   171,
     191,   183,   185,   193,     4,     5,     6,   195,     7,   196,
       8,     9,    10,    11,    12,    24,    25,    26,    27,    24,
      25,    26,    27,    24,    25,    26,    27,   197,   198,   199,
     160,   103,   142,     0,    28,     0,    29,     0,    28,     0,
      29,     0,    28,     0,    29,   105,     0,     0,     0,   154,
      52,     0,     0,    53,    31,     0,    54,    55,    31,     0,
       0,     0,    31,    56,     0,    57,    58,    59,    60,    61,
       0,     0,    52,    62,    63,    53,     0,     0,    54,    55,
       0,     0,     0,     0,     0,    56,     0,    57,    58,    59,
      60,    61,    52,    76,     0,    53,    63,     0,    54,    55,
       0,     0,     0,     0,     0,    56,     0,    57,    58,    59,
      60,    61,    52,     0,    77,    53,    63,     0,    54,    55,
       0,     0,     0,     0,     0,    56,     0,    57,    58,    59,
      60,    61,    52,     0,    83,    53,    63,     0,    54,    55,
       0,     0,     0,     0,     0,    56,     0,    57,    58,    59,
      60,    61,     0,     0,    52,    98,    63,    53,     0,     0,
      54,    55,     0,     0,     0,     0,     0,    56,     0,    57,
      58,    59,    60,    61,    52,    99,   100,    53,    63,     0,
      54,    55,     0,     0,     0,     0,     0,    56,     0,    57,
      58,    59,    60,    61,    52,     0,     0,    53,    63,     0,
      54,    55,     0,     0,     0,     0,     0,    56,     0,    57,
      58,    59,    60,    61,     0,     0,    52,   124,    63,    53,
       0,     0,    54,    55,     0,     0,     0,     0,     0,    56,
       0,    57,    58,    59,    60,    61,    52,   126,     0,    53,
      63,     0,    54,    55,     0,     0,     0,     0,     0,    56,
       0,    57,    58,    59,    60,    61,    52,     0,     0,    53,
      63,     0,    54,    55,     0,     0,     0,     0,     0,    56,
       0,    57,    58,    59,    53,    61,     0,    54,    55,     0,
      63,     0,     0,     0,    56,     0,    57,    58,    59,    53,
      61,     0,   -65,   -65,     0,    63,     0,     0,     0,   -65,
       0,   -65,    58,   -65,     0,    61,     0,     0,     0,     0,
      63
};

static const yytype_int16 yycheck[] =
{
       0,    19,    43,     3,     7,    65,     0,    11,    19,     3,
      22,     4,     4,    31,     3,    15,     4,    20,    21,     9,
      31,    15,     3,    16,    84,    28,    29,    20,    32,   118,
      20,    31,     4,    19,    15,    38,    19,    40,     3,     0,
      74,    33,    34,   103,   133,    31,    16,     4,    31,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,     4,
      63,    64,    28,    19,     4,    37,    38,    39,     9,    72,
      73,    43,    44,    45,    74,   164,   165,    20,   112,    20,
      37,    22,    39,    19,   144,   126,    43,    44,    45,   178,
       9,   180,     4,   182,   128,    31,   185,   100,    22,    30,
      22,    20,   136,    22,   138,   139,    31,   196,   197,   198,
     199,   111,   112,    19,     4,    28,   176,   111,     9,     4,
      31,    30,     3,     4,     5,     6,    31,    10,   128,    30,
     111,   134,   135,    19,     9,    22,   136,    12,   138,   139,
      22,    22,   145,    24,    20,    20,    30,    22,   148,     9,
      31,    28,    12,    19,   148,     3,     4,     5,     6,    28,
      20,    42,    22,   163,    31,    30,   112,   148,    18,   163,
      28,     4,   172,     9,    22,    30,    24,   177,   172,   179,
      19,   181,   163,   177,    17,   179,    19,   181,    30,    30,
     136,   172,   138,   139,    42,    19,   177,    28,   179,    28,
     181,    31,    28,    28,    37,    38,    39,    28,    41,    28,
      43,    44,    45,    46,    47,     3,     4,     5,     6,     3,
       4,     5,     6,     3,     4,     5,     6,    28,    28,    31,
     147,    19,   121,    -1,    22,    -1,    24,    -1,    22,    -1,
      24,    -1,    22,    -1,    24,    29,    -1,    -1,    -1,    29,
       8,    -1,    -1,    11,    42,    -1,    14,    15,    42,    -1,
      -1,    -1,    42,    21,    -1,    23,    24,    25,    26,    27,
      -1,    -1,     8,    31,    32,    11,    -1,    -1,    14,    15,
      -1,    -1,    -1,    -1,    -1,    21,    -1,    23,    24,    25,
      26,    27,     8,    29,    -1,    11,    32,    -1,    14,    15,
      -1,    -1,    -1,    -1,    -1,    21,    -1,    23,    24,    25,
      26,    27,     8,    -1,    30,    11,    32,    -1,    14,    15,
      -1,    -1,    -1,    -1,    -1,    21,    -1,    23,    24,    25,
      26,    27,     8,    -1,    30,    11,    32,    -1,    14,    15,
      -1,    -1,    -1,    -1,    -1,    21,    -1,    23,    24,    25,
      26,    27,    -1,    -1,     8,    31,    32,    11,    -1,    -1,
      14,    15,    -1,    -1,    -1,    -1,    -1,    21,    -1,    23,
      24,    25,    26,    27,     8,    29,    10,    11,    32,    -1,
      14,    15,    -1,    -1,    -1,    -1,    -1,    21,    -1,    23,
      24,    25,    26,    27,     8,    -1,    -1,    11,    32,    -1,
      14,    15,    -1,    -1,    -1,    -1,    -1,    21,    -1,    23,
      24,    25,    26,    27,    -1,    -1,     8,    31,    32,    11,
      -1,    -1,    14,    15,    -1,    -1,    -1,    -1,    -1,    21,
      -1,    23,    24,    25,    26,    27,     8,    29,    -1,    11,
      32,    -1,    14,    15,    -1,    -1,    -1,    -1,    -1,    21,
      -1,    23,    24,    25,    26,    27,     8,    -1,    -1,    11,
      32,    -1,    14,    15,    -1,    -1,    -1,    -1,    -1,    21,
      -1,    23,    24,    25,    11,    27,    -1,    14,    15,    -1,
      32,    -1,    -1,    -1,    21,    -1,    23,    24,    25,    11,
      27,    -1,    14,    15,    -1,    32,    -1,    -1,    -1,    21,
      -1,    23,    24,    25,    -1,    27,    -1,    -1,    -1,    -1,
      32
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     4,    17,    19,    37,    38,    39,    41,    43,    44,
      45,    46,    47,    51,    52,    53,    56,    58,    59,    60,
      20,    22,    52,     4,     3,     4,     5,     6,    22,    24,
      31,    42,    55,    56,     3,     3,     0,    52,     9,    12,
      20,    22,    16,     4,    55,    55,    28,    19,    55,    55,
       4,    60,     8,    11,    14,    15,    21,    23,    24,    25,
      26,    27,    31,    32,     9,    22,    55,     4,    55,    55,
      63,     4,     9,    20,    22,    54,    29,    30,     4,    38,
      57,    60,    61,    30,    22,    55,    55,    55,    55,    55,
      55,    55,    55,    55,    55,    55,    55,    63,    31,    29,
      10,    62,    30,    19,    55,    29,    55,    60,    61,    65,
      31,    19,    22,     4,    28,     4,    33,    34,    31,    63,
      30,    55,    31,    63,    31,     9,    29,     4,    10,    64,
      30,    52,    65,    19,     9,    20,    22,    54,    22,    22,
      57,    30,    62,    28,    19,    20,    54,    61,    19,    31,
      28,    30,    57,    55,    29,    65,    65,    65,    31,    63,
      64,    52,    18,    19,    31,    28,     9,    30,    30,    30,
      28,    28,    19,    52,    57,    57,    19,    19,    31,    19,
      31,    19,    31,    31,    52,    28,    63,    52,    57,    52,
      57,    52,    57,    28,    57,    28,    28,    28,    28,    31,
      57,    57,    57,    57
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
#line 93 "parser.yy"
	{ delete (yyvaluep->str); };

/* Line 1000 of yacc.c  */
#line 1381 "parser.tab.cc"
	break;
      case 7: /* "\"symbol\"" */

/* Line 1000 of yacc.c  */
#line 94 "parser.yy"
	{ delete (*yyvaluep).symbol; };

/* Line 1000 of yacc.c  */
#line 1390 "parser.tab.cc"
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
#line 47 "parser.yy"
{
	yydebug = tp._parse_trace_p;
   /**
    setup the filename each time before parsing
    */
   static int mvvParserInputNumber = 0;
   yylloc.filename = mvv::Symbol::create( tp._filename == "" ? "(input" + nll::core::val2str( mvvParserInputNumber++ ) + ")" : tp._filename );
}

/* Line 1242 of yacc.c  */
#line 1550 "parser.tab.cc"
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
#line 163 "parser.yy"
    { tp._root = (yyvsp[(1) - (1)].astStatements); ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 165 "parser.yy"
    { (yyval.astStatements) = new mvv::parser::AstStatements( (yyloc) ); ;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 166 "parser.yy"
    { (yyval.astStatements) = (yyvsp[(2) - (2)].astStatements); (yyvsp[(2) - (2)].astStatements)->insert( (yyvsp[(1) - (2)].ast) ); ;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 168 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstIf( (yyloc), (yyvsp[(3) - (7)].astExp), (yyvsp[(6) - (7)].astStatements), 0 ); ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 169 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstIf( (yyloc), (yyvsp[(3) - (11)].astExp), (yyvsp[(6) - (11)].astStatements), (yyvsp[(10) - (11)].astStatements) ); ;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 170 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclClass( (yyloc), *(yyvsp[(2) - (5)].symbol), (yyvsp[(4) - (5)].astDecls) ); ;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 176 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (5)].astTypeT), *(yyvsp[(2) - (5)].symbol), (yyvsp[(4) - (5)].astExp) ); ;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 177 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (7)].astTypeT), *(yyvsp[(2) - (7)].symbol), 0, (yyvsp[(5) - (7)].astArgs) ); ;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 178 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (6)].astTypeT), *(yyvsp[(2) - (6)].symbol), (yyvsp[(4) - (6)].astDeclVars) ); ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 179 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (8)].astTypeT), *(yyvsp[(2) - (8)].symbol), (yyvsp[(4) - (8)].astDeclVars), (yyvsp[(7) - (8)].astStatements) ); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 180 "parser.yy"
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
#line 188 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (9)].astTypeT), *(yyvsp[(2) - (9)].symbol), 0, (yyvsp[(7) - (9)].astArgs) ); (yyvsp[(1) - (9)].astTypeT)->setArray( true ); /* we don't handle several dimensions*/ ;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 189 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstExpAssign( (yyloc), (yyvsp[(1) - (4)].astVar), (yyvsp[(3) - (4)].astExp) ); ;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 190 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstExpCall( (yyloc), (yyvsp[(1) - (5)].astVar), (yyvsp[(3) - (5)].astArgs) ); ;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 191 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstReturn( (yyloc), (yyvsp[(2) - (3)].astExp) ); ;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 192 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstReturn( (yyloc) ); ;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 193 "parser.yy"
    { (yyval.ast) = (yyvsp[(2) - (3)].astStatements); ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 194 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstImport( (yyloc), *(yyvsp[(2) - (2)].str) ); ;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 195 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstInclude( (yyloc), *(yyvsp[(2) - (2)].str) ); ;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 198 "parser.yy"
    { (yyval.arrayDim) = new std::vector<mvv::parser::AstExp*>(); ;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 199 "parser.yy"
    { (yyval.arrayDim) = (yyvsp[(4) - (4)].arrayDim); (yyval.arrayDim)->push_back( (yyvsp[(2) - (4)].astExp) ); ;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 203 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstInt( (yyloc), (yyvsp[(1) - (1)].ival) ); ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 204 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstFloat( (yyloc), (yyvsp[(1) - (1)].fval) ); ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 205 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::PLUS ); ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 206 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::MINUS ); ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 207 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::TIMES ); ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 208 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::DIVIDE ); ;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 209 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::AND ); ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 210 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::OR ); ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 211 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::LT ); ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 212 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::LE ); ;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 213 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::GT ); ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 214 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::GE ); ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 215 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::NE ); ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 216 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), new mvv::parser::AstInt( (yyloc), 0 ) , (yyvsp[(2) - (2)].astExp), mvv::parser::AstOpBin::MINUS ); ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 217 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstExpSeq( (yyloc), (yyvsp[(2) - (3)].astExp) ); ;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 218 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstString( (yyloc), *(yyvsp[(1) - (1)].str) ); ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 219 "parser.yy"
    { (yyval.astExp) = (yyvsp[(1) - (1)].astVar); ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 220 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstExpAssign( (yyloc), (yyvsp[(1) - (3)].astVar), (yyvsp[(3) - (3)].astExp) ); ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 221 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstExpTypename( (yyloc), (yyvsp[(2) - (5)].astTypeT), (yyvsp[(4) - (5)].astArgs) );;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 225 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarSimple( (yyloc), *(yyvsp[(1) - (1)].symbol), true ); ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 226 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarArray( (yyloc), new mvv::parser::AstVarSimple( (yyloc), *(yyvsp[(1) - (4)].symbol), true ), (yyvsp[(3) - (4)].astExp) ); ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 227 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarField( (yyloc), (yyvsp[(1) - (3)].astVar), *(yyvsp[(3) - (3)].symbol) ); ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 228 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarArray( (yyloc), (yyvsp[(1) - (4)].astVar), (yyvsp[(3) - (4)].astExp) ); ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 229 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstExpCall( (yyloc), (yyvsp[(1) - (4)].astVar), (yyvsp[(3) - (4)].astArgs) ); ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 233 "parser.yy"
    { (yyval.astDecls) = new mvv::parser::AstDecls( (yyloc) ); ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 234 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(3) - (3)].astDecls); (yyval.astDecls)->insert( (yyvsp[(1) - (3)].astDeclVar) ); ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 235 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(10) - (10)].astDecls); mvv::parser::AstDeclVar* var = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (10)].astTypeT), *(yyvsp[(2) - (10)].symbol), 0, (yyvsp[(7) - (10)].astArgs) ); (yyvsp[(1) - (10)].astTypeT)->setArray( true ); (yyval.astDecls)->insert( var ); ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 236 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(6) - (6)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclClass( (yyloc), *(yyvsp[(2) - (6)].symbol), (yyvsp[(4) - (6)].astDecls) ) ); ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 237 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), *(yyvsp[(2) - (9)].symbol), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 238 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(7) - (7)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (7)].astTypeT), *(yyvsp[(2) - (7)].symbol), (yyvsp[(4) - (7)].astDeclVars) ) ); ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 239 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), mvv::Symbol::create("operator[]"), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 240 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(7) - (7)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (7)].astTypeT), mvv::Symbol::create("operator[]"), (yyvsp[(4) - (7)].astDeclVars) ) ); ;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 241 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), mvv::Symbol::create("operator()"), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 242 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(7) - (7)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (7)].astTypeT), mvv::Symbol::create("operator()"), (yyvsp[(4) - (7)].astDeclVars) ) ); ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 243 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(8) - (8)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), 0, *(yyvsp[(1) - (8)].symbol), (yyvsp[(3) - (8)].astDeclVars), (yyvsp[(6) - (8)].astStatements) ) ); ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 244 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(6) - (6)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), 0, *(yyvsp[(1) - (6)].symbol), (yyvsp[(3) - (6)].astDeclVars) ) ); ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 246 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::VAR ); ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 247 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::INT ); ;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 248 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::FLOAT );;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 249 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::STRING ); ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 250 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::VOID );;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 252 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::SYMBOL, (yyvsp[(1) - (1)].symbol) ); ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 253 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstTypeField( (yyloc), (yyvsp[(1) - (3)].astTypeT), *(yyvsp[(3) - (3)].symbol) ); ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 255 "parser.yy"
    { (yyval.astTypeT) = (yyvsp[(1) - (1)].astTypeT); ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 256 "parser.yy"
    { (yyval.astTypeT) = (yyvsp[(1) - (1)].astTypeT); ;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 259 "parser.yy"
    { (yyval.astDeclVar) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (4)].astTypeT), *(yyvsp[(2) - (4)].symbol), (yyvsp[(4) - (4)].astExp) ); ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 260 "parser.yy"
    { (yyval.astDeclVar) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (3)].astTypeT), *(yyvsp[(2) - (3)].symbol) );
                                        if ( (yyvsp[(3) - (3)].arrayDim)->size() )
                                        {
                                           (yyvsp[(1) - (3)].astTypeT)->setArray( true );
                                           (yyvsp[(1) - (3)].astTypeT)->setSize( (yyvsp[(3) - (3)].arrayDim) );
                                        } 
                                      ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 269 "parser.yy"
    { (yyval.astArgs) = new mvv::parser::AstArgs( (yyloc) ); ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 270 "parser.yy"
    { (yyval.astArgs) = (yyvsp[(3) - (3)].astArgs); (yyval.astArgs)->insert( (yyvsp[(2) - (3)].astExp) ); ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 272 "parser.yy"
    { (yyval.astArgs) = new mvv::parser::AstArgs( (yyloc) ); ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 273 "parser.yy"
    { (yyval.astArgs) = (yyvsp[(2) - (2)].astArgs); (yyval.astArgs)->insert( (yyvsp[(1) - (2)].astExp) ); ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 275 "parser.yy"
    { (yyval.astDeclVars) = new mvv::parser::AstDeclVars( (yyloc) ); ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 276 "parser.yy"
    { (yyval.astDeclVars) = (yyvsp[(3) - (3)].astDeclVars); (yyval.astDeclVars)->insert( (yyvsp[(2) - (3)].astDeclVar) ); ;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 278 "parser.yy"
    { (yyval.astDeclVars) = new mvv::parser::AstDeclVars( (yyloc) ); ;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 279 "parser.yy"
    { (yyval.astDeclVars) = (yyvsp[(2) - (2)].astDeclVars); (yyval.astDeclVars)->insert( (yyvsp[(1) - (2)].astDeclVar) ); ;}
    break;



/* Line 1455 of yacc.c  */
#line 2282 "parser.tab.cc"
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
#line 281 "parser.yy"


