/*
 * interpreter_utils.h: this file is part of the PoCC project.
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
#ifndef POCC_INTERPRETER_UTILS_H
# define POCC_INTERPRETER_UTILS_H

# include <stdarg.h>
# include <stdio.h>

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

# include "interpreter.h"


BEGIN_C_DECLS

extern
char*
pocc_interpreter_execute_command_output(int num_args, ...);

extern
void
pocc_interpreter_execute_command_silent(int num_args, ...);


/**
 * Convenience.
 */
extern
void
pocc_interpreter_display_file(FILE* stream, char* filename);

extern
char*
pocc_interpreter_utils_remove_double_quote(char* s);


/**
 *
 */
extern
void
pocc_interpreter_print_scop_schedule (FILE* fout, scoplib_statement_p stmt,
				      scoplib_scop_p scop, int stmt_id);

/**
 *
 *
 */
extern
void
pocc_interpreter_info(s_interpreter_state_t* state, s_pocc_command_t* cmd);

/**
 *
 *
 */
void
pocc_interpreter_print_scop_summary (FILE* fout, scoplib_scop_p scop);



END_C_DECLS


#endif // POCC_INTERPRETER_UTILS_H
