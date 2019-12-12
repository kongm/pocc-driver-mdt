/*
 * interpreter.c: this file is part of the PoCC project.
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

#include "interpreter.h"
#include "interpreter_utils.h"
#include "interpreter_workspace.h"
#include "interpreter_ponos.h"

#include <pocc/driver-polyfeat.h>
/* #include <pocc/error.h> */
/* #include <polyfeat/polyfeatures.h> */
/* #include <polyfeat/depgraph.h> */
#include <polyfeat/oi.h>


int pocc_scanner(char* cmd);

/**
 * global variable with the last command parsed.
 */
s_pocc_command_t g_pocc_current_command;

/**
 * Ugly workaround, need fix to pass that variable as func. argument
 * everywhere instead.
 */
static s_interpreter_option_t* g_pocc_interpreter_pocc_option;


static
s_interpreter_state_t*
pocc_interpreter_state_malloc()
{
  s_interpreter_state_t* ret = XMALLOC(s_interpreter_state_t, 1);
  int i;
  for (i = 0; i < POCC_INTERPRETER_MAX_WORKSPACE; ++i)
    {
      ret->poptions[i] = NULL;
      ret->puoptions[i] = NULL;
      ret->scop[i] = NULL;
      ret->scop_original[i] = NULL;
      ret->input_filename[i] = NULL;
      ret->active_mode[i] = POCC_INTERPRETER_POCCMODE;
      ret->ponos_options[i] = NULL;
      ret->ponos_space[i] = NULL;
      ret->ponos_constraints_system[i] = NULL;
      ret->ponos_space_is_invalidated[i] = 0;
      ret->ponos_cplex_file[i] = NULL;
    }
  ret->active_workspace = 0;

  return ret;
}


static
void
pocc_interpreter_state_free(s_interpreter_state_t* state)
{
  int i;
  for (i = 0; i < POCC_INTERPRETER_MAX_WORKSPACE; ++i)
    {
      if (state->poptions[i])
	{
	  state->poptions[i]->output_file = NULL;
	  pocc_options_free (state->poptions[i]);
	}
      if (state->scop[i])
	scoplib_scop_free (state->scop[i]);
      if (state->scop_original[i])
	scoplib_scop_free (state->scop_original[i]);
      if (state->input_filename[i])
	XFREE(state->input_filename[i]);
      if (state->ponos_space[i])
	ponos_space_free (state->ponos_space[i]);
      if (state->ponos_constraints_system[i])
	fm_system_free (state->ponos_constraints_system[i]);
      if (state->ponos_options[i])
	ponos_options_free (state->ponos_options[i]);
      if (state->ponos_cplex_file[i])
	XFREE(state->ponos_cplex_file[i]);
    }
  XFREE(state);
}


void
pocc_interpreter_set_pocc_options(s_pocc_options_t* poptions)
{
  poptions->quiet = 1;
  poptions->verbose = 0;
  poptions->polyfeat_cache_is_priv = 1;
  poptions->ponos_coef_are_pos = 1;
  //poptions->polyfeat_cachesize;
}

static
char* pocc_interpreter_select_prompt(s_interpreter_state_t* state)
{
  if (! state)
    return NULL;
  switch (state->active_mode[state->active_workspace])
    {
    case POCC_INTERPRETER_POCCMODE: return "[pocc=> ";
    case POCC_INTERPRETER_PONOSMODE: return "[ponos=> ";
    default: return "[pocc=> ";
    }
  return NULL;
}


static
void
pocc_interpreter_initialize(s_interpreter_state_t* state)
{
  pip_init ();
  pocc_interpreter_initialize_current_workspace (state);

}


static
void
pocc_interpreter_finalize(s_interpreter_state_t* state)
{
  // Be clean.
  pip_close ();
}


static
void
pocc_interpreter_help(s_interpreter_state_t* state, s_pocc_command_t* cmd)
{
  printf ("|> Help:\n");
}


