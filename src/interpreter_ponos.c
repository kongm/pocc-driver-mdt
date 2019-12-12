/*
 * interpreter_ponos.c: this file is part of the PoCC project.
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
/* #include <readline/readline.h> */
/* #include <readline/history.h> */

#include <pocc/driver-clan.h>
#include <pocc/driver-candl.h>
#include <pocc/driver-letsee.h>
#include <pocc/driver-pluto.h>
#include <pocc/driver-codegen.h>
#include <pocc/driver-cloog.h>
#include <pocc/driver-clastops.h>
#include <pocc/driver-ponos.h>
#include <pocc/driver-psimdkzer.h>
#include <pocc/exec.h>
#include <pocc/error.h>
#ifdef POCC_DEVEL_MODE
# include <pocc/driver-cdscgr.h>
#endif


// Assume candl has been built with ISL support.
#ifndef CANDL_SUPPORTS_ISL
# define CANDL_SUPPORTS_ISL
#endif
#include <ponos/ponos.h>
#include <ponos/space.h>
#include <ponos/legal.h>
#include <ponos/solver-gmp.h>
#include <ponos/objectives.h>
#include <ponos/constraints.h>
#include <ponos/codelet.h>
#include <ponos/solver.h>
#include <candl/dependence.h>


#include "interpreter_ponos.h"
#include "interpreter_utils.h"
#include "interpreter_workspace.h"


/**
 * Ugly workaround, need fix to pass that variable as func. argument
 * everywhere instead.
 */
static s_interpreter_option_t* g_pocc_interpreter_ponos_option;


static
int
pocc_interpreter_ponos_process_command(s_interpreter_command_t* all_commands,
				       s_interpreter_state_t* state,
				       s_pocc_command_t* cmd)
{
  int i;
  for (i = 0; all_commands[i].fun_ptr != NULL; ++i)
    if (all_commands[i].cmd_id == g_pocc_current_command.cmd_id)
      {
	if (all_commands[i].fun_ptr)
	  all_commands[i].fun_ptr (state, cmd);
	break;
      }
  if (all_commands[i].fun_ptr == NULL)
    printf ("|> command not found\n");

  return EXIT_SUCCESS;
}


static
void
synchronize_constraints_from_space(s_interpreter_state_t* state)
{
  s_ponos_space_t* space = state->ponos_space[state->active_workspace];
  if (space)
    {
      if (state->ponos_constraints_system[state->active_workspace])
	fm_system_free
	  (state->ponos_constraints_system[state->active_workspace]);
      state->ponos_constraints_system[state->active_workspace] =
	fm_solution_to_system (space->space);
    }
}

static
void
synchronize_space_from_constraints(s_interpreter_state_t* state)
{
  s_ponos_space_t* space = state->ponos_space[state->active_workspace];
  if (space)
    {
      s_fm_system_t* s =
	state->ponos_constraints_system[state->active_workspace];
      if (s)
	{
	  if (space->space)
	    fm_solution_free (space->space);
	  space->space = fm_system_to_solution (s);
	}
    }
}


static
void
ponos_print_constraints_with_id(FILE* stream, s_interpreter_state_t* state)
{
  s_ponos_space_t* space = state->ponos_space[state->active_workspace];
  if (space == NULL || space->space == NULL)
    {
      printf ("|> error: space not ready for display. Try 'compute legal-space'\n");
      return;
    }
  if (state->ponos_constraints_system[state->active_workspace] == NULL)
    {
      state->ponos_constraints_system[state->active_workspace] =
	fm_solution_to_system (space->space);
    }
  s_fm_system_t* s = state->ponos_constraints_system[state->active_workspace];
  int i;
  for (i = 0; i < s->nb_lines; ++i)
    {
      fprintf (stream, "c%d: ", i);
      ponos_space_pretty_print_vector (stream, space, s->lines[i]);
    }
}


static
void
ponos_show_constraint_filter_regexp(s_interpreter_state_t* state,
				    s_pocc_command_t* cmd)
{
  if (! cmd || ! cmd->cmd_args || ! cmd->cmd_args[0])
    {
      printf ("|> error: ill-formed command. try 'display constraints matching\"<regexp>\"'\n");
      return;
    }
  if (! state->ponos_space[state->active_workspace] ||
      ! state->ponos_space[state->active_workspace]->space)
    {
      printf ("|> error: Ponos space not computed yet. try 'compute legal-space'\n");
      return;
    }
  // Make sure the constraints representation is available.
  if (! state->ponos_constraints_system[state->active_workspace])
    {
      state->ponos_constraints_system[state->active_workspace] =
	fm_solution_to_system
	(state->ponos_space[state->active_workspace]->space);
    }
  char* varregexp = cmd->cmd_args[0];
  // Remove the "" around the string, if any.
  varregexp = pocc_interpreter_utils_remove_double_quote (varregexp);

  // Dump the current constraints into temporary file.
  char* tempfilename = "___temp_pocc_interpreter.immediate.tmp";
  FILE* f = fopen (tempfilename, "w");
  if (! f)
    {
      printf ("|> error: cannot create temporary file %s\n", tempfilename);
      return;
    }
  ponos_print_constraints_with_id (f, state);
  fclose (f);

  // Grep with the regex, display the result.
  char* output = pocc_interpreter_execute_command_output
    (3, "grep", varregexp, tempfilename);
  // Delete temporary file.
  pocc_interpreter_execute_command_silent (3, "rm", "-f", tempfilename);

  // Display the output.
  printf ("|> Constraints matching pattern %s:\n", varregexp);
  if (output)
    printf ("%s\n", output);
  else
    printf ("[none]\n");
  XFREE(output);
}


static
void
ponos_help(s_interpreter_state_t* state,
	   s_pocc_command_t* cmd)
{
  printf ("|> help for ponos:\n");
  printf ("|> display options\t\tShow all available options\n");
  printf ("|> set option <option> <value>\tSet <option>=<value>\n");
  printf ("|> compute legal-space\t\tCompute the space of legal schedules\n");
  printf ("Note: the legal space needs to be recomputed when changing any option\n");
  printf ("related to scheduling or dependence analysis. It does not need to be\n");
  printf ("be recomputed when changing solver options.\n");
  printf ("|> compute scheduling-constraints\tCompute one schedule\n");
  printf ("|> compute schedule\t\tCompute one schedule\n");
  printf ("|> display all-vars\t\tList all variables in the space\n");
  printf ("|> display all-vars-detail\tShow details on all variables in the space\n");
  printf ("|> display variable <varname>\t\tShow details for <varname> in the space\n");
  printf ("|> display constraints matching <regexp>\nShow all constraints matching <regexp>\n");
  printf ("|> display space\t\tShow all constraints in the space\n");
  printf ("|> display cplex-lp\t\tShow the last Cplex .lp file\n");
  printf ("|> display constraints matching <regexp>\nShow all constraints matching <regexp>\n");
  printf ("|> display sol-lexmin\tShow current solution\n");
  printf ("|> display constraint <id>\tShow constraint <id>\n");
  printf ("|> delete constraint <id>\tDelete constraint <id>\n");
  printf ("|> insert constraint \"constraint\"\nInsert constraint in space\n");
  printf ("|> insert variable <var>\tInsert new variable in space\n");
  printf ("|> set variable-bounds <var> <lb> <ub>\nSet bounds for variable: <lb> <= <var> <= <ub>\n");
  printf ("|> load constraints <filename>\tLoad constraints from file\n");
  printf ("|> store constraints <filename>\tStore constraints to file\n");
  printf ("|> delete all-constraints\tDelete all constraints\n");
}

static
void
ponos_show(s_interpreter_state_t* state,
	   s_pocc_command_t* cmd)
{
  printf ("|> ponos status:\n");
}


static
void
ponos_end(s_interpreter_state_t* state,
	   s_pocc_command_t* cmd)
{
  // Do nothing, ignore command.
}


static
void
ponos_option_print_solver_type(char* buffer, void* arg)
{
  if (! buffer)
    return;
  sprintf (buffer, "<>");
  if (! arg)
    return;
  int type = *((int*)arg);

  switch (type)
    {
    case PONOS_SOLVER_CPLEX:
      sprintf (buffer, "cplex"); break;
    case PONOS_SOLVER_CPLEX_INCREMENTAL:
      sprintf (buffer, "cplex-incremental"); break;
    case PONOS_SOLVER_PIP:
      sprintf (buffer, "pip"); break;
    default:
      sprintf (buffer, "<unknown>"); break;
    }
}

