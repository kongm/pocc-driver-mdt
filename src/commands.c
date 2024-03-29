/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Substitute the type names.  */
#define YYSTYPE         POCCINTERPRETER_YYSTYPE
/* Substitute the variable and function names.  */
#define yyparse         poccinterpreter_yyparse
#define yylex           poccinterpreter_yylex
#define yyerror         poccinterpreter_yyerror
#define yydebug         poccinterpreter_yydebug
#define yynerrs         poccinterpreter_yynerrs


/* Copy the first part of user declarations.  */
#line 27 "commands.y" /* yacc.c:339  */

#include <stdio.h>
void yyerror(void* scanner, const char *s);

#include "interpreter.h"


#line 81 "commands.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "y.tab.h".  */
#ifndef YY_POCCINTERPRETER_YY_COMMANDS_H_INCLUDED
# define YY_POCCINTERPRETER_YY_COMMANDS_H_INCLUDED
/* Debug traces.  */
#ifndef POCCINTERPRETER_YYDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define POCCINTERPRETER_YYDEBUG 1
#  else
#   define POCCINTERPRETER_YYDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define POCCINTERPRETER_YYDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined POCCINTERPRETER_YYDEBUG */
#if POCCINTERPRETER_YYDEBUG
extern int poccinterpreter_yydebug;
#endif

/* Token type.  */
#ifndef POCCINTERPRETER_YYTOKENTYPE
# define POCCINTERPRETER_YYTOKENTYPE
  enum poccinterpreter_yytokentype
  {
    INT = 258,
    FLOAT = 259,
    ID = 260,
    STRING = 261,
    OP_PLUS = 262,
    OP_MINUS = 263,
    OP_MUL = 264,
    OP_DIV = 265,
    CMD_INFO = 266,
    CMD_SWITCH = 267,
    CMD_LIST = 268,
    CMD_QUIT = 269,
    CMD_SHOW = 270,
    CMD_LOAD = 271,
    CMD_STORE = 272,
    CMD_DISPLAY = 273,
    CMD_SCOP = 274,
    CMD_CONTEXT = 275,
    CMD_HELP = 276,
    CMD_PONOS = 277,
    CMD_NAMESPACE = 278,
    CMD_C = 279,
    CMD_POCC = 280,
    CMD_OPTIONS = 281,
    CMD_INITIALIZE = 282,
    CMD_OPTION = 283,
    CMD_SET = 284,
    CMD_COMPUTE = 285,
    CMD_LEGAL_SPACE = 286,
    CMD_SCHEDULE = 287,
    CMD_SPACE = 288,
    CMD_VARS = 289,
    CMD_CPLEX_LP = 290,
    CMD_WORKSPACE = 291,
    CMD_VAR = 292,
    CMD_ALL_VARS = 293,
    CMD_ALL_VARS_DETAILED = 294,
    CMD_CONSTRAINTS = 295,
    CMD_CONSTRAINT = 296,
    CMD_MATCHING = 297,
    CMD_SCHEDULING_CONSTRAINTS = 298,
    CMD_DELETE = 299,
    TOKEN_COMMENT = 300,
    CMD_INSERT = 301,
    CMD_VARIABLE = 302,
    TOKEN_FIRST = 303,
    TOKEN_LAST = 304,
    CMD_VARIABLE_BOUNDS = 305,
    CMD_SOL_LEXMIN = 306,
    CMD_ALL_CONSTRAINTS = 307,
    CMD_IS_INCLUDED = 308,
    CMD_STATS = 309,
    MAXPRIORITY = 310
  };