static
void
pocc_interpreter_codegen(s_interpreter_state_t* state, s_pocc_command_t* cmd)
{
  if (state->scop[state->active_workspace])
    {
      char* fname = state->poptions[state->active_workspace]->output_file_name;
      state->poptions[state->active_workspace]->output_file =
	fopen (fname, "w");
      if (state->poptions[state->active_workspace]->output_file)
	{
	  pocc_driver_codegen (state->scop[state->active_workspace],
	  		       state->poptions[state->active_workspace],
	  		       state->puoptions[state->active_workspace]);
	  fclose (state->poptions[state->active_workspace]->output_file);
	  pocc_interpreter_display_file
	    (stdout, state->poptions[state->active_workspace]->output_file_name);
	}
      else
	printf ("|> error: cannot open file %s\n", fname);
    }
  else
    printf ("|> nothing to display, empty scop\n");
}


static
void
pocc_interpreter_polyfeat(s_interpreter_state_t* state, s_pocc_command_t* cmd)
{
  if (state->scop[state->active_workspace])
    {
      char* fname = state->poptions[state->active_workspace]->output_file_name;
      state->poptions[state->active_workspace]->output_file =
	fopen (fname, "w");
      if (state->poptions[state->active_workspace]->output_file)
	{
	  s_pocc_options_t* options = state->poptions[state->active_workspace];
	  int old_polyfeatopt = options->polyfeat;
	  int old_pragmatizeropt = options->pragmatizer;
	  options->polyfeat = 1;
	  options->pragmatizer = 1;
	  void* old = options->polyfeat_multi_params;
	  if (old == NULL)
	    {
	      char buffer[1024];
	      sprintf (buffer, "%d,%d,%d", options->polyfeat_linesize,
		       options->polyfeat_cachesize,
		       options->polyfeat_cache_is_priv);
	      options->polyfeat_multi_params = strdup (buffer);
	    }

	  pocc_driver_codegen (state->scop[state->active_workspace],
	  		       state->poptions[state->active_workspace],
	  		       state->puoptions[state->active_workspace]);
	  fclose (state->poptions[state->active_workspace]->output_file);
	  if (old == NULL)
	    {
	      XFREE(options->polyfeat_multi_params);
	      options->polyfeat_multi_params = old;
	    }
	  options->polyfeat = old_polyfeatopt;
	  options->pragmatizer = old_pragmatizeropt;

	}
      else
	printf ("|> error: cannot open file %s\n", fname);
    }
  else
    printf ("|> nothing to display, empty scop\n");
}


static
void
pocc_interpreter_show(s_interpreter_state_t* state,
		      s_pocc_command_t* cmd)
{
  if (! state->scop[state->active_workspace] || ! cmd)
    {
      printf ("|> nothing to show in this workspace\n");
      return;
    }

  switch (cmd->cmd_ids[1])
    {
    case POCC_CMD_SCOP:
      printf ("|> Workspace #%d: scop\n", state->active_workspace);
      scoplib_scop_print_dot_scop (stdout,
				   state->scop[state->active_workspace]);
      break;
    case POCC_CMD_C:
      printf ("|> Workspace #%d: c code\n", state->active_workspace);
      pocc_interpreter_codegen (state, cmd);
      break;
    case POCC_CMD_STATS:
      printf ("|> Workspace #%d: Polyfeat-generated statistics:\n",
	      state->active_workspace);
      pocc_interpreter_polyfeat (state, cmd);
      break;
    case POCC_CMD_WORKSPACE:
      printf ("|> Workspace #%d: info\n", state->active_workspace);
      pocc_interpreter_show_current_workspace (state);
      break;
    case POCC_CMD_END:
      // Default to showing the scop file.
      printf ("|> Workspace #%d: scop\n", state->active_workspace);
      scoplib_scop_print_dot_scop (stdout,
				   state->scop[state->active_workspace]);
      break;
    default:
      printf ("|> error: command does not exist\n");
      break;
    }
}