static
void
ponos_option_set_solver_type(char* buffer_val, void* arg)
{
  if (! arg)
    return;
  int type = 0;
  if (! strcmp (buffer_val, "cplex"))
    type = PONOS_SOLVER_CPLEX;
  else if (! strcmp (buffer_val, "cplex-incremental"))
    type = PONOS_SOLVER_CPLEX_INCREMENTAL;
  else if (! strcmp (buffer_val, "pip"))
    type = PONOS_SOLVER_PIP;
  else
    {
      printf ("|> error: unrecognized solver type %s\n", buffer_val);
      return;
    }
  int* opt = (int*)arg;
  *opt = type;
}

static
void
ponos_option_print_objective(char* buffer, void* arg)
{
  if (! buffer)
    return;
  sprintf (buffer, "<>");
  if (! arg)
    return;
  int type = *((int*)arg);

  switch (type)
    {
    case PONOS_OBJECTIVES_NONE:
      sprintf (buffer, "none"); break;
    case PONOS_OBJECTIVES_CODELET:
      sprintf (buffer, "codelet"); break;
    case PONOS_OBJECTIVES_PLUTO:
      sprintf (buffer, "pluto"); break;
    case PONOS_OBJECTIVES_CUSTOM:
      sprintf (buffer, "custom"); break;
    default:
      sprintf (buffer, "<unknown>"); break;
    }
}


static
void
ponos_option_set_objective(char* buffer_val, void* arg)
{
  if (! arg)
    return;
  int type;
  if (! strcmp (buffer_val,  "none"))
    type = PONOS_OBJECTIVES_NONE;
  else if (! strcmp (buffer_val,  "codelet"))
    type = PONOS_OBJECTIVES_CODELET;
  else if (! strcmp (buffer_val,  "pluto"))
    type = PONOS_OBJECTIVES_PLUTO;
  else if (! strcmp (buffer_val,  "custom"))
    type = PONOS_OBJECTIVES_CUSTOM;
  else
    {
      printf ("|> error: unrecognized scheduler-type %s\n", buffer_val);
      return;
    }
  int* opt = (int*)arg;
  *opt = type;
}

static
void
ponos_option_print_objective_list(char* buffer, void* arg)
{
  if (! buffer)
    return;
  sprintf (buffer, "<>");
  if (! arg)
    return;
  sprintf (buffer, "");
  int* type = (int*)arg;
  int i;
  for (i = 0; type && type[i] != -1; ++i)
    {
      switch (type[i])
	{
	case PONOS_CONSTRAINTS_SUM_ITER_POS:
	  strcat (buffer, "sumiterpos"); break;
	case PONOS_CONSTRAINTS_PARAM_COEF_ZERO:
	  strcat (buffer, "paramcoef0"); break;
	case PONOS_OBJECTIVES_MAX_OUTER_PAR:
	  strcat (buffer, "maxouterpar"); break;
	case PONOS_OBJECTIVES_MAX_INNER_PAR:
	  strcat (buffer, "maxinnerpar"); break;
	case PONOS_OBJECTIVES_MAX_PERMUTABILITY:
	  strcat (buffer, "maxperm"); break;
	case PONOS_OBJECTIVES_MIN_DEP_DISTANCE:
	  strcat (buffer, "mindepdist"); break;
	case PONOS_OBJECTIVES_MAX_DEP_SOLVE:
	  strcat (buffer, "maxdepsolve"); break;
	case PONOS_CONSTRAINTS_LINEAR_INDEP:
	  strcat (buffer, "linearind"); break;
	case PONOS_OBJECTIVES_GAMMA_POS:
	  strcat (buffer, "gammapos"); break;
	case PONOS_OBJECTIVES_TASCHED:
	  strcat (buffer, "tasched"); break;
	case PONOS_OBJECTIVES_MIN_THETA_ITER:
	  strcat (buffer, "minitercoef"); break;
	default:
	  strcat (buffer, "<unknown>"); break;
	}
      if (type[i + 1] != -1)
	strcat (buffer, ",");
    }
}


static
void
ponos_option_set_objective_list(char* buffer_val, void* arg)
{
  if (! arg)
    return;
  int* opt = (int*)arg;
  opt[0] = -1;
  char buffer[1024];
  // Remove the "" around the string, if any.
  buffer_val = pocc_interpreter_utils_remove_double_quote (buffer_val);
  char* pos = buffer_val;
  int i;
  for (i = 0; pos && *pos; ++i)
    {
      int type = -1;
      char* start = pos;
      while (*pos && *pos != ',')
	++pos;
      if (*pos)
	*(pos++) = '\0';
      else
	pos = NULL;

      strcpy (buffer, start);

      if (! strcmp (buffer,  "sumiterpos"))
	type = PONOS_CONSTRAINTS_SUM_ITER_POS;
      else if (! strcmp (buffer,  "paramcoef0"))
	type = PONOS_CONSTRAINTS_PARAM_COEF_ZERO;
      else if (! strcmp (buffer,  "maxouterpar"))
	type = PONOS_OBJECTIVES_MAX_OUTER_PAR;
      else if (! strcmp (buffer,  "maxinnerpar"))
	type = PONOS_OBJECTIVES_MAX_INNER_PAR;
      else if (! strcmp (buffer,  "maxperm"))
	type = PONOS_OBJECTIVES_MAX_PERMUTABILITY;
      else if (! strcmp (buffer,  "mindepdist"))
	type = PONOS_OBJECTIVES_MIN_DEP_DISTANCE;
      else if (! strcmp (buffer,  "maxdepsolve"))
	type = PONOS_OBJECTIVES_MAX_DEP_SOLVE;
      else if (! strcmp (buffer,  "linearind"))
	type = PONOS_CONSTRAINTS_LINEAR_INDEP;
      else if (! strcmp (buffer,  "gammapos"))
	type = PONOS_OBJECTIVES_GAMMA_POS;
      else if (! strcmp (buffer,  "tasched"))
	type = PONOS_OBJECTIVES_TASCHED;
      else if (! strcmp (buffer,  "minitercoef"))
	type = PONOS_OBJECTIVES_MIN_THETA_ITER;
      else
	{
	  printf ("|> error: unrecognized objective-list member %s\n",
		  buffer);
	  return;
	}
      opt[i] = type;
    }
  opt[i] = -1;
}



static
void
ponos_print_options(s_interpreter_state_t* state,
		    s_pocc_command_t* cmd)
{
  printf ("|> Ponos options:\n");
  int i, j;
  char buffer[1024];
  for (i = 0; g_pocc_interpreter_ponos_option[i].longname; ++i)
    {
      char* longname = g_pocc_interpreter_ponos_option[i].longname;
      char* shortname = g_pocc_interpreter_ponos_option[i].shortname;
      char* description = g_pocc_interpreter_ponos_option[i].description;
      void* ptrval = g_pocc_interpreter_ponos_option[i].ptrval;
      void (*printfun)(char*, void*) =
	g_pocc_interpreter_ponos_option[i].fun_ptr_print;
      printf ("|>%s\t%s", shortname, longname);
      int len = strlen (longname);
      int opt_offset = len > 15 ? 1 : (len > 7 ? 2 : 3);
      for (j = 0; j < opt_offset; ++j)
	printf ("\t");
      sprintf (buffer, "<>");
      printfun (buffer, ptrval);
      printf ("%s", buffer);
      len = strlen (buffer);
      opt_offset = len > 7 ? 1 : 2;
      for (j = 0; j < opt_offset; ++j)
	printf ("\t");
      printf ("%s\n", description);
    }
}


static
void
ponos_print_space(s_interpreter_state_t* state,
		  s_pocc_command_t* cmd)
{
  if (state->ponos_space[state->active_workspace])
    {
      if (state->ponos_space_is_invalidated[state->active_workspace])
	{
	  printf ("|> WARNING: the current ponos space is not up-to-date\n");
	  printf ("|> WARNING: run 'compute legal-space' and 'compute schedule'\n");
	}
      ponos_space_print_vars (stdout,
			      state->ponos_space[state->active_workspace]);
      ponos_print_constraints_with_id (stdout, state);
    }
  else
    printf ("|> error: ponos space is (null). run 'compute legal-space'\n");
}

static
void
ponos_print_space_vars(s_interpreter_state_t* state,
		       s_pocc_command_t* cmd)
{
  if (state->ponos_space[state->active_workspace])
    {
      if (state->ponos_space_is_invalidated[state->active_workspace])
	{
	  printf ("|> WARNING: the current ponos space is not up-to-date\n");
	  printf ("|> WARNING: run 'compute legal-space' and 'compute schedule'\n");
	}
      ponos_space_print_vars
	(stdout, state->ponos_space[state->active_workspace]);
    }
  else
    printf ("|> error: ponos space is (null). run 'compute legal-space'\n");
}

