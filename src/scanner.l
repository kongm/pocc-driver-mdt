/*
 * scanner.l: this file is part of the PoCC project.
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

%option noyywrap
%option reentrant noyywrap never-interactive nounistd nostdinit
%option bison-bridge
%option prefix="poccinterpreter_yy"
%option outfile="scanner.c"

%{
#include "commands.h"
#include "interpreter.h"

#define yylex poccinterpreter_yylex
#define YYSTYPE POCCINTERPRETER_YYSTYPE

extern int poccinterpreter_yyparse(void* scanner);
static int handle_token(int token_type, YYSTYPE* poccinterpreter_yylval, void* scanner);

%}

%%

"#"[^\n]*               ;
[ \t\n]			;
"+"			{ return OP_PLUS; }
"-"			{ return OP_MINUS; }
"*"			{ return OP_MUL; }
"/"			{ return OP_DIV; }
"info"       		{ return CMD_INFO; }
"switch"       		{ return CMD_SWITCH; }
"list"       		{ return CMD_LIST; }
"quit"			{ return CMD_QUIT; }
"show"			{ return CMD_SHOW; }
"load"			{ return CMD_LOAD; }
"store"			{ return CMD_STORE; }
"display"		{ return CMD_DISPLAY; }
"scop"			{ return CMD_SCOP; }
"context"      		{ return CMD_CONTEXT; }
"help"			{ return CMD_HELP; }
"ponos"			{ return CMD_PONOS; }
"namespace"    		{ return CMD_NAMESPACE; }
"workspace"    		{ return CMD_WORKSPACE; }
"pocc"    		{ return CMD_POCC; }
"c"			{ return CMD_C; }
"options"      		{ return CMD_OPTIONS; }
"initialize"		{ return CMD_INITIALIZE; }
"option"		{ return CMD_OPTION; }
"set"			{ return CMD_SET; }
"compute"      		{ return CMD_COMPUTE; }
"legal-space"		{ return CMD_LEGAL_SPACE; }
"schedule"		{ return CMD_SCHEDULE; }
"var"			{ return CMD_VAR; }
"vars"			{ return CMD_VARS; }
"all-vars"     		{ return CMD_ALL_VARS; }
"all-vars-detail"	{ return CMD_ALL_VARS_DETAILED; }
"space"			{ return CMD_SPACE; }
"cplex-lp"		{ return CMD_CPLEX_LP; }
"constraint"		{ return CMD_CONSTRAINT; }
"constraints"		{ return CMD_CONSTRAINTS; }
"matching"		{ return CMD_MATCHING; }
"scheduling-constraints" { return CMD_SCHEDULING_CONSTRAINTS; }
"delete"		{ return CMD_DELETE; }
"#"			{ return TOKEN_COMMENT; }
"insert"		{ return CMD_INSERT; }
"variable"		{ return CMD_VARIABLE; }
"first"			{ return TOKEN_FIRST; }
"last"			{ return TOKEN_LAST; }
"variable-bounds"      	{ return CMD_VARIABLE_BOUNDS; }
"sol-lexmin"	      	{ return CMD_SOL_LEXMIN; }
"all-constraints"       { return CMD_ALL_CONSTRAINTS; }
"is-included"           { return CMD_IS_INCLUDED; }
"stats"                 { return CMD_STATS; }


\"(\\.|[^"])*\"		{ return handle_token (STRING, yylval, yyscanner); }
\s*[-]?\s*[0-9]+\s*	{ return handle_token (INT, yylval, yyscanner); }
[0-9]+\.[0-9]+		{ return handle_token (FLOAT, yylval, yyscanner); }
[a-zA-Z_][_a-zA-Z0-9\.\-]+  { return handle_token (ID, yylval, yyscanner); }
.			;

%%


static int handle_token(int token_type, YYSTYPE* poccinterpreter_yylval, 
                        void* scanner)
{
   // Needed to get yytext.
   struct yyguts_t * yyg;
   yyg = (struct yyguts_t*)scanner;
   switch (token_type)
     {
	case INT: poccinterpreter_yylval->intval = atoi (yytext); break;
	case FLOAT: poccinterpreter_yylval->floatval = atof (yytext); break;
	case STRING: poccinterpreter_yylval->strval = strdup (yytext); break;
	case ID: poccinterpreter_yylval->strval = strdup (yytext); break;
        default: poccinterpreter_yylval->token = token_type; break;
     }
   //printf ("token: %s\n", yytext);
   return token_type;
}


int pocc_scanner(char* cmd)
{
  int i;
  g_pocc_current_command.cmd_last = 0;
  g_pocc_current_command.cmd_args_last = 0;
  for (i = 0; i < POCC_INTERPRETER_MAX_ARGS; ++i)
    {
      g_pocc_current_command.cmd_args[i] = NULL;
      g_pocc_current_command.cmd_ids[i] = POCC_CMD_END;
    }
  g_pocc_current_command.cmd_id = POCC_CMD_END;

  yyscan_t scanner;
  poccinterpreter_yylex_init(&scanner);
  YY_BUFFER_STATE buffer = poccinterpreter_yy_scan_string (cmd, scanner);
  poccinterpreter_yyparse (scanner);
  poccinterpreter_yy_delete_buffer (buffer, scanner);
  poccinterpreter_yylex_destroy(scanner);

  if (g_pocc_current_command.cmd_id = POCC_CMD_END)
    g_pocc_current_command.cmd_id = g_pocc_current_command.cmd_ids[0];
}
