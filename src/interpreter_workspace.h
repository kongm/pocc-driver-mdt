/*
 * interpreter_workspace.h: this file is part of the PoCC project.
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
#ifndef POCC_INTERPRETER_WORKSPACE_H
# define POCC_INTERPRETER_WORKSPACE_H

# include <pocc/common.h>

# include "interpreter.h"


BEGIN_C_DECLS

/**
 *
 */
extern
void
pocc_interpreter_initialize_workspace(s_interpreter_state_t* state,
				      int workspace_id);

/**
 *
 */
extern
void
pocc_interpreter_initialize_current_workspace(s_interpreter_state_t* state);

/**
 *
 */
extern
void
pocc_interpreter_show_current_workspace(s_interpreter_state_t* state);


/**
 * Handles workspace * commands.
 *
 */
extern
void
pocc_interpreter_workspace(s_interpreter_state_t* state, s_pocc_command_t* cmd);



END_C_DECLS


#endif // POCC_INTERPRETER_WORKSPACE_H
