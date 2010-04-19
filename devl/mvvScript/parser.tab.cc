
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
    - operator= can't be overloaded: just refcounting. To create a new instance, just recreate another object...
    - operator== checks the address for types... and not the semantic==
    - the type of a function doesn't belong to the signature of the function, the signature of a function is unique
    - a type can be referenced. In this case the reference is directly modified. If a type which is not a primitive
      is referenced & used as argument in a function call, refcounting is not updated
    - include: the included file will be parsed, inclusion order doesn't matter
    - import: the source file will be imported & dll dynamically loaded (with the same name) & entry point run
    - import function: can be from class/global function. Because all declarations must be loaded in memory, it can't be declared multiple times
    - array are also refcounted so they can be shared quickly
    - multidimentional arrays can have a several unknow dim
    
    - TODO arg ref -> create the ref for runtime!
    - TODO add covariant return type when inheritance added
    - TODO check function prototypes when added, not just when used, i.e. class Test{ Test(){} int tralala(){return 0;} float tralala(){return 0.0;} } should have error
    - TODO: declared variable with ref: improve the detection of wrong case (i.e. int n; int& n2 = n; int& n3 = n2 + n;
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
#line 156 "parser.tab.cc"

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
     THIS = 289,
     REF = 290,
     OPERATORPARENT = 291,
     OPERATORBRACKET = 292,
     OPERATOR_PLUS = 293,
     OPERATOR_MINUS = 294,
     OPERATOR_TIMES = 295,
     OPERATOR_DIVIDE = 296,
     OPERATOR_LT = 297,
     OPERATOR_GT = 298,
     OPERATOR_LE = 299,
     OPERATOR_GE = 300,
     OPERATOR_EQ = 301,
     OPERATOR_NE = 302,
     OPERATOR_AND = 303,
     OPERATOR_OR = 304,
     FOR = 305,
     IN = 306,
     VAR = 307,
     CLASS = 308,
     VOID = 309,
     RETURN = 310,
     TYPENAME = 311,
     INT_T = 312,
     FLOAT_T = 313,
     STRING_T = 314,
     IMPORT = 315,
     INCLUDE = 316,
     IFX = 317,
     UMINUS = 318
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 104 "parser.yy"

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
#line 276 "parser.tab.cc"
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
#line 301 "parser.tab.cc"

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
#define YYFINAL  42
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   768

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  64
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  18
/* YYNRULES -- Number of rules.  */
#define YYNRULES  100
/* YYNRULES -- Number of states.  */
#define YYNSTATES  249

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   318

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
      55,    56,    57,    58,    59,    60,    61,    62,    63
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     6,     9,    17,    29,    35,    41,
      49,    57,    66,    74,    79,    89,    94,   100,   104,   107,
     111,   114,   117,   126,   134,   135,   140,   144,   146,   148,
     150,   152,   154,   156,   158,   160,   162,   164,   166,   168,
     170,   172,   174,   176,   178,   182,   186,   190,   194,   198,
     202,   206,   210,   214,   218,   222,   226,   229,   233,   235,
     237,   241,   247,   249,   253,   255,   260,   264,   269,   274,
     276,   277,   281,   292,   299,   309,   318,   327,   334,   344,
     353,   355,   357,   359,   361,   363,   365,   369,   371,   373,
     376,   379,   384,   388,   389,   393,   394,   397,   398,   402,
     403
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      65,     0,    -1,    66,    -1,    -1,    67,    66,    -1,    18,
      23,    70,    31,    20,    66,    29,    -1,    18,    23,    70,
      31,    20,    66,    29,    19,    20,    66,    29,    -1,    53,
       4,    20,    73,    29,    -1,    76,     4,    10,    70,    32,
      -1,    76,     4,    10,    20,    79,    29,    32,    -1,    60,
      76,     4,    23,    81,    31,    32,    -1,    76,     4,    23,
      81,    31,    20,    66,    29,    -1,    76,     4,    23,    70,
      78,    31,    32,    -1,    76,     4,    68,    32,    -1,    76,
       4,    21,    30,    10,    20,    79,    29,    32,    -1,    72,
      10,    70,    32,    -1,    72,    23,    79,    31,    32,    -1,
      55,    70,    32,    -1,    55,    32,    -1,    20,    66,    29,
      -1,    60,     3,    -1,    61,     3,    -1,    76,    69,    23,
      81,    31,    20,    66,    29,    -1,    60,    76,    69,    23,
      81,    31,    32,    -1,    -1,    21,    70,    30,    68,    -1,
      21,    30,    68,    -1,    38,    -1,    39,    -1,    40,    -1,
      41,    -1,    42,    -1,    43,    -1,    44,    -1,    45,    -1,
      46,    -1,    47,    -1,    48,    -1,    49,    -1,    37,    -1,
      36,    -1,     5,    -1,     6,    -1,     7,    -1,    70,    28,
      70,    -1,    70,    25,    70,    -1,    70,    33,    70,    -1,
      70,    12,    70,    -1,    70,     9,    70,    -1,    70,    27,
      70,    -1,    70,    24,    70,    -1,    70,    22,    70,    -1,
      70,    16,    70,    -1,    70,    15,    70,    -1,    70,    26,
      70,    -1,    70,    14,    70,    -1,    25,    70,    -1,    23,
      70,    31,    -1,     3,    -1,    72,    -1,    72,    10,    70,
      -1,    56,    76,    23,    79,    31,    -1,    34,    -1,    72,
      13,    34,    -1,     4,    -1,     4,    21,    70,    30,    -1,
      72,    13,     4,    -1,    72,    21,    70,    30,    -1,    72,
      23,    79,    31,    -1,    71,    -1,    -1,    77,    32,    73,
      -1,    76,     4,    21,    30,    10,    20,    79,    29,    32,
      73,    -1,    53,     4,    20,    73,    29,    73,    -1,    76,
       4,    23,    81,    31,    20,    66,    29,    73,    -1,    60,
      76,     4,    23,    81,    31,    32,    73,    -1,     4,    23,
      81,    31,    20,    66,    29,    73,    -1,     4,    23,    81,
      31,    32,    73,    -1,    76,    69,    23,    81,    31,    20,
      66,    29,    73,    -1,    60,    76,    69,    23,    81,    31,
      32,    73,    -1,    52,    -1,    57,    -1,    58,    -1,    59,
      -1,    54,    -1,     4,    -1,     4,    17,    75,    -1,    75,
      -1,    74,    -1,    74,    35,    -1,    75,    35,    -1,    76,
       4,    10,    70,    -1,    76,     4,    68,    -1,    -1,    11,
      70,    78,    -1,    -1,    70,    78,    -1,    -1,    11,    77,
      80,    -1,    -1,    77,    80,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   228,   228,   230,   231,   233,   234,   235,   241,   242,
     243,   244,   245,   251,   259,   260,   261,   262,   263,   264,
     265,   266,   269,   270,   274,   275,   276,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   318,   319,   321,   322,   323,   324,   325,   326,
     330,   331,   332,   333,   334,   335,   336,   337,   338,   339,
     342,   343,   344,   345,   346,   348,   349,   351,   352,   353,
     354,   357,   358,   367,   368,   370,   371,   373,   374,   376,
     377
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
  "\")\"", "\";\"", "\"*\"", "\"this\"", "\"&\"", "\"operator()\"",
  "\"operator[]\"", "\"operator+\"", "\"operator-\"", "\"operator*\"",
  "\"operator/\"", "\"operator<\"", "\"operator>\"", "\"operator<=\"",
  "\"operator>=\"", "\"operator==\"", "\"operator!=\"", "\"operator&&\"",
  "\"operator||\"", "\"for\"", "\"in\"", "\"var\"", "\"class\"",
  "\"void\"", "\"return\"", "\"typename\"", "\"int type\"",
  "\"float type\"", "\"string type\"", "\"import\"", "\"include\"", "IFX",
  "UMINUS", "$accept", "program", "statements", "statement", "array_decl",
  "operator_def", "rvalue", "thislvalue", "lvalue", "var_decs_class",
  "type_simple", "type_field", "type", "var_dec_simple", "args_add",
  "args", "fn_var_dec_add", "fn_var_dec", 0
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
     315,   316,   317,   318
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    64,    65,    66,    66,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    68,    68,    68,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    70,    70,    70,    70,    70,    70,    70,    70,    70,
      70,    70,    70,    70,    70,    70,    70,    70,    70,    70,
      70,    70,    71,    71,    72,    72,    72,    72,    72,    72,
      73,    73,    73,    73,    73,    73,    73,    73,    73,    73,
      74,    74,    74,    74,    74,    75,    75,    76,    76,    76,
      76,    77,    77,    78,    78,    79,    79,    80,    80,    81,
      81
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     0,     2,     7,    11,     5,     5,     7,
       7,     8,     7,     4,     9,     4,     5,     3,     2,     3,
       2,     2,     8,     7,     0,     4,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     3,     1,     1,
       3,     5,     1,     3,     1,     4,     3,     4,     4,     1,
       0,     3,    10,     6,     9,     8,     8,     6,     9,     8,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     2,
       2,     4,     3,     0,     3,     0,     2,     0,     3,     0,
       2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,    64,     0,     3,    62,    80,     0,    84,     0,    81,
      82,    83,     0,     0,     0,     2,     3,    69,     0,    88,
      87,     0,     0,     0,     0,     0,     0,    58,    64,    41,
      42,    43,     0,     0,    18,     0,     0,    59,    20,    85,
       0,    21,     1,     4,     0,     0,     0,    95,    89,    90,
      24,    40,    39,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,     0,    86,     0,     0,    19,
      70,     0,    56,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    17,     0,     0,    95,     0,
       0,     0,    66,    63,     0,    93,     0,     0,     0,    99,
       0,    99,    65,     0,    85,     0,     0,     0,     0,     0,
      57,    95,    48,    47,    55,    53,    52,    51,    50,    45,
      54,    49,    44,    46,    60,     0,    99,    99,    15,    67,
       0,    96,    68,    95,     0,    24,     0,    93,     0,    97,
       0,    13,     0,     3,    99,     0,     0,     7,    24,     0,
      70,     0,    68,     0,     0,    93,    16,     0,     8,     0,
       0,    26,    24,     0,    24,     0,   100,     0,     0,     0,
       0,    70,     0,     0,     0,     0,    99,    92,    99,    71,
      61,     0,     0,    94,     0,    95,    24,    25,     0,    97,
       3,     3,     5,     0,     0,    99,    99,    91,    24,     0,
       0,    10,    23,     9,     0,    12,    98,     0,     0,     0,
       3,    70,    70,     0,     0,     0,     0,     0,     0,    11,
      22,     3,     0,    77,    73,     0,     0,    95,     3,     3,
      14,     0,    70,    70,    70,     0,     0,     0,     6,    76,
      75,    79,     0,    70,    70,    70,    74,    78,    72
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    14,    15,    16,   161,    65,    95,    17,    37,   107,
      19,    20,   108,   109,   131,    96,   166,   140
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -132
static const yytype_int16 yypact[] =
{
       7,   366,   -10,     7,  -132,  -132,    11,  -132,   275,  -132,
    -132,  -132,   201,    17,    21,  -132,     7,  -132,    26,   -18,
      -6,   384,    38,   361,   361,    15,    36,  -132,    37,  -132,
    -132,  -132,   361,   361,  -132,   218,   499,    27,  -132,    53,
     412,  -132,  -132,  -132,   361,     4,   361,   361,  -132,  -132,
      90,  -132,  -132,  -132,  -132,  -132,  -132,  -132,  -132,  -132,
    -132,  -132,  -132,  -132,  -132,    49,  -132,   521,   541,  -132,
      80,   561,  -132,    65,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,  -132,   361,   361,   361,    66,
      68,   581,  -132,  -132,   603,   623,    62,   281,   287,   210,
      63,   218,  -132,    74,    34,   106,   218,    83,   430,    82,
    -132,   361,   720,  -132,   735,   735,   735,   735,   735,    -3,
     735,   705,    -3,  -132,   685,    93,   218,   218,  -132,  -132,
     361,  -132,    89,   361,   643,    42,   665,   623,   124,   125,
     104,  -132,   111,     7,   218,   126,   458,  -132,    94,   127,
      80,   116,  -132,   117,   123,   623,  -132,   129,  -132,   135,
     319,  -132,   139,   130,    98,   218,  -132,   136,   146,   138,
     137,    80,   147,   148,   361,   329,   218,  -132,   218,  -132,
    -132,   140,   143,  -132,   145,   361,   139,  -132,   149,   125,
       7,     7,   150,    14,   151,   218,   218,   685,    99,   153,
     154,  -132,  -132,  -132,   157,  -132,  -132,   158,   165,   159,
       7,    80,    80,   166,   167,   179,   181,   182,   171,  -132,
    -132,     7,   177,  -132,  -132,   175,   186,   361,     7,     7,
    -132,   180,    80,    80,    80,   191,   195,   196,  -132,  -132,
    -132,  -132,   204,    80,    80,    80,  -132,  -132,  -132
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -132,  -132,    -2,  -132,   -46,   -21,    -1,  -132,     2,   -81,
    -132,   189,     0,   -73,  -131,   -78,    48,    56
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -86
static const yytype_int16 yytable[] =
{
      21,    25,    18,    21,   100,    18,   163,    36,    92,    75,
     125,     1,    40,    24,    43,    26,    21,    48,    18,    90,
      41,    42,    67,    68,   183,     2,   139,     3,   139,    49,
      86,    71,    72,   151,   210,    73,    44,    87,    93,    45,
      45,     4,    39,    91,    69,    94,   211,    46,    46,    47,
      88,    22,   159,   139,   139,   157,    70,   144,    23,     5,
       6,     7,     8,   160,     9,    10,    11,    12,    13,   179,
      22,   139,   101,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   104,   123,   124,   149,   111,   126,
     194,   127,   189,   132,   143,   141,   134,   136,   137,   138,
      97,   138,   177,   139,   174,   139,   146,   204,   174,   215,
     145,    98,   147,    99,   150,   175,   187,   176,   177,   160,
     160,   156,   139,   139,   152,   173,   138,   138,   164,   155,
     223,   224,     5,   105,     7,   167,   165,     9,    10,    11,
     106,   169,   168,    21,   138,    18,   171,   180,   181,   235,
     178,   239,   240,   241,   182,   185,   190,   142,   184,   136,
     160,   188,   246,   247,   248,   138,   191,   192,   193,   209,
     195,   196,   201,   197,   136,   202,   138,   203,   138,   221,
     212,   205,   153,   154,   216,   217,   218,   219,   207,   208,
      21,    21,    18,    18,   220,   138,   138,   225,   226,   227,
     170,   228,   229,   230,    38,    39,   232,   233,   222,   238,
      21,    66,    18,    27,     1,    29,    30,    31,   234,   231,
     242,    21,    39,    18,   243,   244,   236,   237,    21,    21,
      18,    18,   199,    32,   200,    33,   245,   206,     0,     0,
       0,     0,     0,     0,     4,     0,     0,     0,     0,     0,
       0,   213,   214,     5,     0,     7,     0,     0,     9,    10,
      11,     0,     5,     0,     7,     0,    35,     9,    10,    11,
       5,     0,     7,     0,     0,     9,    10,    11,    27,    28,
      29,    30,    31,     0,    27,    28,    29,    30,    31,     0,
      27,    28,    29,    30,    31,     0,     0,     0,    32,     0,
      33,   133,     0,     0,    32,     0,    33,    34,     0,     4,
      32,     0,    33,     0,     0,     4,     0,   135,     0,     0,
       0,     4,    27,    28,    29,    30,    31,     0,     0,     0,
       0,    35,    27,    28,    29,    30,    31,    35,     0,     0,
       0,     0,    32,    35,    33,     0,     0,     0,     0,   186,
       0,     0,    32,     4,    33,     0,     0,     0,     0,   198,
       0,     0,     0,     4,    27,    28,    29,    30,    31,     0,
     -85,     0,     0,     0,     0,    35,     0,     0,     0,     0,
       0,     0,     0,    22,    32,    35,    33,    23,    50,     0,
       0,     0,     0,     0,     0,     4,     0,     0,     0,     0,
       0,   -85,   -85,   -85,   -85,   -85,   -85,   -85,   -85,   -85,
     -85,   -85,   -85,   -85,   -85,   -85,    89,    35,     0,     0,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,   148,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,   172,     0,     0,     0,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    74,     0,
       0,    75,     0,    76,    77,    78,     0,     0,     0,     0,
       0,    79,     0,    80,    81,    82,    83,    84,     0,     0,
      74,    85,    86,    75,     0,    76,    77,    78,     0,     0,
       0,     0,     0,    79,     0,    80,    81,    82,    83,    84,
      74,   102,     0,    75,    86,    76,    77,    78,     0,     0,
       0,     0,     0,    79,     0,    80,    81,    82,    83,    84,
      74,     0,   103,    75,    86,    76,    77,    78,     0,     0,
       0,     0,     0,    79,     0,    80,    81,    82,    83,    84,
      74,     0,   110,    75,    86,    76,    77,    78,     0,     0,
       0,     0,     0,    79,     0,    80,    81,    82,    83,    84,
       0,     0,    74,   128,    86,    75,     0,    76,    77,    78,
       0,     0,     0,     0,     0,    79,     0,    80,    81,    82,
      83,    84,    74,   129,   130,    75,    86,    76,    77,    78,
       0,     0,     0,     0,     0,    79,     0,    80,    81,    82,
      83,    84,    74,     0,     0,    75,    86,    76,    77,    78,
       0,     0,     0,     0,     0,    79,     0,    80,    81,    82,
      83,    84,     0,     0,    74,   158,    86,    75,     0,    76,
      77,    78,     0,     0,     0,     0,     0,    79,     0,    80,
      81,    82,    83,    84,    74,   162,     0,    75,    86,    76,
      77,    78,     0,     0,     0,     0,     0,    79,     0,    80,
      81,    82,    83,    84,    74,     0,     0,    75,    86,    76,
      77,    78,     0,     0,     0,     0,     0,    79,     0,    80,
      81,    82,    75,    84,    76,    77,    78,     0,    86,     0,
       0,     0,    79,     0,    80,    81,    82,    75,    84,   -86,
     -86,   -86,     0,    86,     0,     0,     0,   -86,     0,   -86,
      81,   -86,     0,    84,     0,     0,     0,     0,    86
};

static const yytype_int16 yycheck[] =
{
       0,     3,     0,     3,    50,     3,   137,     8,     4,    12,
      88,     4,    12,    23,    16,     4,    16,    35,    16,    40,
       3,     0,    23,    24,   155,    18,    99,    20,   101,    35,
      33,    32,    33,   111,    20,    35,    10,    10,    34,    13,
      13,    34,     4,    44,    29,    46,    32,    21,    21,    23,
      23,    17,    10,   126,   127,   133,    20,    23,    21,    52,
      53,    54,    55,    21,    57,    58,    59,    60,    61,   150,
      17,   144,    23,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,     4,    86,    87,   108,    23,    23,
     171,    23,   165,    31,    20,    32,    97,    98,    99,    99,
      10,   101,   148,   176,    10,   178,   106,   185,    10,    10,
       4,    21,    29,    23,    32,    21,   162,    23,   164,    21,
      21,    32,   195,   196,    31,   146,   126,   127,     4,   130,
     211,   212,    52,    53,    54,    31,    11,    57,    58,    59,
      60,   143,    31,   143,   144,   143,    20,    31,    31,   227,
      23,   232,   233,   234,    31,    20,    20,   101,    29,   160,
      21,    31,   243,   244,   245,   165,    20,    29,    31,    19,
      23,    23,    32,   174,   175,    32,   176,    32,   178,    20,
      29,    32,   126,   127,    31,    31,    29,    29,   190,   191,
     190,   191,   190,   191,    29,   195,   196,    31,    31,    20,
     144,    20,    20,    32,     3,     4,    29,    32,   210,    29,
     210,    22,   210,     3,     4,     5,     6,     7,    32,   221,
      29,   221,     4,   221,    29,    29,   228,   229,   228,   229,
     228,   229,   176,    23,   178,    25,    32,   189,    -1,    -1,
      -1,    -1,    -1,    -1,    34,    -1,    -1,    -1,    -1,    -1,
      -1,   195,   196,    52,    -1,    54,    -1,    -1,    57,    58,
      59,    -1,    52,    -1,    54,    -1,    56,    57,    58,    59,
      52,    -1,    54,    -1,    -1,    57,    58,    59,     3,     4,
       5,     6,     7,    -1,     3,     4,     5,     6,     7,    -1,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    23,    -1,
      25,    20,    -1,    -1,    23,    -1,    25,    32,    -1,    34,
      23,    -1,    25,    -1,    -1,    34,    -1,    30,    -1,    -1,
      -1,    34,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    56,     3,     4,     5,     6,     7,    56,    -1,    -1,
      -1,    -1,    23,    56,    25,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    23,    34,    25,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    -1,    34,     3,     4,     5,     6,     7,    -1,
       4,    -1,    -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    17,    23,    56,    25,    21,     4,    -1,
      -1,    -1,    -1,    -1,    -1,    34,    -1,    -1,    -1,    -1,
      -1,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,     4,    56,    -1,    -1,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,     4,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,     4,    -1,    -1,    -1,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,     9,    -1,
      -1,    12,    -1,    14,    15,    16,    -1,    -1,    -1,    -1,
      -1,    22,    -1,    24,    25,    26,    27,    28,    -1,    -1,
       9,    32,    33,    12,    -1,    14,    15,    16,    -1,    -1,
      -1,    -1,    -1,    22,    -1,    24,    25,    26,    27,    28,
       9,    30,    -1,    12,    33,    14,    15,    16,    -1,    -1,
      -1,    -1,    -1,    22,    -1,    24,    25,    26,    27,    28,
       9,    -1,    31,    12,    33,    14,    15,    16,    -1,    -1,
      -1,    -1,    -1,    22,    -1,    24,    25,    26,    27,    28,
       9,    -1,    31,    12,    33,    14,    15,    16,    -1,    -1,
      -1,    -1,    -1,    22,    -1,    24,    25,    26,    27,    28,
      -1,    -1,     9,    32,    33,    12,    -1,    14,    15,    16,
      -1,    -1,    -1,    -1,    -1,    22,    -1,    24,    25,    26,
      27,    28,     9,    30,    11,    12,    33,    14,    15,    16,
      -1,    -1,    -1,    -1,    -1,    22,    -1,    24,    25,    26,
      27,    28,     9,    -1,    -1,    12,    33,    14,    15,    16,
      -1,    -1,    -1,    -1,    -1,    22,    -1,    24,    25,    26,
      27,    28,    -1,    -1,     9,    32,    33,    12,    -1,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    22,    -1,    24,
      25,    26,    27,    28,     9,    30,    -1,    12,    33,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    22,    -1,    24,
      25,    26,    27,    28,     9,    -1,    -1,    12,    33,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    22,    -1,    24,
      25,    26,    12,    28,    14,    15,    16,    -1,    33,    -1,
      -1,    -1,    22,    -1,    24,    25,    26,    12,    28,    14,
      15,    16,    -1,    33,    -1,    -1,    -1,    22,    -1,    24,
      25,    26,    -1,    28,    -1,    -1,    -1,    -1,    33
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     4,    18,    20,    34,    52,    53,    54,    55,    57,
      58,    59,    60,    61,    65,    66,    67,    71,    72,    74,
      75,    76,    17,    21,    23,    66,     4,     3,     4,     5,
       6,     7,    23,    25,    32,    56,    70,    72,     3,     4,
      76,     3,     0,    66,    10,    13,    21,    23,    35,    35,
       4,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    69,    75,    70,    70,    29,
      20,    70,    70,    76,     9,    12,    14,    15,    16,    22,
      24,    25,    26,    27,    28,    32,    33,    10,    23,     4,
      69,    70,     4,    34,    70,    70,    79,    10,    21,    23,
      68,    23,    30,    31,     4,    53,    60,    73,    76,    77,
      31,    23,    70,    70,    70,    70,    70,    70,    70,    70,
      70,    70,    70,    70,    70,    79,    23,    23,    32,    30,
      11,    78,    31,    20,    70,    30,    70,    70,    76,    77,
      81,    32,    81,    20,    23,     4,    76,    29,     4,    69,
      32,    79,    31,    81,    81,    70,    32,    79,    32,    10,
      21,    68,    30,    78,     4,    11,    80,    31,    31,    66,
      81,    20,     4,    69,    10,    21,    23,    68,    23,    73,
      31,    31,    31,    78,    29,    20,    30,    68,    31,    77,
      20,    20,    29,    31,    73,    23,    23,    70,    30,    81,
      81,    32,    32,    32,    79,    32,    80,    66,    66,    19,
      20,    32,    29,    81,    81,    10,    31,    31,    29,    29,
      29,    20,    66,    73,    73,    31,    31,    20,    20,    20,
      32,    66,    29,    32,    32,    79,    66,    66,    29,    73,
      73,    73,    29,    29,    29,    32,    73,    73,    73
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
#line 143 "parser.yy"
	{ delete (yyvaluep->str); };

/* Line 1000 of yacc.c  */
#line 1530 "parser.tab.cc"
	break;
      case 8: /* "\"symbol\"" */

/* Line 1000 of yacc.c  */
#line 144 "parser.yy"
	{ delete (*yyvaluep).symbol; };

/* Line 1000 of yacc.c  */
#line 1539 "parser.tab.cc"
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
#line 94 "parser.yy"
{
	yydebug = tp._parse_trace_p;
   /**
    setup the filename each time before parsing
    */
   static int mvvParserInputNumber = 0;
   yylloc.filename = mvv::Symbol::create( tp._filename == "" ? "(input" + nll::core::val2str( mvvParserInputNumber++ ) + ")" : tp._filename );
}

/* Line 1242 of yacc.c  */
#line 1699 "parser.tab.cc"
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
#line 228 "parser.yy"
    { tp._root = (yyvsp[(1) - (1)].astStatements); ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 230 "parser.yy"
    { (yyval.astStatements) = new mvv::parser::AstStatements( (yyloc) ); ;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 231 "parser.yy"
    { (yyval.astStatements) = (yyvsp[(2) - (2)].astStatements); (yyvsp[(2) - (2)].astStatements)->insert( (yyvsp[(1) - (2)].ast) ); ;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 233 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstIf( (yyloc), (yyvsp[(3) - (7)].astExp), (yyvsp[(6) - (7)].astStatements), 0 ); ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 234 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstIf( (yyloc), (yyvsp[(3) - (11)].astExp), (yyvsp[(6) - (11)].astStatements), (yyvsp[(10) - (11)].astStatements) ); ;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 235 "parser.yy"
    { mvv::parser::AstDeclClass* decl = new mvv::parser::AstDeclClass( (yyloc), *(yyvsp[(2) - (5)].symbol), (yyvsp[(4) - (5)].astDecls) ); (yyval.ast) = decl; linkFunctionToClass( *decl ); ;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 241 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (5)].astTypeT), *(yyvsp[(2) - (5)].symbol), (yyvsp[(4) - (5)].astExp) ); ;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 242 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (7)].astTypeT), *(yyvsp[(2) - (7)].symbol), 0, (yyvsp[(5) - (7)].astArgs) ); ;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 243 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(2) - (7)].astTypeT), *(yyvsp[(3) - (7)].symbol), (yyvsp[(5) - (7)].astDeclVars) ); ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 244 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (8)].astTypeT), *(yyvsp[(2) - (8)].symbol), (yyvsp[(4) - (8)].astDeclVars), (yyvsp[(7) - (8)].astStatements) ); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 245 "parser.yy"
    { /* first, concatenate the args*/
                                                                       mvv::parser::AstArgs* args = (yyvsp[(5) - (7)].astArgs);
                                                                       args->getArgs().push_front( (yyvsp[(4) - (7)].astExp) );
                                                                       (yyval.ast) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (7)].astTypeT), *(yyvsp[(2) - (7)].symbol), 0, 0, args );
                                                                     ;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 251 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (4)].astTypeT), *(yyvsp[(2) - (4)].symbol) );
                                                                       if ( (yyvsp[(3) - (4)].arrayDim)->size() )
                                                                       {
                                                                          (yyvsp[(1) - (4)].astTypeT)->setArray( true );
                                                                          (yyvsp[(1) - (4)].astTypeT)->setSize( (yyvsp[(3) - (4)].arrayDim) );
                                                                       } 
                                                                     ;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 259 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (9)].astTypeT), *(yyvsp[(2) - (9)].symbol), 0, (yyvsp[(7) - (9)].astArgs) ); (yyvsp[(1) - (9)].astTypeT)->setArray( true ); /* we don't handle several dimensions*/ ;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 260 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstExpAssign( (yyloc), (yyvsp[(1) - (4)].astVar), (yyvsp[(3) - (4)].astExp) ); ;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 261 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstExpCall( (yyloc), (yyvsp[(1) - (5)].astVar), (yyvsp[(3) - (5)].astArgs) ); ;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 262 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstReturn( (yyloc), (yyvsp[(2) - (3)].astExp) ); ;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 263 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstReturn( (yyloc) ); ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 264 "parser.yy"
    { (yyval.ast) = (yyvsp[(2) - (3)].astStatements); ;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 265 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstImport( (yyloc), *(yyvsp[(2) - (2)].str) ); ;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 266 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstInclude( (yyloc), *(yyvsp[(2) - (2)].str) ); ;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 269 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (8)].astTypeT), *(yyvsp[(2) - (8)].symbol), (yyvsp[(4) - (8)].astDeclVars), (yyvsp[(7) - (8)].astStatements) ); ;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 270 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(2) - (7)].astTypeT), *(yyvsp[(3) - (7)].symbol), (yyvsp[(5) - (7)].astDeclVars) ); ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 274 "parser.yy"
    { (yyval.arrayDim) = new std::vector<mvv::parser::AstExp*>(); ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 275 "parser.yy"
    { (yyval.arrayDim) = (yyvsp[(4) - (4)].arrayDim); (yyval.arrayDim)->push_back( (yyvsp[(2) - (4)].astExp) ); ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 276 "parser.yy"
    { (yyval.arrayDim) = (yyvsp[(3) - (3)].arrayDim); (yyval.arrayDim)->push_back( new mvv::parser::AstInt( (yyloc), 0 ) ); ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 278 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator+" ) ); ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 279 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator-" ) ); ;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 280 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator*" ) ); ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 281 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator/" ) ); ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 282 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator<" ) ); ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 283 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator>" ) ); ;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 284 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator<=" ) ); ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 285 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator>=" ) ); ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 286 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator==" ) ); ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 287 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator!=" ) ); ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 288 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator&&" ) ); ;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 289 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator||" ) ); ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 290 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator[]" ) ); ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 291 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator()" ) ); ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 295 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstInt( (yyloc), (yyvsp[(1) - (1)].ival) ); ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 296 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstFloat( (yyloc), (yyvsp[(1) - (1)].fval) ); ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 297 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstNil( (yyloc) ); ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 298 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::PLUS ); ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 299 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::MINUS ); ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 300 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::TIMES ); ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 301 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::DIVIDE ); ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 302 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::AND ); ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 303 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::OR ); ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 304 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::LT ); ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 305 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::LE ); ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 306 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::GT ); ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 307 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::GE ); ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 308 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::NE ); ;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 309 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::EQ ); ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 310 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), new mvv::parser::AstInt( (yyloc), 0 ) , (yyvsp[(2) - (2)].astExp), mvv::parser::AstOpBin::MINUS ); ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 311 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstExpSeq( (yyloc), (yyvsp[(2) - (3)].astExp) ); ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 312 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstString( (yyloc), *(yyvsp[(1) - (1)].str) ); ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 313 "parser.yy"
    { (yyval.astExp) = (yyvsp[(1) - (1)].astVar); ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 314 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstExpAssign( (yyloc), (yyvsp[(1) - (3)].astVar), (yyvsp[(3) - (3)].astExp) ); ;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 315 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstExpTypename( (yyloc), (yyvsp[(2) - (5)].astTypeT), (yyvsp[(4) - (5)].astArgs) );;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 318 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstThis( (yyloc) ); ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 319 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarField( (yyloc), (yyvsp[(1) - (3)].astVar), mvv::Symbol::create( "this" ) ); ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 321 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarSimple( (yyloc), *(yyvsp[(1) - (1)].symbol), true ); ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 322 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarArray( (yyloc), new mvv::parser::AstVarSimple( (yyloc), *(yyvsp[(1) - (4)].symbol), true ), (yyvsp[(3) - (4)].astExp) ); ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 323 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarField( (yyloc), (yyvsp[(1) - (3)].astVar), *(yyvsp[(3) - (3)].symbol) ); ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 324 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarArray( (yyloc), (yyvsp[(1) - (4)].astVar), (yyvsp[(3) - (4)].astExp) ); ;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 325 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstExpCall( (yyloc), (yyvsp[(1) - (4)].astVar), (yyvsp[(3) - (4)].astArgs) ); ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 326 "parser.yy"
    { (yyval.astVar) = (yyvsp[(1) - (1)].astVar); ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 330 "parser.yy"
    { (yyval.astDecls) = new mvv::parser::AstDecls( (yyloc) ); ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 331 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(3) - (3)].astDecls); (yyval.astDecls)->insert( (yyvsp[(1) - (3)].astDeclVar) ); ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 332 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(10) - (10)].astDecls); mvv::parser::AstDeclVar* var = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (10)].astTypeT), *(yyvsp[(2) - (10)].symbol), 0, (yyvsp[(7) - (10)].astArgs) ); (yyvsp[(1) - (10)].astTypeT)->setArray( true ); (yyval.astDecls)->insert( var ); ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 333 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(6) - (6)].astDecls); mvv::parser::AstDeclClass* decl = new mvv::parser::AstDeclClass( (yyloc), *(yyvsp[(2) - (6)].symbol), (yyvsp[(4) - (6)].astDecls) ); (yyval.astDecls)->insert( decl ); linkFunctionToClass( *decl ); ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 334 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), *(yyvsp[(2) - (9)].symbol), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 335 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(8) - (8)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(2) - (8)].astTypeT), *(yyvsp[(3) - (8)].symbol), (yyvsp[(5) - (8)].astDeclVars) ) ); ;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 336 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(8) - (8)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), 0, *(yyvsp[(1) - (8)].symbol), (yyvsp[(3) - (8)].astDeclVars), (yyvsp[(6) - (8)].astStatements) ) ); ;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 337 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(6) - (6)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), 0, *(yyvsp[(1) - (6)].symbol), (yyvsp[(3) - (6)].astDeclVars) ) ); ;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 338 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), *(yyvsp[(2) - (9)].symbol), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 339 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(8) - (8)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(2) - (8)].astTypeT), *(yyvsp[(3) - (8)].symbol), (yyvsp[(5) - (8)].astDeclVars) ) ); ;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 342 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::VAR ); ;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 343 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::INT ); ;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 344 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::FLOAT );;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 345 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::STRING ); ;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 346 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::VOID );;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 348 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::SYMBOL, (yyvsp[(1) - (1)].symbol) ); ;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 349 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstTypeField( (yyloc), (yyvsp[(3) - (3)].astTypeT), *(yyvsp[(1) - (3)].symbol) ); ;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 351 "parser.yy"
    { (yyval.astTypeT) = (yyvsp[(1) - (1)].astTypeT); ;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 352 "parser.yy"
    { (yyval.astTypeT) = (yyvsp[(1) - (1)].astTypeT); ;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 353 "parser.yy"
    { (yyval.astTypeT) = (yyvsp[(1) - (2)].astTypeT); (yyvsp[(1) - (2)].astTypeT)->setIsAReference( true ); ;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 354 "parser.yy"
    { (yyval.astTypeT) = (yyvsp[(1) - (2)].astTypeT); (yyvsp[(1) - (2)].astTypeT)->setIsAReference( true ); ;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 357 "parser.yy"
    { (yyval.astDeclVar) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (4)].astTypeT), *(yyvsp[(2) - (4)].symbol), (yyvsp[(4) - (4)].astExp) ); ;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 358 "parser.yy"
    { (yyval.astDeclVar) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (3)].astTypeT), *(yyvsp[(2) - (3)].symbol) );
                                        if ( (yyvsp[(3) - (3)].arrayDim)->size() )
                                        {
                                           (yyvsp[(1) - (3)].astTypeT)->setArray( true );
                                           (yyvsp[(1) - (3)].astTypeT)->setSize( (yyvsp[(3) - (3)].arrayDim) );
                                        } 
                                      ;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 367 "parser.yy"
    { (yyval.astArgs) = new mvv::parser::AstArgs( (yyloc) ); ;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 368 "parser.yy"
    { (yyval.astArgs) = (yyvsp[(3) - (3)].astArgs); (yyval.astArgs)->insert( (yyvsp[(2) - (3)].astExp) ); ;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 370 "parser.yy"
    { (yyval.astArgs) = new mvv::parser::AstArgs( (yyloc) ); ;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 371 "parser.yy"
    { (yyval.astArgs) = (yyvsp[(2) - (2)].astArgs); (yyval.astArgs)->insert( (yyvsp[(1) - (2)].astExp) ); ;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 373 "parser.yy"
    { (yyval.astDeclVars) = new mvv::parser::AstDeclVars( (yyloc) ); ;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 374 "parser.yy"
    { (yyval.astDeclVars) = (yyvsp[(3) - (3)].astDeclVars); (yyval.astDeclVars)->insert( (yyvsp[(2) - (3)].astDeclVar) ); ;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 376 "parser.yy"
    { (yyval.astDeclVars) = new mvv::parser::AstDeclVars( (yyloc) ); ;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 377 "parser.yy"
    { (yyval.astDeclVars) = (yyvsp[(2) - (2)].astDeclVars); (yyval.astDeclVars)->insert( (yyvsp[(1) - (2)].astDeclVar) ); ;}
    break;



/* Line 1455 of yacc.c  */
#line 2596 "parser.tab.cc"
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
#line 379 "parser.yy"


