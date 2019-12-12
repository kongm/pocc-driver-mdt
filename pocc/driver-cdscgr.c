/*
 * driver-cdscgr.c: this file is part of the PoCC project.
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

#include <pocc/driver-cdscgr.h>
#include <candl/ddv.h>
#include <pocc/driver-pluto.h>
#include <pocc/driver-codegen.h>
#include <pocc/error.h>
#include <past/past.h>
#include <past/pprint.h>
#include <past/past_api.h>
#include <irconverter/past2scop.h>


/**
 * Visitor to collect the sub-trees to become 'macro-statements'.
 *
 */
static
void traverse_collect_forloop_to_delete (s_past_node_t* node, void* data)
{
  // current node is a for?
  if (past_node_is_a (node, past_for))
    {
      // retrieve the arguments from the void* 'data' pointer.
      char* point_loop_iter = ((void**)(data))[0];
      s_past_node_t** to_delete = ((void**)(data))[1];
      // cast the node 'node' into a past_for_t* named pf.
      PAST_DECLARE_TYPED(for, pf, node);
      // Is the loop iterator name equal to 'point_loop_iter'?
      if (! strcmp (pf->iterator->data, point_loop_iter))
	{
	  // Yes, store its body in the list of nodes to delete.
	  int count;
	  for (count = 0; to_delete[count]; ++count)
	    ;
	  to_delete[count] = pf->body;
	}
    }
}


/**
 * Emit the new CDSC-GR text file.
 *
 * @param transformed_scop: created from a shallow copy of orig_scop,
 *			    it must not be modified.
 *
 */
static
void
create_new_cdscgr_graph (scoplib_scop_p transformed_scop,
			 scoplib_scop_p orig_scop,
			 s_past_node_t* ast,
			 CandlDependence* cdeps)
{
  // Loop depth (for tile loops) to be modeled at the CDSC-GR level.
  /// FIXME: there is no reason to use a single value for all statements.
  int num_tile_dim_cdscgr = 2;
  int debug = 1;
  int i, j, k;

  // Pretty-print, for debugging.
  if (debug)
    scoplib_scop_print_dot_scop (stdout, transformed_scop);


  // Collect all sets of statements surrounded by an identical
  // inner-most tile loop, to create the subset of dependences
  // associated to them, and emit the corresponding CDSC-GR graph (eg,
  // [A:c1,c2] -> newStmt1(c1,c2) -> [B:c1, c2+42]).
  scoplib_statement_p s;
  scoplib_statement_p s2;
  // cannot have more sets than the number of statements.
  int num_stmts = 0;
  int num_component = 0;
  for (s = transformed_scop->statement; s; s = s->next)
    ++num_stmts;
  int stmt_processed[num_stmts];
  scoplib_statement_p component[num_stmts + 1];
  for (i = 0; i < num_stmts; ++i)
    stmt_processed[i] = 0;
  for (s = transformed_scop->statement, i = 0; s; s = s->next, ++i)
    {
      // Reset the component.
      for (j = 0; j <= num_stmts; ++j)
	component[j] = NULL;
      if (stmt_processed[i] == 1)
	continue;
      // Inspect the statements to build a component.
      for (s2 = transformed_scop->statement, j = 0; s2; s2 = s2->next, ++j)
	{
	  if (stmt_processed[j] == 1)
	    continue;

	  if (component[0] == NULL)
	    {
	      component[0] = s2;
	      stmt_processed[j] = 1;
	    }
	  else
	    {
	      // Check if s2 is surrounded by the same tile loops as
	      // the current component.
	      for (k = 0; k < 2 * num_tile_dim_cdscgr; ++k)
		{
		  int c1 = SCOPVAL_get_si(component[0]->schedule->p[k][component[0]->schedule->NbColumns - 1]);
		  int c2 = SCOPVAL_get_si(s2->schedule->p[k][s2->schedule->NbColumns - 1]);
		  if (c1 != c2)
		    break;
		}
	      if (k == 2 * num_tile_dim_cdscgr)
		{
		  // It is. Add it to the current component.
		  for (k = 0; component[k]; ++k)
		    ;
		  component[k] = s2;
		  stmt_processed[j] = 1;
		}
	    }
	}
      // Process the component.
      if (debug)
	{
	  printf ("== Component #%d\n", num_component);
	  for (k = 0; component[k]; ++k)
	    {
	      scoplib_statement_p next = component[k]->next;
	      component[k]->next = NULL;
	      scoplib_statement_print (stdout, component[k]);
	      component[k]->next = next;
	    }

	}
      // a. Collect the dependences between the statements in the
      // components, from cdeps and component[].

      // b. Prune the dependence polyhedra, to keep only the useful ones
      /// FIXME: not sure yet the best way to do that.

      // c. Collect the arrays read/written by the component (will
      // become item collections).

      // d. Look at the result of b. and c. to emit the CDSC-GR
      // graph. We will need regions in the general case.



      ++num_component;
    }
}






