
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
    - includes/import are evaluated first
    
    - TODO arg ref -> create the ref for runtime!
    - TODO add covariant return type when inheritance added
    - TODO check function prototypes when added, not just when used, i.e. class Test{ Test(){} int tralala(){return 0;} float tralala(){return 0.0;} } should have error
    - TODO: declared variable with ref: improve the detection of wrong case (i.e. int n; int& n2 = n; int& n3 = n2 + n; // int fn( int& n ){ return n; } int& n = fn(5);
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
#line 157 "parser.tab.cc"

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
     WHILE = 273,
     BREAK = 274,
     IF = 275,
     ELSE = 276,
     LBRACE = 277,
     LBRACK = 278,
     LE = 279,
     LPAREN = 280,
     LT = 281,
     MINUS = 282,
     NE = 283,
     OR = 284,
     PLUS = 285,
     RBRACE = 286,
     RBRACK = 287,
     RPAREN = 288,
     SEMI = 289,
     TIMES = 290,
     THIS = 291,
     REF = 292,
     OPERATORPARENT = 293,
     OPERATORBRACKET = 294,
     OPERATOR_PLUS = 295,
     OPERATOR_MINUS = 296,
     OPERATOR_TIMES = 297,
     OPERATOR_DIVIDE = 298,
     OPERATOR_LT = 299,
     OPERATOR_GT = 300,
     OPERATOR_LE = 301,
     OPERATOR_GE = 302,
     OPERATOR_EQ = 303,
     OPERATOR_NE = 304,
     OPERATOR_AND = 305,
     OPERATOR_OR = 306,
     FOR = 307,
     IN = 308,
     VAR = 309,
     CLASS = 310,
     VOID = 311,
     RETURN = 312,
     TYPENAME = 313,
     INT_T = 314,
     FLOAT_T = 315,
     STRING_T = 316,
     IMPORT = 317,
     INCLUDE = 318,
     IFX = 319,
     UMINUS = 320
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 105 "parser.yy"

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
#line 279 "parser.tab.cc"
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
#line 304 "parser.tab.cc"

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
#define YYFINAL  46
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   792

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  66
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  18
/* YYNRULES -- Number of rules.  */
#define YYNRULES  102
/* YYNRULES -- Number of states.  */
#define YYNSTATES  258

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   320

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
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     6,     9,    17,    29,    35,    43,
      46,    52,    60,    68,    77,    85,    90,   100,   105,   111,
     115,   118,   122,   125,   128,   137,   145,   146,   151,   155,
     157,   159,   161,   163,   165,   167,   169,   171,   173,   175,
     177,   179,   181,   183,   185,   187,   189,   193,   197,   201,
     205,   209,   213,   217,   221,   225,   229,   233,   237,   240,
     244,   246,   248,   252,   258,   260,   264,   266,   271,   275,
     280,   285,   287,   288,   292,   303,   310,   320,   329,   338,
     345,   355,   364,   366,   368,   370,   372,   374,   376,   380,
     382,   384,   387,   390,   395,   399,   400,   404,   405,   408,
     409,   413,   414
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      67,     0,    -1,    68,    -1,    -1,    69,    68,    -1,    20,
      25,    72,    33,    22,    68,    31,    -1,    20,    25,    72,
      33,    22,    68,    31,    21,    22,    68,    31,    -1,    55,
       4,    22,    75,    31,    -1,    18,    25,    72,    33,    22,
      68,    31,    -1,    19,    34,    -1,    78,     4,    10,    72,
      34,    -1,    78,     4,    10,    22,    81,    31,    34,    -1,
      62,    78,     4,    25,    83,    33,    34,    -1,    78,     4,
      25,    83,    33,    22,    68,    31,    -1,    78,     4,    25,
      72,    80,    33,    34,    -1,    78,     4,    70,    34,    -1,
      78,     4,    23,    32,    10,    22,    81,    31,    34,    -1,
      74,    10,    72,    34,    -1,    74,    25,    81,    33,    34,
      -1,    57,    72,    34,    -1,    57,    34,    -1,    22,    68,
      31,    -1,    62,     3,    -1,    63,     3,    -1,    78,    71,
      25,    83,    33,    22,    68,    31,    -1,    62,    78,    71,
      25,    83,    33,    34,    -1,    -1,    23,    72,    32,    70,
      -1,    23,    32,    70,    -1,    40,    -1,    41,    -1,    42,
      -1,    43,    -1,    44,    -1,    45,    -1,    46,    -1,    47,
      -1,    48,    -1,    49,    -1,    50,    -1,    51,    -1,    39,
      -1,    38,    -1,     5,    -1,     6,    -1,     7,    -1,    72,
      30,    72,    -1,    72,    27,    72,    -1,    72,    35,    72,
      -1,    72,    12,    72,    -1,    72,     9,    72,    -1,    72,
      29,    72,    -1,    72,    26,    72,    -1,    72,    24,    72,
      -1,    72,    16,    72,    -1,    72,    15,    72,    -1,    72,
      28,    72,    -1,    72,    14,    72,    -1,    27,    72,    -1,
      25,    72,    33,    -1,     3,    -1,    74,    -1,    74,    10,
      72,    -1,    58,    78,    25,    81,    33,    -1,    36,    -1,
      74,    13,    36,    -1,     4,    -1,     4,    23,    72,    32,
      -1,    74,    13,     4,    -1,    74,    23,    72,    32,    -1,
      74,    25,    81,    33,    -1,    73,    -1,    -1,    79,    34,
      75,    -1,    78,     4,    23,    32,    10,    22,    81,    31,
      34,    75,    -1,    55,     4,    22,    75,    31,    75,    -1,
      78,     4,    25,    83,    33,    22,    68,    31,    75,    -1,
      62,    78,     4,    25,    83,    33,    34,    75,    -1,     4,
      25,    83,    33,    22,    68,    31,    75,    -1,     4,    25,
      83,    33,    34,    75,    -1,    78,    71,    25,    83,    33,
      22,    68,    31,    75,    -1,    62,    78,    71,    25,    83,
      33,    34,    75,    -1,    54,    -1,    59,    -1,    60,    -1,
      61,    -1,    56,    -1,     4,    -1,     4,    17,    77,    -1,
      77,    -1,    76,    -1,    76,    37,    -1,    77,    37,    -1,
      78,     4,    10,    72,    -1,    78,     4,    70,    -1,    -1,
      11,    72,    80,    -1,    -1,    72,    80,    -1,    -1,    11,
      79,    82,    -1,    -1,    79,    82,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   231,   231,   233,   234,   236,   237,   238,   239,   240,
     246,   247,   248,   249,   250,   256,   264,   265,   266,   267,
     268,   269,   270,   271,   274,   275,   279,   280,   281,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   323,   324,   326,   327,   328,   329,
     330,   331,   335,   336,   337,   338,   339,   340,   341,   342,
     343,   344,   347,   348,   349,   350,   351,   353,   354,   356,
     357,   358,   359,   362,   363,   372,   373,   375,   376,   378,
     379,   381,   382
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
  "\">\"", "\"::\"", "\"while\"", "\"break\"", "\"if\"", "\"else\"",
  "\"{\"", "\"[\"", "\"<=\"", "\"(\"", "\"<\"", "\"-\"", "\"!=\"",
  "\"||\"", "\"+\"", "\"}\"", "\"]\"", "\")\"", "\";\"", "\"*\"",
  "\"this\"", "\"&\"", "\"operator()\"", "\"operator[]\"", "\"operator+\"",
  "\"operator-\"", "\"operator*\"", "\"operator/\"", "\"operator<\"",
  "\"operator>\"", "\"operator<=\"", "\"operator>=\"", "\"operator==\"",
  "\"operator!=\"", "\"operator&&\"", "\"operator||\"", "\"for\"",
  "\"in\"", "\"var\"", "\"class\"", "\"void\"", "\"return\"",
  "\"typename\"", "\"int type\"", "\"float type\"", "\"string type\"",
  "\"import\"", "\"include\"", "IFX", "UMINUS", "$accept", "program",
  "statements", "statement", "array_decl", "operator_def", "rvalue",
  "thislvalue", "lvalue", "var_decs_class", "type_simple", "type_field",
  "type", "var_dec_simple", "args_add", "args", "fn_var_dec_add",
  "fn_var_dec", 0
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
     315,   316,   317,   318,   319,   320
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    66,    67,    68,    68,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    70,    70,    70,    71,
      71,    71,    71,    71,    71,    71,    71,    71,    71,    71,
      71,    71,    71,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    73,    73,    74,    74,    74,    74,
      74,    74,    75,    75,    75,    75,    75,    75,    75,    75,
      75,    75,    76,    76,    76,    76,    76,    77,    77,    78,
      78,    78,    78,    79,    79,    80,    80,    81,    81,    82,
      82,    83,    83
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     0,     2,     7,    11,     5,     7,     2,
       5,     7,     7,     8,     7,     4,     9,     4,     5,     3,
       2,     3,     2,     2,     8,     7,     0,     4,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     3,
       1,     1,     3,     5,     1,     3,     1,     4,     3,     4,
       4,     1,     0,     3,    10,     6,     9,     8,     8,     6,
       9,     8,     1,     1,     1,     1,     1,     1,     3,     1,
       1,     2,     2,     4,     3,     0,     3,     0,     2,     0,
       3,     0,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,    66,     0,     0,     0,     3,    64,    82,     0,    86,
       0,    83,    84,    85,     0,     0,     0,     2,     3,    71,
       0,    90,    89,     0,     0,     0,     0,     9,     0,     0,
       0,    60,    66,    43,    44,    45,     0,     0,    20,     0,
       0,    61,    22,    87,     0,    23,     1,     4,     0,     0,
       0,    97,    91,    92,    26,    42,    41,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,     0,
      88,     0,     0,     0,    21,    72,     0,    58,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      19,     0,     0,    97,     0,     0,     0,    68,    65,     0,
      95,     0,     0,     0,   101,     0,   101,    67,     0,     0,
      87,     0,     0,     0,     0,     0,    59,    97,    50,    49,
      57,    55,    54,    53,    52,    47,    56,    51,    46,    48,
      62,     0,   101,   101,    17,    69,     0,    98,    70,    97,
       0,    26,     0,    95,     0,    99,     0,    15,     0,     3,
       3,   101,     0,     0,     7,    26,     0,    72,     0,    70,
       0,     0,    95,    18,     0,    10,     0,     0,    28,    26,
       0,    26,     0,   102,     0,     0,     0,     0,     0,    72,
       0,     0,     0,     0,   101,    94,   101,    73,    63,     0,
       0,    96,     0,    97,    26,    27,     0,    99,     3,     3,
       8,     5,     0,     0,   101,   101,    93,    26,     0,     0,
      12,    25,    11,     0,    14,   100,     0,     0,     0,     3,
      72,    72,     0,     0,     0,     0,     0,     0,    13,    24,
       3,     0,    79,    75,     0,     0,    97,     3,     3,    16,
       0,    72,    72,    72,     0,     0,     0,     6,    78,    77,
      81,     0,    72,    72,    72,    76,    80,    74
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    16,    17,    18,   168,    69,   100,    19,    41,   113,
      21,    22,    23,   115,   137,   101,   173,   146
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -131
static const yytype_int16 yypact[] =
{
     197,   353,   -15,   -20,   -10,   197,  -131,  -131,    12,  -131,
     258,  -131,  -131,  -131,    51,    33,    19,  -131,   197,  -131,
      21,     0,    26,   370,    39,   348,   348,  -131,   348,    34,
      42,  -131,    44,  -131,  -131,  -131,   348,   348,  -131,   128,
     475,    37,  -131,    52,   384,  -131,  -131,  -131,   348,     2,
     348,   348,  -131,  -131,    -2,  -131,  -131,  -131,  -131,  -131,
    -131,  -131,  -131,  -131,  -131,  -131,  -131,  -131,  -131,    43,
    -131,   499,   521,   543,  -131,    -3,   565,  -131,    45,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
    -131,   348,   348,   348,    61,    66,   587,  -131,  -131,   611,
     633,    64,   268,   302,   242,    67,   128,  -131,    82,    86,
      68,   105,   128,    83,   401,    89,  -131,   348,   740,  -131,
     757,   757,   757,   757,   757,    -5,   757,   723,    -5,  -131,
     701,    84,   128,   128,  -131,  -131,   348,  -131,    90,   348,
     655,    25,   679,   633,   112,   116,    98,  -131,   100,   197,
     197,   128,   113,   432,  -131,    16,   109,    -3,   103,  -131,
     106,   107,   633,  -131,   114,  -131,   125,   308,  -131,   115,
     124,    77,   128,  -131,   126,   127,   133,   135,   134,    -3,
     143,   144,   348,   314,   128,  -131,   128,  -131,  -131,   137,
     139,  -131,   142,   348,   115,  -131,   146,   116,   197,   197,
    -131,   157,    11,   152,   128,   128,   701,    79,   153,   159,
    -131,  -131,  -131,   154,  -131,  -131,   163,   165,   177,   197,
      -3,    -3,   167,   175,   187,   188,   189,   178,  -131,  -131,
     197,   190,  -131,  -131,   184,   186,   348,   197,   197,  -131,
     191,    -3,    -3,    -3,   193,   195,   199,  -131,  -131,  -131,
    -131,   198,    -3,    -3,    -3,  -131,  -131,  -131
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -131,  -131,     6,  -131,   -49,   -32,    -8,  -131,     4,   -91,
    -131,   207,   -14,     9,  -130,   -90,    38,    -7
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -88
static const yytype_int16 yytable[] =
{
      44,   110,    40,   131,    20,   105,    97,    80,   102,    20,
      26,    29,    95,   170,    27,    28,    30,    71,    72,    46,
      73,   103,    20,   104,    47,    78,   182,   158,    76,    77,
      91,    48,   191,   219,    49,   166,    45,    52,    98,   183,
      96,   184,    99,    43,    50,   220,    51,    92,   167,   164,
      49,     7,   111,     9,    42,    43,    11,    12,    13,   112,
      50,   114,    93,    53,    75,    74,   187,    25,   106,    24,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   156,   129,   130,    24,   132,   182,   203,   224,
     144,   133,   144,   151,   140,   142,   143,   138,   153,   148,
     167,   147,   167,   213,   149,     7,   185,     9,   150,   152,
      11,    12,    13,   145,   154,   145,   171,   159,   144,   144,
     195,   181,   185,   157,   163,   160,   161,   172,   162,   232,
     233,   174,    43,   175,   186,   179,   188,   144,   167,   189,
     190,   145,   145,   114,   178,   192,   244,   193,   198,   199,
     248,   249,   250,    20,    20,   176,   177,   196,   144,   142,
     145,   255,   256,   257,   200,   114,   201,   202,   204,   205,
     144,   210,   144,   211,   206,   142,   212,   208,   218,   209,
     214,   197,     7,   221,     9,   227,   225,    11,    12,    13,
     144,   144,   226,   145,   228,   145,   229,   222,   223,   230,
     234,     1,    20,    20,   216,   217,   114,   114,   235,   236,
     237,   238,   239,   145,   145,     2,     3,     4,   242,     5,
     243,   241,   247,    20,   251,   231,   252,   114,   114,   114,
     253,    70,   254,     6,    20,   215,   240,     0,   114,   114,
     114,    20,    20,   245,   246,    31,     1,    33,    34,    35,
       0,     7,     8,     9,    10,     0,    11,    12,    13,    14,
      15,    31,    32,    33,    34,    35,     0,    36,     0,    37,
       0,    31,    32,    33,    34,    35,     0,     0,     6,     0,
       0,     0,     0,    36,     0,    37,     0,     0,     0,     0,
     139,     0,    38,    36,     6,    37,     7,     0,     9,     0,
      39,    11,    12,    13,     6,    31,    32,    33,    34,    35,
       0,    31,    32,    33,    34,    35,    39,    31,    32,    33,
      34,    35,     0,     0,     0,     0,    39,    36,     0,    37,
       0,     0,     0,    36,   141,    37,     0,     0,     6,    36,
     194,    37,     0,     0,     6,     0,   207,     0,     0,     0,
       6,    31,    32,    33,    34,    35,     0,   -87,     0,     0,
      39,     0,     0,     0,     0,     0,    39,     0,     0,     0,
      24,     0,    39,    36,    54,    37,    25,     0,     0,     0,
       0,     0,     0,     0,     6,     0,     0,     0,    94,     0,
     -87,   -87,   -87,   -87,   -87,   -87,   -87,   -87,   -87,   -87,
     -87,   -87,   -87,   -87,   -87,   155,    39,     0,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,   180,     0,     0,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    79,     0,     0,    80,     0,    81,
      82,    83,     0,     0,     0,     0,     0,     0,     0,    84,
       0,    85,    86,    87,    88,    89,     0,     0,    79,    90,
      91,    80,     0,    81,    82,    83,     0,     0,     0,     0,
       0,     0,     0,    84,     0,    85,    86,    87,    88,    89,
      79,   107,     0,    80,    91,    81,    82,    83,     0,     0,
       0,     0,     0,     0,     0,    84,     0,    85,    86,    87,
      88,    89,    79,     0,   108,    80,    91,    81,    82,    83,
       0,     0,     0,     0,     0,     0,     0,    84,     0,    85,
      86,    87,    88,    89,    79,     0,   109,    80,    91,    81,
      82,    83,     0,     0,     0,     0,     0,     0,     0,    84,
       0,    85,    86,    87,    88,    89,    79,     0,   116,    80,
      91,    81,    82,    83,     0,     0,     0,     0,     0,     0,
       0,    84,     0,    85,    86,    87,    88,    89,     0,     0,
      79,   134,    91,    80,     0,    81,    82,    83,     0,     0,
       0,     0,     0,     0,     0,    84,     0,    85,    86,    87,
      88,    89,    79,   135,   136,    80,    91,    81,    82,    83,
       0,     0,     0,     0,     0,     0,     0,    84,     0,    85,
      86,    87,    88,    89,    79,     0,     0,    80,    91,    81,
      82,    83,     0,     0,     0,     0,     0,     0,     0,    84,
       0,    85,    86,    87,    88,    89,     0,     0,    79,   165,
      91,    80,     0,    81,    82,    83,     0,     0,     0,     0,
       0,     0,     0,    84,     0,    85,    86,    87,    88,    89,
      79,   169,     0,    80,    91,    81,    82,    83,     0,     0,
       0,     0,     0,     0,     0,    84,     0,    85,    86,    87,
      88,    89,    79,     0,     0,    80,    91,    81,    82,    83,
       0,     0,     0,     0,     0,     0,     0,    84,     0,    85,
      86,    87,    80,    89,    81,    82,    83,     0,    91,     0,
       0,     0,     0,     0,    84,     0,    85,    86,    87,    80,
      89,   -88,   -88,   -88,     0,    91,     0,     0,     0,     0,
       0,   -88,     0,   -88,    86,   -88,     0,    89,     0,     0,
       0,     0,    91
};

static const yytype_int16 yycheck[] =
{
      14,     4,    10,    93,     0,    54,     4,    12,    10,     5,
      25,     5,    44,   143,    34,    25,     4,    25,    26,     0,
      28,    23,    18,    25,    18,    39,    10,   117,    36,    37,
      35,    10,   162,    22,    13,    10,     3,    37,    36,    23,
      48,    25,    50,     4,    23,    34,    25,    10,    23,   139,
      13,    54,    55,    56,     3,     4,    59,    60,    61,    62,
      23,    75,    25,    37,    22,    31,   157,    23,    25,    17,
      25,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,   114,    91,    92,    17,    25,    10,   179,    10,
     104,    25,   106,    25,   102,   103,   104,    33,   112,   106,
      23,    34,    23,   193,    22,    54,   155,    56,    22,     4,
      59,    60,    61,   104,    31,   106,     4,    33,   132,   133,
     169,   153,   171,    34,    34,   132,   133,    11,   136,   220,
     221,    33,     4,    33,    25,    22,    33,   151,    23,    33,
      33,   132,   133,   157,   151,    31,   236,    22,    22,    22,
     241,   242,   243,   149,   150,   149,   150,    33,   172,   167,
     151,   252,   253,   254,    31,   179,    31,    33,    25,    25,
     184,    34,   186,    34,   182,   183,    34,   184,    21,   186,
      34,   172,    54,    31,    56,    31,    33,    59,    60,    61,
     204,   205,    33,   184,    31,   186,    31,   204,   205,    22,
      33,     4,   198,   199,   198,   199,   220,   221,    33,    22,
      22,    22,    34,   204,   205,    18,    19,    20,    34,    22,
      34,    31,    31,   219,    31,   219,    31,   241,   242,   243,
      31,    24,    34,    36,   230,   197,   230,    -1,   252,   253,
     254,   237,   238,   237,   238,     3,     4,     5,     6,     7,
      -1,    54,    55,    56,    57,    -1,    59,    60,    61,    62,
      63,     3,     4,     5,     6,     7,    -1,    25,    -1,    27,
      -1,     3,     4,     5,     6,     7,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    25,    -1,    27,    -1,    -1,    -1,    -1,
      22,    -1,    34,    25,    36,    27,    54,    -1,    56,    -1,
      58,    59,    60,    61,    36,     3,     4,     5,     6,     7,
      -1,     3,     4,     5,     6,     7,    58,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    58,    25,    -1,    27,
      -1,    -1,    -1,    25,    32,    27,    -1,    -1,    36,    25,
      32,    27,    -1,    -1,    36,    -1,    32,    -1,    -1,    -1,
      36,     3,     4,     5,     6,     7,    -1,     4,    -1,    -1,
      58,    -1,    -1,    -1,    -1,    -1,    58,    -1,    -1,    -1,
      17,    -1,    58,    25,     4,    27,    23,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,     4,    -1,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,     4,    58,    -1,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,     4,    -1,    -1,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,     9,    -1,    -1,    12,    -1,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,
      -1,    26,    27,    28,    29,    30,    -1,    -1,     9,    34,
      35,    12,    -1,    14,    15,    16,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    24,    -1,    26,    27,    28,    29,    30,
       9,    32,    -1,    12,    35,    14,    15,    16,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    24,    -1,    26,    27,    28,
      29,    30,     9,    -1,    33,    12,    35,    14,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,    26,
      27,    28,    29,    30,     9,    -1,    33,    12,    35,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,
      -1,    26,    27,    28,    29,    30,     9,    -1,    33,    12,
      35,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    24,    -1,    26,    27,    28,    29,    30,    -1,    -1,
       9,    34,    35,    12,    -1,    14,    15,    16,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    24,    -1,    26,    27,    28,
      29,    30,     9,    32,    11,    12,    35,    14,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,    26,
      27,    28,    29,    30,     9,    -1,    -1,    12,    35,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,
      -1,    26,    27,    28,    29,    30,    -1,    -1,     9,    34,
      35,    12,    -1,    14,    15,    16,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    24,    -1,    26,    27,    28,    29,    30,
       9,    32,    -1,    12,    35,    14,    15,    16,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    24,    -1,    26,    27,    28,
      29,    30,     9,    -1,    -1,    12,    35,    14,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,    26,
      27,    28,    12,    30,    14,    15,    16,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    24,    -1,    26,    27,    28,    12,
      30,    14,    15,    16,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    24,    -1,    26,    27,    28,    -1,    30,    -1,    -1,
      -1,    -1,    35
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     4,    18,    19,    20,    22,    36,    54,    55,    56,
      57,    59,    60,    61,    62,    63,    67,    68,    69,    73,
      74,    76,    77,    78,    17,    23,    25,    34,    25,    68,
       4,     3,     4,     5,     6,     7,    25,    27,    34,    58,
      72,    74,     3,     4,    78,     3,     0,    68,    10,    13,
      23,    25,    37,    37,     4,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    71,
      77,    72,    72,    72,    31,    22,    72,    72,    78,     9,
      12,    14,    15,    16,    24,    26,    27,    28,    29,    30,
      34,    35,    10,    25,     4,    71,    72,     4,    36,    72,
      72,    81,    10,    23,    25,    70,    25,    32,    33,    33,
       4,    55,    62,    75,    78,    79,    33,    25,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    81,    25,    25,    34,    32,    11,    80,    33,    22,
      72,    32,    72,    72,    78,    79,    83,    34,    83,    22,
      22,    25,     4,    78,    31,     4,    71,    34,    81,    33,
      83,    83,    72,    34,    81,    34,    10,    23,    70,    32,
      80,     4,    11,    82,    33,    33,    68,    68,    83,    22,
       4,    71,    10,    23,    25,    70,    25,    75,    33,    33,
      33,    80,    31,    22,    32,    70,    33,    79,    22,    22,
      31,    31,    33,    75,    25,    25,    72,    32,    83,    83,
      34,    34,    34,    81,    34,    82,    68,    68,    21,    22,
      34,    31,    83,    83,    10,    33,    33,    31,    31,    31,
      22,    68,    75,    75,    33,    33,    22,    22,    22,    34,
      68,    31,    34,    34,    81,    68,    68,    31,    75,    75,
      75,    31,    31,    31,    34,    75,    75,    75
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
#line 144 "parser.yy"
	{ delete (yyvaluep->str); };

/* Line 1000 of yacc.c  */
#line 1545 "parser.tab.cc"
	break;
      case 8: /* "\"symbol\"" */

/* Line 1000 of yacc.c  */
#line 145 "parser.yy"
	{ delete (*yyvaluep).symbol; };

/* Line 1000 of yacc.c  */
#line 1554 "parser.tab.cc"
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
#line 95 "parser.yy"
{
	yydebug = tp._parse_trace_p;
   /**
    setup the filename each time before parsing
    */
   static int mvvParserInputNumber = 0;
   yylloc.filename = mvv::Symbol::create( tp._filename == "" ? "(input" + nll::core::val2str( mvvParserInputNumber++ ) + ")" : tp._filename );
}

/* Line 1242 of yacc.c  */
#line 1714 "parser.tab.cc"
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
#line 231 "parser.yy"
    { tp._root = (yyvsp[(1) - (1)].astStatements); ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 233 "parser.yy"
    { (yyval.astStatements) = new mvv::parser::AstStatements( (yyloc) ); ;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 234 "parser.yy"
    { (yyval.astStatements) = (yyvsp[(2) - (2)].astStatements); (yyvsp[(2) - (2)].astStatements)->insert( (yyvsp[(1) - (2)].ast) ); ;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 236 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstIf( (yyloc), (yyvsp[(3) - (7)].astExp), (yyvsp[(6) - (7)].astStatements), 0 ); ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 237 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstIf( (yyloc), (yyvsp[(3) - (11)].astExp), (yyvsp[(6) - (11)].astStatements), (yyvsp[(10) - (11)].astStatements) ); ;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 238 "parser.yy"
    { mvv::parser::AstDeclClass* decl = new mvv::parser::AstDeclClass( (yyloc), *(yyvsp[(2) - (5)].symbol), (yyvsp[(4) - (5)].astDecls) ); (yyval.ast) = decl; linkFunctionToClass( *decl ); ;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 239 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstWhile( (yyloc), (yyvsp[(3) - (7)].astExp), (yyvsp[(6) - (7)].astStatements) ); ;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 240 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstBreak( (yyloc) ); ;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 246 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (5)].astTypeT), *(yyvsp[(2) - (5)].symbol), (yyvsp[(4) - (5)].astExp) ); ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 247 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (7)].astTypeT), *(yyvsp[(2) - (7)].symbol), 0, (yyvsp[(5) - (7)].astArgs) ); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 248 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(2) - (7)].astTypeT), *(yyvsp[(3) - (7)].symbol), (yyvsp[(5) - (7)].astDeclVars) ); ;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 249 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (8)].astTypeT), *(yyvsp[(2) - (8)].symbol), (yyvsp[(4) - (8)].astDeclVars), (yyvsp[(7) - (8)].astStatements) ); ;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 250 "parser.yy"
    { /* first, concatenate the args*/
                                                                       mvv::parser::AstArgs* args = (yyvsp[(5) - (7)].astArgs);
                                                                       args->getArgs().push_front( (yyvsp[(4) - (7)].astExp) );
                                                                       (yyval.ast) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (7)].astTypeT), *(yyvsp[(2) - (7)].symbol), 0, 0, args );
                                                                     ;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 256 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (4)].astTypeT), *(yyvsp[(2) - (4)].symbol) );
                                                                       if ( (yyvsp[(3) - (4)].arrayDim)->size() )
                                                                       {
                                                                          (yyvsp[(1) - (4)].astTypeT)->setArray( true );
                                                                          (yyvsp[(1) - (4)].astTypeT)->setSize( (yyvsp[(3) - (4)].arrayDim) );
                                                                       } 
                                                                     ;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 264 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (9)].astTypeT), *(yyvsp[(2) - (9)].symbol), 0, (yyvsp[(7) - (9)].astArgs) ); (yyvsp[(1) - (9)].astTypeT)->setArray( true ); /* we don't handle several dimensions*/ ;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 265 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstExpAssign( (yyloc), (yyvsp[(1) - (4)].astVar), (yyvsp[(3) - (4)].astExp) ); ;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 266 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstExpCall( (yyloc), (yyvsp[(1) - (5)].astVar), (yyvsp[(3) - (5)].astArgs) ); ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 267 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstReturn( (yyloc), (yyvsp[(2) - (3)].astExp) ); ;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 268 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstReturn( (yyloc) ); ;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 269 "parser.yy"
    { (yyval.ast) = (yyvsp[(2) - (3)].astStatements); ;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 270 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstImport( (yyloc), *(yyvsp[(2) - (2)].str) ); ;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 271 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstInclude( (yyloc), *(yyvsp[(2) - (2)].str) ); ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 274 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (8)].astTypeT), *(yyvsp[(2) - (8)].symbol), (yyvsp[(4) - (8)].astDeclVars), (yyvsp[(7) - (8)].astStatements) ); ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 275 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(2) - (7)].astTypeT), *(yyvsp[(3) - (7)].symbol), (yyvsp[(5) - (7)].astDeclVars) ); ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 279 "parser.yy"
    { (yyval.arrayDim) = new std::vector<mvv::parser::AstExp*>(); ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 280 "parser.yy"
    { (yyval.arrayDim) = (yyvsp[(4) - (4)].arrayDim); (yyval.arrayDim)->push_back( (yyvsp[(2) - (4)].astExp) ); ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 281 "parser.yy"
    { (yyval.arrayDim) = (yyvsp[(3) - (3)].arrayDim); (yyval.arrayDim)->push_back( new mvv::parser::AstInt( (yyloc), 0 ) ); ;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 283 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator+" ) ); ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 284 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator-" ) ); ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 285 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator*" ) ); ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 286 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator/" ) ); ;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 287 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator<" ) ); ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 288 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator>" ) ); ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 289 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator<=" ) ); ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 290 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator>=" ) ); ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 291 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator==" ) ); ;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 292 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator!=" ) ); ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 293 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator&&" ) ); ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 294 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator||" ) ); ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 295 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator[]" ) ); ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 296 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator()" ) ); ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 300 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstInt( (yyloc), (yyvsp[(1) - (1)].ival) ); ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 301 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstFloat( (yyloc), (yyvsp[(1) - (1)].fval) ); ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 302 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstNil( (yyloc) ); ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 303 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::PLUS ); ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 304 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::MINUS ); ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 305 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::TIMES ); ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 306 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::DIVIDE ); ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 307 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::AND ); ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 308 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::OR ); ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 309 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::LT ); ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 310 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::LE ); ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 311 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::GT ); ;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 312 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::GE ); ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 313 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::NE ); ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 314 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::EQ ); ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 315 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), new mvv::parser::AstInt( (yyloc), 0 ) , (yyvsp[(2) - (2)].astExp), mvv::parser::AstOpBin::MINUS ); ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 316 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstExpSeq( (yyloc), (yyvsp[(2) - (3)].astExp) ); ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 317 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstString( (yyloc), *(yyvsp[(1) - (1)].str) ); ;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 318 "parser.yy"
    { (yyval.astExp) = (yyvsp[(1) - (1)].astVar); ;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 319 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstExpAssign( (yyloc), (yyvsp[(1) - (3)].astVar), (yyvsp[(3) - (3)].astExp) ); ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 320 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstExpTypename( (yyloc), (yyvsp[(2) - (5)].astTypeT), (yyvsp[(4) - (5)].astArgs) );;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 323 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstThis( (yyloc) ); ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 324 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarField( (yyloc), (yyvsp[(1) - (3)].astVar), mvv::Symbol::create( "this" ) ); ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 326 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarSimple( (yyloc), *(yyvsp[(1) - (1)].symbol), true ); ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 327 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarArray( (yyloc), new mvv::parser::AstVarSimple( (yyloc), *(yyvsp[(1) - (4)].symbol), true ), (yyvsp[(3) - (4)].astExp) ); ;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 328 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarField( (yyloc), (yyvsp[(1) - (3)].astVar), *(yyvsp[(3) - (3)].symbol) ); ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 329 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarArray( (yyloc), (yyvsp[(1) - (4)].astVar), (yyvsp[(3) - (4)].astExp) ); ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 330 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstExpCall( (yyloc), (yyvsp[(1) - (4)].astVar), (yyvsp[(3) - (4)].astArgs) ); ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 331 "parser.yy"
    { (yyval.astVar) = (yyvsp[(1) - (1)].astVar); ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 335 "parser.yy"
    { (yyval.astDecls) = new mvv::parser::AstDecls( (yyloc) ); ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 336 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(3) - (3)].astDecls); (yyval.astDecls)->insert( (yyvsp[(1) - (3)].astDeclVar) ); ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 337 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(10) - (10)].astDecls); mvv::parser::AstDeclVar* var = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (10)].astTypeT), *(yyvsp[(2) - (10)].symbol), 0, (yyvsp[(7) - (10)].astArgs) ); (yyvsp[(1) - (10)].astTypeT)->setArray( true ); (yyval.astDecls)->insert( var ); ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 338 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(6) - (6)].astDecls); mvv::parser::AstDeclClass* decl = new mvv::parser::AstDeclClass( (yyloc), *(yyvsp[(2) - (6)].symbol), (yyvsp[(4) - (6)].astDecls) ); (yyval.astDecls)->insert( decl ); linkFunctionToClass( *decl ); ;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 339 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), *(yyvsp[(2) - (9)].symbol), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 340 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(8) - (8)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(2) - (8)].astTypeT), *(yyvsp[(3) - (8)].symbol), (yyvsp[(5) - (8)].astDeclVars) ) ); ;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 341 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(8) - (8)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), 0, *(yyvsp[(1) - (8)].symbol), (yyvsp[(3) - (8)].astDeclVars), (yyvsp[(6) - (8)].astStatements) ) ); ;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 342 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(6) - (6)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), 0, *(yyvsp[(1) - (6)].symbol), (yyvsp[(3) - (6)].astDeclVars) ) ); ;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 343 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), *(yyvsp[(2) - (9)].symbol), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 344 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(8) - (8)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(2) - (8)].astTypeT), *(yyvsp[(3) - (8)].symbol), (yyvsp[(5) - (8)].astDeclVars) ) ); ;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 347 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::VAR ); ;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 348 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::INT ); ;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 349 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::FLOAT );;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 350 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::STRING ); ;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 351 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::VOID );;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 353 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::SYMBOL, (yyvsp[(1) - (1)].symbol) ); ;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 354 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstTypeField( (yyloc), (yyvsp[(3) - (3)].astTypeT), *(yyvsp[(1) - (3)].symbol) ); ;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 356 "parser.yy"
    { (yyval.astTypeT) = (yyvsp[(1) - (1)].astTypeT); ;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 357 "parser.yy"
    { (yyval.astTypeT) = (yyvsp[(1) - (1)].astTypeT); ;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 358 "parser.yy"
    { (yyval.astTypeT) = (yyvsp[(1) - (2)].astTypeT); (yyvsp[(1) - (2)].astTypeT)->setIsAReference( true ); ;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 359 "parser.yy"
    { (yyval.astTypeT) = (yyvsp[(1) - (2)].astTypeT); (yyvsp[(1) - (2)].astTypeT)->setIsAReference( true ); ;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 362 "parser.yy"
    { (yyval.astDeclVar) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (4)].astTypeT), *(yyvsp[(2) - (4)].symbol), (yyvsp[(4) - (4)].astExp) ); ;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 363 "parser.yy"
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
#line 372 "parser.yy"
    { (yyval.astArgs) = new mvv::parser::AstArgs( (yyloc) ); ;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 373 "parser.yy"
    { (yyval.astArgs) = (yyvsp[(3) - (3)].astArgs); (yyval.astArgs)->insert( (yyvsp[(2) - (3)].astExp) ); ;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 375 "parser.yy"
    { (yyval.astArgs) = new mvv::parser::AstArgs( (yyloc) ); ;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 376 "parser.yy"
    { (yyval.astArgs) = (yyvsp[(2) - (2)].astArgs); (yyval.astArgs)->insert( (yyvsp[(1) - (2)].astExp) ); ;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 378 "parser.yy"
    { (yyval.astDeclVars) = new mvv::parser::AstDeclVars( (yyloc) ); ;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 379 "parser.yy"
    { (yyval.astDeclVars) = (yyvsp[(3) - (3)].astDeclVars); (yyval.astDeclVars)->insert( (yyvsp[(2) - (3)].astDeclVar) ); ;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 381 "parser.yy"
    { (yyval.astDeclVars) = new mvv::parser::AstDeclVars( (yyloc) ); ;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 382 "parser.yy"
    { (yyval.astDeclVars) = (yyvsp[(2) - (2)].astDeclVars); (yyval.astDeclVars)->insert( (yyvsp[(1) - (2)].astDeclVar) ); ;}
    break;



/* Line 1455 of yacc.c  */
#line 2625 "parser.tab.cc"
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
#line 384 "parser.yy"


