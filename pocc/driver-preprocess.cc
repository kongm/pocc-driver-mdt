/*
 * driver-ddganalyze.c: this file is part of the PoCC project.
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

#include <math.h>

#include <pocc/driver-preprocess.h>
#include <candl/ddv.h>
#include <candl/candl.h>
#include <candl/dependence.h>
#include <candl/program.h>
#include <pocc/driver-pluto.h>
#include <pocc/driver-codegen.h>
#include <pocc/driver-ponos.h>
#include <pocc/driver-cloog.h>
#include <pocc/error.h>
#include <past/past.h>
#include <past/pprint.h>
#include <past/past_api.h>
#include <irconverter/past2scop.h>

#include <letsee/graph.h>

/// Grrrrr
#ifdef Q
# undef Q
#endif
#include <isl/map.h>
#include <isl/set.h>
#include <polyfeat/barvinokinterface.h>


/**
 * Parameterize a SCoP. 
 *
 *
 */
void
pocc_preprocess_parameterize_scop(scoplib_scop_p scop, int slack)
{
  if (! scop)
    return;
  // 1. Compute the number of parameters needed.
  int i, j;
  int num_stmt = scoplib_statement_number (scop->statement);

  candl_program_p candl_program;
  int* indices[num_stmt];
  int unique_loop_index = 1;
  scoplib_statement_p curr_elt = scop->statement;
  for (i = 0; i < num_stmt; ++i)
    {
      indices[i] = XMALLOC(int, curr_elt->nb_iterators);
      for (j = 0; j < curr_elt->nb_iterators; ++j)
	indices[i][j] = unique_loop_index++;
      curr_elt = curr_elt->next;
    }
  candl_program = candl_program_convert_scop (scop, (int**)indices);
  for (i = 0; i < num_stmt; ++i)
    free (indices[i]);
  CandlDependence* candl_deps =
    candl_dependence_read_from_scop (scop, candl_program);
  int num_deps;
  CandlDependence* dep;
  for (dep = candl_deps, num_deps = 0; dep; dep = dep->next, num_deps++)
    ;

  int max_param_count = 4*num_stmt + 4*num_deps;
  int scalar_offset = slack;
  int parameters[max_param_count];
  int pos = 0;
  scoplib_statement_p stm;
  for (stm = scop->statement; stm; stm = stm->next)
    {
      scoplib_matrix_p d = stm->domain->elt;
      for (i = 0; i < d->NbRows; ++i)
	{
	  int val = SCOPVAL_get_si(d->p[i][d->NbColumns - 1]);
	  for (j = 0; j < pos; ++j)
	    if (parameters[j] - scalar_offset <= val &&
		val <= parameters[j] + scalar_offset)
	      break;
	  if (j == pos)
	    {
	      parameters[pos++] = val;
	      if (pos > max_param_count)
		{
		  fprintf (stderr, "Error: driver-ddganalyze.c:parameterize_scop: too few entries in the array to store scalar values for parameterization\n");
		  exit (1);
		}
	    }
	}
    }
  for (dep = candl_deps; dep; dep = dep->next)
    {
      CandlMatrix* d = dep->domain;
      for (i = 0; i < d->NbRows; ++i)
	{
	  int val = CANDL_get_si(d->p[i][d->NbColumns - 1]);
	  for (j = 0; j < pos; ++j)
	    if (parameters[j] - scalar_offset <= val &&
		val <= parameters[j] + scalar_offset)
	      break;
	  if (j == pos)
	    {
	      parameters[pos++] = val;
	      if (pos > max_param_count)
		{
		  fprintf (stderr, "Error: driver-ddganalyze.c:parameterize_scop: too few entries in the array to store scalar values for parameterization\n");
		  exit (1);
		}
	    }
	}
    }
  printf ("number of statements: %d, number of dependences: %d, number of parameters needed: %d\n",
	  num_stmt, num_deps, pos);
  int num_params = pos;
  /* for (i = 0; i < num_params; ++i) */
  /*   printf ("%d ", parameters[i]); */
  /* printf ("\n"); */
  /* exit (42); */

  // 2. Insert parameters everywhere.
  // 2.a: statements.
  for (stm = scop->statement; stm; stm = stm->next)
    {
      scoplib_matrix_p d = stm->domain->elt;
      scoplib_matrix_p newd = scoplib_matrix_malloc (d->NbRows, d->NbColumns + num_params);
      int offset = stm->nb_iterators + 1;
      for (i = 0; i < d->NbRows; ++i)
	{
	  for (j = 0; j < d->NbColumns - 1; ++j)
	    SCOPVAL_set_si(newd->p[i][j], SCOPVAL_get_si(d->p[i][j]));
	  int val = SCOPVAL_get_si(d->p[i][j]);
	  if (val >= -scalar_offset && val <= scalar_offset)
	    {
	      SCOPVAL_set_si(newd->p[i][newd->NbColumns - 1], val);
	      continue;
	    }
	  for (j = 0; j < num_params; ++j)
	  if (parameters[j] - scalar_offset <= val &&
	      val <= parameters[j] + scalar_offset)
	    break;
	  if (val < 0)
	    SCOPVAL_set_si(newd->p[i][offset + j], -1);
	  else
	    SCOPVAL_set_si(newd->p[i][offset + j], 1);
	  SCOPVAL_set_si(newd->p[i][newd->NbColumns - 1], val - parameters[j]);
	}
      scoplib_matrix_free (d);
      stm->domain->elt = newd;
    }

  // 2.b: dependences.
  for (dep = candl_deps; dep; dep = dep->next)
    {
      CandlMatrix* d = dep->domain;
      CandlMatrix* newd =
	candl_matrix_malloc (d->NbRows, d->NbColumns + num_params);
      int offset = dep->source->depth + dep->target->depth + 1;
      for (i = 0; i < d->NbRows; ++i)
	{
	  for (j = 0; j < d->NbColumns - 1; ++j)
	    CANDL_set_si(newd->p[i][j], CANDL_get_si(d->p[i][j]));
	  int val = CANDL_get_si(d->p[i][j]);
	  if (val >= -scalar_offset && val <= scalar_offset)
	    {
	      CANDL_set_si(newd->p[i][newd->NbColumns - 1], val);
	      continue;
	    }
	  for (j = 0; j < num_params; ++j)
	  if (parameters[j] - scalar_offset <= val &&
	      val <= parameters[j] + scalar_offset)
	    break;
	  if (val < 0)
	    CANDL_set_si(newd->p[i][offset + j], -1);
	  else
	    CANDL_set_si(newd->p[i][offset + j], 1);
	  CANDL_set_si(newd->p[i][newd->NbColumns - 1], val - parameters[j]);
	}
      candl_matrix_free (dep->domain);
      dep->domain = newd;
    }
  // Embed the updated dependences in the scop.
  candl_dependence_update_scop_with_deps (scop, candl_deps);
  candl_dependence_free (candl_deps);
  candl_program_free (candl_program);

  // 2.c: in the scop.
  scop->context = scoplib_matrix_malloc (num_params, num_params + 2);
  for (i = 0; i < num_params; ++i)
    {
      SCOPVAL_set_si(scop->context->p[i][0], 1);
      SCOPVAL_set_si(scop->context->p[i][i+1], 1);
      SCOPVAL_set_si(scop->context->p[i][num_params + 1], -parameters[i]);
    }
  scop->nb_parameters = num_params;
  scop->parameters = XMALLOC(char*, num_params + 1);
  for (i = 0; i < num_params; ++i)
    {
      char buffer[32];
      sprintf (buffer, "N%d", i);
      scop->parameters[i] = strdup (buffer);
    }
}

 
int
pocc_driver_preprocess (scoplib_scop_p program,
			s_pocc_options_t* poptions,
			s_pocc_utils_options_t* puoptions)
{
  if (! poptions->quiet)
    printf ("[PoCC] Running SCoP pre-processor\n");
  
  // 2. Parameterize it.
  if (poptions->scop_preprocess_parameterize_slack > 1)
    pocc_preprocess_parameterize_scop
      (program, poptions->scop_preprocess_parameterize_slack);
  scoplib_scop_print (stdout, program);
  exit (1);
  /* // Pretty-print the scop, for debugging. */
  /* if (debug) */
  /*   scoplib_scop_print_dot_scop (stdout, scop); */

  /* // Pretty-print the dependences, for debugging. */
  /* if (debug) */
  /*   { */
  /*     char* strdeps = scoplib_scop_tag_content(scop, "<dependence-polyhedra>", */
  /* 					       "</dependence-polyhedra>"); */
  /*     printf ("DEPS: \n%s\n", strdeps); */
  /*     free (strdeps); */
  /*   } */

  return EXIT_SUCCESS;
}