/**
 * Automatically called after CLooG code generation. Enables the
 * handling of the AST produced by CLooG. In-place modification of
 * 'root' expected.
 *
 */
void
pocc_driver_cdscgr_pastops (s_past_node_t* root,
			    scoplib_scop_p program,
			    s_pocc_options_t* poptions,
			    s_pocc_utils_options_t* puoptions)
{
  int i;
  int debug = 1;
  printf ("[PoCC] CDSC-GR: AST post-processing\n");

  // Pretty-print, for debugging.
  if (debug)
    past_pprint (stdout, root);

  // 1. Compute polyhedral dependences on the transformed code.
  // 1.a Get the scop representation of the tree.
  scoplib_scop_p scop =
    past2scop_control_only (root, program, 1);
  // delete the option tags (out-of-date from shallow copy of 'program').
  scop->optiontags = NULL;
  // 1.b Compute dependence polyhedra with Candl.
  CandlOptions* coptions = candl_options_malloc ();
  coptions->scalar_privatization = poptions->pluto_scalpriv;
  CandlProgram* cprogram = candl_program_convert_scop (scop, NULL);
  CandlDependence* cdeps = candl_dependence (cprogram, coptions);

  // 1.c Use the dependence polyhedra to generate the new CDSC-GR graph.
  create_new_cdscgr_graph (scop, program, root, cdeps);

  // 1.d Cleaning.
  candl_dependence_pprint (stdout, cdeps);
  candl_dependence_free (cdeps);
  candl_program_free (cprogram);
  candl_options_free (coptions);
  scoplib_scop_shallow_free (scop);

  // 2. Prefix traversal to collect the subtrees to be replaced by
  // macro statements.
  // 2.a setup the data structures needed for the visitor.
  int num_for_loops = past_count_for_loops (root);
  // NULL-terminated array storing the subtrees to be deleted.
  s_past_node_t** to_delete = XMALLOC(s_past_node_t*, num_for_loops + 1);
  for (i = 0; i <= num_for_loops; ++i)
    to_delete[i] = NULL;
  // Leave loops with iterator c2 (and surrounding loops) in the tree.
  /// FIXME: pass the number of loops to store at the CDSC-GR level as
  /// argument/option instead of hardwiring 2 loops (c1 and c2).
  char* outer_iter_name = strdup ("c2");
  void* data[2]; data[0] = outer_iter_name; data[1] = to_delete;
  // 2.b prefix visit.
  // See ir/past/past/include/past/past.h for the visitor API.
  past_visitor (root, traverse_collect_forloop_to_delete, (void*)data,
		NULL, NULL);

  // 2.c Replace sub-trees to be replaced by a new macro-statement,
  // and emit the 'env' for the associated new step collection.
  //
  /// FIXME: the building of 'env' is not robust to conditionals in
  /// the output code that encapsulate tile loops (possibly incorrect
  /// code generated, as conditionals will be ignored). Conditionals
  /// inside the point loops are supported. Fixing this limitation
  /// requires to use min/max functions in the step collection range,
  /// or to move to regions.
  char buff[512];
  for (i = 0; to_delete[i]; ++i)
    {
      // Create a new node newStmt42(i, j, ...) where i, j, ... are
      // the surrounding loop iterators.
      sprintf (buff, "newStmt%d", i);
      // All IDs in a PAST tree contain one s_symbol_t structure,
      // storing the actual data (e.g., "i") used to pretty-print the
      // symbol. Optionally, a symbol table can be used for management
      // (pass NULL as first argument otherwise).
      s_symbol_t* stmsym = symbol_add_from_char (NULL, buff);
      // Linked list of nodes (siblings), to be used as argument to
      // the function 'newStmt'.
      s_past_node_t* surrounding_iter_list = NULL;
      s_past_node_t** curr = &surrounding_iter_list;
      s_past_node_t* tmp = to_delete[i]->parent;
      int num_surrounding_loops = 0;
      // Create a linked list of the surrounding loop iterators. It
      // will become the arguments to the function call newStmt(...
      while (tmp)
	{
	  if (past_node_is_a(tmp, past_for))
	    {
	      PAST_DECLARE_TYPED(for, pf, tmp);
	      s_past_node_t* iter = past_clone ((s_past_node_t*)pf->iterator);
	      *curr = iter;
	      curr = &(iter->next);
	      ++num_surrounding_loops;
	    }
	  tmp = tmp->parent;
	}
      // Create the actual new sub-tree for the macro-statement.
      // See ir/past/past/include/past/past.h for PAST node class.
      // See ir/past/past/include/past/past_api.h for PAST additional API.
      s_past_node_t* new_stmt =
	past_node_statement_create
	(past_node_funcall_create
	 (past_node_varref_create (stmsym), surrounding_iter_list));
      // Replace the original subtree by the function call.
      past_replace_node (to_delete[i], new_stmt);

      // Print the macros on stderr.
      /// FIXME: missing the \ to continue to next line.
      /// I advise printing to a file instead of stderr, and process
      /// this file with a sed script after it was generated.
      fprintf (stderr, "#define %s(", buff);
      for (tmp = surrounding_iter_list; tmp; tmp = tmp->next)
	{
	  fprintf (stderr, "%s", ((s_past_varref_t*)tmp)->symbol->data);
	  if (tmp->next)
	    fprintf (stderr, ", ");
	}
      fprintf (stderr, ")\n");
      past_pprint (stderr, to_delete[i]);

      // Emit the 'env' for the new step collection. Be dumb: simply
      // print the loop bounds for the surrounding loops in CDSC-GR
      // syntax.
      fprintf (stderr, "# Step collections:\n");
      fprintf (stderr, "env -> (coll%s : {", buff);
      // Store the loop pointers in AST order.
      s_past_node_t* outer_loops[num_surrounding_loops];
      int count = num_surrounding_loops - 1;
      tmp = to_delete[i]->parent;
      while (tmp)
	{
	  if (past_node_is_a(tmp, past_for))
	    {
	      PAST_DECLARE_TYPED(for, pf, tmp);
	      outer_loops[count--] = tmp;
	    }
	  tmp = tmp->parent;
	}
      // Emit the range.
      for (count = 0; count < num_surrounding_loops; ++count)
	{
	  PAST_DECLARE_TYPED(for, pf, outer_loops[count]);
	  // pf->init is always 'i = xxx', so take the RHS of the '=' operator.
	  PAST_DECLARE_TYPED(binary, pbi, pf->init);
	  past_pprint (stderr, pbi->rhs);
	  fprintf (stderr, " .. ");
	  // pf->test is always 'i <= xxx', so take the RHS of the
	  // '<=' operator.
	  PAST_DECLARE_TYPED(binary, pbt, pf->test);
	  past_pprint (stderr, pbt->rhs);
	  fprintf (stderr, "}");
	  if (count + 1 < num_surrounding_loops)
	    fprintf (stderr, ", {");
	}
      fprintf (stderr, ");\n");

      // Delete the sub-tree.
      past_deep_free (to_delete[i]);
    }
  // 2.d Cleaning.
  XFREE(to_delete);
  XFREE(outer_iter_name);


  // Pretty-print, for debugging.
  if (debug)
    past_pprint (stdout, root);
}