#endif
/* Tokens.  */
#define INT 258
#define FLOAT 259
#define ID 260
#define STRING 261
#define OP_PLUS 262
#define OP_MINUS 263
#define OP_MUL 264
#define OP_DIV 265
#define CMD_INFO 266
#define CMD_SWITCH 267
#define CMD_LIST 268
#define CMD_QUIT 269
#define CMD_SHOW 270
#define CMD_LOAD 271
#define CMD_STORE 272
#define CMD_DISPLAY 273
#define CMD_SCOP 274
#define CMD_CONTEXT 275
#define CMD_HELP 276
#define CMD_PONOS 277
#define CMD_NAMESPACE 278
#define CMD_C 279
#define CMD_POCC 280
#define CMD_OPTIONS 281
#define CMD_INITIALIZE 282
#define CMD_OPTION 283
#define CMD_SET 284
#define CMD_COMPUTE 285
#define CMD_LEGAL_SPACE 286
#define CMD_SCHEDULE 287
#define CMD_SPACE 288
#define CMD_VARS 289
#define CMD_CPLEX_LP 290
#define CMD_WORKSPACE 291
#define CMD_VAR 292
#define CMD_ALL_VARS 293
#define CMD_ALL_VARS_DETAILED 294
#define CMD_CONSTRAINTS 295
#define CMD_CONSTRAINT 296
#define CMD_MATCHING 297
#define CMD_SCHEDULING_CONSTRAINTS 298
#define CMD_DELETE 299
#define TOKEN_COMMENT 300
#define CMD_INSERT 301
#define CMD_VARIABLE 302
#define TOKEN_FIRST 303
#define TOKEN_LAST 304
#define CMD_VARIABLE_BOUNDS 305
#define CMD_SOL_LEXMIN 306
#define CMD_ALL_CONSTRAINTS 307
#define CMD_IS_INCLUDED 308
#define CMD_STATS 309
#define MAXPRIORITY 310

/* Value type.  */
#if ! defined POCCINTERPRETER_YYSTYPE && ! defined POCCINTERPRETER_YYSTYPE_IS_DECLARED

union POCCINTERPRETER_YYSTYPE
{
#line 41 "commands.y" /* yacc.c:355  */

  int	intval;
  float	floatval;
  char*	strval;
  int	token;
  int	rule_output;

#line 247 "commands.c" /* yacc.c:355  */
};

typedef union POCCINTERPRETER_YYSTYPE POCCINTERPRETER_YYSTYPE;
# define POCCINTERPRETER_YYSTYPE_IS_TRIVIAL 1
# define POCCINTERPRETER_YYSTYPE_IS_DECLARED 1
#endif



int poccinterpreter_yyparse (void *scanner);

#endif /* !YY_POCCINTERPRETER_YY_COMMANDS_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 263 "commands.c" /* yacc.c:358  */

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
#else
typedef signed char yytype_int8;
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
# elif ! defined YYSIZE_T
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
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined POCCINTERPRETER_YYSTYPE_IS_TRIVIAL && POCCINTERPRETER_YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   83

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  56
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  7
/* YYNRULES -- Number of rules.  */
#define YYNRULES  54
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  88

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   310

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
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
      55
};

#if POCCINTERPRETER_YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    63,    63,    64,    65,    79,    84,    89,    95,   101,
     105,   109,   113,   117,   121,   125,   131,   135,   139,   143,
     147,   151,   155,   159,   163,   171,   179,   183,   187,   191,
     197,   203,   210,   216,   222,   228,   234,   238,   244,   250,
     258,   266,   279,   280,   281,   285,   286,   290,   291,   292,
     293,   294,   300,   301,   302
};
#endif

#if POCCINTERPRETER_YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INT", "FLOAT", "ID", "STRING",
  "OP_PLUS", "OP_MINUS", "OP_MUL", "OP_DIV", "CMD_INFO", "CMD_SWITCH",
  "CMD_LIST", "CMD_QUIT", "CMD_SHOW", "CMD_LOAD", "CMD_STORE",
  "CMD_DISPLAY", "CMD_SCOP", "CMD_CONTEXT", "CMD_HELP", "CMD_PONOS",
  "CMD_NAMESPACE", "CMD_C", "CMD_POCC", "CMD_OPTIONS", "CMD_INITIALIZE",
  "CMD_OPTION", "CMD_SET", "CMD_COMPUTE", "CMD_LEGAL_SPACE",
  "CMD_SCHEDULE", "CMD_SPACE", "CMD_VARS", "CMD_CPLEX_LP", "CMD_WORKSPACE",
  "CMD_VAR", "CMD_ALL_VARS", "CMD_ALL_VARS_DETAILED", "CMD_CONSTRAINTS",
  "CMD_CONSTRAINT", "CMD_MATCHING", "CMD_SCHEDULING_CONSTRAINTS",
  "CMD_DELETE", "TOKEN_COMMENT", "CMD_INSERT", "CMD_VARIABLE",
  "TOKEN_FIRST", "TOKEN_LAST", "CMD_VARIABLE_BOUNDS", "CMD_SOL_LEXMIN",
  "CMD_ALL_CONSTRAINTS", "CMD_IS_INCLUDED", "CMD_STATS", "MAXPRIORITY",
  "$accept", "command", "commands_arg", "position", "id_or_string",
  "commands_unique", "namespace", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310
};
# endif