static
void
ponos_print_cplex_lp(s_interpreter_state_t* state,
		     s_pocc_command_t* cmd)
{
  if (state->ponos_space[state->active_workspace])
    {
      if (state->ponos_space_is_invalidated[state->active_workspace])
	{
	  printf ("|> WARNING: the current ponos space is not up-to-date\n");
	  printf ("|> WARNING: run 'compute legal-space' and 'compute schedule'\n");
	}
      if (state->ponos_cplex_file[state->active_workspace])
	{
	  char* buffer = state->ponos_cplex_file[state->active_workspace];
	  printf ("|> last Cplex lp file solved (%s):\n", buffer);
	  pocc_interpreter_display_file (stdout, buffer);
	}
    }
  else
    printf ("|> error: ponos space is (null). run 'compute legal-space'\n");
}

static
void ponos_options_initialize(s_ponos_options_t* popts,
			      s_pocc_options_t* poptions)
{
  if ((! poptions) || (! popts))
    return;

  popts->debug = poptions->ponos_debug;
  popts->build_2d_plus_one = poptions->ponos_build_2d_plus_one;
  popts->maxscale_solver = poptions->ponos_maxscale_solver;
  popts->noredundancy_solver = poptions->ponos_noredundancy_solver;
  popts->legality_constant = poptions->ponos_legality_constant_K;
  popts->schedule_bound = poptions->ponos_schedule_bound;
  popts->schedule_size = poptions->ponos_schedule_dim;
  popts->solver = poptions->ponos_solver_type;
  popts->solver_precond = poptions->ponos_solver_precond;
  popts->quiet = 1; // poptions->ponos_quiet;
  /* popts->quiet = 0; // poptions->ponos_quiet; */
  popts->schedule_coefs_are_pos = poptions->ponos_coef_are_pos;
  popts->objective = poptions->ponos_objective;
  popts->pipsolve_lp = poptions->ponos_pipsolve_lp;
  popts->pipsolve_gmp = poptions->ponos_pipsolve_gmp;
  popts->output_file_template = strdup (poptions->output_file_name);
  popts->legal_space_normalization = poptions->ponos_solver_precond;

  if (popts->output_file_template)
    {
      int len = strlen (popts->output_file_template);
      if (len >= 2)
	popts->output_file_template[len - 2] = '\0';
    }
  int i;
  for (i = 0; poptions->ponos_objective_list[i] != -1; ++i)
    popts->objective_list[i] = poptions->ponos_objective_list[i];
  popts->objective_list[i] = -1;
}


static
void
ponos_initialize(s_interpreter_state_t* state,
		 s_pocc_command_t* cmd)
{
  /* printf ("|> ponos initialize:\n"); */

  // Set FM mode to gmp.
  fm_mode_set_to_gmp ();

  if (state->ponos_options[state->active_workspace] == NULL)
    {
      state->ponos_options[state->active_workspace] = ponos_options_malloc ();
      ponos_options_initialize (state->ponos_options[state->active_workspace],
				state->poptions[state->active_workspace]);
    }
}


static
void
ponos_set_variable_bounds(s_interpreter_state_t* state,
			  s_pocc_command_t* cmd)
{
  if (! cmd || ! cmd->cmd_args || ! cmd->cmd_args[0])
    {
      printf ("|> error: ill-formed set variable-bounds command\n");
      printf ("|> try 'set variable-bounds <varname> 0 1'\n");
      return;
    }
  if (! state->ponos_space[state->active_workspace])
    {
      printf ("|> error: Ponos space not computed yet. try 'compute legal-space'\n");
      return;
    }
  char* name = cmd->cmd_args[0];
  name = pocc_interpreter_utils_remove_double_quote (name);
  s_ponos_space_t* space = state->ponos_space[state->active_workspace];
  // Check the variable is not already in the space.
  int i;
  for (i = 0; i < space->num_vars; ++i)
    if (! strcmp (space->vars[i]->name, name))
      break;
  if (i == space->num_vars)
    {
      printf ("|> error: variable %s not in the space\n", name);
      return;
    }
  int lb = (int) ((intptr_t)cmd->cmd_args[1]);
  int ub = (int) ((intptr_t)cmd->cmd_args[2]);
  ponos_space_var_set_bounds (space->vars[i], lb, ub);

  printf ("|> variable %s bounds set: %d <= %s <= %d\n", name, lb, name, ub);
}


static
void
ponos_print_space_var_info(FILE* stream, s_ponos_var_t* v)
{
  char buffer[1024];
  fprintf (stream, "|> Info on variable %s:\n", v->name);
  fprintf (stream, "name:\t\t\t%s\n", v->name);
  fprintf (stream, "position in space:\t%d\n", v->abs_pos);
  strcpy (buffer, "<unknown>");
  switch (v->type)
    {
    case PONOS_VAR_THETA_ITER:
      strcpy (buffer, "theta_iterator"); break;
    case PONOS_VAR_THETA_PARAM:
      strcpy (buffer, "theta_parameter"); break;
    case PONOS_VAR_THETA_CST:
      strcpy (buffer, "theta_constant"); break;
    case PONOS_VAR_DELTA:
      strcpy (buffer, "delta"); break;
    case PONOS_VAR_RHO:
      strcpy (buffer, "rho"); break;
    case PONOS_VAR_GAMMA_ITER:
      strcpy (buffer, "gamma_iter"); break;
    case PONOS_VAR_GAMMA_PARAM:
      strcpy (buffer, "gamma_param"); break;
    case PONOS_VAR_GAMMA_CST:
      strcpy (buffer, "gamma_cst"); break;
    case PONOS_VAR_SIGMA:
      strcpy (buffer, "sigma"); break;
    case PONOS_VAR_U_DIST:
      strcpy (buffer, "U_dist"); break;
    case PONOS_VAR_OPT_SUM_VAR:
      strcpy (buffer, "optim_var"); break;
    case PONOS_VAR_CST_SUM_VAR:
      strcpy (buffer, "constraint_var"); break;
    case PONOS_VAR_GAMMA_REF:
      strcpy (buffer, "gamma_ref"); break;
    case PONOS_VAR_NU_REF:
      strcpy (buffer, "nu_ref"); break;
    case PONOS_VAR_ORDER:
      strcpy (buffer, "order?"); break;
    case PONOS_VAR_BETA_ITER:
      strcpy (buffer, "beta_iter"); break;
    case PONOS_VAR_BIN_USERDEF:
      strcpy (buffer, "binary-user-defined"); break;
    case PONOS_VAR_INT_USERDEF:
      strcpy (buffer, "int-user-defined"); break;
    }

  fprintf (stream, "variable type:\t\t%s\n", buffer);
  fprintf (stream, "lower bound:\t\t%d\n", v->lower_bound);
  fprintf (stream, "upper bound:\t\t%Ld\n", v->upper_bound);
  if (v->is_maximized)
    strcpy (buffer, "maximize");
  else
    strcpy (buffer, "minimize");
  fprintf (stream, "optimization:\t\t%s\n", buffer);
  fprintf (stream, "current optimal value:\t%d\n", v->optimal_value);
  fprintf (stream, "schedule dimension:\t%d\n", v->dim);
  fprintf (stream, "statement ptr:\t\t%p\n", v->scop_ptr);
  if (v->usr)
    fprintf (stream, "user ptr:\t%p\n", v->usr);
}


static
void
ponos_print_space_var_details(s_interpreter_state_t* state,
			      s_pocc_command_t* cmd)
{
  if (! cmd || ! cmd->cmd_args || ! cmd->cmd_args[0])
    {
      printf ("|> error: ill-formed command. try 'display var <varname>'\n");
      return;
    }
  if (! state->ponos_space[state->active_workspace])
    {
      printf ("|> error: Ponos space not computed yet. try 'compute legal-space'\n");
      return;
    }
  char* varname = cmd->cmd_args[0];
  s_ponos_space_t* space = state->ponos_space[state->active_workspace];
  int i;
  for (i = 0; i < space->num_vars; ++i)
    {
      if (space->vars[i] && space->vars[i]->name &&
	  ! strcmp (space->vars[i]->name, varname))
	{
	  ponos_print_space_var_info (stdout, space->vars[i]);
	  break;
	}
    }
  if (i == space->num_vars)
    printf ("|> error: variable %s not found in the space. try 'display all-vars'\n", varname);
}


