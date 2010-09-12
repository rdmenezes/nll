/* A Bison parser, made by GNU Bison 2.4.2.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
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
#define YYBISON_VERSION "2.4.2"

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
    - a name of function/variable/class must be unique, else confusing => to be changed
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
    - fn & var CAN have the same name (except at global scope) => binding priority is always var name
   
    
    - check function decl inside function
    - TODO=> not valid anymore? int a[ 5 ]; float b[ 5 ]; a = b; => we copy the content of b in a and cast if necessary
    - no typedef declared inside function->else need to develop a 'temporary type', additionally it helps with the function pointer typedef
    
    
    principles:
    - int, float, string are primitive types. the are copied by value
    - structures are copied by reference and are automatically allocated/deallocated using ref counting
    - operator= can't be overloaded: just refcounting. To create a new instance, just recreate another object...
    - operator== checks the address for types... and not the semantic==
    - the type of a function doesn't belong to the signature of the function, the signature of a function must be unique
    - a type can be referenced. In this case the reference is directly modified. If a type which is not a primitive
      is referenced & used as argument in a function call, refcounting is not updated
    - include: the included file will be parsed, inclusion order doesn't matter
    - import: the source file will be imported & dll dynamically loaded (with the same name) & entry point run
    - import function: can be from class/global function. Because all declarations must be loaded in memory, it can't be declared multiple times
    - array are also refcounted so they can be shared quickly
    - multidimentional arrays can have a several unknow dim
    - includes/import are evaluated first
    - destructor: will be invoked very soon after the end of life of the object. However as the object
                  is stored using smart pointers, they might still have a ref in the result register!
    - reference: in a class (TODO extend?) a reference will be taken to the first assignment if it is not initialized
    - default init for var member in a class [array (the array itself, not the contained values), object, default member init], other are not initilized
    - When an object is constructed: first, initalize automatic variables (member variable (init, class construction, array))
      then call the constructor.
    - automatic construction in class members. Problem of cyclic dependencies: the member must be initialized by NULL
    - arrays are dynamic: int a[ 5 ]; int b[ 400 ]; a = b; is correct!    
    - when importing custom type: if a destructor has to be called, we MUST modify in the constructor the original object and NOT returning a new one! (and so might need to resize the vector)
    - int n = Test().array[ 1 ]; => WILL crash. An object is created and stored in the temporary register, however, we need to use this register to index the array and lost the ref on the object which has been deallocated
    
    - TODO typedef printing to fix
    - possibly TODO: if we authorize to pass array reference, we need to update the _forceUnref in AstExpAssign (like for typedef int[] IntArray; void fn( IntArray& a ){ int aa[5]; aa[ 0 ] = 18; a = aa;}  IntArray nn; fn( nn ); int n = nn[0]; --but with -- "void fn( int[]& a )" )
    - TODO can't mix array & ref? check what should be the behaviour
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
 
 bool hasDefaultValue( const mvv::parser::AstDeclVars& vars )
 {
   for ( unsigned n = 0; n < vars.getVars().size(); ++n )
   {
      if ( vars.getVars()[ n ]->getInit() )
         return true;
   }
   return false;
 }


/* Line 189 of yacc.c  */
#line 178 "parser.tab.cc"

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

/* Line 214 of yacc.c  */
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



