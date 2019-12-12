/*
 * pocc-openscop.c: this file is part of the PoCC project.
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
 * Louis-Noel Pouchet <Louis-Noel.Pouchet@inria.fr>
 *
 */
#if HAVE_CONFIG_H
# include <pocc-utils/config.h>
#endif
#include <pocc/common.h>

#include <stdio.h>
#include <pocc/pocc-openscop.h>
#include <pocc/driver-ponos.h>
#include <pocc/driver-codegen.h>
#include <pocc/exec.h>
#include <pocc/error.h>
#include <irconverter/osl2scoplib.h>


scoplib_scop_p
pocc_openscop_driver(osl_scop_p input_scop, FILE* output_stream)
{
  // (0) Initialize and get options.
  s_pocc_options_t* poptions = pocc_options_malloc ();
  s_pocc_utils_options_t* puoptions = pocc_utils_options_malloc ();
  pip_init ();

  /// Special: manually set the pocc options we want. Probably that
  /// should be passed as argument to this function, modify as needed.
  /// See driver/pocc/options.c for some details.

  // Context (OpenMP/SIMD) profile.
  poptions->SIMD_vector_size_in_bytes = 32; // AVX
  poptions->element_size_in_bytes = 8; // double precision
  poptions->vectorizer_vectorize_loops = 1; // enable simdization. May be very
					   // fragile, turn to 0 if bugs ;-)

  // Ponos options.
  poptions->ponos = 1; // enable ponos
  poptions->ponos_solver_type = PONOS_SOLVER_PIP; // set solver to pip, not gmp.
  poptions->ponos_solver_precond = 0;
  poptions->ponos_coef_are_pos = 1; // \theta >= 0
  poptions->ponos_debug = 0;
  poptions->ponos_legality_constant_K = 3; // K constant in formulation.
  poptions->ponos_schedule_bound = 10; // -10 <= \theta <= 10
  poptions->ponos_schedule_dim = 3; // build a 3d schedule.
  poptions->ponos_objective = PONOS_OBJECTIVES_CUSTOM;
  poptions->ponos_objective_list[0] = PONOS_OBJECTIVES_TASCHED;
  poptions->ponos_objective_list[1] = -1;


  // Converter used to convert clan osl output to valid scoplib. Does
  // not support existential variables, union of domains, etc.
  scoplib_scop_p scop = irconverter_osl2scoplib (input_scop, 1);

  // Perform Ponos.
  if (poptions->ponos)
    if (pocc_driver_ponos (scop, poptions, puoptions) == EXIT_FAILURE)
      exit (EXIT_FAILURE);

  scoplib_scop_p output_scop = scoplib_scop_dup (scop);
  if (poptions->cloogify_schedules)
    pocc_cloogify_scop (output_scop);

  // Perform codgen.
  if (poptions->codegen && output_stream)
    pocc_driver_codegen (scop, poptions, puoptions);

  // Be clean.
  scoplib_scop_free (scop);

  pip_close ();
  if (! poptions->quiet)
    printf ("[PoCC] All done.\n");
  pocc_options_free (poptions);

  return output_scop;
}