/**
 * Automatically called after CLooG code generation. Enables the
 * handling of the AST produced by CLooG. In-place modification of
 * 'root' expected.
 *
 */
void
pocc_driver_cdscgr_pastops_XXX (s_past_node_t* root,
			    scoplib_scop_p program,
			    s_pocc_options_t* poptions,
			    s_pocc_utils_options_t* puoptions)
{
  int i;
  int debug = 1;
  printf ("[PoCC] CDSC-GR: AST post-processing\n");

  // Pretty-print, for debugging.
  if (debug)
    past_pprint (stdout, root);

  // 1. Compute polyhedral dependences on the transformed code.
  // 1.a Get the scop representation of the tree.
  scoplib_scop_p scop =
    past2scop_control_only (root, program, 1);
  // delete the option tags (out-of-date from shallow copy of 'program').
  scop->optiontags = NULL;
  // 1.b Compute dependence polyhedra with Candl.
  CandlOptions* coptions = candl_options_malloc ();
  coptions->scalar_privatization = poptions->pluto_scalpriv;
  CandlProgram* cprogram = candl_program_convert_scop (scop, NULL);
  CandlDependence* cdeps = candl_dependence (cprogram, coptions);

  // 1.c Use the dependence polyhedra to generate the new CDSC-GR graph.
  create_new_cdscgr_graph (scop, program, root, cdeps);

  // 1.d Cleaning.
  candl_dependence_pprint (stdout, cdeps);
  candl_dependence_free (cdeps);
  candl_program_free (cprogram);
  candl_options_free (coptions);
  scoplib_scop_shallow_free (scop);

  // 2. Prefix traversal to collect the subtrees to be replaced by
  // macro statements.
  // 2.a setup the data structures needed for the visitor.
  int num_for_loops = past_count_for_loops (root);
  // NULL-terminated array storing the subtrees to be deleted.
  s_past_node_t** to_delete = XMALLOC(s_past_node_t*, num_for_loops + 1);
  for (i = 0; i <= num_for_loops; ++i)
    to_delete[i] = NULL;
  // Leave loops with iterator c2 (and surrounding loops) in the tree.
  /// FIXME: pass the number of loops to store at the CDSC-GR level as
  /// argument/option instead of hardwiring 2 loops (c1 and c2).
  char* outer_iter_name = strdup ("c2");
  void* data[2]; data[0] = outer_iter_name; data[1] = to_delete;
  // 2.b prefix visit.
  // See ir/past/past/include/past/past.h for the visitor API.
  past_visitor (root, traverse_collect_forloop_to_delete, (void*)data,
		NULL, NULL);

  // 2.c Replace sub-trees to be replaced by a new macro-statement,
  // and emit the 'env' for the associated new step collection.
  //
  /// FIXME: the building of 'env' is not robust to conditionals in
  /// the output code that encapsulate tile loops (possibly incorrect
  /// code generated, as conditionals will be ignored). Conditionals
  /// inside the point loops are supported. Fixing this limitation
  /// requires to use min/max functions in the step collection range,
  /// or to move to regions.
  char buff[512];
  for (i = 0; to_delete[i]; ++i)
    {
      // Create a new node newStmt42(i, j, ...) where i, j, ... are
      // the surrounding loop iterators.
      sprintf (buff, "newStmt%d", i);
      // All IDs in a PAST tree contain one s_symbol_t structure,
      // storing the actual data (e.g., "i") used to pretty-print the
      // symbol. Optionally, a symbol table can be used for management
      // (pass NULL as first argument otherwise).
      s_symbol_t* stmsym = symbol_add_from_char (NULL, buff);
      // Linked list of nodes (siblings), to be used as argument to
      // the function 'newStmt'.
      s_past_node_t* surrounding_iter_list = NULL;
      s_past_node_t** curr = &surrounding_iter_list;
      s_past_node_t* tmp = to_delete[i]->parent;
      int num_surrounding_loops = 0;
      // Create a linked list of the surrounding loop iterators. It
      // will become the arguments to the function call newStmt(...
      while (tmp)
	{
	  if (past_node_is_a(tmp, past_for))
	    {
	      PAST_DECLARE_TYPED(for, pf, tmp);
	      s_past_node_t* iter = past_clone ((s_past_node_t*)pf->iterator);
	      *curr = iter;
	      curr = &(iter->next);
	      ++num_surrounding_loops;
	    }
	  tmp = tmp->parent;
	}
      // Create the actual new sub-tree for the macro-statement.
      // See ir/past/past/include/past/past.h for PAST node class.
      // See ir/past/past/include/past/past_api.h for PAST additional API.
      s_past_node_t* new_stmt =
	past_node_statement_create
	(past_node_funcall_create
	 (past_node_varref_create (stmsym), surrounding_iter_list));
      // Replace the original subtree by the function call.
      past_replace_node (to_delete[i], new_stmt);

      // Print the macros on stderr.
      /// FIXME: missing the \ to continue to next line.
      /// I advise printing to a file instead of stderr, and process
      /// this file with a sed script after it was generated.
      fprintf (stderr, "#define %s(", buff);
      for (tmp = surrounding_iter_list; tmp; tmp = tmp->next)
	{
	  fprintf (stderr, "%s", ((s_past_varref_t*)tmp)->symbol->data);
	  if (tmp->next)
	    fprintf (stderr, ", ");
	}
      fprintf (stderr, ")\n");
      past_pprint (stderr, to_delete[i]);

      // Emit the 'env' for the new step collection. Be dumb: simply
      // print the loop bounds for the surrounding loops in CDSC-GR
      // syntax.
      fprintf (stderr, "# Step collections:\n");
      fprintf (stderr, "env -> (coll%s : {", buff);
      // Store the loop pointers in AST order.
      s_past_node_t* outer_loops[num_surrounding_loops];
      int count = num_surrounding_loops - 1;
      tmp = to_delete[i]->parent;
      while (tmp)
	{
	  if (past_node_is_a(tmp, past_for))
	    {
	      PAST_DECLARE_TYPED(for, pf, tmp);
	      outer_loops[count--] = tmp;
	    }
	  tmp = tmp->parent;
	}
      // Emit the range.
      for (count = 0; count < num_surrounding_loops; ++count)
	{
	  PAST_DECLARE_TYPED(for, pf, outer_loops[count]);
	  // pf->init is always 'i = xxx', so take the RHS of the '=' operator.
	  PAST_DECLARE_TYPED(binary, pbi, pf->init);
	  past_pprint (stderr, pbi->rhs);
	  fprintf (stderr, " .. ");
	  // pf->test is always 'i <= xxx', so take the RHS of the
	  // '<=' operator.
	  PAST_DECLARE_TYPED(binary, pbt, pf->test);
	  past_pprint (stderr, pbt->rhs);
	  fprintf (stderr, "}");
	  if (count + 1 < num_surrounding_loops)
	    fprintf (stderr, ", {");
	}
      fprintf (stderr, ");\n");

      // Delete the sub-tree.
      past_deep_free (to_delete[i]);
    }
  // 2.d Cleaning.
  XFREE(to_delete);
  XFREE(outer_iter_name);


  // Pretty-print, for debugging.
  if (debug)
    past_pprint (stdout, root);
}


