/*
 * interpreter_utils.c: this file is part of the PoCC project.
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
#if HAVE_CONFIG_H
# include <pocc-utils/config.h>
#endif
#include <pocc/common.h>

#include <stdio.h>

#include <pocc/exec.h>

#include "interpreter_utils.h"


char*
pocc_interpreter_execute_command_output(int num_args, ...)
{
    va_list arguments;
    char* args[num_args + 1];
    va_start (arguments, num_args);
    int i;
    for (i = 0; i < num_args; ++i)
      args[i] = va_arg (arguments, char*);
    va_end (arguments);
    args[i] = NULL;
    char* ret = pocc_exec_string_noexit (args, POCC_EXECV_HIDE_OUTPUT);
    return ret;
}

void
pocc_interpreter_execute_command_silent(int num_args, ...)
{
    va_list arguments;
    char* args[num_args + 1];
    va_start (arguments, num_args);
    int i;
    for (i = 0; i < num_args; ++i)
      args[i] = va_arg (arguments, char*);
    va_end (arguments);
    args[i] = NULL;
    pocc_exec (args, POCC_EXECV_HIDE_OUTPUT);
}


void
pocc_interpreter_display_file(FILE* stream, char* filename)
{
  FILE* f = fopen (filename, "r");
  if (f)
    {
      char buffer[1024];
      while (fgets(buffer, 1024, f) != NULL)
	fprintf (stream, "%s", buffer);
      fclose (f);
    }
  else
    printf ("|> error: cannot display file %s\n", filename);
}


void
pocc_interpreter_print_scop_schedule(FILE* fout, scoplib_statement_p stmt,
				     scoplib_scop_p scop,
				     int stmt_id)
{
  int i, j;

  fprintf (fout, "Theta_S%d(", stmt_id);
  for (i = 0; i < stmt->nb_iterators; ++i)
    {
      fprintf (fout, "%s", stmt->iterators[i]);
      if (i < stmt->nb_iterators - 1)
	fprintf (fout, ",");
    }
  fprintf (fout, ")\t=\t(");

  for (i = 0; i < stmt->schedule->NbRows; ++i)
    {
      int is_first = 1;
      for (j = 1; j < stmt->schedule->NbColumns; ++j)
	{
	  int val = SCOPVAL_get_si(stmt->schedule->p[i][j]);
	  if (val)
	    {
	      char* str;
	      if (j <= stmt->nb_iterators)
		str = stmt->iterators[j - 1];
	      else if (j < stmt->schedule->NbColumns - 1)
		str = scop->parameters[j - 1 - stmt->nb_iterators];
	      else
		str = NULL;

	      if (! is_first)
		if (val > 0)
		  fprintf (fout, "+");
	      if (str && (val < 1 || val > 1))
		fprintf (fout, "%d%s", val, str);
	      else if (str)
		fprintf (fout, "%s", str);
	      else
		fprintf (fout, "%d", val);
	      is_first = 0;
	    }
	}
      if (is_first == 1)
	fprintf (fout, "0");
      if (i < stmt->schedule->NbRows - 1)
	fprintf (fout, ", ");
    }
  fprintf (fout, ")\n");


}


void
pocc_interpreter_print_scop_summary(FILE* fout, scoplib_scop_p scop)
{
  fprintf (fout, "|> Scop summary:\n");
  int nb_stmts;
  int i;
  scoplib_statement_p s;
  for (s = scop->statement, nb_stmts = 0; s; s = s->next, ++nb_stmts)
    ;
  fprintf (fout, "Number of statements:\t%d\n", nb_stmts);
  fprintf (fout, "Symbols:\t\t");
  for (i = 0; i < scop->nb_arrays; ++i)
    fprintf (fout, "%s ", scop->arrays[i]);
  fprintf (fout, "\n");
  fprintf (fout, "Parameter names:\t");
  for (i = 0; i < scop->nb_parameters; ++i)
    fprintf (fout, "%s ", scop->parameters[i]);
  fprintf (fout, "\n");
  for (s = scop->statement, nb_stmts = 0; s; s = s->next, ++nb_stmts)
    pocc_interpreter_print_scop_schedule (fout, s, scop, nb_stmts);
}


void
pocc_interpreter_info(s_interpreter_state_t* state, s_pocc_command_t* cmd)
{
  printf ("|> Workspace #%d: info\n", state->active_workspace);
  pocc_interpreter_show_current_workspace (state);
}


char*
pocc_interpreter_utils_remove_double_quote(char* s)
{
  if (s && *s == '"')
    {
      ++s;
      s[strlen (s) - 1] = '\0';
    }
  return s;
}
