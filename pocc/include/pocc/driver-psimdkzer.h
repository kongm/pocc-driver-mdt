/*
 * driver-psimdkzer.h: this file is part of the PoCC project.
 *
 * PoCC, the Polyhedral Compiler Collection package
 *
 * Copyright (C) 2012 Louis-Noel Pouchet
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
 * Martin Kong <kongm@cse.ohio-state.edu>
 *
 */
#ifndef POCC_DRIVER_SPIRAL_H
# define POCC_DRIVER_SPIRAL_H

# include <stdio.h>

# if HAVE_CONFIG_H
#  include <pocc-utils/config.h>
# endif

# include <pocc/common.h>
# include <pocc-utils/options.h>
# include <pocc/options.h>
# include <pocc/exec.h>

# ifndef SCOPLIB_INT_T_IS_LONGLONG
#  define SCOPLIB_INT_T_IS_LONGLONG
# endif
# define LINEAR_VALUE_IS_LONGLONG
# include <scoplib/scop.h>
# include <past/past.h>

//# include <psimdkzer/psimdkzer.h>
//# include <psimdkzer/options.h>




#define PSIMDKZER_TARGET_PREVECTOR 1
#define PSIMDKZER_TARGET_SPIRAL 2
#define PSIMDKZER_TARGET_NAIVE 3
#define PSIMDKZER_TARGET_NOVEC 4

#define PSIMDKZER_VECTOR_ISA_SSE 1
#define PSIMDKZER_VECTOR_ISA_AVX 2
#define PSIMDKZER_VECTOR_ISA_BGQ 3
#define PSIMDKZER_VECTOR_ISA_MIC 4
#define PSIMDKZER_SCALAR_DATATYPE_FLOAT 1
#define PSIMDKZER_SCALAR_DATATYPE_DOUBLE 2

#define PSIMDKZER_FULL_TILE 1
#define PSIMDKZER_PARTIAL_TILE 2

#define PSIMDKZER_MAX_DIM 10


BEGIN_C_DECLS

extern
void
pocc_driver_psimdkzer_schedule_reskew (scoplib_scop_p program,
				       s_pocc_options_t* poptions,
				       s_pocc_utils_options_t* puoptions);


extern
void
pocc_driver_psimdkzer (s_past_node_t  * root,
		       scoplib_scop_p scop,
		       s_pocc_options_t * poptions,
		       s_pocc_utils_options_t * puoptions);



END_C_DECLS


#endif // POCC_DRIVER_SPIRAL_H
