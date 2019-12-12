/*
 * interpreter_workspace.c: this file is part of the PoCC project.
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

#include "interpreter_workspace.h"
#include "interpreter_utils.h"


void
pocc_interpreter_initialize_workspace(s_interpreter_state_t* state,
				      int workspace_id)
{
  if (state->poptions[workspace_id] == NULL)
    state->poptions[workspace_id] = pocc_options_malloc ();
  if (state->puoptions[workspace_id] == NULL)
    state->puoptions[workspace_id] = pocc_utils_options_malloc ();
  pocc_interpreter_set_pocc_options (state->poptions[workspace_id]);

  if (state->poptions[workspace_id]->trash)
    {
      char* args[3];
      args[0] = STR_POCC_ROOT_DIR "/generators/scripts/trash";
      args[1] =
	strdup (state->poptions[state->active_workspace]->input_file_name);
      args[1][strlen (args[1]) - 2] = '\0';
      args[2] = NULL;
      pocc_exec (args, POCC_EXECV_HIDE_OUTPUT);
      XFREE(args[1]);
    }

}


void
pocc_interpreter_initialize_current_workspace(s_interpreter_state_t* state)
{
  pocc_interpreter_initialize_workspace (state, state->active_workspace);
}


static
void
pocc_interpreter_show_workspace(s_interpreter_state_t* state,
			      int ctxt_id)
{
  if (state->scop[ctxt_id])
    printf ("SCoP: \t\t\tbuilt\t\t('show scop' ready)\n");
  else
    printf ("SCoP: \t\t\tunset\n");
  if (state->poptions[ctxt_id])
    {
      s_pocc_options_t* opts = state->poptions[ctxt_id];
      printf ("PoCC Options: \t\tset\n");
      printf ("Input file: \t\t%s\n", opts->input_file_name);
      /* printf ("Output file: %s\n", opts->output_file_name); */
      if (state->scop[ctxt_id])
	{
	  printf ("C file: \t\tsupported\t('show c' ready)\n");
	  printf ("Ponos: \t\t\t");
	  if (state->ponos_options[ctxt_id])
	    printf ("options set\n");
	  else
	    printf ("unset\t\t\n");
	  printf ("Ponos legal space: \t");
	  if (state->ponos_space[ctxt_id])
	    printf ("ready\n");
	  else
	    printf ("unset\t\t(call 'ponos compute legal-space')\n");
	  if (state->ponos_options[ctxt_id] &&
	      state->ponos_options[ctxt_id]->solver == PONOS_SOLVER_CPLEX)
	    {
	      printf ("Ponos cplex status: \t");
	      if (state->ponos_cplex_file[ctxt_id])
		printf ("solved\n");
	      else
		printf ("unset\t\t(call 'ponos compute schedule')\n");
	    }
	}
    }
  else
    printf ("PoCC Options: \t\tunset \t(workspace not ready)\n");
  if (state->scop[ctxt_id])
    pocc_interpreter_print_scop_summary (stdout, state->scop[ctxt_id]);
}


void
pocc_interpreter_show_current_workspace(s_interpreter_state_t* state)
{
  pocc_interpreter_show_workspace (state, state->active_workspace);
}


static
void
pocc_interpreter_display_all_workspaces(s_interpreter_state_t* state,
					s_pocc_command_t* cmd)
{
  int i;
  int count = 0;
  for (i = 0; i < POCC_INTERPRETER_MAX_WORKSPACE; ++i)
    if (state->poptions[i])
      {
	printf ("|> Workspace #%d\n", i);
	pocc_interpreter_show_workspace (state, i);
	printf ("\n");
	++count;
      }
  if (count)
    printf ("\n");
  printf ("|> Active workspace(s): %d/64", count);
  printf ("\n");
}


static
void
pocc_interpreter_switch_workspace (s_interpreter_state_t* state,
				   s_pocc_command_t* cmd)
{
  if (! cmd)
    {
      printf ("|> error: command is empty\n");
      return;
    }
  if (cmd->cmd_args_last < 1)
    {
      printf ("|> error: missing workspace number (syntax: workspace switch <num>)\n");
      return;
    }
  void** args = cmd->cmd_args;
  int cid = (int) ((intptr_t)args[0]);
  if (cid >= 0 && cid <= 63)
    {
      state->active_workspace = cid;
      pocc_interpreter_initialize_current_workspace (state);
      printf ("|> Switched to workspace #%d\n", cid);
	}
  else
    printf ("|> error: cannot switch to workspace #%d (64 workspaces max)\n", cid);
}


/**
 * Main wrapper for workspace * commands.
 *
 */
void
pocc_interpreter_workspace(s_interpreter_state_t* state, s_pocc_command_t* cmd)
{
  if (! cmd)
    {
      printf ("|> error: command is empty\n");
      return;
    }
  switch (cmd->cmd_ids[1])
    {
    case POCC_CMD_LIST:
      pocc_interpreter_display_all_workspaces (state, cmd);
      break;
    case POCC_CMD_SWITCH:
      pocc_interpreter_switch_workspace (state, cmd);
      break;
    case POCC_CMD_INFO:
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
