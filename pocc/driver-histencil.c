/*
 * driver-histencil.c: this file is part of the PoCC project.
 *
 * PoCC, the Polyhedral Compiler Collection package
 *
 * Copyright (C) 2014 Louis-Noel Pouchet
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

#include <pocc/driver-pastops.h>
#include <past/past_api.h>
#include <past/pprint.h>
#include <irconverter/past2scop.h>
#include <candl/candl.h>
#include <past/symbols.h>
#include <pvectorizer/vectorize.h>
#include <punroller/punroll.h>
#include <ptile/pocc_driver.h>
#include <pocc/driver-psimdkzer.h>
#include <pocc/driver-ptile.h>
#ifdef POCC_DEVEL_MODE
# include <pocc/driver-cdscgr.h>
#endif

#ifdef POCC_HOST_MODE
#include <parr/options.h>
#include <parr/parr.h>
#include <rotreg/options.h>
#include <rotreg/rotreg.h>
#include <sicgen/sicgen.h>
#include <sicgen/options.h>
#endif

#include <past/past.h>
#include <past/past_api.h>
#include <past/pprint.h>
#include <irconverter/pastScopRecognizer.h>
#include <irconverter/pastScopConverter.h>

s_parr_options_t * read_histencil_options (scoplib_scop_p scop,
					   int * simd_isa,
					   int * isfloat,
					   int * veclen,
					   s_pocc_options_t* poptions)
{
  s_parr_options_t * parroptions;
  FILE * f = NULL;
  if (poptions->histencil_optfilename)
    f = fopen (poptions->histencil_optfilename,"r");
  else
    f = fopen ("parr.opts","r");
  if (!f)
    {
  fprintf (stderr,"[PoCC] Did not find parr.opts file. ");
  fprintf (stderr,"Please create one with permutation vector, unrolling factors and retiming vectors\n");
  exit (1);
    }

  int n_iters;
  int n_stmts;
  int strip_dim;
  int vlen;
  int bits;
  char isa[10];
  char precision[10];
  if (fscanf (f,"%s %s %d %d %d",isa,precision,&n_iters,&n_stmts,&strip_dim) != 5)
    {
  fprintf (stderr,"[PoCC] Error. Expected <sse/avx1/avx2> <single/double> <nbr. dims> <nbr. stmts> <strip dimension> <veclen>");
  fprintf (stderr,"In first line of parr.opts\n");
  exit(1);
    }

  int width;
  if (strcmp (isa,"sse") == 0)
  {
    *simd_isa = SIMD_SSE;
    width = 128;
  }
  else if  (strcmp (isa,"avx1") == 0)
  {
    *simd_isa = SIMD_AVX1;
    width = 256;
  }
  else if  (strcmp (isa,"avx2") == 0)
  {
    *simd_isa = SIMD_AVX2;
    width = 256;
  }
  else
  {
    fprintf (stderr,"HiStencil options: unrecognized SIMD-ISA.\n");
    fprintf (stderr,"Must be sse, avx1 or avx2.\n");
    exit (1);
  }

  if (strcmp (precision,"single") == 0)
  {
    *isfloat = 1;
    bits = 32;
  }
  else if (strcmp (precision,"double") == 0)
  {
    *isfloat = 0;
    bits = 64;
  }
  else
  {
    fprintf (stderr,"HiStencil options: unrecognized floating point precision.\n");
    fprintf (stderr,"Must be single or double.\n");
    exit (1);
  }

  *veclen = width / bits;

  // validate a few thing before continuing
  int i;
  scoplib_statement_p stmt;

  for (i=0, stmt = scop->statement; stmt; stmt = stmt->next, i++)
  {
    if (n_iters != stmt->nb_iterators)
    {
      fprintf (stderr,"[PoCC] Number of iterators do not match for statement %d\n",i);
      fprintf (stderr,"Please check\n");
      exit (1);
    }
  }

/*
  if (i != n_stmts)
    {
  fprintf (stderr,"[PoCC] Number of statements found in scop and parr.opts ");
  fprintf (stderr,"do not match. Please check\n");
  exit (1);
    }
*/

  parroptions = parr_options_malloc (n_iters,n_stmts);

  parroptions->strip_dim = strip_dim;
  parroptions->vlen = *veclen;

  for (i=0; i<n_iters; ++i)
    fscanf (f,"%d",&parroptions->permvec[i]);

  for (i=0; i<n_iters; ++i)
    fscanf (f,"%d",&parroptions->unrollvec[i]);

  for (i=0; i<n_stmts; ++i)
    {
  int j;
  for (j=0; j<n_iters; ++j)
  fscanf (f,"%d",&parroptions->retvecs[i][j]);
    }

  fclose (f);

  return parroptions;
}


void dump_tree (s_past_node_t * tree, char * fname)
{
  FILE * f;
  f = fopen (fname,"w");
  past_pprint (f,tree);
  fclose (f);
}

/**
 * In HOST mode (Hi-order stencil), only this driver is being called.
 *
 */