static
void
pocc_interpreter_load(s_interpreter_state_t* state, s_pocc_command_t* cmd)
{
  if (! cmd)
    {
      printf ("|> error: command is empty\n");
      return;
    }
  if (cmd->cmd_args_last < 1)
    {
      printf ("|> error: missing filename (syntax: load <filename>)\n");
      return;
    }
  char* filename = (char*)cmd->cmd_args[0];
  FILE* inputf = fopen (filename, "r");
  if (inputf == NULL)
    {
      printf ("|> error: cannot open file %s\n", filename);
      return;
    }

  printf ("|> load file: %s\n", filename);
  pocc_interpreter_initialize_current_workspace (state);
  if (state->scop[state->active_workspace])
    scoplib_scop_free (state->scop[state->active_workspace]);
  if (state->scop_original[state->active_workspace])
    {
      scoplib_scop_free (state->scop_original[state->active_workspace]);
      state->scop_original[state->active_workspace] = NULL;
    }
  state->scop[state->active_workspace] =
    pocc_driver_clan (inputf, state->poptions[state->active_workspace],
		      state->puoptions[state->active_workspace]);
  if (state->poptions[state->active_workspace]->input_file)
    fclose (state->poptions[state->active_workspace]->input_file);
  state->poptions[state->active_workspace]->input_file = inputf;
  if (state->poptions[state->active_workspace]->input_file_name)
    XFREE(state->poptions[state->active_workspace]->input_file_name);
  state->poptions[state->active_workspace]->input_file_name = strdup (filename);
  if (state->scop[state->active_workspace])
    {
      state->input_filename[state->active_workspace] = filename;
      char buffer[1024];
      sprintf (buffer, "___temp_output_pocc_interpreter_%d.tmp.c",
	       state->active_workspace);
      state->poptions[state->active_workspace]->output_file_name =
	strdup (buffer);
      // Store duplicate of original scop, w/o schedule modif.
      state->scop_original[state->active_workspace] =
	scoplib_scop_dup (state->scop[state->active_workspace]);
    }
  else
    printf ("|> error loading %s\n", filename);
}


static
void
pocc_interpreter_store(s_interpreter_state_t* state, s_pocc_command_t* cmd)
{
  printf ("|> store:\n");
}



static
void
pocc_interpreter_namespace(s_interpreter_state_t* state, s_pocc_command_t* cmd)
{
  if (! cmd)
    {
      printf ("|> error: command is empty\n");
      return;
    }
  switch (cmd->cmd_ids[1])
    {
    case POCC_CMD_POCC:
      state->active_mode[state->active_workspace] = POCC_INTERPRETER_POCCMODE;
      printf ("|> namespace pocc enabled\n");
      break;
    case POCC_CMD_PONOS:
      state->active_mode[state->active_workspace] = POCC_INTERPRETER_PONOSMODE;
      printf ("|> namespace ponos enabled\n");
      break;
    default:
      printf ("|> error: namespace not supported\n");
      break;
    }
}


void
pocc_interpreter_option_print_int(char* buffer, void* arg)
{
  if (! buffer)
    return;
  sprintf (buffer, "<>");
  if (! arg)
    return;
  int type = *((int*)arg);
  sprintf (buffer, "%d", type);
}

void
pocc_interpreter_option_set_int(char* buffer_val, void* arg)
{
  if (! arg)
    return;
  int val = (int) ((intptr_t)buffer_val);
  int* type = (int*)arg;
  if (type)
    *type = val;
  else
    printf ("|> error: target pointer is null\n");
}


void
pocc_interpreter_option_print_charp(char* buffer, void* arg)
{
  if (! buffer)
    return;
  sprintf (buffer, "<>");
  if (! arg)
    return;
  char* str = *((char**)arg);
  sprintf (buffer, "%s", str);
}

void
pocc_interpreter_option_set_charp(char* buffer_val, void* arg)
{
  if (! arg)
    return;
  char** str = ((char**)arg);
  *str = strdup (buffer_val);
}