/**
 * To be replaced with a real, robust implem. For the moment, simply
 * assume the input schedule is correct in the input scop, and use
 * that for standard dependence analysis.
 *
 */
static
CandlDependence*
compute_cdscgr_deps (scoplib_scop_p scop, CandlProgram* cprogram)
{
  // Be dumb, simply assume the input scop schedule is correct for the
  // moment. Requires the input scop schedule to be set to a 2d+1 form
  // for all statements.
  CandlOptions* coptions = candl_options_malloc ();

  /// FIXME: a quick look at other modules indicates that setting up
  /// correctly the 'index' field of src & target statements in a
  /// CandlDependence is not required for pluto to operate.

  /// FIXME: replace this by a real DSA-aware algo.
  CandlDependence* deps = candl_dependence (cprogram, coptions);

  // Be clean.
  candl_options_free (coptions);

  return deps;
}


/**
 * Entry point.
 *
 *
 */
void
pocc_driver_cdscgr_XXXX (FILE* input_file,
		    s_pocc_options_t* poptions,
		    s_pocc_utils_options_t* puoptions)
{
  int debug = 1;

  printf ("[PoCC] Start CDSC-GR pass\n");
  if (input_file == NULL)
    pocc_error ("Invalid input file (cannot be opened)\n");

  // 1. Read the scop.
  scoplib_scop_p scop = scoplib_scop_read (input_file);

  // Pretty-print the scop, for debugging.
  if (debug)
    scoplib_scop_print_dot_scop (stdout, scop);

  // 2. Compute the list of candl dependences.
  CandlProgram* cprogram = candl_program_convert_scop (scop, NULL);
  CandlDependence* deps = compute_cdscgr_deps (scop, cprogram);

  // 3. Embed the dependences in the scop.
  candl_dependence_update_scop_with_deps (scop, deps);

  // Pretty-print the dependences, for debugging.
  if (debug)
    {
      char* strdeps = scoplib_scop_tag_content(scop, "<dependence-polyhedra>",
					       "</dependence-polyhedra>");
      printf ("DEPS: \n%s\n", strdeps);
      free (strdeps);
    }

  // Pretty-print, for debugging.
  if (debug)
    scoplib_scop_print_dot_scop (stdout, scop);

  // 4. Call pluto, instructing to use dependences embedded in the
  // scop file.
  poptions->pluto_external_candl = 1;
  pocc_driver_pluto (scop, poptions, puoptions);

  //int nb_dim_tiled = some_func (scop);
  int nb_dim_tiled = 2;
  

  // Pretty-print, for debugging.
  if (debug)
    scoplib_scop_print_dot_scop (stdout, scop);

  // 5. Generate PAST tree using CLooG. It will call
  // 'pocc_driver_cdscgr_pastops' with the AST generated after CLooG.
  poptions->cloog_f = 1;
  poptions->cloog_l = 3;
  pocc_driver_codegen (scop, poptions, puoptions);

  // Be clean.
  scoplib_scop_free (scop);
  candl_program_free (cprogram);

  printf ("[PoCC] CDSC-GR pass: done\n");
}