static
void
ponos_print_space_vars_detailed(s_interpreter_state_t* state,
				s_pocc_command_t* cmd)
{
  if (! cmd)
    {
      printf ("|> error: command is empty\n");
      return;
    }
  if (! state->ponos_space[state->active_workspace])
    {
      printf ("|> error: Ponos space not computed yet. try 'compute legal-space'\n");
      return;
    }
  int i;
  s_ponos_space_t* space = state->ponos_space[state->active_workspace];
  for (i = 0; i < space->num_vars; ++i)
    ponos_print_space_var_info (stdout, space->vars[i]);
}

static
void
ponos_print_current_lexmin(s_interpreter_state_t* state,
			   s_pocc_command_t* cmd)
{
  if (! cmd)
    {
      printf ("|> error: command is empty\n");
      return;
    }
  s_ponos_space_t* space = state->ponos_space[state->active_workspace];
  if (! space)
    {
      printf ("|> error: Ponos space not computed yet. try 'compute legal-space'\n");
      return;
    }
  if (! space->has_valid_optimal_solution)
    {
      printf ("|> error: no lexmin computed yet. try 'compute schedule' first\n");
      return;
    }
  int i;
  printf ("|> Lexicographic minimal solution for the space\n");
  printf ("   computed during last call to 'compute schedule'\n");
  printf ("variable\tvalue\t\t\tvariable range\n");
  char buffer[32];
  for (i = 0; i < space->num_vars; ++i)
    {
      if (space->vars[i]->optimal_value)
	strcpy (buffer, "***\t");
      else
	strcpy (buffer, "\t");
      printf ("%s =", space->vars[i]->name);
      if (strlen (space->vars[i]->name) > 5)
	printf ("\t");
      else
	printf ("\t\t");
      printf ("%d\t%s\t%d <= %s <= %d\n",
	      space->vars[i]->optimal_value,
	      buffer,
	      space->vars[i]->lower_bound,
	      space->vars[i]->name,
	      space->vars[i]->upper_bound);
    }
}


static
void
ponos_display(s_interpreter_state_t* state,
	      s_pocc_command_t* cmd)
{
  if (! cmd)
    {
      printf ("|> error: command is empty\n");
      return;
    }
  ponos_initialize (state, cmd);
  switch (cmd->cmd_ids[1])
    {
    case POCC_CMD_OPTIONS:
      ponos_print_options (state, cmd); break;
    case POCC_CMD_SPACE:
      ponos_print_space (state, cmd); break;
    case POCC_CMD_ALL_VARS:
      ponos_print_space_vars (state, cmd); break;
    case POCC_CMD_ALL_VARS_DETAILED:
      ponos_print_space_vars_detailed (state, cmd); break;
    case POCC_CMD_VAR:
      ponos_print_space_var_details (state, cmd); break;
    case POCC_CMD_CONSTRAINTS:
      {
	if (cmd->cmd_ids[2] == POCC_CMD_MATCHING)
	  ponos_show_constraint_filter_regexp (state, cmd);
	else
	  {
	    printf ("|> error: try 'display constraints matching \"D_*\"'\n");
	    printf ("to get all constraints involving D_ variables\n");
	  }
	break;
      }
    case POCC_CMD_SOL_LEXMIN:
      ponos_print_current_lexmin (state, cmd); break;
    default:
      printf ("|> error: command not supported\n"); break;
    }
}

static
void
ponos_set(s_interpreter_state_t* state,
	  s_pocc_command_t* cmd)
{
  if (! cmd || ! cmd->cmd_args || ! cmd->cmd_args[0])
    {
      printf ("|> error: ill-formed set command\n");
      return;
    }
  switch (cmd->cmd_ids[1])
    {
    case POCC_CMD_OPTION:
      {
	int i;
	s_interpreter_option_t* popts = g_pocc_interpreter_ponos_option;
	for (i = 0; popts[i].longname != NULL; ++i)
	  if (! strcmp (cmd->cmd_args[0], popts[i].longname))
	    {
	      popts[i].fun_ptr_set (cmd->cmd_args[1], popts[i].ptrval);
	      char buffer[1024];
	      popts[i].fun_ptr_print (buffer, popts[i].ptrval);
	      printf ("|> %s set to %s\n", popts[i].longname, buffer);
	      break;
	    }
	if (popts[i].longname == NULL)
	  printf ("|> error: trying to set unknown option (try 'ponos display options'\n");
	break;
      }
    case POCC_CMD_VARIABLE_BOUNDS:
      {
	ponos_set_variable_bounds (state, cmd);
	break;
      }
    default: printf ("|> unsupported set command\n"); break;
    }
}

static
int
find_statement_id(void* ptr, scoplib_scop_p scop)
{
  if (! scop)
    return -1;
  scoplib_statement_p s;
  int id;
  for (s = scop->statement, id = 0; s; s = s->next, ++id)
    if (s == ptr)
      break;
  if (s)
    return id;
  return -1;
}

static
scoplib_statement_p
find_statement_ptr_by_id(int stmt_id, scoplib_scop_p scop)
{
  if (! scop || stmt_id < 0)
    return NULL;
  scoplib_statement_p s = scop->statement;
  for (; stmt_id && s; s = s->next, --stmt_id)
    ;
  if (! s)
    return NULL;
  return s;
}

static
void
update_scoplib_statement_pointers (s_interpreter_state_t* state)
{
  s_ponos_space_t* space = state->ponos_space[state->active_workspace];
  scoplib_scop_p orig_scop = state->scop_original[state->active_workspace];
  scoplib_scop_p scop = state->scop[state->active_workspace];
  int i;
  for (i = 0; i < space->num_vars; ++i)
    {
      if (space->vars[i]->scop_ptr)
	{
	  int stmt_id = find_statement_id (space->vars[i]->scop_ptr, orig_scop);
	  if (stmt_id != -1)
	    space->vars[i]->scop_ptr =
	      find_statement_ptr_by_id (stmt_id, scop);
	}
    }
}


static
void
ponos_compute_legalspace(s_interpreter_state_t* state,
			 s_pocc_command_t* cmd)
{
  printf ("|> compute legal space. This may take a while...\n");

  s_ponos_options_t* options = state->ponos_options[state->active_workspace];
  if (! options)
    {
      options = ponos_options_malloc ();
      ponos_options_initialize (state->ponos_options[state->active_workspace],
				state->poptions[state->active_workspace]);
      state->ponos_options[state->active_workspace] = options;
    }

  if (! options->quiet)
    printf ("[Ponos] Start scheduling\n");

  scoplib_scop_p scop = state->scop_original[state->active_workspace];
  if (! scop)
    {
      printf ("|> error: SCoP is (null), load a file first\n");
      return;
    }

  // Perform candl.
  /// TODO: read dependence from the scop.
  CandlOptions* coptions = candl_options_malloc ();
  CandlProgram* cprogram = candl_program_convert_scop (scop, NULL);
  CandlDependence* deps = candl_dependence (cprogram, coptions);

  // Experimental.
  if (options->candl_deps_prune_transcover)
    deps = candl_dependence_prune_transitively_covered (deps);
  // Simplify dependences with ISL, if needed.
  /// FIXME: Broken?
  if (options->candl_deps_isl_simplify)
    candl_dependence_isl_simplify(deps, cprogram);

  // Compute semantics-preserving space, using deltas and rhos.
  if (! options->quiet)
    printf ("[Ponos] Build legality constraints\n");
  s_ponos_space_t* space =
    ponos_legal_build_semantics_constraints (scop, deps, options);

  if (options->debug)
    {
      ponos_space_print (stdout, space);
      ponos_space_print_vars (stdout, space);
    }
  // Re-initialize the space and constraints_system with the new value.
  if (state->ponos_space[state->active_workspace])
    {
      ponos_space_free (state->ponos_space[state->active_workspace]);
      if (state->ponos_constraints_system[state->active_workspace])
	fm_system_free
	  (state->ponos_constraints_system[state->active_workspace]);
    }
  state->ponos_space[state->active_workspace] = space;
  state->ponos_constraints_system[state->active_workspace] =
    fm_solution_to_system (space->space);

  if (options->debug == 1)
    printf ("space has %d vars\n", space->num_vars);

  // Update scoplib_statement_p pointers in legal space to using the
  // 'state->scop' instead of 'state->orignal_scop'.
  update_scoplib_statement_pointers (state);
  space->scop = state->scop[state->active_workspace];

  // Be clean.
  candl_dependence_free (deps);
  candl_program_free (cprogram);
  candl_options_free (coptions);

  if (! options->quiet)
    printf ("[Ponos] done computing legal space\n");
}