/* Line 214 of yacc.c  */
#line 306 "parser.tab.cc"
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
#line 331 "parser.tab.cc"

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
# if defined YYENABLE_NLS && YYENABLE_NLS
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
#define YYFINAL  52
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   998

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  72
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  18
/* YYNRULES -- Number of rules.  */
#define YYNRULES  115
/* YYNRULES -- Number of states.  */
#define YYNSTATES  308

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   326

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
      65,    66,    67,    68,    69,    70,    71
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     6,     9,    17,    29,    35,    43,
      46,    51,    59,    65,    73,    81,    90,    98,   103,   113,
     118,   124,   128,   131,   135,   138,   141,   150,   158,   161,
     162,   167,   171,   173,   175,   177,   179,   181,   183,   185,
     187,   189,   191,   193,   195,   197,   199,   201,   203,   205,
     207,   209,   213,   217,   221,   225,   229,   233,   237,   241,
     245,   249,   253,   257,   261,   265,   268,   272,   274,   276,
     280,   286,   288,   292,   294,   299,   303,   308,   313,   315,
     316,   320,   331,   338,   348,   357,   366,   374,   384,   393,
     401,   410,   416,   425,   427,   429,   431,   433,   435,   437,
     441,   443,   445,   448,   451,   455,   459,   464,   468,   469,
     473,   474,   477,   478,   482,   483
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      73,     0,    -1,    74,    -1,    -1,    75,    74,    -1,    21,
      26,    78,    34,    23,    74,    32,    -1,    21,    26,    78,
      34,    23,    74,    32,    22,    23,    74,    32,    -1,    60,
       4,    23,    81,    32,    -1,    19,    26,    78,    34,    23,
      74,    32,    -1,    20,    35,    -1,    64,    84,     4,    35,
      -1,    64,    84,    26,    89,    34,     4,    35,    -1,    84,
       4,    11,    78,    35,    -1,    84,     4,    11,    23,    87,
      32,    35,    -1,    68,    84,     4,    26,    89,    34,    35,
      -1,    84,     4,    26,    89,    34,    23,    74,    32,    -1,
      84,     4,    26,    78,    86,    34,    35,    -1,    84,     4,
      76,    35,    -1,    84,     4,    24,    33,    11,    23,    87,
      32,    35,    -1,    80,    11,    78,    35,    -1,    80,    26,
      87,    34,    35,    -1,    62,    78,    35,    -1,    62,    35,
      -1,    23,    74,    32,    -1,    68,     3,    -1,    69,     3,
      -1,    84,    77,    26,    89,    34,    23,    74,    32,    -1,
      68,    84,    77,    26,    89,    34,    35,    -1,    78,    35,
      -1,    -1,    24,    78,    33,    76,    -1,    24,    33,    76,
      -1,    43,    -1,    44,    -1,    45,    -1,    46,    -1,    47,
      -1,    48,    -1,    49,    -1,    50,    -1,    51,    -1,    52,
      -1,    53,    -1,    54,    -1,    42,    -1,    41,    -1,    55,
      -1,    56,    -1,     5,    -1,     6,    -1,     7,    -1,    78,
      39,    78,    -1,    78,    40,    78,    -1,    78,    31,    78,
      -1,    78,    28,    78,    -1,    78,    36,    78,    -1,    78,
      13,    78,    -1,    78,    10,    78,    -1,    78,    30,    78,
      -1,    78,    27,    78,    -1,    78,    25,    78,    -1,    78,
      17,    78,    -1,    78,    16,    78,    -1,    78,    29,    78,
      -1,    78,    15,    78,    -1,    28,    78,    -1,    26,    78,
      34,    -1,     3,    -1,    80,    -1,    80,    11,    78,    -1,
      63,    84,    26,    87,    34,    -1,    37,    -1,    80,    14,
      37,    -1,     4,    -1,     4,    24,    78,    33,    -1,    80,
      14,     4,    -1,    80,    24,    78,    33,    -1,    80,    26,
      87,    34,    -1,    79,    -1,    -1,    85,    35,    81,    -1,
      84,     4,    24,    33,    11,    23,    87,    32,    35,    81,
      -1,    60,     4,    23,    81,    32,    81,    -1,    84,     4,
      26,    89,    34,    23,    74,    32,    81,    -1,    68,    84,
       4,    26,    89,    34,    35,    81,    -1,     4,    26,    89,
      34,    23,    74,    32,    81,    -1,    68,     4,    26,    89,
      34,    35,    81,    -1,    84,    77,    26,    89,    34,    23,
      74,    32,    81,    -1,    68,    84,    77,    26,    89,    34,
      35,    81,    -1,    68,     9,     4,    26,    34,    35,    81,
      -1,     9,     4,    26,    34,    23,    74,    32,    81,    -1,
      64,    84,     4,    35,    81,    -1,    64,    84,    26,    89,
      34,     4,    35,    81,    -1,    59,    -1,    65,    -1,    66,
      -1,    67,    -1,    61,    -1,     4,    -1,     4,    18,    83,
      -1,    83,    -1,    82,    -1,    82,    38,    -1,    83,    38,
      -1,    83,    24,    33,    -1,    82,    24,    33,    -1,    84,
       4,    11,    78,    -1,    84,     4,    76,    -1,    -1,    12,
      78,    86,    -1,    -1,    78,    86,    -1,    -1,    12,    85,
      88,    -1,    -1,    85,    88,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   260,   260,   262,   263,   265,   266,   267,   268,   269,
     270,   271,   284,   285,   286,   287,   288,   294,   302,   303,
     304,   305,   306,   307,   308,   309,   312,   313,   314,   318,
     319,   320,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   366,   367,   369,   370,   371,   372,   373,   374,   378,
     379,   381,   382,   383,   384,   385,   386,   387,   388,   389,
     390,   391,   392,   401,   402,   403,   404,   405,   407,   408,
     410,   411,   412,   413,   414,   415,   418,   419,   438,   439,
     441,   442,   444,   445,   447,   448
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "$undefined", "\"string\"",
  "\"identifier\"", "\"integer\"", "\"float\"", "\"NULL\"", "\"symbol\"",
  "\"~\"", "\"&&\"", "\"=\"", "\",\"", "\"/\"", "\".\"", "\"==\"",
  "\">=\"", "\">\"", "\"::\"", "\"while\"", "\"break\"", "\"if\"",
  "\"else\"", "\"{\"", "\"[\"", "\"<=\"", "\"(\"", "\"<\"", "\"-\"",
  "\"!=\"", "\"||\"", "\"+\"", "\"}\"", "\"]\"", "\")\"", "\";\"", "\"*\"",
  "\"this\"", "\"&\"", "\"<<\"", "\">>\"", "\"operator()\"",
  "\"operator[]\"", "\"operator+\"", "\"operator-\"", "\"operator*\"",
  "\"operator/\"", "\"operator<\"", "\"operator>\"", "\"operator<=\"",
  "\"operator>=\"", "\"operator==\"", "\"operator!=\"", "\"operator&&\"",
  "\"operator||\"", "\"operator<<\"", "\"operator>>\"", "\"for\"",
  "\"in\"", "\"var\"", "\"class\"", "\"void\"", "\"return\"",
  "\"typename\"", "\"typedef\"", "\"int type\"", "\"float type\"",
  "\"string type\"", "\"import\"", "\"include\"", "IFX", "UMINUS",
  "$accept", "program", "statements", "statement", "array_decl",
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
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    72,    73,    74,    74,    75,    75,    75,    75,    75,
      75,    75,    75,    75,    75,    75,    75,    75,    75,    75,
      75,    75,    75,    75,    75,    75,    75,    75,    75,    76,
      76,    76,    77,    77,    77,    77,    77,    77,    77,    77,
      77,    77,    77,    77,    77,    77,    77,    77,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    78,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    78,    78,    78,
      78,    79,    79,    80,    80,    80,    80,    80,    80,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    82,    82,    82,    82,    82,    83,    83,
      84,    84,    84,    84,    84,    84,    85,    85,    86,    86,
      87,    87,    88,    88,    89,    89
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     0,     2,     7,    11,     5,     7,     2,
       4,     7,     5,     7,     7,     8,     7,     4,     9,     4,
       5,     3,     2,     3,     2,     2,     8,     7,     2,     0,
       4,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     3,     1,     1,     3,
       5,     1,     3,     1,     4,     3,     4,     4,     1,     0,
       3,    10,     6,     9,     8,     8,     7,     9,     8,     7,
       8,     5,     8,     1,     1,     1,     1,     1,     1,     3,
       1,     1,     2,     2,     3,     3,     4,     3,     0,     3,
       0,     2,     0,     3,     0,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,    67,    73,    48,    49,    50,     0,     0,     0,     3,
       0,     0,    71,    93,     0,    97,     0,     0,     0,    94,
      95,    96,     0,     0,     0,     2,     3,     0,    78,    68,
     101,   100,     0,     0,     0,     0,     9,     0,     0,    73,
       0,    68,    65,     0,    22,     0,    98,     0,     0,    24,
       0,    25,     1,     4,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    28,     0,     0,     0,     0,
       0,     0,   110,     0,   102,     0,   103,    29,    45,    44,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    46,    47,     0,    99,     0,     0,     0,    23,
      66,     0,   110,    79,    21,   110,     0,   114,     0,     0,
      57,    56,    64,    62,    61,    60,    59,    54,    63,    58,
      53,    55,    51,    52,    69,    75,    72,     0,   108,     0,
     105,   104,     0,     0,   114,     0,   114,    74,     0,     0,
      69,     0,    98,     0,     0,     0,     0,     0,     0,     0,
       0,    10,     0,   112,     0,   114,   114,    19,    76,     0,
     111,    77,   110,     0,    29,     0,   108,     0,    17,     0,
       3,     3,    77,   114,     0,     0,     0,    98,     0,     0,
       7,    29,     0,    79,    70,    29,     0,   115,     0,     0,
       0,   108,    20,     0,    12,     0,     0,    31,    29,     0,
       0,     0,     0,     0,     0,     0,    79,     0,   114,   114,
       0,     0,     0,     0,     0,   114,   107,   114,    80,   112,
       0,     0,     0,   109,     0,   110,    29,    30,     0,     3,
       3,     8,     5,     0,     0,     0,    79,     0,     0,     0,
     114,   114,   106,    29,     0,     0,   113,    11,    14,    27,
      13,     0,    16,     0,     0,     0,     3,     3,    79,    91,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      26,     3,     0,     0,    82,     0,    79,    79,     0,     0,
     110,     3,     3,    18,     0,    79,    79,    79,    86,    89,
      79,    79,     0,     0,     0,     6,    85,    90,    92,    84,
      88,     0,    79,    79,    79,    83,    87,    81
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    24,    25,    26,   197,    94,    27,    28,    41,   147,
      30,    31,   148,   149,   160,   129,   187,   154
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -156
static const yytype_int16 yypact[] =
{
     310,  -156,   442,  -156,  -156,  -156,   -11,    -8,    13,   310,
     444,   444,  -156,  -156,    47,  -156,   354,   287,   287,  -156,
    -156,  -156,    44,    51,    60,  -156,   310,   595,  -156,    35,
       7,    77,   469,    62,   444,   444,  -156,   444,    56,    68,
     623,   113,  -156,    74,  -156,   651,    90,    69,    -1,  -156,
     495,  -156,  -156,  -156,   444,   444,   444,   444,   444,   444,
     444,   444,   444,   444,   444,  -156,   444,   444,   444,   444,
      -2,   444,   444,    79,  -156,    83,  -156,    72,  -156,  -156,
    -156,  -156,  -156,  -156,  -156,  -156,  -156,  -156,  -156,  -156,
    -156,  -156,  -156,  -156,    87,  -156,   679,   707,   735,  -156,
    -156,   444,   444,     3,  -156,   444,    82,   287,    95,    96,
     945,  -156,   962,   962,   962,   962,   962,     1,   962,   928,
       1,  -156,   906,   906,   427,  -156,  -156,   763,   794,    98,
    -156,  -156,   360,   396,   185,    94,   287,  -156,   110,   112,
     906,   106,     6,   137,   138,   287,   134,   115,   522,   109,
     114,  -156,   145,   141,   120,   287,   287,  -156,  -156,   444,
    -156,   122,   444,   822,     9,   850,   794,   124,  -156,   125,
     310,   310,  -156,   287,   135,   139,    12,    24,   156,   548,
    -156,    80,   140,     3,  -156,    32,   287,  -156,   159,   130,
     131,   794,  -156,   144,  -156,   146,   402,  -156,   148,   147,
     155,   161,   150,   153,   160,   162,     3,   152,   287,   287,
     171,   172,   177,   444,   408,   287,  -156,   287,  -156,   141,
     169,   170,   181,  -156,   183,   444,   148,  -156,   184,   310,
     310,  -156,   157,   189,   197,   174,     3,   173,   187,   190,
     287,   287,   878,    33,   194,   201,  -156,  -156,  -156,  -156,
    -156,   191,  -156,   204,   206,   219,   310,   310,     3,  -156,
     241,   212,   214,   228,   229,   242,   243,   244,   234,  -156,
    -156,   310,   232,   238,  -156,   237,     3,     3,   239,   245,
     444,   310,   310,  -156,   251,     3,     3,     3,  -156,  -156,
       3,     3,   252,   255,   256,  -156,  -156,  -156,  -156,  -156,
    -156,   257,     3,     3,     3,  -156,  -156,  -156
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -156,  -156,    -3,  -156,   -67,   -49,    18,  -156,     4,    19,
    -156,   240,     0,   103,  -155,   -97,    71,  -115
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -99
static const yytype_int16 yytable[] =
{
      32,   109,   125,   106,    29,   141,    38,   142,   150,    32,
     135,   199,   143,    29,    55,    35,   207,    47,    48,   167,
     195,   169,    50,    53,    33,   107,    32,    36,    40,    42,
      29,    73,   173,   196,    45,   126,   223,    66,   208,    37,
     189,   190,    33,   213,   265,    74,    69,    49,    46,    70,
     209,    43,    96,    97,    51,    98,   196,   196,   204,    71,
      52,    72,    13,   144,    15,   193,    46,   145,    19,    20,
      21,   146,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   132,   121,   122,   123,   124,    99,   127,
     128,   213,    34,   237,   238,   105,   133,   103,   134,   182,
     244,    75,   245,    13,   214,    15,   215,   152,    33,    19,
      20,    21,   130,   136,   216,    76,   131,   151,   216,   140,
     128,   155,   156,   128,   101,   263,   264,    70,   251,   168,
     212,   227,   161,   170,   152,   171,   152,    71,   177,   102,
     172,   174,   175,   178,   183,   176,   179,   180,   184,   185,
     163,   165,   166,   186,   188,   152,   152,   192,   200,   201,
     210,   205,   206,   220,   221,   222,   217,   202,   203,   225,
      32,    32,   196,   152,    29,    29,   224,   191,   229,   255,
     128,   228,   231,   292,   230,   232,   152,   236,     1,     2,
       3,     4,     5,    13,   233,    15,   234,   239,   240,    19,
      20,    21,   218,   241,   247,   248,   258,   260,   152,   152,
     153,    10,   256,    11,   165,   152,   249,   152,   250,   252,
     257,   261,    12,   268,   262,   235,   253,   254,   266,    32,
      32,   242,   165,    29,    29,   267,   269,   153,   270,   153,
     152,   152,   271,   128,    13,   275,    15,   276,    17,   277,
      19,    20,    21,   272,   273,   259,    32,    32,   153,   153,
      29,    29,   278,   279,   285,   280,   281,   282,   284,   283,
     286,    32,   287,    95,   290,    29,   153,   274,   293,   294,
     291,    32,    32,   295,   301,    29,    29,   302,   303,   219,
     246,    46,   304,     0,     0,   288,   289,     0,   128,     0,
       0,     0,     0,     0,   296,   297,   298,     0,     0,   299,
     300,   153,   153,     1,     2,     3,     4,     5,   153,     0,
     153,   305,   306,   307,     0,     0,     0,     0,     0,     6,
       7,     8,     0,     9,     0,     0,    10,     0,    11,     0,
       0,     0,     0,   153,   153,     0,    13,    12,    15,     0,
       0,     0,    19,    20,    21,     0,     0,     1,    39,     3,
       4,     5,     0,     1,    39,     3,     4,     5,     0,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      10,     0,    11,   162,     0,     0,    10,     0,    11,    44,
       0,    12,     0,     0,     0,     0,     0,    12,     0,     1,
      39,     3,     4,     5,     0,     1,    39,     3,     4,     5,
       0,     1,    39,     3,     4,     5,     0,    17,     0,     0,
       0,     0,    10,    17,    11,     0,     0,     0,    10,   164,
      11,     0,     0,    12,    10,   226,    11,    54,     0,    12,
      55,   243,    56,    57,    58,    12,   -98,     1,    39,     3,
       4,     5,    59,     0,    60,    61,    62,    63,    64,    17,
      33,     0,   157,    66,     0,    17,    34,     0,     0,     0,
      10,    17,    11,    77,     0,     0,     0,     0,     0,     0,
     -98,    12,     0,   -98,   -98,   -98,   -98,   -98,   -98,   -98,
     -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   108,
       0,     0,     0,     0,     0,     0,     0,    17,     0,     0,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,   181,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,   211,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    54,     0,     0,    55,     0,
      56,    57,    58,     0,     0,     0,     0,     0,     0,     0,
      59,     0,    60,    61,    62,    63,    64,     0,     0,     0,
      65,    66,     0,    54,    67,    68,    55,     0,    56,    57,
      58,     0,     0,     0,     0,     0,     0,     0,    59,     0,
      60,    61,    62,    63,    64,     0,     0,   100,     0,    66,
       0,    54,    67,    68,    55,     0,    56,    57,    58,     0,
       0,     0,     0,     0,     0,     0,    59,     0,    60,    61,
      62,    63,    64,     0,     0,     0,   104,    66,     0,    54,
      67,    68,    55,     0,    56,    57,    58,     0,     0,     0,
       0,     0,     0,     0,    59,     0,    60,    61,    62,    63,
      64,     0,   137,     0,     0,    66,     0,    54,    67,    68,
      55,     0,    56,    57,    58,     0,     0,     0,     0,     0,
       0,     0,    59,     0,    60,    61,    62,    63,    64,     0,
       0,   138,     0,    66,     0,    54,    67,    68,    55,     0,
      56,    57,    58,     0,     0,     0,     0,     0,     0,     0,
      59,     0,    60,    61,    62,    63,    64,     0,     0,   139,
       0,    66,     0,    54,    67,    68,    55,     0,    56,    57,
      58,     0,     0,     0,     0,     0,     0,     0,    59,     0,
      60,    61,    62,    63,    64,     0,   158,     0,     0,    66,
       0,     0,    67,    68,    54,     0,   159,    55,     0,    56,
      57,    58,     0,     0,     0,     0,     0,     0,     0,    59,
       0,    60,    61,    62,    63,    64,     0,     0,     0,     0,
      66,     0,    54,    67,    68,    55,     0,    56,    57,    58,
       0,     0,     0,     0,     0,     0,     0,    59,     0,    60,
      61,    62,    63,    64,     0,     0,     0,   194,    66,     0,
      54,    67,    68,    55,     0,    56,    57,    58,     0,     0,
       0,     0,     0,     0,     0,    59,     0,    60,    61,    62,
      63,    64,     0,   198,     0,     0,    66,     0,    54,    67,
      68,    55,     0,    56,    57,    58,     0,     0,     0,     0,
       0,     0,     0,    59,     0,    60,    61,    62,    63,    64,
       0,     0,     0,     0,    66,     0,    54,    67,    68,    55,
       0,    56,    57,    58,     0,     0,     0,     0,     0,     0,
       0,    59,     0,    60,    61,    62,    63,    64,    54,     0,
       0,    55,    66,    56,    57,    58,     0,     0,     0,     0,
       0,     0,     0,    59,     0,    60,    61,    62,    55,    64,
      56,    57,    58,     0,    66,     0,     0,     0,     0,     0,
      59,     0,    60,    61,    62,    55,    64,   -99,   -99,   -99,
       0,    66,     0,     0,     0,     0,     0,   -99,     0,   -99,
      61,   -99,     0,    64,     0,     0,     0,     0,    66
};

static const yytype_int16 yycheck[] =
{
       0,    50,     4,     4,     0,   102,     9,     4,   105,     9,
      77,   166,     9,     9,    13,    26,     4,    17,    18,   134,
      11,   136,    22,    26,    18,    26,    26,    35,    10,    11,
      26,    24,    26,    24,    16,    37,   191,    36,    26,    26,
     155,   156,    18,    11,    11,    38,    11,     3,     4,    14,
      26,     4,    34,    35,     3,    37,    24,    24,   173,    24,
       0,    26,    59,    60,    61,   162,     4,    64,    65,    66,
      67,    68,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    11,    66,    67,    68,    69,    32,    71,
      72,    11,    24,   208,   209,    26,    24,    23,    26,   148,
     215,    24,   217,    59,    24,    61,    26,   107,    18,    65,
      66,    67,    33,    26,   181,    38,    33,    35,   185,   101,
     102,    26,    26,   105,    11,   240,   241,    14,   225,    35,
     179,   198,    34,    23,   134,    23,   136,    24,     4,    26,
      34,     4,     4,     9,    35,   145,   146,    32,    34,     4,
     132,   133,   134,    12,    34,   155,   156,    35,    34,    34,
       4,    26,    23,     4,    34,    34,    26,   170,   171,    23,
     170,   171,    24,   173,   170,   171,    32,   159,    23,    22,
     162,    34,    32,   280,    23,    32,   186,    35,     3,     4,
       5,     6,     7,    59,    34,    61,    34,    26,    26,    65,
      66,    67,   183,    26,    35,    35,    32,    34,   208,   209,
     107,    26,    23,    28,   196,   215,    35,   217,    35,    35,
      23,    34,    37,    32,    34,   206,   229,   230,    34,   229,
     230,   213,   214,   229,   230,    34,    32,   134,    32,   136,
     240,   241,    23,   225,    59,     4,    61,    35,    63,    35,
      65,    66,    67,   256,   257,   236,   256,   257,   155,   156,
     256,   257,    34,    34,    32,    23,    23,    23,   271,    35,
      32,   271,    35,    33,    35,   271,   173,   258,   281,   282,
      35,   281,   282,    32,    32,   281,   282,    32,    32,   186,
     219,     4,    35,    -1,    -1,   276,   277,    -1,   280,    -1,
      -1,    -1,    -1,    -1,   285,   286,   287,    -1,    -1,   290,
     291,   208,   209,     3,     4,     5,     6,     7,   215,    -1,
     217,   302,   303,   304,    -1,    -1,    -1,    -1,    -1,    19,
      20,    21,    -1,    23,    -1,    -1,    26,    -1,    28,    -1,
      -1,    -1,    -1,   240,   241,    -1,    59,    37,    61,    -1,
      -1,    -1,    65,    66,    67,    -1,    -1,     3,     4,     5,
       6,     7,    -1,     3,     4,     5,     6,     7,    -1,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      26,    -1,    28,    23,    -1,    -1,    26,    -1,    28,    35,
      -1,    37,    -1,    -1,    -1,    -1,    -1,    37,    -1,     3,
       4,     5,     6,     7,    -1,     3,     4,     5,     6,     7,
      -1,     3,     4,     5,     6,     7,    -1,    63,    -1,    -1,
      -1,    -1,    26,    63,    28,    -1,    -1,    -1,    26,    33,
      28,    -1,    -1,    37,    26,    33,    28,    10,    -1,    37,
      13,    33,    15,    16,    17,    37,     4,     3,     4,     5,
       6,     7,    25,    -1,    27,    28,    29,    30,    31,    63,
      18,    -1,    35,    36,    -1,    63,    24,    -1,    -1,    -1,
      26,    63,    28,     4,    -1,    -1,    -1,    -1,    -1,    -1,
      38,    37,    -1,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,     4,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,     4,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,     4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    10,    -1,    -1,    13,    -1,
      15,    16,    17,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,    -1,    27,    28,    29,    30,    31,    -1,    -1,    -1,
      35,    36,    -1,    10,    39,    40,    13,    -1,    15,    16,
      17,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,
      27,    28,    29,    30,    31,    -1,    -1,    34,    -1,    36,
      -1,    10,    39,    40,    13,    -1,    15,    16,    17,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,
      29,    30,    31,    -1,    -1,    -1,    35,    36,    -1,    10,
      39,    40,    13,    -1,    15,    16,    17,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,
      31,    -1,    33,    -1,    -1,    36,    -1,    10,    39,    40,
      13,    -1,    15,    16,    17,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    -1,    27,    28,    29,    30,    31,    -1,
      -1,    34,    -1,    36,    -1,    10,    39,    40,    13,    -1,
      15,    16,    17,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,    -1,    27,    28,    29,    30,    31,    -1,    -1,    34,
      -1,    36,    -1,    10,    39,    40,    13,    -1,    15,    16,
      17,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,
      27,    28,    29,    30,    31,    -1,    33,    -1,    -1,    36,
      -1,    -1,    39,    40,    10,    -1,    12,    13,    -1,    15,
      16,    17,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      -1,    27,    28,    29,    30,    31,    -1,    -1,    -1,    -1,
      36,    -1,    10,    39,    40,    13,    -1,    15,    16,    17,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,
      28,    29,    30,    31,    -1,    -1,    -1,    35,    36,    -1,
      10,    39,    40,    13,    -1,    15,    16,    17,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,
      30,    31,    -1,    33,    -1,    -1,    36,    -1,    10,    39,
      40,    13,    -1,    15,    16,    17,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,    31,
      -1,    -1,    -1,    -1,    36,    -1,    10,    39,    40,    13,
      -1,    15,    16,    17,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,    -1,    27,    28,    29,    30,    31,    10,    -1,
      -1,    13,    36,    15,    16,    17,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,    -1,    27,    28,    29,    13,    31,
      15,    16,    17,    -1,    36,    -1,    -1,    -1,    -1,    -1,
      25,    -1,    27,    28,    29,    13,    31,    15,    16,    17,
      -1,    36,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,
      28,    29,    -1,    31,    -1,    -1,    -1,    -1,    36
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     7,    19,    20,    21,    23,
      26,    28,    37,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    73,    74,    75,    78,    79,    80,
      82,    83,    84,    18,    24,    26,    35,    26,    74,     4,
      78,    80,    78,     4,    35,    78,     4,    84,    84,     3,
      84,     3,     0,    74,    10,    13,    15,    16,    17,    25,
      27,    28,    29,    30,    31,    35,    36,    39,    40,    11,
      14,    24,    26,    24,    38,    24,    38,     4,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    77,    83,    78,    78,    78,    32,
      34,    11,    26,    23,    35,    26,     4,    26,     4,    77,
      78,    78,    78,    78,    78,    78,    78,    78,    78,    78,
      78,    78,    78,    78,    78,     4,    37,    78,    78,    87,
      33,    33,    11,    24,    26,    76,    26,    33,    34,    34,
      78,    87,     4,     9,    60,    64,    68,    81,    84,    85,
      87,    35,    84,    85,    89,    26,    26,    35,    33,    12,
      86,    34,    23,    78,    33,    78,    78,    89,    35,    89,
      23,    23,    34,    26,     4,     4,    84,     4,     9,    84,
      32,     4,    77,    35,    34,     4,    12,    88,    34,    89,
      89,    78,    35,    87,    35,    11,    24,    76,    33,    86,
      34,    34,    74,    74,    89,    26,    23,     4,    26,    26,
       4,     4,    77,    11,    24,    26,    76,    26,    81,    85,
       4,    34,    34,    86,    32,    23,    33,    76,    34,    23,
      23,    32,    32,    34,    34,    81,    35,    89,    89,    26,
      26,    26,    78,    33,    89,    89,    88,    35,    35,    35,
      35,    87,    35,    74,    74,    22,    23,    23,    32,    81,
      34,    34,    34,    89,    89,    11,    34,    34,    32,    32,
      32,    23,    74,    74,    81,     4,    35,    35,    34,    34,
      23,    23,    23,    35,    74,    32,    32,    35,    81,    81,
      35,    35,    87,    74,    74,    32,    81,    81,    81,    81,
      81,    32,    32,    32,    35,    81,    81,    81
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
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

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
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
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

/* Line 1009 of yacc.c  */
#line 166 "parser.yy"
	{ delete (yyvaluep->str); };

/* Line 1009 of yacc.c  */
#line 1649 "parser.tab.cc"
	break;
      case 8: /* "\"symbol\"" */

/* Line 1009 of yacc.c  */
#line 167 "parser.yy"
	{ delete (*yyvaluep).symbol; };

/* Line 1009 of yacc.c  */
#line 1658 "parser.tab.cc"
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

#if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 1;
#endif

/* User initialization code.  */

/* Line 1251 of yacc.c  */
#line 117 "parser.yy"
{
	yydebug = tp._parse_trace_p;
   /**
    setup the filename each time before parsing
    */
   static int mvvParserInputNumber = 0;
   yylloc.filename = mvv::Symbol::create( tp._filename == "" ? "(input" + nll::core::val2str( mvvParserInputNumber++ ) + ")" : tp._filename );
}

/* Line 1251 of yacc.c  */
#line 1818 "parser.tab.cc"
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

/* Line 1464 of yacc.c  */
#line 260 "parser.yy"
    { tp._root = (yyvsp[(1) - (1)].astStatements); ;}
    break;

  case 3:

/* Line 1464 of yacc.c  */
#line 262 "parser.yy"
    { (yyloc).filename = mvv::Symbol::create( tp._filename == "" ? "(input)" : tp._filename ); (yyval.astStatements) = new mvv::parser::AstStatements( (yyloc) ); ;}
    break;

  case 4:

/* Line 1464 of yacc.c  */
#line 263 "parser.yy"
    { (yyval.astStatements) = (yyvsp[(2) - (2)].astStatements); (yyvsp[(2) - (2)].astStatements)->insert( (yyvsp[(1) - (2)].ast) ); ;}
    break;

  case 5:

/* Line 1464 of yacc.c  */
#line 265 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstIf( (yyloc), (yyvsp[(3) - (7)].astExp), (yyvsp[(6) - (7)].astStatements), 0 ); ;}
    break;

  case 6:

/* Line 1464 of yacc.c  */
#line 266 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstIf( (yyloc), (yyvsp[(3) - (11)].astExp), (yyvsp[(6) - (11)].astStatements), (yyvsp[(10) - (11)].astStatements) ); ;}
    break;

  case 7:

/* Line 1464 of yacc.c  */
#line 267 "parser.yy"
    { mvv::parser::AstDeclClass* decl = new mvv::parser::AstDeclClass( (yyloc), *(yyvsp[(2) - (5)].symbol), (yyvsp[(4) - (5)].astDecls) ); (yyval.ast) = decl; linkFunctionToClass( *decl ); ;}
    break;

  case 8:

/* Line 1464 of yacc.c  */
#line 268 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstWhile( (yyloc), (yyvsp[(3) - (7)].astExp), (yyvsp[(6) - (7)].astStatements) ); ;}
    break;

  case 9:

/* Line 1464 of yacc.c  */
#line 269 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstBreak( (yyloc) ); ;}
    break;

  case 10:

/* Line 1464 of yacc.c  */
#line 270 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstTypedef( (yyloc), (yyvsp[(2) - (4)].astTypeT), *(yyvsp[(3) - (4)].symbol) );;}
    break;

  case 11:

/* Line 1464 of yacc.c  */
#line 271 "parser.yy"
    { mvv::parser::AstFunctionType* tt = new mvv::parser::AstFunctionType( (yyloc), (yyvsp[(2) - (7)].astTypeT), (yyvsp[(4) - (7)].astDeclVars) );
                                                                                            mvv::parser::AstTypedef* t = new mvv::parser::AstTypedef( (yyloc), tt, *(yyvsp[(6) - (7)].symbol) );
                                                                                            if ( hasDefaultValue( tt->getArgs() ) )
                                                                                            {
                                                                                                yyerror( &(yyloc), tp, "funtion pointer cannot have default values" );
                                                                                            }
                                                                                            (yyval.ast) = t;
                                                                                          ;}
    break;

  case 12:

/* Line 1464 of yacc.c  */
#line 284 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (5)].astTypeT), *(yyvsp[(2) - (5)].symbol), (yyvsp[(4) - (5)].astExp) ); ;}
    break;

  case 13:

/* Line 1464 of yacc.c  */
#line 285 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (7)].astTypeT), *(yyvsp[(2) - (7)].symbol), 0, (yyvsp[(5) - (7)].astArgs) ); ;}
    break;

  case 14:

/* Line 1464 of yacc.c  */
#line 286 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(2) - (7)].astTypeT), *(yyvsp[(3) - (7)].symbol), (yyvsp[(5) - (7)].astDeclVars) ); ;}
    break;

  case 15:

/* Line 1464 of yacc.c  */
#line 287 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (8)].astTypeT), *(yyvsp[(2) - (8)].symbol), (yyvsp[(4) - (8)].astDeclVars), (yyvsp[(7) - (8)].astStatements) ); ;}
    break;

  case 16:

/* Line 1464 of yacc.c  */
#line 288 "parser.yy"
    { /* first, concatenate the args*/
                                                                       mvv::parser::AstArgs* args = (yyvsp[(5) - (7)].astArgs);
                                                                       args->getArgs().push_front( (yyvsp[(4) - (7)].astExp) );
                                                                       (yyval.ast) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (7)].astTypeT), *(yyvsp[(2) - (7)].symbol), 0, 0, args );
                                                                     ;}
    break;

  case 17:

/* Line 1464 of yacc.c  */
#line 294 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (4)].astTypeT), *(yyvsp[(2) - (4)].symbol) );
                                                                       if ( (yyvsp[(3) - (4)].arrayDim)->size() )
                                                                       {
                                                                          (yyvsp[(1) - (4)].astTypeT)->setArray( true );
                                                                          (yyvsp[(1) - (4)].astTypeT)->setSize( (yyvsp[(3) - (4)].arrayDim) );
                                                                       } 
                                                                     ;}
    break;

  case 18:

/* Line 1464 of yacc.c  */
#line 302 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (9)].astTypeT), *(yyvsp[(2) - (9)].symbol), 0, (yyvsp[(7) - (9)].astArgs) ); (yyvsp[(1) - (9)].astTypeT)->setArray( true ); /* we don't handle several dimensions*/ ;}
    break;

  case 19:

/* Line 1464 of yacc.c  */
#line 303 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstExpAssign( (yyloc), (yyvsp[(1) - (4)].astVar), (yyvsp[(3) - (4)].astExp) ); ;}
    break;

  case 20:

/* Line 1464 of yacc.c  */
#line 304 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstExpCall( (yyloc), (yyvsp[(1) - (5)].astVar), (yyvsp[(3) - (5)].astArgs) ); ;}
    break;

  case 21:

/* Line 1464 of yacc.c  */
#line 305 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstReturn( (yyloc), (yyvsp[(2) - (3)].astExp) ); ;}
    break;

  case 22:

/* Line 1464 of yacc.c  */
#line 306 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstReturn( (yyloc) ); ;}
    break;

  case 23:

/* Line 1464 of yacc.c  */
#line 307 "parser.yy"
    { (yyval.ast) = (yyvsp[(2) - (3)].astStatements); ;}
    break;

  case 24:

/* Line 1464 of yacc.c  */
#line 308 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstImport( (yyloc), *(yyvsp[(2) - (2)].str) ); ;}
    break;

  case 25:

/* Line 1464 of yacc.c  */
#line 309 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstInclude( (yyloc), *(yyvsp[(2) - (2)].str) ); ;}
    break;

  case 26:

/* Line 1464 of yacc.c  */
#line 312 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (8)].astTypeT), *(yyvsp[(2) - (8)].symbol), (yyvsp[(4) - (8)].astDeclVars), (yyvsp[(7) - (8)].astStatements) ); ;}
    break;

  case 27:

/* Line 1464 of yacc.c  */
#line 313 "parser.yy"
    { (yyval.ast) = new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(2) - (7)].astTypeT), *(yyvsp[(3) - (7)].symbol), (yyvsp[(5) - (7)].astDeclVars) ); ;}
    break;

  case 28:

/* Line 1464 of yacc.c  */
#line 314 "parser.yy"
    { (yyval.ast) = (yyvsp[(1) - (2)].astExp); ;}
    break;

  case 29:

/* Line 1464 of yacc.c  */
#line 318 "parser.yy"
    { (yyval.arrayDim) = new std::vector<mvv::parser::AstExp*>(); ;}
    break;

  case 30:

/* Line 1464 of yacc.c  */
#line 319 "parser.yy"
    { (yyval.arrayDim) = (yyvsp[(4) - (4)].arrayDim); (yyval.arrayDim)->push_back( (yyvsp[(2) - (4)].astExp) ); ;}
    break;

  case 31:

/* Line 1464 of yacc.c  */
#line 320 "parser.yy"
    { (yyval.arrayDim) = (yyvsp[(3) - (3)].arrayDim); (yyval.arrayDim)->push_back( new mvv::parser::AstInt( (yyloc), 0 ) ); ;}
    break;

  case 32:

/* Line 1464 of yacc.c  */
#line 322 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator+" ) ); ;}
    break;

  case 33:

/* Line 1464 of yacc.c  */
#line 323 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator-" ) ); ;}
    break;

  case 34:

/* Line 1464 of yacc.c  */
#line 324 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator*" ) ); ;}
    break;

  case 35:

/* Line 1464 of yacc.c  */
#line 325 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator/" ) ); ;}
    break;

  case 36:

/* Line 1464 of yacc.c  */
#line 326 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator<" ) ); ;}
    break;

  case 37:

/* Line 1464 of yacc.c  */
#line 327 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator>" ) ); ;}
    break;

  case 38:

/* Line 1464 of yacc.c  */
#line 328 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator<=" ) ); ;}
    break;

  case 39:

/* Line 1464 of yacc.c  */
#line 329 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator>=" ) ); ;}
    break;

  case 40:

/* Line 1464 of yacc.c  */
#line 330 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator==" ) ); ;}
    break;

  case 41:

/* Line 1464 of yacc.c  */
#line 331 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator!=" ) ); ;}
    break;

  case 42:

/* Line 1464 of yacc.c  */
#line 332 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator&&" ) ); ;}
    break;

  case 43:

/* Line 1464 of yacc.c  */
#line 333 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator||" ) ); ;}
    break;

  case 44:

/* Line 1464 of yacc.c  */
#line 334 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator[]" ) ); ;}
    break;

  case 45:

/* Line 1464 of yacc.c  */
#line 335 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator()" ) ); ;}
    break;

  case 46:

/* Line 1464 of yacc.c  */
#line 336 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator<<" ) ); ;}
    break;

  case 47:

/* Line 1464 of yacc.c  */
#line 337 "parser.yy"
    { (yyval.symbol) = new mvv::Symbol( mvv::Symbol::create ( "operator>>" ) ); ;}
    break;

  case 48:

/* Line 1464 of yacc.c  */
#line 341 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstInt( (yyloc), (yyvsp[(1) - (1)].ival) ); ;}
    break;

  case 49:

/* Line 1464 of yacc.c  */
#line 342 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstFloat( (yyloc), (yyvsp[(1) - (1)].fval) ); ;}
    break;

  case 50:

/* Line 1464 of yacc.c  */
#line 343 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstNil( (yyloc) ); ;}
    break;

  case 51:

/* Line 1464 of yacc.c  */
#line 344 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::STREAMO ); ;}
    break;

  case 52:

/* Line 1464 of yacc.c  */
#line 345 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::STREAMI ); ;}
    break;

  case 53:

/* Line 1464 of yacc.c  */
#line 346 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::PLUS ); ;}
    break;

  case 54:

/* Line 1464 of yacc.c  */
#line 347 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::MINUS ); ;}
    break;

  case 55:

/* Line 1464 of yacc.c  */
#line 348 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::TIMES ); ;}
    break;

  case 56:

/* Line 1464 of yacc.c  */
#line 349 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::DIVIDE ); ;}
    break;

  case 57:

/* Line 1464 of yacc.c  */
#line 350 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::AND ); ;}
    break;

  case 58:

/* Line 1464 of yacc.c  */
#line 351 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::OR ); ;}
    break;

  case 59:

/* Line 1464 of yacc.c  */
#line 352 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::LT ); ;}
    break;

  case 60:

/* Line 1464 of yacc.c  */
#line 353 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::LE ); ;}
    break;

  case 61:

/* Line 1464 of yacc.c  */
#line 354 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::GT ); ;}
    break;

  case 62:

/* Line 1464 of yacc.c  */
#line 355 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::GE ); ;}
    break;

  case 63:

/* Line 1464 of yacc.c  */
#line 356 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::NE ); ;}
    break;

  case 64:

/* Line 1464 of yacc.c  */
#line 357 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), (yyvsp[(1) - (3)].astExp), (yyvsp[(3) - (3)].astExp), mvv::parser::AstOpBin::EQ ); ;}
    break;

  case 65:

/* Line 1464 of yacc.c  */
#line 358 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstOpBin( (yyloc), new mvv::parser::AstInt( (yyloc), 0 ) , (yyvsp[(2) - (2)].astExp), mvv::parser::AstOpBin::MINUS ); ;}
    break;

  case 66:

/* Line 1464 of yacc.c  */
#line 359 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstExpSeq( (yyloc), (yyvsp[(2) - (3)].astExp) ); ;}
    break;

  case 67:

/* Line 1464 of yacc.c  */
#line 360 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstString( (yyloc), *(yyvsp[(1) - (1)].str) ); ;}
    break;

  case 68:

/* Line 1464 of yacc.c  */
#line 361 "parser.yy"
    { (yyval.astExp) = (yyvsp[(1) - (1)].astVar); ;}
    break;

  case 69:

/* Line 1464 of yacc.c  */
#line 362 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstExpAssign( (yyloc), (yyvsp[(1) - (3)].astVar), (yyvsp[(3) - (3)].astExp) ); ;}
    break;

  case 70:

/* Line 1464 of yacc.c  */
#line 363 "parser.yy"
    { (yyval.astExp) = new mvv::parser::AstExpTypename( (yyloc), (yyvsp[(2) - (5)].astTypeT), (yyvsp[(4) - (5)].astArgs) );;}
    break;

  case 71:

/* Line 1464 of yacc.c  */
#line 366 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstThis( (yyloc) ); ;}
    break;

  case 72:

/* Line 1464 of yacc.c  */
#line 367 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarField( (yyloc), (yyvsp[(1) - (3)].astVar), mvv::Symbol::create( "this" ) ); ;}
    break;

  case 73:

/* Line 1464 of yacc.c  */
#line 369 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarSimple( (yyloc), *(yyvsp[(1) - (1)].symbol), true ); ;}
    break;

  case 74:

/* Line 1464 of yacc.c  */
#line 370 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarArray( (yyloc), new mvv::parser::AstVarSimple( (yyloc), *(yyvsp[(1) - (4)].symbol), true ), (yyvsp[(3) - (4)].astExp) ); ;}
    break;

  case 75:

/* Line 1464 of yacc.c  */
#line 371 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarField( (yyloc), (yyvsp[(1) - (3)].astVar), *(yyvsp[(3) - (3)].symbol) ); ;}
    break;

  case 76:

/* Line 1464 of yacc.c  */
#line 372 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstVarArray( (yyloc), (yyvsp[(1) - (4)].astVar), (yyvsp[(3) - (4)].astExp) ); ;}
    break;

  case 77:

/* Line 1464 of yacc.c  */
#line 373 "parser.yy"
    { (yyval.astVar) = new mvv::parser::AstExpCall( (yyloc), (yyvsp[(1) - (4)].astVar), (yyvsp[(3) - (4)].astArgs) ); ;}
    break;

  case 78:

/* Line 1464 of yacc.c  */
#line 374 "parser.yy"
    { (yyval.astVar) = (yyvsp[(1) - (1)].astVar); ;}
    break;

  case 79:

/* Line 1464 of yacc.c  */
#line 378 "parser.yy"
    { (yyval.astDecls) = new mvv::parser::AstDecls( (yyloc) ); ;}
    break;

  case 80:

/* Line 1464 of yacc.c  */
#line 379 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(3) - (3)].astDecls); (yyval.astDecls)->insert( (yyvsp[(1) - (3)].astDeclVar) ); ;}
    break;

  case 81:

/* Line 1464 of yacc.c  */
#line 381 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(10) - (10)].astDecls); mvv::parser::AstDeclVar* var = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (10)].astTypeT), *(yyvsp[(2) - (10)].symbol), 0, (yyvsp[(7) - (10)].astArgs) ); (yyvsp[(1) - (10)].astTypeT)->setArray( true ); (yyval.astDecls)->insert( var ); ;}
    break;

  case 82:

/* Line 1464 of yacc.c  */
#line 382 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(6) - (6)].astDecls); mvv::parser::AstDeclClass* decl = new mvv::parser::AstDeclClass( (yyloc), *(yyvsp[(2) - (6)].symbol), (yyvsp[(4) - (6)].astDecls) ); (yyval.astDecls)->insert( decl ); linkFunctionToClass( *decl ); ;}
    break;

  case 83:

/* Line 1464 of yacc.c  */
#line 383 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), *(yyvsp[(2) - (9)].symbol), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 84:

/* Line 1464 of yacc.c  */
#line 384 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(8) - (8)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(2) - (8)].astTypeT), *(yyvsp[(3) - (8)].symbol), (yyvsp[(5) - (8)].astDeclVars) ) ); ;}
    break;

  case 85:

/* Line 1464 of yacc.c  */
#line 385 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(8) - (8)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), 0, *(yyvsp[(1) - (8)].symbol), (yyvsp[(3) - (8)].astDeclVars), (yyvsp[(6) - (8)].astStatements) ) ); ;}
    break;

  case 86:

/* Line 1464 of yacc.c  */
#line 386 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(7) - (7)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), 0, *(yyvsp[(2) - (7)].symbol), (yyvsp[(4) - (7)].astDeclVars) ) ); ;}
    break;

  case 87:

/* Line 1464 of yacc.c  */
#line 387 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(9) - (9)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(1) - (9)].astTypeT), *(yyvsp[(2) - (9)].symbol), (yyvsp[(4) - (9)].astDeclVars), (yyvsp[(7) - (9)].astStatements) ) ); ;}
    break;

  case 88:

/* Line 1464 of yacc.c  */
#line 388 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(8) - (8)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), (yyvsp[(2) - (8)].astTypeT), *(yyvsp[(3) - (8)].symbol), (yyvsp[(5) - (8)].astDeclVars) ) ); ;}
    break;

  case 89:

/* Line 1464 of yacc.c  */
#line 389 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(7) - (7)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), 0, mvv::platform::Symbol::create( ("~" + std::string( (yyvsp[(3) - (7)].symbol)->getName() )).c_str() ), new mvv::parser::AstDeclVars((yyloc)) ) ); ;}
    break;

  case 90:

/* Line 1464 of yacc.c  */
#line 390 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(8) - (8)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstDeclFun( (yyloc), 0, mvv::platform::Symbol::create( ("~" + std::string( (yyvsp[(2) - (8)].symbol)->getName() )).c_str() ), new mvv::parser::AstDeclVars((yyloc)), (yyvsp[(6) - (8)].astStatements) ) ); ;}
    break;

  case 91:

/* Line 1464 of yacc.c  */
#line 391 "parser.yy"
    { (yyval.astDecls) = (yyvsp[(5) - (5)].astDecls); (yyval.astDecls)->insert( new mvv::parser::AstTypedef( (yyloc), (yyvsp[(2) - (5)].astTypeT), *(yyvsp[(3) - (5)].symbol) ) );;}
    break;

  case 92:

/* Line 1464 of yacc.c  */
#line 392 "parser.yy"
    { mvv::parser::AstFunctionType* tt = new mvv::parser::AstFunctionType( (yyloc), (yyvsp[(2) - (8)].astTypeT), (yyvsp[(4) - (8)].astDeclVars) );
                                                                                               mvv::parser::AstTypedef* t = new mvv::parser::AstTypedef( (yyloc), tt, *(yyvsp[(6) - (8)].symbol) );
                                                                                               if ( hasDefaultValue( tt->getArgs() ) )
                                                                                               {
                                                                                                  yyerror( &(yyloc), tp, "funtion pointer cannot have default values" );
                                                                                               }
                                                                                               (yyval.astDecls) = (yyvsp[(8) - (8)].astDecls); (yyval.astDecls)->insert( t );
                                                                                             ;}
    break;

  case 93:

/* Line 1464 of yacc.c  */
#line 401 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::VAR ); ;}
    break;

  case 94:

/* Line 1464 of yacc.c  */
#line 402 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::CMP_INT ); ;}
    break;

  case 95:

/* Line 1464 of yacc.c  */
#line 403 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::CMP_FLOAT );;}
    break;

  case 96:

/* Line 1464 of yacc.c  */
#line 404 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::STRING ); ;}
    break;

  case 97:

/* Line 1464 of yacc.c  */
#line 405 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::VOID );;}
    break;

  case 98:

/* Line 1464 of yacc.c  */
#line 407 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstType( (yyloc), mvv::parser::AstType::SYMBOL, (yyvsp[(1) - (1)].symbol) ); ;}
    break;

  case 99:

/* Line 1464 of yacc.c  */
#line 408 "parser.yy"
    { (yyval.astTypeT) = new mvv::parser::AstTypeField( (yyloc), (yyvsp[(3) - (3)].astTypeT), *(yyvsp[(1) - (3)].symbol) ); ;}
    break;

  case 100:

/* Line 1464 of yacc.c  */
#line 410 "parser.yy"
    { (yyval.astTypeT) = (yyvsp[(1) - (1)].astTypeT); ;}
    break;

  case 101:

/* Line 1464 of yacc.c  */
#line 411 "parser.yy"
    { (yyval.astTypeT) = (yyvsp[(1) - (1)].astTypeT); ;}
    break;

  case 102:

/* Line 1464 of yacc.c  */
#line 412 "parser.yy"
    { (yyval.astTypeT) = (yyvsp[(1) - (2)].astTypeT); (yyvsp[(1) - (2)].astTypeT)->setIsAReference( true ); ;}
    break;

  case 103:

/* Line 1464 of yacc.c  */
#line 413 "parser.yy"
    { (yyval.astTypeT) = (yyvsp[(1) - (2)].astTypeT); (yyvsp[(1) - (2)].astTypeT)->setIsAReference( true ); ;}
    break;

  case 104:

/* Line 1464 of yacc.c  */
#line 414 "parser.yy"
    { (yyval.astTypeT) = (yyvsp[(1) - (3)].astTypeT); (yyvsp[(1) - (3)].astTypeT)->setArray( true ) ;}
    break;

  case 105:

/* Line 1464 of yacc.c  */
#line 415 "parser.yy"
    { (yyval.astTypeT) = (yyvsp[(1) - (3)].astTypeT); (yyvsp[(1) - (3)].astTypeT)->setArray( true ) ;}
    break;

  case 106:

/* Line 1464 of yacc.c  */
#line 418 "parser.yy"
    { (yyval.astDeclVar) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (4)].astTypeT), *(yyvsp[(2) - (4)].symbol), (yyvsp[(4) - (4)].astExp) ); ;}
    break;

  case 107:

/* Line 1464 of yacc.c  */
#line 419 "parser.yy"
    { (yyval.astDeclVar) = new mvv::parser::AstDeclVar( (yyloc), (yyvsp[(1) - (3)].astTypeT), *(yyvsp[(2) - (3)].symbol) );
                                        if ( (yyvsp[(3) - (3)].arrayDim)->size() )
                                        {
                                           (yyvsp[(1) - (3)].astTypeT)->setArray( true );
                                           (yyvsp[(1) - (3)].astTypeT)->setSize( (yyvsp[(3) - (3)].arrayDim) );
                                        } 
                                      ;}
    break;

  case 108:

/* Line 1464 of yacc.c  */
#line 438 "parser.yy"
    { (yyval.astArgs) = new mvv::parser::AstArgs( (yyloc) ); ;}
    break;

  case 109:

/* Line 1464 of yacc.c  */
#line 439 "parser.yy"
    { (yyval.astArgs) = (yyvsp[(3) - (3)].astArgs); (yyval.astArgs)->insert( (yyvsp[(2) - (3)].astExp) ); ;}
    break;

  case 110:

/* Line 1464 of yacc.c  */
#line 441 "parser.yy"
    { (yyval.astArgs) = new mvv::parser::AstArgs( (yyloc) ); ;}
    break;

  case 111:

/* Line 1464 of yacc.c  */
#line 442 "parser.yy"
    { (yyval.astArgs) = (yyvsp[(2) - (2)].astArgs); (yyval.astArgs)->insert( (yyvsp[(1) - (2)].astExp) ); ;}
    break;

  case 112:

/* Line 1464 of yacc.c  */
#line 444 "parser.yy"
    { (yyval.astDeclVars) = new mvv::parser::AstDeclVars( (yyloc) ); ;}
    break;

  case 113:

/* Line 1464 of yacc.c  */
#line 445 "parser.yy"
    { (yyval.astDeclVars) = (yyvsp[(3) - (3)].astDeclVars); (yyval.astDeclVars)->insert( (yyvsp[(2) - (3)].astDeclVar) ); ;}
    break;

  case 114:

/* Line 1464 of yacc.c  */
#line 447 "parser.yy"
    { (yyval.astDeclVars) = new mvv::parser::AstDeclVars( (yyloc) ); ;}
    break;

  case 115:

/* Line 1464 of yacc.c  */
#line 448 "parser.yy"
    { (yyval.astDeclVars) = (yyvsp[(2) - (2)].astDeclVars); (yyval.astDeclVars)->insert( (yyvsp[(1) - (2)].astDeclVar) ); ;}
    break;



/* Line 1464 of yacc.c  */
#line 2834 "parser.tab.cc"
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



/* Line 1684 of yacc.c  */
#line 450 "parser.yy"