/**
 * Entry point.
 *
 *
 */
void
pocc_driver_cdscgr (FILE* input_file,
		    s_pocc_options_t* poptions,
		    s_pocc_utils_options_t* puoptions)
{
  int debug = 1;

  printf ("[PoCC] Start CDSC-GR pass\n");
  if (input_file == NULL)
    pocc_error ("Invalid input file (cannot be opened)\n");

  // 1. Read the scop.
  scoplib_scop_p scop = scoplib_scop_read (input_file);

  // Pretty-print the scop, for debugging.
  if (debug)
    scoplib_scop_print_dot_scop (stdout, scop);

  // 2. Compute the list of candl dependences.
  CandlProgram* cprogram = candl_program_convert_scop (scop, NULL);
  CandlDependence* deps = compute_cdscgr_deps (scop, cprogram);

  // 3. Embed the dependences in the scop.
  candl_dependence_update_scop_with_deps (scop, deps);

  // Pretty-print the dependences, for debugging.
  if (debug)
    {
      char* strdeps = scoplib_scop_tag_content(scop, "<dependence-polyhedra>",
					       "</dependence-polyhedra>");
      printf ("DEPS: \n%s\n", strdeps);
      free (strdeps);
    }

  // Pretty-print, for debugging.
  if (debug)
    scoplib_scop_print_dot_scop (stdout, scop);

  // 4. Call pluto, instructing to use dependences embedded in the
  // scop file.
  poptions->pluto_external_candl = 1;
  pocc_driver_pluto (scop, poptions, puoptions);

  //int nb_dim_tiled = some_func (scop);
  int nb_dim_tiled = 2;
  

  // Pretty-print, for debugging.
  if (debug)
    scoplib_scop_print_dot_scop (stdout, scop);

  // 5. Generate PAST tree using CLooG. It will call
  // 'pocc_driver_cdscgr_pastops' with the AST generated after CLooG.
  poptions->cloog_f = 1;
  poptions->cloog_l = 3;
  pocc_driver_codegen (scop, poptions, puoptions);

  // Be clean.
  scoplib_scop_free (scop);
  candl_program_free (cprogram);

  printf ("[PoCC] CDSC-GR pass: done\n");
}
