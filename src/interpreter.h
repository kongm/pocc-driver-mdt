/*
 * interpreter.h: this file is part of the PoCC project.
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
#ifndef POCC_INTERPRETER_H
# define POCC_INTERPRETER_H

# include <pocc/common.h>

# include <pocc/driver-clan.h>
# include <pocc/driver-candl.h>
# include <pocc/driver-letsee.h>
# include <pocc/driver-pluto.h>
# include <pocc/driver-codegen.h>
# include <pocc/driver-cloog.h>
# include <pocc/driver-clastops.h>
# include <pocc/driver-ponos.h>
# include <pocc/driver-psimdkzer.h>
# include <pocc/exec.h>
# include <pocc/error.h>
# ifdef POCC_DEVEL_MODE
#  include <pocc/driver-cdscgr.h>
# endif

/**
 * Unique identifier of all keywords in the language.
 *
 */
# define POCC_CMD_END			-2
# define POCC_CMD_ERROR			-1
# define POCC_CMD_HELP			0
# define POCC_CMD_QUIT			1
# define POCC_CMD_SHOW			2
# define POCC_CMD_LOAD			3
# define POCC_CMD_STORE			4
# define POCC_CMD_DISPLAY		5
# define POCC_CMD_CODEGEN		6
# define POCC_CMD_CONTEXT		7
# define POCC_CMD_PONOS			8
# define POCC_CMD_LIST			9
# define POCC_CMD_INFO			10
# define POCC_CMD_SWITCH		11
# define POCC_CMD_SCOP			12
# define POCC_CMD_C			13
# define POCC_CMD_NAMESPACE    		14
# define POCC_CMD_POCC    		15
# define POCC_CMD_INITIALIZE    	16
# define POCC_CMD_OPTIONS	    	17
# define POCC_CMD_OPTION	    	18
# define POCC_CMD_SET		    	19
# define POCC_CMD_COMPUTE	    	20
# define POCC_CMD_LEGAL_SPACE	    	21
# define POCC_CMD_SCHEDULE	    	22
# define POCC_CMD_SPACE		  	23
# define POCC_CMD_VARS			24
# define POCC_CMD_CPLEX_LP	    	25
# define POCC_CMD_WORKSPACE	    	26
# define POCC_CMD_VAR		    	27
# define POCC_CMD_ALL_VARS		28
# define POCC_CMD_ALL_VARS_DETAILED     29
# define POCC_CMD_CONSTRAINTS		30
# define POCC_CMD_CONSTRAINT		31
# define POCC_CMD_DELETE		32
# define POCC_CMD_MATCHING		33
# define POCC_CMD_SCHEDULING_CONSTRAINTS	34
# define POCC_CMD_INSERT		35
# define POCC_CMD_VARIABLE		36
# define POCC_CMD_VARIABLE_BOUNDS      	37
# define POCC_CMD_SOL_LEXMIN      	38
# define POCC_CMD_ALL_CONSTRAINTS      	39
# define POCC_CMD_IS_INCLUDED      	40
# define POCC_CMD_STATS		      	41



# define POCC_INTERPRETER_MAX_ARGS	64

# define POCC_INTERPRETER_POCCMODE	0
# define POCC_INTERPRETER_PONOSMODE	1

# define POCC_INTERPRETER_SPACE_LAST_POS -2


#define pocc_interpreter_register_next_command(s) \
  g_pocc_current_command.cmd_ids[g_pocc_current_command.cmd_last++] = s


BEGIN_C_DECLS

struct s_pocc_command
{
  int		cmd_id;
  void*		cmd_args[POCC_INTERPRETER_MAX_ARGS];
  int		cmd_ids[POCC_INTERPRETER_MAX_ARGS];
  int		cmd_last;
  int		cmd_args_last;
};
typedef struct s_pocc_command s_pocc_command_t;

extern
s_pocc_command_t g_pocc_current_command;

#define POCC_INTERPRETER_MAX_WORKSPACE	128

struct s_interpreter_state
{
  s_pocc_options_t*		poptions[POCC_INTERPRETER_MAX_WORKSPACE];
  s_pocc_utils_options_t*	puoptions[POCC_INTERPRETER_MAX_WORKSPACE];
  scoplib_scop_p		scop[POCC_INTERPRETER_MAX_WORKSPACE];
  scoplib_scop_p		scop_original[POCC_INTERPRETER_MAX_WORKSPACE];
  char*				input_filename[POCC_INTERPRETER_MAX_WORKSPACE];
  int				active_workspace;
  int				active_mode[POCC_INTERPRETER_MAX_WORKSPACE];
  s_ponos_options_t*		ponos_options[POCC_INTERPRETER_MAX_WORKSPACE];
  s_ponos_space_t*		ponos_space[POCC_INTERPRETER_MAX_WORKSPACE];
  s_fm_system_t*		ponos_constraints_system[POCC_INTERPRETER_MAX_WORKSPACE];
  int				ponos_space_is_invalidated[POCC_INTERPRETER_MAX_WORKSPACE];
  char*				ponos_cplex_file[POCC_INTERPRETER_MAX_WORKSPACE];
};
typedef struct s_interpreter_state s_interpreter_state_t;


/**
 * User command description.
 *
 */
struct s_interpreter_command
{
  int cmd_id;
  void (*fun_ptr)(s_interpreter_state_t*, s_pocc_command_t*);
  char* cmd_name;
};
typedef struct s_interpreter_command s_interpreter_command_t;


/**
 * Store a list of options that can be set for a pass.
 *
 */
struct s_interpreter_option
{
  char* longname;
  char* shortname;
  char* description;
  void* ptrval;
  void (*fun_ptr_print)(char*, void*);
  void (*fun_ptr_set)(char*, void*);
};
typedef struct s_interpreter_option s_interpreter_option_t;


/**
 * Convenience.
 */
extern
void
pocc_interpreter_set_pocc_options(s_pocc_options_t* poptions);

extern
void
pocc_interpreter_option_print_int(char* buffer, void* arg);

extern
void
pocc_interpreter_option_set_int(char* buffer_val, void* arg);

extern
void
pocc_interpreter_option_print_charp(char* buffer, void* arg);

extern
void
pocc_interpreter_option_set_charp(char* buffer_val, void* arg);



/**
 * Interpreter for PoCC.
 *
 */
extern
int
pocc_interpreter();

END_C_DECLS


#endif // POCC_INTERPRETER_H