static
void
ponos_compute_scheduling_constraints(s_interpreter_state_t* state,
				     s_pocc_command_t* cmd)
{
  printf ("|> Compute scheduling constraints/objective and insert them\n");

  s_ponos_options_t* options = state->ponos_options[state->active_workspace];
  if (! options)
    {
      options = ponos_options_malloc ();
      ponos_options_initialize (state->ponos_options[state->active_workspace],
				state->poptions[state->active_workspace]);
      state->ponos_options[state->active_workspace] = options;
    }

  scoplib_scop_p scop = state->scop[state->active_workspace];
  if (! scop)
    {
      printf ("|> error: SCoP is (null), load a file first\n");
      return;
    }

  s_ponos_space_t* space = state->ponos_space[state->active_workspace];
  if (! space)
    {
      printf ("|> error: Ponos space is (null), run 'compute legal-space' first\n");
      return;
    }
  /* ponos_constraints_delta_up (space, options); */

  // Make sure any pending constraint is in the space.
  synchronize_space_from_constraints (state);
  // Make sure the constraint space is built.
  synchronize_constraints_from_space (state);

  // Embed optimization objective constraints.
  if (options->objective == PONOS_OBJECTIVES_CODELET)
    {
      if (! options->quiet)
	printf ("[Ponos] Embed objective constraints: codelet\n");

      // Create iterator map for non-fvd and fvd (See PLDI'13 paper)
      s_af_iterator_map_t * G =
        ponos_codelet_build_complete_af_iterator_map (space, options);

      ponos_constraints_param_coef_zero (space, options);
      ponos_constraints_sum_iter_pos (space, options);
      ponos_objectives_gamma_pos (space, options);

      //int ** gamma_iters;
      ponos_codelet_stride_constraints (space, G, options);

      ponos_codelet_boolean_constraints (space, options, PONOS_VAR_SIGMA);
      ponos_codelet_boolean_constraints (space, options, PONOS_VAR_NU_REF);
      ponos_codelet_boolean_constraints (space, options, PONOS_VAR_GAMMA_REF);

      ponos_codelet_objective_max_stride01_refs (space, options);

      //ponos_codelet_objective_force_outer_fusion (space, options);

      ponos_constraints_linear_indep (space, options);

      ponos_objectives_max_inner_par (space, options);
      ponos_objectives_min_iter_coef (space, options);

      // Free the iterator map
      ponos_codelet_free_af_iterator_map (G);

    }
  else if (options->objective == PONOS_OBJECTIVES_PLUTO)
    {
      if (! options->quiet)
	printf ("[Ponos] Embed objective constraints: pluto\n");

      // Constraints first.
      ponos_constraints_param_coef_zero (space, options);
      ponos_constraints_sum_iter_pos (space, options);

      // Objectives then, in reverse order of importance.
/*       ponos_objectives_max_inner_par (space, options); */
      ponos_objectives_max_dep_solve (space, options);
      ponos_objectives_max_outer_par (space, options);
      ponos_objectives_max_permutability (space, options);
      ponos_objectives_min_dep_distance (space, options);

      if (options->debug)
	ponos_space_print_vars (stdout, space);
    }
  else if (options->objective == PONOS_OBJECTIVES_CUSTOM)
    {
      if (! options->quiet)
	printf ("[Ponos] Embed objective constraints: custom\n");
      int i;
      for (i = 0; options->objective_list[i] != -1; ++i)
	{
	  switch (options->objective_list[i])
	    {
	    case PONOS_CONSTRAINTS_SUM_ITER_POS:
	      {
		if (! options->quiet)
		  printf ("[Ponos] Embed constraint: sum of iter are pos\n");
		ponos_constraints_sum_iter_pos (space, options);
		break;
	      }
	    case PONOS_CONSTRAINTS_PARAM_COEF_ZERO:
	      {
		if (! options->quiet)
		  printf ("[Ponos] Embed constraint: param coef are 0\n");
		ponos_constraints_param_coef_zero (space, options);
		break;
	      }
	    case PONOS_OBJECTIVES_MAX_OUTER_PAR:
	      {
		if (! options->quiet)
		  printf ("[Ponos] Embed objective: max outer par\n");
		ponos_objectives_max_outer_par (space, options);
		break;
	      }
	    case PONOS_OBJECTIVES_MAX_INNER_PAR:
	      {
		if (! options->quiet)
		  printf ("[Ponos] Embed objective: max inner par\n");
		ponos_objectives_max_inner_par (space, options);
		break;
	      }
	    case PONOS_OBJECTIVES_MAX_PERMUTABILITY:
	      {
		if (! options->quiet)
		  printf ("[Ponos] Embed objective: max permutability\n");
		ponos_objectives_max_permutability (space, options);
		break;
	      }
	    case PONOS_OBJECTIVES_MIN_DEP_DISTANCE:
	      {
		if (! options->quiet)
		  printf ("[Ponos] Embed objective: min dep distance\n");
		ponos_objectives_min_dep_distance (space, options);
		break;
	      }
	    case PONOS_OBJECTIVES_MAX_DEP_SOLVE:
	      {
		if (! options->quiet)
		  printf ("[Ponos] Embed objective: max #dep solved\n");
		ponos_objectives_max_dep_solve (space, options);
		break;
	      }
	    case PONOS_CONSTRAINTS_LINEAR_INDEP:
	      {
		if (! options->quiet)
		  printf ("[Ponos] Embed constraint: linear independence\n");
		ponos_constraints_linear_indep (space, options);
		break;
	      }
	    case PONOS_OBJECTIVES_GAMMA_POS:
	      {
		if (! options->quiet)
		  printf ("[Ponos] Embed constraint: gamma pos\n");
		ponos_objectives_gamma_pos (space, options);
		break;
	      }
	    case PONOS_OBJECTIVES_TASCHED:
	      {
		if (! options->quiet)
		  printf ("[Ponos] Embed constraint: TA scheduler\n");
		ponos_objectives_tasched (space, options);
		break;
	      }
	    case PONOS_OBJECTIVES_MIN_THETA_ITER:
	      {
		if (! options->quiet)
		  printf ("[Ponos] Embed constraint: TA scheduler\n");
		ponos_objectives_min_iter_coef (space, options);
		break;
	      }
	    default:
		if (! options->quiet)
		  printf ("[Ponos][WARNING] Constraint/objective not implemented\n");
		break;
	    }
	}
    }

  // Update the constraints system representation.
  synchronize_constraints_from_space (state);
}


static
void
embed_variable_bounds_in_space (s_ponos_space_t* space)
{
  int i;

  s_fm_solution_t* sol = fm_solution_dup (space->space);
  for (i = 0; i < space->num_vars; ++i)
    {
      // Create lower bound inequality.
      s_fm_vector_t* v = fm_vector_alloc (i + 3);
      fm_vector_set_ineq (v);
      fm_vector_assign_llint_idx (v, 1, i + 1);
      fm_vector_assign_llint_idx (v, -space->vars[i]->lower_bound, i + 2);
      fm_solution_add_unique_line_at (sol, v, i + 1);

      // Create upper bound inequality.
      v = fm_vector_alloc (i + 3);
      fm_vector_set_ineq (v);
      fm_vector_assign_llint_idx (v, -1, i + 1);
      fm_vector_assign_llint_idx (v, space->vars[i]->lower_bound, i + 2);
      fm_solution_add_unique_line_at (space->space, v, i + 1);
    }
  space->space = sol;
}


