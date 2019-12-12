/*
 * commands.y: this file is part of the PoCC project.
 *
 * PoCC, the Polyhedral Compiler Collection package
 *
 * Copyright (C) 2017 Louis-Noel Pouchet
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * The complete GNU General Public Licence Notice can be found as the
 * `COPYING.LESSER' file in the root directory.
 *
 * Author:
 * Louis-Noel Pouchet <pouchet@colostate.edu>
 *
 */


%{
#include <stdio.h>
void yyerror(void* scanner, const char *s);

#include "interpreter.h"

%}

%define api.prefix {poccinterpreter_yy}
%define parse.error verbose
%define api.pure full
%lex-param { void *scanner }
%parse-param { void *scanner }

%union {
  int	intval;
  float	floatval;
  char*	strval;
  int	token;
  int	rule_output;
}

%token <intval> INT
%token <floatval> FLOAT
%token <strval> ID
%token <strval> STRING
%token OP_PLUS OP_MINUS OP_MUL OP_DIV
%token CMD_INFO CMD_SWITCH CMD_LIST CMD_QUIT CMD_SHOW CMD_LOAD CMD_STORE CMD_DISPLAY CMD_SCOP CMD_CONTEXT CMD_HELP CMD_PONOS CMD_NAMESPACE CMD_C CMD_POCC CMD_OPTIONS CMD_INITIALIZE CMD_OPTION CMD_SET CMD_COMPUTE CMD_LEGAL_SPACE CMD_SCHEDULE CMD_SPACE CMD_VARS CMD_CPLEX_LP CMD_WORKSPACE CMD_VAR CMD_ALL_VARS CMD_ALL_VARS_DETAILED CMD_CONSTRAINTS CMD_CONSTRAINT CMD_MATCHING CMD_SCHEDULING_CONSTRAINTS CMD_DELETE TOKEN_COMMENT CMD_INSERT CMD_VARIABLE TOKEN_FIRST TOKEN_LAST CMD_VARIABLE_BOUNDS CMD_SOL_LEXMIN CMD_ALL_CONSTRAINTS CMD_IS_INCLUDED CMD_STATS

%left MAXPRIORITY /* Dummy token to help in removing shift/reduce conflicts */

%type <strval> id_or_string
%type <intval> position

%%

command:
 | namespace commands_arg %prec MAXPRIORITY
 | namespace commands_unique
 ;

/* command: */
/*  | comment namespace commands_arg %prec MAXPRIORITY */
/*  | comment namespace commands_unique */
/*  ; */

/* comment: */
/*    TOKEN_COMMENT { pocc_interpreter_register_next_command(POCC_CMD_END); } */
/*  | */
/*  ; */