#define YYPACT_NINF -33

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-33)))

#define YYTABLE_NINF -3

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      25,   -33,   -33,    12,   -10,   -33,   -33,   -33,    -1,    -2,
       4,    16,   -33,    51,   -33,   -18,     9,    47,   -16,   -25,
     -26,   -32,   -33,   -33,   -33,   -33,   -33,   -33,   -33,    17,
     -33,    19,   -33,   -33,   -33,   -33,   -33,   -13,    28,    34,
     -33,   -33,   -33,    38,    69,   -33,   -33,   -33,    42,   -33,
      74,   -33,    73,    75,    69,    76,   -33,    69,    69,    69,
     -33,   -33,    69,   -33,   -33,    65,   -33,   -33,    79,   -33,
     -33,   -33,   -33,   -33,   -33,   -33,    -3,    -3,   -33,   -33,
     -33,    80,   -33,   -33,   -33,   -33,   -33,   -33
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      54,    53,    52,     0,     0,     1,    50,    48,    49,     0,
       0,     0,    47,     0,    51,     0,     0,     0,     0,     0,
       0,     0,     3,     4,     9,    12,    10,    11,     5,     0,
       6,     0,    18,    19,    22,    20,    21,     0,     0,     0,
      23,    17,    16,     0,     0,    26,    28,    27,     0,    14,
       0,    13,     0,     0,     0,     0,    36,     0,     0,     0,
       7,     8,     0,    33,    30,     0,    45,    46,     0,    29,
      15,    32,    34,    37,    35,    38,     0,     0,    31,    25,
      24,     0,    42,    43,    44,    39,    40,    41
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -33,   -33,   -33,     2,     7,   -33,   -33
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     3,    22,    85,    68,    23,     4
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      82,     6,    52,    28,     7,     8,     9,    10,    11,    30,
      43,    12,     5,    13,    59,    57,    55,    14,    24,    15,
      16,    58,    60,    25,    61,    -2,    17,    56,    53,    62,
      54,    18,    44,    63,    19,    26,    20,    21,    29,    64,
      45,    46,    32,    65,    31,    83,    84,     1,    69,    33,
       2,    34,    47,    27,    35,    36,    37,    38,    49,    50,
      51,    73,    48,    39,    75,    76,    77,    40,    79,    78,
      66,    67,    80,    41,    66,    67,    42,    70,    71,    86,
      72,    74,    81,    87
};

static const yytype_uint8 yycheck[] =
{
       3,    11,    18,     5,    14,    15,    16,    17,    18,     5,
      28,    21,     0,    23,    46,    41,    41,    27,    19,    29,
      30,    47,     5,    24,     5,     0,    36,    52,    44,    42,
      46,    41,    50,     5,    44,    36,    46,    47,    40,     5,
      31,    32,    26,     5,    40,    48,    49,    22,     6,    33,
      25,    35,    43,    54,    38,    39,    40,    41,    11,    12,
      13,    54,    53,    47,    57,    58,    59,    51,     3,    62,
       5,     6,    65,    22,     5,     6,    25,     3,     5,    77,
       5,     5,     3,     3
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    22,    25,    57,    62,     0,    11,    14,    15,    16,
      17,    18,    21,    23,    27,    29,    30,    36,    41,    44,
      46,    47,    58,    61,    19,    24,    36,    54,     5,    40,
       5,    40,    26,    33,    35,    38,    39,    40,    41,    47,
      51,    22,    25,    28,    50,    31,    32,    43,    53,    11,
      12,    13,    18,    44,    46,    41,    52,    41,    47,    46,
       5,     5,    42,     5,     5,     5,     5,     6,    60,     6,
       3,     5,     5,    60,     5,    60,    60,    60,    60,     3,
      60,     3,     3,    48,    49,    59,    59,     3
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    56,    57,    57,    57,    58,    58,    58,    58,    58,
      58,    58,    58,    58,    58,    58,    58,    58,    58,    58,
      58,    58,    58,    58,    58,    58,    58,    58,    58,    58,
      58,    58,    58,    58,    58,    58,    58,    58,    58,    58,
      58,    58,    59,    59,    59,    60,    60,    61,    61,    61,
      61,    61,    62,    62,    62
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     2,     2,     3,     3,     2,
       2,     2,     2,     2,     2,     3,     2,     2,     2,     2,
       2,     2,     2,     2,     4,     4,     2,     2,     2,     3,
       3,     4,     3,     3,     3,     3,     2,     3,     3,     4,
       4,     5,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     0
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (scanner, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if POCCINTERPRETER_YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, scanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void *scanner)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (scanner);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void *scanner)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, scanner);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule, void *scanner)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              , scanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, scanner); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !POCCINTERPRETER_YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !POCCINTERPRETER_YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
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
static YYSIZE_T
yystrlen (const char *yystr)
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
static char *
yystpcpy (char *yydest, const char *yysrc)
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

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, void *scanner)
{
  YYUSE (yyvaluep);
  YYUSE (scanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void *scanner)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
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

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

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
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

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
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, scanner);
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
      if (yytable_value_is_error (yyn))
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
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 5:
#line 79 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_LOAD);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ((yyvsp[0].strval));
   }