static
void
ponos_compute_schedule(s_interpreter_state_t* state,
		       s_pocc_command_t* cmd)
{
  scoplib_scop_p scop = state->scop[state->active_workspace];
  if (! scop)
    {
      printf ("|> error: SCoP is (null), load a file first\n");
      return;
    }

  s_ponos_space_t* space = state->ponos_space[state->active_workspace];
  if (! space)
    {
      printf ("|> error: Ponos space is (null), run 'compute legal-space' first\n");
      return;
    }
  s_ponos_options_t* options = state->ponos_options[state->active_workspace];

  printf ("|> Compute schedule. This may take a while...\n");
  // Make sure any pending constraint is in the space.
  if (! state->ponos_constraints_system[state->active_workspace])
    {
      printf ("|> internal error: something went wrong, the system is (null)\n");
      return;
    }
  synchronize_space_from_constraints (state);
  // Backup the user solution.
  s_fm_solution_t* old_sol = space->space;
  // Insert all variable bounds, needed for PIP solving.
  embed_variable_bounds_in_space (space);

  // Invalidate previous solution.
  space->has_valid_optimal_solution = 0;
  
  // Solve the system.
  if (! options->quiet)
    printf ("[Ponos] Solve ILP\n");
  if (options->pipsolve_gmp)
    ponos_solver_gmp (space, scop, options);
  else
    ponos_solver (space, scop, options);

  if (options->solver == PONOS_SOLVER_CPLEX)
    {
      char buffer[1024];
      sprintf (buffer, "%s.lp", options->output_file_template);
      state->ponos_cplex_file[state->active_workspace] = strdup (buffer);
    }

  if (! space->has_valid_optimal_solution)
      printf ("|> error: the current constraint system has no solution!\n");
  else
    {
      if (! options->quiet)
	printf ("[Ponos] done computing schedule\n");

      printf ("|> Schedule computed (solver run: %fs)\n",
	      space->last_solver_time);
      pocc_interpreter_print_scop_summary (stdout, scop);
    }
  // Restore space.
  fm_solution_free (space->space);
  space->space = old_sol;
}

static
void
ponos_compute_is_included(s_interpreter_state_t* state,
			  s_pocc_command_t* cmd)
{
  scoplib_scop_p scop = state->scop[state->active_workspace];
  if (! scop)
    {
      printf ("|> error: SCoP is (null), load a file first\n");
      return;
    }

  s_ponos_space_t* space = state->ponos_space[state->active_workspace];
  if (! space)
    {
      printf ("|> error: Ponos space is (null), run 'compute legal-space' first\n");
      return;
    }
  s_ponos_options_t* options = state->ponos_options[state->active_workspace];
  synchronize_space_from_constraints (state);
  synchronize_constraints_from_space (state);
  // Edit the space to add schedule coef. as constraints.
  int space_thetavar_pos;
  for (space_thetavar_pos = 0;
       space->vars[space_thetavar_pos]->type & PONOS_VAR_THETA == 0;
       ++space_thetavar_pos)
    ;
  char* buff = cmd->cmd_args[0];
  printf ("|> Checking schedule %s\n", cmd->cmd_args[0]);
  buff = pocc_interpreter_utils_remove_double_quote (buff);
  s_fm_system_t* tempsys =
    fm_system_dup (state->ponos_constraints_system[state->active_workspace]);
  while (buff && *buff)
    {
      char buffer[64];
      int pos = 0;
      int multiplier = 1;
      if (*buff == '-')
	{
	  multiplier = -1;
	  ++buff;
	}
      while (*buff >= '0' && *buff <= '9')
	buffer[pos++] = *(buff++);
      buffer[pos] = '\0';
      int val = atoi (buffer) * multiplier;
      s_fm_vector_t* v = fm_vector_alloc (space->num_vars + 2);
      fm_vector_set_eq (v);
      fm_vector_assign_llint_idx (v, -1, 1 + space_thetavar_pos++);
      fm_vector_assign_llint_idx (v, val, space->num_vars + 1);
      fm_system_add_line (tempsys, v);
      if (*buff == ',')
	++buff;
      else
	break;
    }
  if (*buff || space->vars[space_thetavar_pos]->type & PONOS_VAR_THETA)
    {
      printf ("|> warning: schedule is incomplete (not enough/too much entries)\n");
      /* fm_system_free (tempsys); */
      /* return; */
    }
  // Check if the system has a solution. If so, we're good, the
  // solution is acceptable.
  int quietval = options->quiet;
  int solvertype = options->solver;
  options->quiet = 1;
  options->solver = PONOS_SOLVER_CPLEX;
  if (ponos_solver_cplex_check_is_empty (space, options, tempsys))
    printf ("|> Solution for thetas not included in the space of constraints\n");
  else
    printf ("|> Solution for thetas is valid\n");
  options->quiet = quietval;
  options->solver = solvertype;
  fm_system_free (tempsys);
}



static
void
ponos_compute(s_interpreter_state_t* state,
	      s_pocc_command_t* cmd)
{
  if (! cmd)
    {
      printf ("|> error: ill-formed compute command\n");
      return;
    }
  switch (cmd->cmd_ids[1])
    {
    case POCC_CMD_LEGAL_SPACE:
      {
	ponos_compute_legalspace (state, cmd);
	break;
      }
    case POCC_CMD_SCHEDULING_CONSTRAINTS:
      {
	ponos_compute_scheduling_constraints (state, cmd);
	break;
      }
    case POCC_CMD_SCHEDULE:
      {
	ponos_compute_schedule (state, cmd);
	break;
      }
    case POCC_CMD_IS_INCLUDED:
      {
	ponos_compute_is_included (state, cmd);
	break;
      }
    default: printf ("|> unsupported compute command\n"); break;
    }
}

static
void
ponos_constraint_display_one(s_interpreter_state_t* state,
			     s_pocc_command_t* cmd)
{
  if (! cmd || ! cmd->cmd_args || ! cmd->cmd_args[0])
    {
      printf ("|> error: ill-formed constraint display command\n");
      printf ("|> try 'constraint display c1' to see the first constraint\n");
      return;
    }
  if (! state->ponos_space[state->active_workspace])
    {
      printf ("|> error: Ponos space not computed yet. try 'compute legal-space'\n");
      return;
    }
  s_ponos_space_t* space = state->ponos_space[state->active_workspace];
  // Make sure the constraints representation is available.
  if (! state->ponos_constraints_system[state->active_workspace])
    {
      state->ponos_constraints_system[state->active_workspace] =
	fm_solution_to_system (space->space);
    }
  char* cid_str = cmd->cmd_args[0];
  // skip 'c' in the constraint identifier.
  ++cid_str;
  int cid = atoi (cid_str);

  s_fm_system_t* s = state->ponos_constraints_system[state->active_workspace];
  if (cid < 0 || cid >= s->nb_lines)
    printf ("|> error: constraint c%d not in the space (only c0-c%d exist)\n",
	    cid, s->nb_lines);
  else
    {
      printf ("c%d: ", cid);
      ponos_space_pretty_print_vector (stdout, space, s->lines[cid]);
    }
}

static int is_space(char c) { return (c == ' ' || c == '\t'); }
static int is_digit(char c) { return (c >= '0' && c <= '9'); }
static int is_comp_op(char c) { return (c == '=' || c == '>'); }
static int is_sign(char c) { return (c == '-' || c == '+'); }

static
s_fm_vector_t*
ponos_constraint_parse_from_string (char* str, s_ponos_space_t* space)
{
  s_fm_vector_t* v = fm_vector_alloc (space->num_vars + 2);
  //c1: S0_i1_0 -S1_i1_0 -S1_i2_0 +S0_p0_0 +U1_0 >= -10
  int sign = 1;
  long long int weight = 1;
  char buffer_num[64];
  char buffer_varname[1024];
  char* start;
  char old;
  char* base = str;
  int mult = 1;

  // Check if the constraint starts by an identifier, if so, skip it.
  char* tmp = str;
  while (tmp && *tmp && *tmp != ':')
    ++tmp;
  if (*tmp)
    {
      while (str && *str && *str != ':')
	++str;
      ++str;
    }

  // Main scanning loop.
  while (str && *str)
    {
      // Skip whitespace
      while (is_space (*str))
	++str;
      // Parse sign:
      if (*str == '-')
	{
	  sign = -1;
	  ++str;
	}
      else if (*str == '+')
	{
	  ++str;
	  sign = 1;
	}
      // Skip whitespace
      while (is_space (*str))
	++str;
      // Parse weight
      if (is_digit (*str))
	{
	  start = str;
	  while (*str && is_digit (*str))
	    ++str;
	  old = *str;
	  *str = '\0';
	  strcpy (buffer_num, start);
	  weight = atoll (buffer_num);
	  weight *= sign;
	  *str = old;
	}
      // Parse variable name.
      start = str;
      while (*str && ! is_comp_op (*str) && ! is_space (*str)
	     && ! is_sign (*str))
	++str;
      // We got a variable name.
      if (start != str)
	{
	  old = *str;
	  *str = '\0';
	  strcpy (buffer_varname, start);
	  *str = old;
	  // Add the value to the constraint.
	  int i;
	  for (i = 0; i < space->num_vars; ++i)
	    if (! strcmp (space->vars[i]->name, buffer_varname))
	      break;
	  if (i == space->num_vars)
	    {
	      printf ("|> error adding constraint %s:\n", base);
	      printf ("variable name %s does not exist in the space\n",
		      buffer_varname);
	      fm_vector_free (v);
	      return NULL;
	    }
	  fm_vector_assign_llint_idx (v, weight * mult * sign, i + 1);
	}
      else
	{
	  // Skip whitespace
	  while (*str == ' ')
	    ++str;
	  if (is_comp_op (*str))
	    {
	      // detect constraint type. Only supported: = and >=
	      if (*str == '>')
		{
		  fm_vector_set_ineq (v);
		  ++str;
		}
	      else if (*str == '=')
		fm_vector_set_eq (v);
	      else
		{
		  printf ("|> error adding constraint %s:\n", base);
		  printf ("Only operators supported are >= and =\n");
		  fm_vector_free (v);
		  return NULL;
		}
	      ++str;
	      mult = -1;
	      sign = 1;
	    }
	  else if (*str == '\0')
	    {
	      // We got the final scalar weight (rhs of operator).
	      fm_vector_assign_llint_idx (v, weight * mult * sign, v->size - 1);
	    }
	  else
	    {
	      printf ("|> error adding constraint %s:\n", base);
	      printf ("Scalar numbers can only appear on the right of >= or =\n");
	      fm_vector_free (v);
	      return NULL;
	    }
	}
    }

  return v;
}