commands_arg:
   CMD_LOAD ID    {
   pocc_interpreter_register_next_command(POCC_CMD_LOAD);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ($2);
   }
 | CMD_STORE ID    {
   pocc_interpreter_register_next_command(POCC_CMD_STORE);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ($2);
   }
 | CMD_LOAD CMD_CONSTRAINTS ID    {
   pocc_interpreter_register_next_command(POCC_CMD_LOAD);
   pocc_interpreter_register_next_command(POCC_CMD_CONSTRAINTS);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ($3);
   }
 | CMD_STORE CMD_CONSTRAINTS ID    {
   pocc_interpreter_register_next_command(POCC_CMD_STORE);
   pocc_interpreter_register_next_command(POCC_CMD_CONSTRAINTS);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ($3);
   }
 | CMD_SHOW CMD_SCOP {
   pocc_interpreter_register_next_command(POCC_CMD_SHOW);
   pocc_interpreter_register_next_command(POCC_CMD_SCOP);
   }
 | CMD_SHOW CMD_WORKSPACE {
   pocc_interpreter_register_next_command(POCC_CMD_SHOW);
   pocc_interpreter_register_next_command(POCC_CMD_WORKSPACE);
   }
 | CMD_SHOW CMD_STATS {
   pocc_interpreter_register_next_command(POCC_CMD_SHOW);
   pocc_interpreter_register_next_command(POCC_CMD_STATS);
   }
 | CMD_SHOW CMD_C {
   pocc_interpreter_register_next_command(POCC_CMD_SHOW);
   pocc_interpreter_register_next_command(POCC_CMD_C);
   }
 | CMD_WORKSPACE CMD_LIST  {
   pocc_interpreter_register_next_command(POCC_CMD_WORKSPACE);
   pocc_interpreter_register_next_command(POCC_CMD_LIST);
   }
 | CMD_WORKSPACE CMD_INFO  {
   pocc_interpreter_register_next_command(POCC_CMD_WORKSPACE);
   pocc_interpreter_register_next_command(POCC_CMD_INFO);
   }
 | CMD_WORKSPACE CMD_SWITCH INT {
   pocc_interpreter_register_next_command(POCC_CMD_WORKSPACE);
   pocc_interpreter_register_next_command(POCC_CMD_SWITCH);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     (void *)((intptr_t)($3));
   }
 | CMD_NAMESPACE CMD_POCC {
   pocc_interpreter_register_next_command(POCC_CMD_NAMESPACE);
   pocc_interpreter_register_next_command(POCC_CMD_POCC);
   }
 | CMD_NAMESPACE CMD_PONOS {
   pocc_interpreter_register_next_command(POCC_CMD_NAMESPACE);
   pocc_interpreter_register_next_command(POCC_CMD_PONOS);
   }
 | CMD_DISPLAY CMD_OPTIONS {
   pocc_interpreter_register_next_command(POCC_CMD_DISPLAY);
   pocc_interpreter_register_next_command(POCC_CMD_OPTIONS);
   }
 | CMD_DISPLAY CMD_SPACE {
   pocc_interpreter_register_next_command(POCC_CMD_DISPLAY);
   pocc_interpreter_register_next_command(POCC_CMD_SPACE);
   }
 | CMD_DISPLAY CMD_ALL_VARS {
   pocc_interpreter_register_next_command(POCC_CMD_DISPLAY);
   pocc_interpreter_register_next_command(POCC_CMD_ALL_VARS);
   }
 | CMD_DISPLAY CMD_ALL_VARS_DETAILED {
   pocc_interpreter_register_next_command(POCC_CMD_DISPLAY);
   pocc_interpreter_register_next_command(POCC_CMD_ALL_VARS_DETAILED);
   }
 | CMD_DISPLAY CMD_CPLEX_LP {
   pocc_interpreter_register_next_command(POCC_CMD_DISPLAY);
   pocc_interpreter_register_next_command(POCC_CMD_CPLEX_LP);
   }
 | CMD_DISPLAY CMD_SOL_LEXMIN {
   pocc_interpreter_register_next_command(POCC_CMD_DISPLAY);
   pocc_interpreter_register_next_command(POCC_CMD_SOL_LEXMIN);
   }
 | CMD_SET CMD_OPTION ID id_or_string {
   pocc_interpreter_register_next_command(POCC_CMD_SET);
   pocc_interpreter_register_next_command(POCC_CMD_OPTION);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ($3);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ($4);
   }
 | CMD_SET CMD_OPTION ID INT {
   pocc_interpreter_register_next_command(POCC_CMD_SET);
   pocc_interpreter_register_next_command(POCC_CMD_OPTION);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ($3);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     (void *)((intptr_t)($4));
   }
 | CMD_COMPUTE CMD_LEGAL_SPACE {
   pocc_interpreter_register_next_command(POCC_CMD_COMPUTE);
   pocc_interpreter_register_next_command(POCC_CMD_LEGAL_SPACE);
   }
 | CMD_COMPUTE CMD_SCHEDULING_CONSTRAINTS {
   pocc_interpreter_register_next_command(POCC_CMD_COMPUTE);
   pocc_interpreter_register_next_command(POCC_CMD_SCHEDULING_CONSTRAINTS);
   }
 | CMD_COMPUTE CMD_SCHEDULE {
   pocc_interpreter_register_next_command(POCC_CMD_COMPUTE);
   pocc_interpreter_register_next_command(POCC_CMD_SCHEDULE);
   }
 | CMD_COMPUTE CMD_IS_INCLUDED STRING {
   pocc_interpreter_register_next_command(POCC_CMD_COMPUTE);
   pocc_interpreter_register_next_command(POCC_CMD_IS_INCLUDED);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ($3);
   }
 | CMD_DISPLAY CMD_VARIABLE ID {
   pocc_interpreter_register_next_command(POCC_CMD_DISPLAY);
   pocc_interpreter_register_next_command(POCC_CMD_VAR);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ($3);
   }
 | CMD_DISPLAY CMD_CONSTRAINTS CMD_MATCHING id_or_string {
   pocc_interpreter_register_next_command(POCC_CMD_DISPLAY);
   pocc_interpreter_register_next_command(POCC_CMD_CONSTRAINTS);
   pocc_interpreter_register_next_command(POCC_CMD_MATCHING);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ($4);
   }
 | CMD_CONSTRAINT CMD_DISPLAY ID {
   pocc_interpreter_register_next_command(POCC_CMD_CONSTRAINT);
   pocc_interpreter_register_next_command(POCC_CMD_DISPLAY);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ($3);
   }
 | CMD_DISPLAY CMD_CONSTRAINT ID {
   pocc_interpreter_register_next_command(POCC_CMD_CONSTRAINT);
   pocc_interpreter_register_next_command(POCC_CMD_DISPLAY);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ($3);
   }
 | CMD_CONSTRAINT CMD_DELETE ID {
   pocc_interpreter_register_next_command(POCC_CMD_CONSTRAINT);
   pocc_interpreter_register_next_command(POCC_CMD_DELETE);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ($3);
   }
 | CMD_DELETE CMD_CONSTRAINT ID {
   pocc_interpreter_register_next_command(POCC_CMD_CONSTRAINT);
   pocc_interpreter_register_next_command(POCC_CMD_DELETE);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ($3);
   }
 | CMD_DELETE CMD_ALL_CONSTRAINTS {
   pocc_interpreter_register_next_command(POCC_CMD_DELETE);
   pocc_interpreter_register_next_command(POCC_CMD_ALL_CONSTRAINTS);
   }
 | CMD_CONSTRAINT CMD_INSERT id_or_string  {
   pocc_interpreter_register_next_command(POCC_CMD_CONSTRAINT);
   pocc_interpreter_register_next_command(POCC_CMD_INSERT);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ($3);
   }
 | CMD_INSERT CMD_CONSTRAINT id_or_string  {
   pocc_interpreter_register_next_command(POCC_CMD_INSERT);
   pocc_interpreter_register_next_command(POCC_CMD_CONSTRAINT);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ($3);
   }
 | CMD_INSERT CMD_VARIABLE id_or_string position  {
   pocc_interpreter_register_next_command(POCC_CMD_INSERT);
   pocc_interpreter_register_next_command(POCC_CMD_VARIABLE);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ($3);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     (void *)((intptr_t)($4));
   }
 | CMD_VARIABLE CMD_INSERT id_or_string position  {
   pocc_interpreter_register_next_command(POCC_CMD_INSERT);
   pocc_interpreter_register_next_command(POCC_CMD_VARIABLE);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ($3);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     (void *)((intptr_t)($4));
   }
 | CMD_SET CMD_VARIABLE_BOUNDS id_or_string INT INT  {
   pocc_interpreter_register_next_command(POCC_CMD_SET);
   pocc_interpreter_register_next_command(POCC_CMD_VARIABLE_BOUNDS);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     strdup ($3);
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     (void *)((intptr_t)($4));
   g_pocc_current_command.cmd_args[g_pocc_current_command.cmd_args_last++] =
     (void *)((intptr_t)($5));
   }
 ;

