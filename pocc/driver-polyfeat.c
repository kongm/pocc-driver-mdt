/*
 * driver-polyfeat.c: this file is part of the PoCC project.
 *
 * PoCC, the Polyhedral Compiler Collection package
 *
 * Copyright (C) 2011 Louis-Noel Pouchet
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

#include <pocc/driver-polyfeat.h>
#include <pocc/error.h>
#include <polyfeat/polyfeatures.h>
#include <polyfeat/depgraph.h>
#include <polyfeat/oi.h>

void
pocc_driver_polyfeat (scoplib_scop_p program,
		      s_past_node_t* root,
		      s_pocc_options_t* poptions,
		      s_pocc_utils_options_t* puoptions)
{
  if (! poptions->quiet)
    printf ("[PoCC] Running Polyfeat\n");

  if (poptions->polyfeat_legacy)
    {
      /// FIXME: not maintained.
      char buffer[strlen (poptions->output_file_name) + 6];
      strcpy (buffer, poptions->output_file_name);
      strcat (buffer, ".feat");
      FILE* outfile = fopen (buffer, "w");
      if (outfile)
	{
	  polyfeat_depgraph_extract (outfile, program, poptions->polyfeat_rar);
	  fclose (outfile);
	}
      else
	pocc_error ("Cannot open output file\n");
    }
  else if (poptions->polyfeat_ast_stats)
    {
      // For the moment, this is only compatible with pluto w/o
      // tiling.
      if (poptions->pluto_tile || !poptions->pluto || !poptions->aster_output)
	{
	  printf ("[PoCC] Error: --polyfeat-ast (AST stats) can only work in conjunction with the pluto scheduler, WITHOUT tiling. It also needs --output-aster flag. Aborting.\n");
	  exit (1);
	}
      polyfeat_ast_stats (program,
			  root,
			  NULL,
			  poptions->polyfeat_multi_params,
			  poptions->element_size_in_bytes,
			  poptions->parameter_values,
			  poptions->SIMD_vector_size_in_bytes,
			  poptions->SIMD_supports_fma,
			  poptions->num_omp_threads,
			  poptions->verbose);
    }
  else
    {
      polyfeat_estimate_oi (program,
			    root,
			    poptions->polyfeat_multi_params,
			    poptions->element_size_in_bytes,
			    poptions->parameter_values,
			    poptions->SIMD_vector_size_in_bytes,
			    poptions->SIMD_supports_fma,
			    poptions->num_omp_threads,
			    poptions->verbose);
    }
}