#line 1416 "commands.c" /* yacc.c:1646  */
    break;

  case 6:
#line 84 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_STORE);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ((yyvsp[0].strval));
   }
#line 1426 "commands.c" /* yacc.c:1646  */
    break;

  case 7:
#line 89 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_LOAD);
   pocc_interpreter_register_next_command(POCC_CMD_CONSTRAINTS);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ((yyvsp[0].strval));
   }
#line 1437 "commands.c" /* yacc.c:1646  */
    break;

  case 8:
#line 95 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_STORE);
   pocc_interpreter_register_next_command(POCC_CMD_CONSTRAINTS);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ((yyvsp[0].strval));
   }
#line 1448 "commands.c" /* yacc.c:1646  */
    break;

  case 9:
#line 101 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_SHOW);
   pocc_interpreter_register_next_command(POCC_CMD_SCOP);
   }
#line 1457 "commands.c" /* yacc.c:1646  */
    break;

  case 10:
#line 105 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_SHOW);
   pocc_interpreter_register_next_command(POCC_CMD_WORKSPACE);
   }
#line 1466 "commands.c" /* yacc.c:1646  */
    break;

  case 11:
#line 109 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_SHOW);
   pocc_interpreter_register_next_command(POCC_CMD_STATS);
   }
#line 1475 "commands.c" /* yacc.c:1646  */
    break;

  case 12:
#line 113 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_SHOW);
   pocc_interpreter_register_next_command(POCC_CMD_C);
   }
#line 1484 "commands.c" /* yacc.c:1646  */
    break;

  case 13:
#line 117 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_WORKSPACE);
   pocc_interpreter_register_next_command(POCC_CMD_LIST);
   }
#line 1493 "commands.c" /* yacc.c:1646  */
    break;

  case 14:
#line 121 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_WORKSPACE);
   pocc_interpreter_register_next_command(POCC_CMD_INFO);
   }
#line 1502 "commands.c" /* yacc.c:1646  */
    break;

  case 15:
#line 125 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_WORKSPACE);
   pocc_interpreter_register_next_command(POCC_CMD_SWITCH);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     (void *)((intptr_t)((yyvsp[0].intval)));
   }
#line 1513 "commands.c" /* yacc.c:1646  */
    break;

  case 16:
#line 131 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_NAMESPACE);
   pocc_interpreter_register_next_command(POCC_CMD_POCC);
   }
#line 1522 "commands.c" /* yacc.c:1646  */
    break;

  case 17:
#line 135 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_NAMESPACE);
   pocc_interpreter_register_next_command(POCC_CMD_PONOS);
   }
#line 1531 "commands.c" /* yacc.c:1646  */
    break;

  case 18:
#line 139 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_DISPLAY);
   pocc_interpreter_register_next_command(POCC_CMD_OPTIONS);
   }
#line 1540 "commands.c" /* yacc.c:1646  */
    break;

  case 19:
#line 143 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_DISPLAY);
   pocc_interpreter_register_next_command(POCC_CMD_SPACE);
   }
#line 1549 "commands.c" /* yacc.c:1646  */
    break;

  case 20:
#line 147 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_DISPLAY);
   pocc_interpreter_register_next_command(POCC_CMD_ALL_VARS);
   }
#line 1558 "commands.c" /* yacc.c:1646  */
    break;

  case 21:
#line 151 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_DISPLAY);
   pocc_interpreter_register_next_command(POCC_CMD_ALL_VARS_DETAILED);
   }
#line 1567 "commands.c" /* yacc.c:1646  */
    break;

  case 22:
#line 155 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_DISPLAY);
   pocc_interpreter_register_next_command(POCC_CMD_CPLEX_LP);
   }
#line 1576 "commands.c" /* yacc.c:1646  */
    break;

  case 23:
#line 159 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_DISPLAY);
   pocc_interpreter_register_next_command(POCC_CMD_SOL_LEXMIN);
   }
#line 1585 "commands.c" /* yacc.c:1646  */
    break;

  case 24:
#line 163 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_SET);
   pocc_interpreter_register_next_command(POCC_CMD_OPTION);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ((yyvsp[-1].strval));
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ((yyvsp[0].strval));
   }
#line 1598 "commands.c" /* yacc.c:1646  */
    break;

  case 25:
#line 171 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_SET);
   pocc_interpreter_register_next_command(POCC_CMD_OPTION);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ((yyvsp[-1].strval));
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     (void *)((intptr_t)((yyvsp[0].intval)));
   }
#line 1611 "commands.c" /* yacc.c:1646  */
    break;

  case 26:
#line 179 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_COMPUTE);
   pocc_interpreter_register_next_command(POCC_CMD_LEGAL_SPACE);
   }
#line 1620 "commands.c" /* yacc.c:1646  */
    break;

  case 27:
#line 183 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_COMPUTE);
   pocc_interpreter_register_next_command(POCC_CMD_SCHEDULING_CONSTRAINTS);
   }
#line 1629 "commands.c" /* yacc.c:1646  */
    break;

  case 28:
#line 187 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_COMPUTE);
   pocc_interpreter_register_next_command(POCC_CMD_SCHEDULE);
   }
#line 1638 "commands.c" /* yacc.c:1646  */
    break;

  case 29:
#line 191 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_COMPUTE);
   pocc_interpreter_register_next_command(POCC_CMD_IS_INCLUDED);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ((yyvsp[0].strval));
   }
#line 1649 "commands.c" /* yacc.c:1646  */
    break;

  case 30:
#line 197 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_DISPLAY);
   pocc_interpreter_register_next_command(POCC_CMD_VAR);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ((yyvsp[0].strval));
   }
#line 1660 "commands.c" /* yacc.c:1646  */
    break;

  case 31:
#line 203 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_DISPLAY);
   pocc_interpreter_register_next_command(POCC_CMD_CONSTRAINTS);
   pocc_interpreter_register_next_command(POCC_CMD_MATCHING);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ((yyvsp[0].strval));
   }
#line 1672 "commands.c" /* yacc.c:1646  */
    break;

  case 32:
#line 210 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_CONSTRAINT);
   pocc_interpreter_register_next_command(POCC_CMD_DISPLAY);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ((yyvsp[0].strval));
   }
#line 1683 "commands.c" /* yacc.c:1646  */
    break;

  case 33:
#line 216 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_CONSTRAINT);
   pocc_interpreter_register_next_command(POCC_CMD_DISPLAY);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ((yyvsp[0].strval));
   }
#line 1694 "commands.c" /* yacc.c:1646  */
    break;

  case 34:
#line 222 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_CONSTRAINT);
   pocc_interpreter_register_next_command(POCC_CMD_DELETE);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ((yyvsp[0].strval));
   }
#line 1705 "commands.c" /* yacc.c:1646  */
    break;

  case 35:
#line 228 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_CONSTRAINT);
   pocc_interpreter_register_next_command(POCC_CMD_DELETE);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ((yyvsp[0].strval));
   }
#line 1716 "commands.c" /* yacc.c:1646  */
    break;

  case 36:
#line 234 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_DELETE);
   pocc_interpreter_register_next_command(POCC_CMD_ALL_CONSTRAINTS);
   }
#line 1725 "commands.c" /* yacc.c:1646  */
    break;

  case 37:
#line 238 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_CONSTRAINT);
   pocc_interpreter_register_next_command(POCC_CMD_INSERT);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ((yyvsp[0].strval));
   }
#line 1736 "commands.c" /* yacc.c:1646  */
    break;

  case 38:
#line 244 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_INSERT);
   pocc_interpreter_register_next_command(POCC_CMD_CONSTRAINT);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ((yyvsp[0].strval));
   }
