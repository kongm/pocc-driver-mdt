/*
 * driver-clan.c: this file is part of the PoCC project.
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

#include <pocc/driver-clan.h>
#include <clan/options.h>
#include <clan/clan.h>
#include <osl/scop.h>
#include <irconverter/osl2scoplib.h>


/**
 * Entry point.
 *
 *
 */
scoplib_scop_p
pocc_driver_clan (FILE* program,
		  s_pocc_options_t* poptions,
		  s_pocc_utils_options_t* puoptions)
{
  if (poptions->read_input_scop_file)
    {
      if (! poptions->quiet)
	printf ("[PoCC] Reading input scop\n");

      return scoplib_scop_read (poptions->input_file);
    }

  if (! poptions->quiet)
    printf ("[PoCC] Running Clan\n");
  clan_options_p coptions = clan_options_malloc ();
  coptions->bounded_context = poptions->clan_bounded_context;
  coptions->noloopcontext = 1;
  osl_scop_p s = clan_scop_extract (program, coptions);

  scoplib_scop_p scop = irconverter_osl2scoplib (s, 1);
  osl_scop_free (s);
  /* For clan 0.6.2 and before: */
  /* scoplib_scop_p scop = clan_scop_extract (program, coptions); */

  /* Also deal with the context information. */
  if (poptions->set_default_parameter_values)
    {
      int nb_cols = scop->context->NbColumns;
      scoplib_matrix_free (scop->context);
      scop->context = scoplib_matrix_malloc (nb_cols - 2, nb_cols);
      int i;
      for (i = 0; i < nb_cols - 2; ++i)
	{
	  SCOPVAL_set_si(scop->context->p[i][0], 1);
	  SCOPVAL_set_si(scop->context->p[i][i + 1], 1);
	  SCOPVAL_set_si(scop->context->p[i][nb_cols - 1], -32);
	}
    }

/*   scoplib_scop_print (stdout, scop); */

  return scop;
}