void
pocc_driver_histencil (scoplib_scop_p program,
		       s_past_node_t* root,
		       s_pocc_options_t* poptions,
		       s_pocc_utils_options_t* puoptions)
{
  s_past_node_t* output_tree = past_clone (root);

  // (1) Ensure the tree fits the restrictions (3 address form,
  // convolution stencil, etc.).
  /// FIXME: do it.

  // (2) Implement parametric strip-mining on the outer-most loop.
  s_past_node_t** outer_loops = past_outer_loops (output_tree);
  if (! outer_loops || outer_loops[0] == NULL || outer_loops[1] != NULL)
    {
      fprintf (stderr, "[PoCC][ERROR] Only one outer-most loop supported\n");
      exit (1);
    }
  s_past_node_t* lb = NULL;
  s_past_node_t* ub = NULL;
  cs_past_node_type_t* comp_op = NULL;
  past_pprint (stdout, outer_loops[0]);
  if (! past_get_canonical_for_loop_elts (outer_loops[0], &lb, &ub, &comp_op))
    {
      fprintf (stderr,
	       "[PoCC][ERROR] Only canonical outer loops can be handled\n");
      exit (1);
    }
  PAST_DECLARE_TYPED(for, pfin, outer_loops[0]);
  s_past_node_t* ubin;
  if (comp_op == past_leq)
    ubin = past_node_binary_create (past_add,
				    past_clone (ub),
				    past_node_value_create_from_int (1));
  else
    ubin = past_clone (ub);

  char buff[128];
  sprintf (buff, "__tilesize");
  s_symbol_t* tilesize =
    symbol_add_from_char (pfin->iterator->sym_table, buff);
  sprintf (buff, "st_%s", pfin->iterator->name_str);
  s_symbol_t* newiter =
    symbol_add_from_char (pfin->iterator->sym_table, buff);
  s_past_node_t* newoutfor =
  // parfor (i_i = lbi; i_i op ubi; i_i += T)
    past_node_parfor_create
    (past_node_binary_create
     (past_assign,
      past_node_varref_create (newiter),
      past_clone (lb)),
     past_node_binary_create (comp_op,
			      past_node_varref_create (newiter),
			      past_clone (ub)),
     newiter,
     past_node_binary_create
     (past_addassign,
      past_node_varref_create (newiter),
      past_node_varref_create (tilesize)),
  // for (i = i_i; i < min(i_i + T, ubin); ++i)
    past_node_for_create
    (past_node_binary_create
     (past_assign,
      past_node_varref_create (pfin->iterator),
      past_node_varref_create (newiter)),
     past_node_binary_create
     (past_lt,
      past_node_varref_create (pfin->iterator),
      past_node_binary_create
      (past_min,
       past_node_binary_create
       (past_add,
	past_node_varref_create (newiter),
	past_node_varref_create (tilesize)),
       ubin)),
     pfin->iterator,
     past_node_unary_create
     (past_inc_after, past_node_varref_create (pfin->iterator)),
     pfin->body));

  past_replace_node (outer_loops[0], newoutfor);

  // (3) Extract the scop corresponding to the stripmined loop.
  scoplib_scop_p scop = NULL;
  s_past_node_t** nodes =
    pastScopRecognizer (newoutfor, poptions->verbose);
  if (!nodes || ! nodes[0] || nodes[1] != NULL)
    {
      pocc_error ("[PoCC] Cannot detect a single SCoP at this stage");
      poptions->output_past_tree = output_tree;
    }
  scop = pastScopConverter (nodes[0]);
  if (! scop || scop->statement == NULL)
    {
      pocc_error ("[PoCC] Cannot detect a single SCoP at this stage");
      poptions->output_past_tree = output_tree;
    }

  // (4) Process it for high-order stencil optimization

  // PaRR?
  // parr(scop)
  s_parr_options_t * parropts;
  int isa;
  int isfloat;
  int veclen;
  parropts = read_histencil_options (scop, &isa, &isfloat, &veclen, poptions);
  parr_pocc_driver (scop, nodes[0], parropts);
  pocc_driver_pastops_rebuild_statement_bodies (scop, output_tree);
  dump_tree (output_tree,"retiming.c");

  // Rotating register transformation
  s_rotreg_options_t * regopts = rotreg_options_malloc ();
  regopts->veclen = veclen;
  regopts->isfloat = isfloat;
  if (parropts->strip_dim <= past_max_loop_depth (output_tree) - 2) 
    regopts->dimension = parropts->permvec[parropts->strip_dim];
  else
    regopts->dimension = parropts->permvec [past_max_loop_depth (output_tree) - 2];
  printf ("Vector length: %d\n", regopts->veclen);
  printf ("Is float     : %s\n", regopts->isfloat ? "yes" : "no");
  printf ("Dimension    : %d\n", regopts->dimension);
  rotreg_driver (output_tree, scop, regopts);
  dump_tree (output_tree,"reg.c");

  // Call SICGen for SIMD Intrinsic code generation
  s_sicgen_options_t * sicopts = sicgen_options_malloc ();
  sicopts->veclen = veclen;
  sicopts->isfloat = isfloat;
  sicopts->data_width = isfloat ? 32 : 64;
  sicopts->isa = isa;
  sicopts->codegen_mode = MODE_HISTENCIL;
  sicgen_driver (output_tree, scop, sicopts);
  dump_tree (output_tree,"simd.c");   

  /* // Debug printing: */
  /* past_pprint (stdout, output_tree); */

  // Store the output tree.
  poptions->output_past_tree = output_tree;
}
