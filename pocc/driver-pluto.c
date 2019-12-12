/*
 * driver-pluto.c: this file is part of the PoCC project.
 *
 * PoCC, the Polyhedral Compiler Collection package
 *
 * Copyright (C) 2009 Louis-Noel Pouchet
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

#include <stdlib.h>

#include <pocc/driver-pluto.h>
#include <pocc/driver-candl.h>
#include <pocc/driver-cloog.h>
#include <pluto/pocc-driver.h>


// Ugly forward declaration to avoid compilation warning. Would have
// been simpler to create an option.h file in Pluto...
PlutoOptions* pluto_options_alloc ();

int
pocc_driver_pluto (scoplib_scop_p program,
		  s_pocc_options_t* poptions,
		  s_pocc_utils_options_t* puoptions)
{
  if (! poptions->quiet)
    printf ("[PoCC] Running Pluto\n");

  PlutoOptions* ploptions = pluto_options_alloc ();
  ploptions->parallel = poptions->pluto_parallel;
  ploptions->tile = poptions->pluto_tile;
  ploptions->rar = poptions->pluto_rar;
  ploptions->unroll = poptions->pluto_unroll;
  ploptions->fuse = poptions->pluto_fuse;
  ploptions->polyunroll = poptions->pluto_polyunroll;
  ploptions->bee = poptions->pluto_bee;
  ploptions->prevector = poptions->pluto_prevector;
  ploptions->ufactor = poptions->pluto_ufactor; // int
  ploptions->quiet = poptions->pluto_quiet | poptions->quiet;
  ploptions->silent = poptions->quiet;
  ploptions->context = poptions->pluto_context; // int
  ploptions->cloogf = poptions->cloog_f; // int
  ploptions->cloogl = poptions->cloog_l; // int
  ploptions->ft = poptions->pluto_ft; // int
  ploptions->lt = poptions->pluto_lt; // int
/*   ploptions->ft = poptions->cloog_f; // int */
/*   ploptions->lt = poptions->cloog_l; // int */


  ploptions->multipipe = poptions->pluto_multipipe;
  ploptions->l2tile = poptions->pluto_l2tile;
  ploptions->lastwriter = poptions->pluto_lastwriter;
  ploptions->scalpriv = poptions->pluto_scalpriv;
  ploptions->external_deps = poptions->pluto_external_candl;
  ploptions->candl_simplify_deps = poptions->candl_deps_isl_simplify;
  ploptions->candl_deps_prune_transcover =
    poptions->candl_deps_prune_transcover;
  //ploptions->debug = poptions->verbose;
  /* Set RAR on cost function only option. */
  ploptions->rar_cf = poptions->pluto_rar_cf;
  ploptions->names_are_strings = poptions->names_are_strings;

  ploptions->tiling_in_scattering = poptions->pluto_tiling_in_scatt;
  ploptions->bound_coefficients = poptions->pluto_bound_coefficients;
  ploptions->no_skewing = poptions->pluto_no_skewing;

  // If set to 1, will produce .pluto.aster file.
  ploptions->aster_output = poptions->aster_output;

  pocc_options_init_cloog (poptions);
  puoptions->cloog_options = (void*) poptions->cloog_options;

  // Ensure Candl has been run, and scop contains dependence
  // information, if pluto-ext-candl option is set.
  if (poptions->pluto_external_candl)
    {
      char* candldeps =
	scoplib_scop_tag_content (program, "<dependence-polyhedra>",
				  "</dependence-polyhedra>");
      if (candldeps == NULL)
	{
	  // Dependence computation with candl was not done.
	  pocc_driver_candl (program, poptions, puoptions);
	}
      else
	free (candldeps);
    }
  // Parameterize the scop, and insert new params mNi = -Ni.
  if (! program->optiontags)
    {
      if (poptions->scop_parameterize_constants)
	{
	  scoplib_scop_p newscop =
	    scoplib_scop_parameterize_constants_in_domains
	    (program, poptions->scop_preprocess_parameterize_slack);
	  scoplib_scop_free (program);
	  program = newscop;
	}
      scoplib_scop_p newscop = scoplib_scop_insert_negative_params (program);
      // In-place copy, as 'program' ptr cannot change.
      scoplib_matrix_free (program->context);
      XFREE(program->parameters);
      scoplib_statement_free (program->statement);
      program->context = newscop->context;
      
      program->nb_parameters = newscop->nb_parameters;
      program->parameters = newscop->parameters;
      program->statement = newscop->statement;
      XFREE(newscop->arrays);
    }

  if (pluto_pocc (program, ploptions, puoptions) == EXIT_FAILURE)
    return EXIT_FAILURE;
  poptions->cloog_options = puoptions->cloog_options;

  if (poptions->output_scoplib_file_name)
    {
      scoplib_scop_p tempscop = scoplib_scop_dup (program);
      if (poptions->cloogify_schedules)
	pocc_cloogify_scop (tempscop);
      FILE* scopf = fopen (poptions->output_scoplib_file_name, "w");
      if (scopf)
	{
	  scoplib_scop_print_dot_scop (scopf, tempscop);
	  fclose (scopf);
	}
      scoplib_scop_free (tempscop);
    }

  if (poptions->aster_output)
    {
      char* cmd = (char*) malloc (sizeof(char) * 8192);
      strcpy (cmd, "cp .pluto.aster ");
      strcat (cmd, poptions->output_file_name);
      cmd[strlen (cmd) - 2] = '\0';
      strcat (cmd, ".aster");
      system (cmd);

      strcpy (cmd, "mv -f .pluto.aster .pocc.aster");
      system (cmd);

      free (cmd);
    }


  return EXIT_SUCCESS;
}