static
void
pocc_print_options(s_interpreter_state_t* state,
		    s_pocc_command_t* cmd)
{
  printf ("|> PoCC options:\n");
  int i, j;
  char buffer[1024];
  for (i = 0; g_pocc_interpreter_pocc_option[i].longname; ++i)
    {
      char* longname = g_pocc_interpreter_pocc_option[i].longname;
      char* shortname = g_pocc_interpreter_pocc_option[i].shortname;
      char* description = g_pocc_interpreter_pocc_option[i].description;
      void* ptrval = g_pocc_interpreter_pocc_option[i].ptrval;
      void (*printfun)(char*, void*) =
	g_pocc_interpreter_pocc_option[i].fun_ptr_print;
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
pocc_interpreter_display(s_interpreter_state_t* state, s_pocc_command_t* cmd)
{
  if (! cmd)
    {
      printf ("|> error: command is empty\n");
      return;
    }
  pocc_interpreter_initialize_current_workspace (state);
  switch (cmd->cmd_ids[1])
    {
    case POCC_CMD_OPTIONS:
      pocc_print_options (state, cmd); break;
    default:
      printf ("|> error: command not supported\n"); break;
    }
}

static
void pocc_interpreter_user_ctxt_print(char* buffer, void* arg)
{
  if (! arg || !buffer)
    return;
  int* tab = *(int**)arg;
  int i;
  char buff[64];
  for (i = 0; tab && tab[i] != -1; ++i)
    {
      sprintf (buff, "%d", tab[i]);
      if (i > 0)
	{
	  strcat (buffer, ",");
	  strcat (buffer, buff);
	}
      else
	strcpy (buffer, buff);
    }
}


static
void pocc_interpreter_user_ctxt_set(char* buffer_val, void* arg)
{
  int len = strlen (buffer_val);
  if (len)
    {
      char* tmp = buffer_val;
      int num = 1;
      int ii;
      for (ii = 0; ii < len; ++ii)
	if (buffer_val[ii] == ',')
	  ++num;
      int** ptrval = (int**)arg;
      *ptrval = XMALLOC(int, num + 1);
      ii = 0;
      if (*tmp == '"' || *tmp == '\'')
	++tmp;
      while (*tmp && ii < num)
	{
	  char buff[64];
	  int pos = 0;
	  while (pos < 63 && *tmp && (*tmp >= '0' && *tmp <= '9'))
	    buff[pos++] = *(tmp++);
	  buff[pos] = '\0';
	  (*ptrval)[ii++] = atoi (buff);
	  while (*tmp && (*tmp <= '0' || *tmp >= '9'))
	    ++tmp;
	}
      (*ptrval)[ii] = -1;
    }
 }


static
void
pocc_interpreter_set_option(s_interpreter_state_t* state, s_pocc_command_t* cmd)
{
  int i;
  s_interpreter_option_t* popts = g_pocc_interpreter_pocc_option;
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
    printf ("|> error: trying to set unknown option (try 'pocc display options'\n");

}


static
void
pocc_interpreter_set(s_interpreter_state_t* state, s_pocc_command_t* cmd)
{
  if (! cmd || ! cmd->cmd_args || ! cmd->cmd_args[0])
    {
      printf ("|> error: ill-formed set command\n");
      return;
    }
  switch (cmd->cmd_ids[1])
    {
    case POCC_CMD_OPTION: pocc_interpreter_set_option (state, cmd); break;
    default: printf ("|> unsupported set command\n"); break;
    }
}


static
void
pocc_interpreter_print_command(s_interpreter_state_t* state,
			       s_pocc_command_t* cmd)
{
  if (! cmd)
    printf ("|> error: command is empty\n");
  int i;
  printf ("|> command to be executed: ");
  const char* str;
  for (i = 0; cmd->cmd_ids[i] != POCC_CMD_END; ++i)
    {
      switch (cmd->cmd_ids[i])
	{
	case POCC_CMD_END: str = "end"; break;
	case POCC_CMD_ERROR: str = "error"; break;
	case POCC_CMD_HELP: str = "help"; break;
	case POCC_CMD_QUIT: str = "quit"; break;
	case POCC_CMD_SHOW: str = "show"; break;
	case POCC_CMD_LOAD: str = "load"; break;
	case POCC_CMD_STORE: str = "store"; break;
	case POCC_CMD_DISPLAY: str = "display"; break;
	case POCC_CMD_CODEGEN: str = "codegen"; break;
	case POCC_CMD_WORKSPACE: str = "workspace"; break;
	case POCC_CMD_PONOS: str = "ponos"; break;
	case POCC_CMD_LIST: str = "list"; break;
	case POCC_CMD_INFO: str = "info"; break;
	case POCC_CMD_SWITCH: str = "switch"; break;
	case POCC_CMD_SCOP: str = "scop"; break;
	case POCC_CMD_C: str = "c"; break;
	case POCC_CMD_NAMESPACE: str = "namespace"; break;
	case POCC_CMD_POCC: str = "pocc"; break;
	case POCC_CMD_OPTIONS: str = "options"; break;
	case POCC_CMD_INITIALIZE: str = "initialize"; break;
	case POCC_CMD_OPTION: str = "option"; break;
	case POCC_CMD_SET: str = "set"; break;
	case POCC_CMD_COMPUTE: str = "compute"; break;
	case POCC_CMD_LEGAL_SPACE: str = "legal-space"; break;
	case POCC_CMD_SCHEDULE: str = "schedule"; break;
	case POCC_CMD_SPACE: str = "space"; break;
	case POCC_CMD_VARS: str = "vars"; break;
	case POCC_CMD_CPLEX_LP: str = "cplex-lp"; break;
	case POCC_CMD_VAR: str = "var"; break;
	case POCC_CMD_ALL_VARS: str = "all-vars"; break;
	case POCC_CMD_ALL_VARS_DETAILED: str = "all-vars-detail"; break;
	default: str = "<unknown>"; break;
	}
      printf ("%s ", str);
    }
  printf (" (with %d args)\n", cmd->cmd_args_last);

}


static
int
pocc_interpreter_process_command(s_interpreter_command_t* all_commands,
				 s_interpreter_state_t* state)
{
  int i;
  s_pocc_command_t* cmd = &g_pocc_current_command;
  if (cmd->cmd_id == POCC_CMD_QUIT)
    return 1;

  if (cmd->cmd_id == POCC_CMD_END)
    return 0;

  // Execute a ponos command:
  if ((state->active_mode[state->active_workspace] == POCC_INTERPRETER_PONOSMODE
       || cmd->cmd_id == POCC_CMD_PONOS)
      && (cmd->cmd_id != POCC_CMD_POCC))
    {
      pocc_interpreter_ponos (state, cmd);
      return EXIT_SUCCESS;
    }

  // Strip the POCC_CMD_POCC prefix, if any.
  if (cmd->cmd_id == POCC_CMD_POCC)
    {
      if (cmd->cmd_ids[0] == POCC_CMD_POCC)
	{
	  int i;
	  for (i = 0; cmd->cmd_ids[i] != POCC_CMD_END; ++i)
	    cmd->cmd_ids[i] = cmd->cmd_ids[i + 1];
	}
      cmd->cmd_id = cmd->cmd_ids[0];
    }

  // Set up the PoCC options structure.
  s_pocc_options_t* popts = state->poptions[state->active_workspace];

  s_interpreter_option_t all_options[] =
    {
      { "verbose", "v", "1: enable verbose output",
	&(popts->verbose),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { "quiet", "", "1: turn off all output\n\nContext options:",
	&(popts->quiet),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { "bounded-ctxt", "", "1: Bound all parameters to >= 1",
	&(popts->clan_bounded_context),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { "default-ctxt", "", "1: Bound all parameters to >= 32",
	&(popts->set_default_parameter_values),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { "user-ctxt", "", "NUM1,NUM2,...: set parameters to\n\t\t\t\t\t\tvalue",
	&(popts->parameter_values),
	pocc_interpreter_user_ctxt_print, pocc_interpreter_user_ctxt_set },

      { "pragmatizer", "", "1: Mark parallel loops with\n\t\t\t\t\t\tpragmas omp/simd",
	&(popts->pragmatizer),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { "simdizer", "", "1: Enable SIMDization (experimental)",
	&(popts->vectorizer_vectorize_loops),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { "scalar-elt-size", "", "NUM: Size (bytes) of scalars\n\t\t\t\t\t\tExample: set to 4 for float",
	&(popts->element_size_in_bytes),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { "simd-vec-size", "", "NUM: Size (bytes) of SIMD vectors\n\t\t\t\t\t\tExample: set to 32 for AVX/AVX2",
	&(popts->SIMD_vector_size_in_bytes),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { "cache-line-size", "", "NUM: Size (bytes)",
	&(popts->polyfeat_linesize),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },
      { "cache-size", "", "NUM: Size (bytes)",
	&(popts->polyfeat_cachesize),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },
      { "cache-is-private", "", "1: Cache is private",
	&(popts->polyfeat_cache_is_priv),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { "omp-threads-count", "", "NUM: Number of OpenMP threads",
	&(popts->num_omp_threads),
	pocc_interpreter_option_print_int, pocc_interpreter_option_set_int },

      { NULL, NULL, NULL, 0, NULL}
    };
  g_pocc_interpreter_pocc_option =
    (s_interpreter_option_t*) &all_options;

  // Execute a pocc command:
  for (i = 0; all_commands[i].fun_ptr != NULL; ++i)
    if (all_commands[i].cmd_id == cmd->cmd_id)
      {
	if (all_commands[i].fun_ptr)
	  all_commands[i].fun_ptr (state, cmd);
	break;
      }
  if (all_commands[i].fun_ptr == NULL)
    printf ("|> command not found\n");

  return EXIT_SUCCESS;
}


int pocc_interpreter()
{

  // Base PoCC commands.
  s_interpreter_command_t all_commands[] =
    {
      { POCC_CMD_HELP, pocc_interpreter_help, "help" },
      { POCC_CMD_SHOW, pocc_interpreter_show, "show" },
      { POCC_CMD_LOAD, pocc_interpreter_load, "load" },
      { POCC_CMD_STORE, pocc_interpreter_store, "store" },
      { POCC_CMD_DISPLAY, pocc_interpreter_display, "display" },
      { POCC_CMD_WORKSPACE, pocc_interpreter_workspace, "workspace" },
      { POCC_CMD_INFO, pocc_interpreter_info, "info" },
      { POCC_CMD_PONOS, pocc_interpreter_ponos, "ponos" },
      { POCC_CMD_NAMESPACE, pocc_interpreter_namespace, "namespace" },
      { POCC_CMD_SET, pocc_interpreter_set, "set" },
      { POCC_CMD_ERROR, NULL, NULL },
      { -1, NULL, NULL}
    };

  s_interpreter_state_t* state =  pocc_interpreter_state_malloc ();
  pocc_interpreter_initialize_current_workspace (state);

  int stop = 0;
  char buffer[1024];
  while (! stop)
    {
      char* prompt = pocc_interpreter_select_prompt (state);
      fprintf (stdout, prompt);
      char* line = fgets (buffer, 1024, stdin);
      if (line)
	{
	  /* printf ("line=%s\n", line); */
	  pocc_scanner (line);
	  /* pocc_interpreter_print_command(state, &g_pocc_current_command); */
	  stop = pocc_interpreter_process_command (all_commands, state);
	  fprintf (stdout, "\n");
	}
      else
	// Something bad happened...
	if (feof (stdin))
	  stop = 1;
    }

  pocc_interpreter_state_free (state);

  return 0;
}