static
void
ponos_constraint_insert(s_interpreter_state_t* state,
			s_pocc_command_t* cmd)
{
  if (! cmd || ! cmd->cmd_args || ! cmd->cmd_args[0])
    {
      printf ("|> error: ill-formed constraint display command\n");
      printf ("|> try 'constraint display c1' to see the first constraint\n");
      return;
    }
  if (! state->ponos_space[state->active_workspace])
    {
      printf ("|> error: Ponos space not computed yet. try 'compute legal-space'\n");
      return;
    }
  if (! state->ponos_constraints_system[state->active_workspace])
    {
      state->ponos_constraints_system[state->active_workspace] =
	fm_solution_to_system
	(state->ponos_space[state->active_workspace]->space);
    }
  char* cst = cmd->cmd_args[0];
  cst = pocc_interpreter_utils_remove_double_quote (cst);
  s_ponos_space_t* space = state->ponos_space[state->active_workspace];
  s_fm_vector_t* v = ponos_constraint_parse_from_string (cst, space);
  if (v)
    {
      s_fm_system_t* constraints =
	state->ponos_constraints_system[state->active_workspace];
      fm_system_add_line (constraints, v);
      if (space->space)
	fm_solution_free (space->space);
      space->space = fm_system_to_solution (constraints);
      printf ("|> Inserted constraint %s in last row\n", cst);
    }
}

static
void
ponos_constraint_delete(s_interpreter_state_t* state,
			     s_pocc_command_t* cmd)
{
  if (! cmd || ! cmd->cmd_args || ! cmd->cmd_args[0])
    {
      printf ("|> error: ill-formed constraint display command\n");
      printf ("|> try 'constraint display c1' to see the first constraint\n");
      return;
    }
  if (! state->ponos_space[state->active_workspace])
    {
      printf ("|> error: Ponos space not computed yet. try 'compute legal-space'\n");
      return;
    }
  if (! state->ponos_constraints_system[state->active_workspace])
    {
      printf ("|> internal error: constraints system is (null)\n");
      return;
    }
  char* cid_str = cmd->cmd_args[0];
  // skip 'c' in the constraint identifier.
  ++cid_str;
  int cid = atoi (cid_str);
  s_fm_system_t* s = state->ponos_constraints_system[state->active_workspace];
  if (cid < 0 || cid >= s->nb_lines)
    printf ("|> error: constraint c%d not in the space (only c0-c%d exist)\n",
	    cid, s->nb_lines);
  else
    {
      // Delete the constraint:
      fm_system_remove_line (s, cid);
      s_ponos_space_t* space = state->ponos_space[state->active_workspace];
      if (space->space)
	fm_solution_free (space->space);
      space->space = fm_system_to_solution (s);
      printf ("|> Deleted constraint c%d\n", cid);
    }
}

static
void
ponos_variable_insert(s_interpreter_state_t* state,
		      s_pocc_command_t* cmd)
{
  if (! cmd || ! cmd->cmd_args || ! cmd->cmd_args[0])
    {
      printf ("|> error: ill-formed insert variable command\n");
      printf ("|> try 'insert variable c1 first' or\n");
      printf ("|> try 'insert variable c1 last' or\n");
      printf ("|> try 'insert variable c1 23' to insert it at position 23\n");
      return;
    }
  if (! state->ponos_space[state->active_workspace])
    {
      printf ("|> error: Ponos space not computed yet. try 'compute legal-space'\n");
      return;
    }
  // Make sure any pending constraint is in the space.
  synchronize_space_from_constraints (state);
  // Make sure the constraint space is built.
  synchronize_constraints_from_space (state);

  char* name = cmd->cmd_args[0];
  name = pocc_interpreter_utils_remove_double_quote (name);
  s_ponos_space_t* space = state->ponos_space[state->active_workspace];
  // Check the variable is not already in the space.
  int i;
  for (i = 0; i < space->num_vars; ++i)
    if (! strcmp (space->vars[i]->name, name))
      break;
  if (i != space->num_vars)
    {
      printf ("|> error: variable %s already in the space\n", name);
      return;
    }
  // Insert the variable.
  int pos = (int) ((intptr_t)cmd->cmd_args[1]);
  if (pos == POCC_INTERPRETER_SPACE_LAST_POS)
    pos = space->num_vars;
  if (pos >= 0 && pos <= space->num_vars)
    {
      s_ponos_var_t* var =
	ponos_space_var_create (name, PONOS_VAR_INT_USERDEF, 0, 0, 0, NULL);
      if (pos != space->num_vars)
	ponos_space_insert_variable_at_pos (space, var, pos);
      else
	ponos_space_insert_variable_last (space, var);
    }
  else
    {
      printf ("|> error: position %d for variable %s out of range\n", name);
      return;
    }

  // Update the constraints.
  synchronize_constraints_from_space (state);

  printf ("|> Inserted variable %s at position %d\n", name, pos);
}

static
void
ponos_load_constraints_file(s_interpreter_state_t* state,
			    s_pocc_command_t* cmd)
{
  if (! cmd || ! cmd->cmd_args || ! cmd->cmd_args[0])
    {
      printf ("|> error: ill-formed load constraints <filename> command\n");
      return;
    }
  if (! state->ponos_space[state->active_workspace])
    {
      printf ("|> error: Ponos space not computed yet. try 'compute legal-space'\n");
      return;
    }
  // Make sure any pending constraint is in the space.
  synchronize_space_from_constraints (state);
  // Make sure the constraint space is built.
  synchronize_constraints_from_space (state);

  s_ponos_space_t* space = state->ponos_space[state->active_workspace];
  char* filename = cmd->cmd_args[0];
  FILE* f = fopen (filename, "r");
  if (! f)
    {
      printf ("|> error: cannot open file %s\n", filename);
      return;
    }
  char* buffer = XMALLOC(char, 32000);
  while (fgets (buffer, 32000, f))
    {
      // Strip the last \n, if any.
      int len = strlen (buffer);
      if (buffer[len - 1] == '\n')
	buffer[len - 1] = '\0';
      s_fm_vector_t* v = ponos_constraint_parse_from_string (buffer, space);
      fm_system_add_line
	(state->ponos_constraints_system[state->active_workspace], v);
    }
  fclose (f);
  synchronize_space_from_constraints (state);
  printf ("|> Constraints loaded from %s\n", filename);
}

static
void
ponos_store_constraints_file(s_interpreter_state_t* state,
			     s_pocc_command_t* cmd)
{
  if (! cmd || ! cmd->cmd_args || ! cmd->cmd_args[0])
    {
      printf ("|> error: ill-formed load constraints <filename> command\n");
      return;
    }
  if (! state->ponos_space[state->active_workspace])
    {
      printf ("|> error: Ponos space not computed yet. try 'compute legal-space'\n");
      return;
    }
  // Make sure the constraint space is built.
  synchronize_constraints_from_space (state);

  s_ponos_space_t* space = state->ponos_space[state->active_workspace];
  char* filename = cmd->cmd_args[0];
  FILE* f = fopen (filename, "w");
  if (! f)
    {
      printf ("|> error: cannot open file %s\n", filename);
      return;
    }
  ponos_print_constraints_with_id (f, state);
  fclose (f);
  printf ("|> Constraints saved in %s\n", filename);
}