position:
   INT { $$ = $1; }
 | TOKEN_FIRST { $$ = 0; }
 | TOKEN_LAST { $$ = POCC_INTERPRETER_SPACE_LAST_POS; }
 ;

id_or_string:
   ID { $$ = $1; }
 | STRING { $$ = $1; }


commands_unique:
   CMD_HELP { pocc_interpreter_register_next_command(POCC_CMD_HELP); }
 | CMD_QUIT { pocc_interpreter_register_next_command(POCC_CMD_QUIT); }
 | CMD_SHOW { pocc_interpreter_register_next_command(POCC_CMD_SHOW); }
 | CMD_INFO { pocc_interpreter_register_next_command(POCC_CMD_INFO); }
 | CMD_INITIALIZE {
   pocc_interpreter_register_next_command(POCC_CMD_INITIALIZE);
   }
 ;

namespace:
   CMD_POCC %prec MAXPRIORITY { pocc_interpreter_register_next_command(POCC_CMD_POCC); }
 | CMD_PONOS %prec MAXPRIORITY { pocc_interpreter_register_next_command(POCC_CMD_PONOS); }
 |
 ;

%%

void yyerror(void* scanner, const char *s)
{
  printf ("|> ");
  printf (s);
  printf ("\n");
  pocc_interpreter_register_next_command(POCC_CMD_ERROR);
  g_pocc_current_command.cmd_id = POCC_CMD_ERROR;
}
