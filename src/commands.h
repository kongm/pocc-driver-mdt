/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

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
#line 41 "commands.y" /* yacc.c:1909  */

  int	intval;
  float	floatval;
  char*	strval;
  int	token;
  int	rule_output;

#line 180 "commands.h" /* yacc.c:1909  */
};

typedef union POCCINTERPRETER_YYSTYPE POCCINTERPRETER_YYSTYPE;
# define POCCINTERPRETER_YYSTYPE_IS_TRIVIAL 1
# define POCCINTERPRETER_YYSTYPE_IS_DECLARED 1
#endif



int poccinterpreter_yyparse (void *scanner);

#endif /* !YY_POCCINTERPRETER_YY_COMMANDS_H_INCLUDED  */
