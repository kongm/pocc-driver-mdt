/*
 * driver-preprocess.h: this file is part of the PoCC project.
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
 * Louis-Noel Pouchet <pouchet@colostate.edu
 *
 */
#ifndef POCC_DRIVER_PREPROCESS_H
# define POCC_DRIVER_PREPROCESS_H

# include <stdio.h>

# if HAVE_CONFIG_H
#  include <pocc-utils/config.h>
# endif

# include <pocc/common.h>
# include <pocc-utils/options.h>
# include <pocc/options.h>
# ifndef LINEAR_VALUE_IS_LONGLONG
#  define LINEAR_VALUE_IS_LONGLONG
# endif
# ifndef CANDL_SUPPORTS_SCOPLIB
#  define CANDL_SUPPORTS_SCOPLIB
# endif
# include <candl/program.h>
# include <candl/dependence.h>
# include <candl/options.h>
# ifndef SCOPLIB_INT_T_IS_LONGLONG
#  define SCOPLIB_INT_T_IS_LONGLONG
# endif
# include <scoplib/scop.h>
# include <past/past.h>


BEGIN_C_DECLS


/**
 * Parameterize a SCoP.
 *
 *
 */
extern
void
pocc_preprocess_parameterize_scop(scoplib_scop_p scop, int slack);


/**
 * Main entry point.
 *
 */
extern
int
pocc_driver_preprocess(scoplib_scop_p program,
		       s_pocc_options_t* poptions,
		       s_pocc_utils_options_t* puoptions);


END_C_DECLS


#endif // POCC_DRIVER_PREPROCESS_H