#line 1747 "commands.c" /* yacc.c:1646  */
    break;

  case 39:
#line 250 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_INSERT);
   pocc_interpreter_register_next_command(POCC_CMD_VARIABLE);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ((yyvsp[-1].strval));
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     (void *)((intptr_t)((yyvsp[0].intval)));
   }
#line 1760 "commands.c" /* yacc.c:1646  */
    break;

  case 40:
#line 258 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_INSERT);
   pocc_interpreter_register_next_command(POCC_CMD_VARIABLE);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ((yyvsp[-1].strval));
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     (void *)((intptr_t)((yyvsp[0].intval)));
   }
#line 1773 "commands.c" /* yacc.c:1646  */
    break;

  case 41:
#line 266 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_SET);
   pocc_interpreter_register_next_command(POCC_CMD_VARIABLE_BOUNDS);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ((yyvsp[-2].strval));
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     (void *)((intptr_t)((yyvsp[-1].intval)));
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     (void *)((intptr_t)((yyvsp[0].intval)));
   }
#line 1788 "commands.c" /* yacc.c:1646  */
    break;

  case 42:
#line 279 "commands.y" /* yacc.c:1646  */
    { (yyval.intval) = (yyvsp[0].intval); }
#line 1794 "commands.c" /* yacc.c:1646  */
    break;

  case 43:
#line 280 "commands.y" /* yacc.c:1646  */
    { (yyval.intval) = 0; }
#line 1800 "commands.c" /* yacc.c:1646  */
    break;

  case 44:
#line 281 "commands.y" /* yacc.c:1646  */
    { (yyval.intval) = POCC_INTERPRETER_SPACE_LAST_POS; }
#line 1806 "commands.c" /* yacc.c:1646  */
    break;

  case 45:
#line 285 "commands.y" /* yacc.c:1646  */
    { (yyval.strval) = (yyvsp[0].strval); }
#line 1812 "commands.c" /* yacc.c:1646  */
    break;

  case 46:
#line 286 "commands.y" /* yacc.c:1646  */
    { (yyval.strval) = (yyvsp[0].strval); }
#line 1818 "commands.c" /* yacc.c:1646  */
    break;

  case 47:
#line 290 "commands.y" /* yacc.c:1646  */
    { pocc_interpreter_register_next_command(POCC_CMD_HELP); }
#line 1824 "commands.c" /* yacc.c:1646  */
    break;

  case 48:
#line 291 "commands.y" /* yacc.c:1646  */
    { pocc_interpreter_register_next_command(POCC_CMD_QUIT); }
#line 1830 "commands.c" /* yacc.c:1646  */
    break;

  case 49:
#line 292 "commands.y" /* yacc.c:1646  */
    { pocc_interpreter_register_next_command(POCC_CMD_SHOW); }
#line 1836 "commands.c" /* yacc.c:1646  */
    break;

  case 50:
#line 293 "commands.y" /* yacc.c:1646  */
    { pocc_interpreter_register_next_command(POCC_CMD_INFO); }
#line 1842 "commands.c" /* yacc.c:1646  */
    break;

  case 51:
#line 294 "commands.y" /* yacc.c:1646  */
    {
   pocc_interpreter_register_next_command(POCC_CMD_INITIALIZE);
   }
#line 1850 "commands.c" /* yacc.c:1646  */
    break;

  case 52:
#line 300 "commands.y" /* yacc.c:1646  */
    { pocc_interpreter_register_next_command(POCC_CMD_POCC); }
#line 1856 "commands.c" /* yacc.c:1646  */
    break;

  case 53:
#line 301 "commands.y" /* yacc.c:1646  */
    { pocc_interpreter_register_next_command(POCC_CMD_PONOS); }
#line 1862 "commands.c" /* yacc.c:1646  */
    break;


#line 1866 "commands.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (scanner, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (scanner, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



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
                      yytoken, &yylval, scanner);
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

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
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


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, scanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


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

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (scanner, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, scanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, scanner);
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
  return yyresult;
}
#line 305 "commands.y" /* yacc.c:1906  */


void yyerror(void* scanner, const char *s)
{
  printf ("|> ");
  printf (s);
  printf ("\n");
  pocc_interpreter_register_next_command(POCC_CMD_ERROR);
  g_pocc_current_command.cmd_id = POCC_CMD_ERROR;
}