static
void
ponos_delete_all_constraints(s_interpreter_state_t* state,
			     s_pocc_command_t* cmd)
{
  if (! cmd)
    {
      printf ("|> error: ill-formed clear constraints command\n");
      return;
    }
  if (! state->ponos_space[state->active_workspace])
    {
      printf ("|> error: Ponos space not computed yet. try 'compute legal-space'\n");
      return;
    }
  s_fm_system_t* s = state->ponos_constraints_system[state->active_workspace];
  if (s)
    fm_system_free (s);
  s = fm_system_alloc
    (0, state->ponos_space[state->active_workspace]->num_vars + 2);
  state->ponos_constraints_system[state->active_workspace] = s;
  // Make sure the constraint space is built.
  synchronize_space_from_constraints (state);

  printf ("|> All constraints deleted\n");
}

static
void
ponos_constraint(s_interpreter_state_t* state,
		 s_pocc_command_t* cmd)
{
  if (! cmd)
    {
      printf ("|> error: ill-formed constraint command\n");
      return;
    }
  switch (cmd->cmd_ids[1])
    {
    case POCC_CMD_DISPLAY:
      {
	ponos_constraint_display_one (state, cmd);
	break;
      }
    case POCC_CMD_DELETE:
      {
	ponos_constraint_delete (state, cmd);
	break;
      }
    case POCC_CMD_INSERT:
      {
	ponos_constraint_insert (state, cmd);
	break;
      }
    default: printf ("|> unsupported compute command\n"); break;
    }
}


static
void
ponos_insert(s_interpreter_state_t* state,
	     s_pocc_command_t* cmd)
{
  if (! cmd)
    {
      printf ("|> error: ill-formed insert command\n");
      return;
    }
  switch (cmd->cmd_ids[1])
    {
    case POCC_CMD_CONSTRAINT:
      {
	ponos_constraint_insert (state, cmd);
	break;
      }
    case POCC_CMD_VARIABLE:
      {
	ponos_variable_insert (state, cmd);
	break;
      }
    default: printf ("|> unsupported insert command\n"); break;
    }
}

static
void
ponos_load(s_interpreter_state_t* state,
	   s_pocc_command_t* cmd)
{
  if (! cmd)
    {
      printf ("|> error: ill-formed insert command\n");
      return;
    }
  switch (cmd->cmd_ids[1])
    {
    case POCC_CMD_CONSTRAINTS:
      {
	ponos_load_constraints_file (state, cmd);
	break;
      }
    default: printf ("|> unsupported load command\n"); break;
    }
}

static
void
ponos_store(s_interpreter_state_t* state,
	    s_pocc_command_t* cmd)
{
  if (! cmd)
    {
      printf ("|> error: ill-formed insert command\n");
      return;
    }
  switch (cmd->cmd_ids[1])
    {
    case POCC_CMD_CONSTRAINTS:
      {
	ponos_store_constraints_file (state, cmd);
	break;
      }
    default: printf ("|> unsupported load command\n"); break;
    }
}

static
void
ponos_delete(s_interpreter_state_t* state,
	     s_pocc_command_t* cmd)
{
  if (! cmd)
    {
      printf ("|> error: ill-formed delete command\n");
      return;
    }
  switch (cmd->cmd_ids[1])
    {
    case POCC_CMD_ALL_CONSTRAINTS:
      {
	ponos_delete_all_constraints (state, cmd);
	break;
      }
    default: printf ("|> unsupported delete command\n"); break;
    }
}


void
pocc_interpreter_ponos(s_interpreter_state_t* state,
		       s_pocc_command_t* cmd)
{
  s_interpreter_command_t all_commands[] =
    {
      { POCC_CMD_HELP, ponos_help, "help" },
      { POCC_CMD_SHOW, ponos_show, "show" },
      { POCC_CMD_DISPLAY, ponos_display, "display" },
      { POCC_CMD_INITIALIZE, ponos_initialize, "initialize" },
      { POCC_CMD_SET, ponos_set, "set" },
      { POCC_CMD_COMPUTE, ponos_compute, "compute" },
      { POCC_CMD_INFO, pocc_interpreter_info, "info" },
      { POCC_CMD_CONSTRAINT, ponos_constraint, "constraint" },
      { POCC_CMD_INSERT, ponos_insert, "insert" },
      { POCC_CMD_LOAD, ponos_load, "load" },
      { POCC_CMD_STORE, ponos_store, "store" },
      { POCC_CMD_DELETE, ponos_delete, "delete" },
      { POCC_CMD_END, ponos_end, "" },
      { POCC_CMD_ERROR, NULL, NULL },
      { -1, NULL, NULL}
    };

  // Ensure the options are intialized iff pocc options are already
  // initialized.
  ponos_initialize (state, cmd);

  s_ponos_options_t* popts = state->ponos_options[state->active_workspace];

  s_interpreter_option_t all_options[] =
    {
      { "debug", "d", "1: enable highly verbose output",
	&(popts->debug),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { "quiet", "", "1: turn off all output\n\nDependence analysis options:",
	&(popts->quiet),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { "simplify-candl-dep", "", "1: simplify dependence polyhedra",
	&(popts->candl_deps_isl_simplify),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { "prune-candl-dep", "", "1: prune transitively covered\n\t\t\t\t\t\tdependences\n\nSolver options:",
	&(popts->candl_deps_prune_transcover),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { "solver-type", "", "{pip,cplex,cplex-incremental}:\n\t\t\t\t\t\tselect ILP solver",
	&(popts->solver),
	ponos_option_print_solver_type, ponos_option_set_solver_type },

      { "solver-precondition", "", "1: FM-precondition the ILP",
	&(popts->solver_precond),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { "solver-simplify", "", "1: Remove redundancy in ILP",
	&(popts->legal_space_normalization),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { "fm-maxscale", "", "1: enable maxscale FM solver",
	&(popts->maxscale_solver),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { "fm-noredundant", "", "1: enable redundancy elimination\n\t\t\t\t\t\tin FM",
	&(popts->noredundancy_solver),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { "relax-lp", "", "1: relax problem to LP",
	&(popts->pipsolve_lp),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { "pip-gmp", "", "1: use PIP in GMP arbitrary\n\t\t\t\t\t\tprecision mode\n\nScheduler options:",
	&(popts->pipsolve_gmp),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },


      { "scheduler-mode", "", "{none,custom,codelet,pluto}:\n\t\t\t\t\t\tscheduler type",
	&(popts->objective),
	ponos_option_print_objective, ponos_option_set_objective },

      { "objective-list", "", "{sumiterpos,paramcoef0,\n\t\t\t\t\t\tmaxouterpar,maxinnerpar,maxperm,\n\t\t\t\t\t\tmindepdist,maxdepsolve,linearind\n\t\t\t\t\t\tgammapos,tasched,minitercoef}:\n\t\t\t\t\t\tcomma-separated list of\n\t\t\t\t\t\tobjectives, for custom scheduler",
	&(popts->objective_list),
	ponos_option_print_objective_list, ponos_option_set_objective_list },


      { "sched2dp1", "", "1: enforce 2d+1 schedule",
	&(popts->build_2d_plus_one),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { "K-value", "", "NUM: value of K constant in ILP",
	&(popts->legality_constant),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { "schedule-size", "", "NUM: dimensionality of schedule\n\nOutput schedule options:",
	&(popts->schedule_size),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { "schedule-coef-bound", "", "NUM: bound -NUM <= theta <= NUM",
	&(popts->schedule_bound),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { "schedule-coef-N", "", "1: set theta >= 0\n",
	&(popts->schedule_coefs_are_pos),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { NULL, NULL, NULL, 0, NULL}
    };
  g_pocc_interpreter_ponos_option =
    (s_interpreter_option_t*) &all_options;

  if (! cmd)
    {
      printf ("|> error: empty command\n");
      return;
    }

  // Remove the first command from the list if it is CMD_PONOS:
  // redundant info.
  if (cmd->cmd_id == POCC_CMD_PONOS)
    {
      if (cmd->cmd_ids[0] == POCC_CMD_PONOS)
	{
	  int i;
	  for (i = 0; cmd->cmd_ids[i] != POCC_CMD_END; ++i)
	    cmd->cmd_ids[i] = cmd->cmd_ids[i + 1];
	}
      cmd->cmd_id = cmd->cmd_ids[0];
    }

  pocc_interpreter_ponos_process_command(all_commands, state, cmd);
}
