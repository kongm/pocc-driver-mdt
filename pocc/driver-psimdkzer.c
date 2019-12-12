/*
 * driver-psimdkzer.c: this file is part of the PSimdKzer project.
 *
 * PSimdKzer, the Polyhedral SIMD Vectorizer
 *
 * Copyright (C) 2013 Martin Kong
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

#include <pocc/driver-psimdkzer.h>
#include <pocc/driver-cloog.h>
#include <pocc/driver-clastops.h>
#include <pocc/driver-ponos.h>
#include <pocc/error.h>
#include <candl/program.h>
#include <pocc-utils/options.h>
//#ifndef CLOOG_INT_GMP
//# define CLOOG_INT_GMP
#include <cloog/cloog.h>
#include <irconverter/past2scop.h>
#include <scoptools/past2scop.h>
#include <psimdkzer/scop2past.h>
#include <psimdkzer/psimdkzer.h>
#include <psimdkzer/options.h>
#include <past/past_api.h>
#include <clasttools/clast2past.h>
#include <assert.h>
//#endif


static
void
reskew_schedule (scoplib_statement_p stmt, int skewfac)
{
  int i;
  for (i=1; i<stmt->schedule->NbRows; ++i)
    if (stmt->schedule->p[i][1]>1)
  {
    stmt->schedule->p[i][1] = skewfac;
  }
}

void
pocc_driver_psimdkzer_schedule_reskew (
  scoplib_scop_p program,
  s_pocc_options_t* poptions,
  s_pocc_utils_options_t* puoptions )
{
  // Assume time dimension is the first column in the scop
  scoplib_statement_p stmt;
  int skewfac = poptions->psimdkzer_reskew;
  for (stmt=program->statement; stmt; stmt=stmt->next)
  {
    reskew_schedule (stmt,skewfac);
  }
}

static
int get_ft_dimensionality (s_past_node_t * node)
{
  s_past_node_t ** inner = past_inner_loops (node);
  s_past_node_t * current;
  int d = 1;
  for (current = inner[0]; current != node; current = current->parent, d++);
  XFREE (inner);
  return d;
}

typedef struct {
  s_past_node_t ** ret;
  int max_dim;
} s_ft_data;


// MK: collect the full tiles of maximum dimensionality
static
void
traverse_get_fulltiles (s_past_node_t* node, void* data)
{
  if (past_node_is_a (node, past_for))
  {
    s_ft_data * ret = (s_ft_data*)(data);
    PAST_DECLARE_TYPED(for, pf, node);
    if (pf->type == e_past_fulltile_loop)
		{
      if (get_ft_dimensionality(node) == ret->max_dim)
      {
  			s_past_node_t** fts = ret->ret;
  			int k;
  			for (k = 0; fts && fts[k]; ++k);
  			fts[k] = node;
      }
		}
  }
}


static
void
traverse_count_fulltiles (s_past_node_t* node, void* data)
{
  if (past_node_is_a (node, past_for))
  {
    PAST_DECLARE_TYPED(for, pf, node);
    if (pf->type == e_past_fulltile_loop)
		{
			int * count = (int*)data;
      (*count) += 1;
		}
  }
}


static
s_past_node_t**
collect_all_full_tiles (s_past_node_t* root,int max_dim)
{
  s_past_node_t** ret;
  s_ft_data data;
  int nft = 0;
  past_visitor (root, traverse_count_fulltiles, &nft, NULL, NULL);
  if (!nft) return NULL;
  ret = XMALLOC (s_past_node_t*,nft+1);
  data.ret = ret;
  data.max_dim = max_dim;
  int i;
  for (i=0; i<nft+1; ++i)
    ret[i] = NULL;
  past_visitor (root, traverse_get_fulltiles, (void*)(&data), NULL, NULL);

  return ret;
}



static
int is_included (s_symbol_t* s, s_symbol_t** syms)
{
  int i;
  for (i = 0; syms && syms[i]; ++i)
    if (! strcmp ((char*)s->data,  (char*)syms[i]->data ) )
      return 1;
  return 0;
}

static
s_symbol_t ** collect_ft_loop_iterators ( s_past_node_t * node )
{
	s_past_node_t * curr;
	int nlevels ;

	for ( nlevels = 0, curr = node;
				! past_node_is_a ( curr, past_root );
				curr = curr->parent )
	{
		if ( past_node_is_a ( curr, past_for ) )
		{
			++nlevels;
		}
	}
	if ( ! nlevels ) return 0;
	s_symbol_t ** it = XMALLOC ( s_symbol_t *, nlevels+1 );

	int i;
	for ( i = 0, curr = node;
				! past_node_is_a ( curr, past_root );
				curr = curr->parent )
	{
		if ( past_node_is_a ( curr, past_for ) )
		{
			PAST_DECLARE_TYPED ( for, pf, curr );
			it[i] = pf->iterator;
			++i;
		}
	}

	it[i] = NULL;
	return it;

}


static int
rootHasFullTileLoop (s_past_node_t * root)
{
	int i,j;
	s_past_node_t ** outer = past_outer_loops ( root );
	s_past_node_t ** inner = past_inner_loops ( root );
	s_symbol_t ** it = collect_ft_loop_iterators ( inner[0] );

	if ( !it ) return 0;

	int dim_ok, all_dim_ok;
	for ( i=0; outer[i]; i++) ;
	for ( j=0; inner[j]; j++) ;

	// Expect only one outermost loop and one inner most loop.
	// At this point we only have scops, and some
	// may not even have loops in it. So it's not such a waste.

	//if ( i != 1 || j != 1 ) return 0;

	// Important check: guarantee that at least one loop iterator
	// is present in the access function of the fastest varying dimension.
	// Many loop nests are marked as full tiles by PTile, but could
	// consist of edge cases e.g A[1] or A[n-2] or something like that.
	// So we are not interested in those loops.
	PAST_DECLARE_TYPED ( for, pfi, inner[0] );
	s_past_node_t ** data = past_collect_nodetype ( pfi->body, past_assign );
	s_past_node_t * curr;
	for ( i=0; data[i]; i++ )
	{
		all_dim_ok = 0;
		PAST_DECLARE_TYPED ( binary, pb, data[i] );
		int k = 0;
		for ( curr = pb->lhs ; past_node_is_a ( curr, past_arrayref ); )
		{
			PAST_DECLARE_TYPED ( binary, ar, curr );
			s_symbol_t** syms =  past_collect_read_symbols ( ar->rhs );
			dim_ok = 0;
			// check if at least one loop iterator appears in the access function
			// of the current dimension (from fastest to slowest varying)
			for ( j=0; it[j]; ++j )
			{
				if ( is_included ( it[j], syms ) )
				{
					dim_ok = 1;
				}
			}
			// if dimension is ok, all_dim_ok will increment 1, otherwise
			// remains the same
			all_dim_ok += dim_ok;
			curr = ar->lhs;
			++k;

		}
		// number of dimensions ok must be equal to the dimensionality
		// of the array (k)
		if ( all_dim_ok < k ) return 0;
	}

	// Finally, we like the loop nest if it is, naturally, marked as a
	// full tile by PTile.
	PAST_DECLARE_TYPED ( for, pfo, outer[0] );

  int res;
  res = pfo->type == e_past_fulltile_loop;

	XFREE ( it );
	return  res ;
}


// MK: to be removed as the FT detection now is done through scops
static int
rootHasPartialTileLoop (s_past_node_t * root)
{

	int i,j;
	s_past_node_t ** outer = past_outer_loops ( root );
	int no;
	for ( no = 0; outer[no]; no++ );
	if ( no > 1 ) return 1;

	s_past_node_t ** inner = past_inner_loops ( outer[0] );
	s_symbol_t ** it = collect_ft_loop_iterators ( inner[1] );



	if ( !it ) return 0;

	int dim_ok, all_dim_ok;
	for ( i=0; outer[i]; i++) ;
	for ( j=0; inner[j]; j++) ;

	// Expect only one outermost loop and one inner most loop.
	// At this point we only have scops, and some
	// may not even have loops in it. So it's not such a waste.

	//if ( i != 1 || j != 1 ) return 0;

	// Important check: guarantee that at least one loop iterator
	// is present in the access function of the fastest varying dimension.
	// Many loop nests are marked as full tiles by PTile, but could
	// consist of edge cases e.g A[1] or A[n-2] or something like that.
	// So we are not interested in those loops.
	PAST_DECLARE_TYPED ( for, pfi, inner[1] );
	s_past_node_t ** data = past_collect_nodetype ( pfi->body, past_assign );
	s_past_node_t * curr;
	for ( i=0; data[i]; i++ )
	{
		all_dim_ok = 0;
		PAST_DECLARE_TYPED ( binary, pb, data[i] );
		int k = 0;
		for ( curr = pb->lhs ; past_node_is_a ( curr, past_arrayref ); )
		{
			PAST_DECLARE_TYPED ( binary, ar, curr );
			s_symbol_t** syms =  past_collect_read_symbols ( ar->rhs );
			dim_ok = 0;
			// check if at least one loop iterator appears in the access function
			// of the current dimension (from fastest to slowest varying)
			for ( j=0; it[j]; ++j )
			{
				if ( is_included ( it[j], syms ) )
				{
					dim_ok = 1;
				}
			}
			// assume FVD can have constant indices
			//if ( curr == pb->lhs ) dim_ok = 1;
			// if dimension is ok, all_dim_ok will increment 1, otherwise
			// remains the same
			all_dim_ok += dim_ok;
			curr = ar->lhs;
			++k;

		}
		// number of dimensions ok must be equal to the dimensionality
		// of the array (k)
		if ( all_dim_ok < 2 )
		{
			return 0;
		}
	}


	// Finally, we like the loop nest if it is, naturally, marked as a
	// full tile by PTile.
	PAST_DECLARE_TYPED ( for, pfo, outer[0] );

	int res;
	// if this option is set, assume that any scop found is a full-tile
	res  = pfo->type == e_past_tile_loop && pfo->type != e_past_fulltile_loop;

	XFREE ( it );
	return  res ;
}

static
int scop_has_fulltile (scoplib_scop_p scop)
{
  // Description: since we assume that the scop is that of a fulltile
  // (as tagged by its past-tree), we don't want it to be a degenerate
  // case. So we check that the array on the lhs of the first statement
  // has non-constant access on all of its dimensions.
  scoplib_statement_p stmt;
  stmt = scop->statement;
  scoplib_matrix_p mat;
  mat = stmt->write;
  int nb_params = scop->nb_parameters;
  int nb_iters =  stmt->nb_iterators;
  int nb_rows = mat->NbRows;
  int nb_cols = mat->NbColumns;
  int all_dim_ok = 1;
  int i;
  int j;
  for (i=0; i<nb_rows; ++i)
  {
    int dim_ok = 0;
    // each dimension needs at least one iterator in its AF
    for (j=1; j<=nb_iters; ++j)
      dim_ok = dim_ok || (mat->p[i][j] != 0);
    all_dim_ok = all_dim_ok && dim_ok;
  }
  return all_dim_ok;
}


/**
 *  Generate code for transformed scop.
 *
 * (1) Create fake tile iterators inside the .scop: polyhedral
 *     tiling performed by Pluto does not update the iterators list.
 * (2) Generate statement macros.
 * (3) Convert the .scop to CloogProgram structure.
 * (4) Generate declaration for the new iterators.
 * (5) Generate polyhedral scanning code with CLooG algorithm
 * (6) Call pocc_driver_clastops, to generate AST-based code and
 *     pretty-print it.
 * (7) Final post-processing using PoCC internal scripts (timer
 *     code, unrolling, etc.) and full code generation.
 *
 */
static
s_past_node_t *
fulltile_codegen (scoplib_scop_p program,
		     s_pocc_options_t* poptions,
		     s_pocc_utils_options_t* puoptions)
{
  if (! poptions->quiet)
    printf ("[PoCC] Starting Codegen\n");
  /* Backup the default output file. */
  FILE* out_file = poptions->output_file;
  FILE* body_file = fopen (".body.c", "w");
  if (body_file == NULL)
    pocc_error ("Cannot create file .body.c\n");
  poptions->output_file = body_file;

  /* (1) Update statement iterators with tile iterators, if needed. */
  scoplib_statement_p stm;
  int i;
  for (stm = program->statement; stm; stm = stm->next)
    {
      int nb_it = stm->domain->elt->NbColumns - program->context->NbColumns;

      if (stm->nb_iterators != nb_it)
	{
	  char** iters = XMALLOC(char*, nb_it);
	  for (i = 0; i < nb_it - stm->nb_iterators; ++i)
	    {
	      iters[i] = XMALLOC(char, 16);
	      sprintf (iters[i], "fk%d", i);
	    }
	  for (; i < nb_it; ++i)
	    iters[i] = stm->iterators[i - nb_it + stm->nb_iterators];
	  XFREE(stm->iterators);
	  stm->iterators = iters;
	  stm->nb_iterators = nb_it;
	}
    }

  /* (2) Generate polyhedral scanning code with CLooG. */
  if (! poptions->quiet)
    printf ("[PoCC] Running CLooG\n");
  CloogOptions* coptions = poptions->cloog_options;
  if (coptions == NULL)
    {
      CloogState* cstate = cloog_state_malloc ();
      poptions->cloog_options = coptions = cloog_options_malloc (cstate);
    }
  coptions->language = 'c';

  if (poptions->cloog_f != POCC_CLOOG_UNDEF)
    coptions->f = poptions->cloog_f;
  if (poptions->cloog_l != POCC_CLOOG_UNDEF)
    coptions->l = poptions->cloog_l;

  struct clast_stmt* root =
    pocc_driver_cloog (program, coptions, poptions, puoptions);

  /* (3) Call Clast modules (and pretty-print if required). */
  pocc_driver_clastops (program, root, poptions, puoptions);

  /* (4) Call PAST modules (and pretty-print if required) */
  // Convert to PAST IR.
  if (! poptions->quiet)
	  printf ("[PAST] Converting CLAST to PoCC AST\n");
  s_past_node_t* pastroot = clast2past (root, 1);

  return pastroot;

}


static
void
scop2past_loop (scoplib_scop_p scop, s_past_node_t * loop)
{
  /* printf ("inscop:\n"); */
  /* scoplib_scop_print (stdout, scop); */
  /* printf ("inpast:\n"); */
  /* past_pprint (stdout, loop); */

  scoplib_statement_p stmt;
  s_past_node_t ** cstmt;
  past_set_parent (loop);
  s_past_node_t * next;
  next = loop->next;
  loop->next = NULL;

  cstmt = past_collect_nodetype (loop,past_cloogstmt);
  loop->next = next;
  int n;
  n = past_count_nodetype (loop,past_cloogstmt);
  s_past_node_t ** pstmt;
  pstmt = XMALLOC (s_past_node_t*,n+1);
  int i;

  if (!cstmt)
  {
    printf ("driver-psimdkzer.c: error. No statements founds\n");
    assert (0);
  }

  for (i=0; cstmt && cstmt[i] && i<n; ++i)//stmt=scop->statement; stmt; stmt=stmt->next,i++)
  {

    int j;
    if (past_node_is_a (cstmt[i], past_cloogstmt))
      {
	PAST_DECLARE_TYPED (cloogstmt,cs,cstmt[i]);
	/// FIXME: LNP: incorrect loop:
	/* for (j = 0, stmt = scop->statement; j < cs->stmt_number - 1;  */
	/*      ++j, stmt = stmt->next) */
	/*   ; */
	for (j = 0, stmt = scop->statement; j < i; 
	     ++j, stmt = stmt->next)
	  ;
	if (stmt == NULL)
	  {
	    fprintf (stderr, "[PoCC][PSimdKzer] NULL statement\n");
	    assert (0);
	  }
	pstmt[i] = scop2past_stmt (scop,stmt);
	if (i>0) pstmt[i-1]->next = pstmt[i];
	pstmt[i]->next = NULL;
      }
    else
      {
	fprintf (stderr, "[PoCC][PSimdKzer] Unsupported statement type\n");
	assert (0);
      }
  }

  past_replace_node (cstmt[0],pstmt[0]);
//  for (i=0; cstmt && cstmt[i]; ++i)
//    past_deep_free (cstmt[i]);

  XFREE (cstmt);
}


static
void
scop2past (scoplib_scop_p scop, s_past_node_t * root)
{
  int n;
  int i;
  scoplib_statement_p stmt;
  s_past_node_t ** inner;
  past_set_parent (root);
  inner = past_inner_loops (root);
/*
  printf ("-------scop2past----------------------\n");
  past_pprint (stdout,root);
  printf ("------------------------------\n");
*/
  for (i=0; inner && inner[i]; ++i)
  {
//    PAST_DECLARE_TYPED (for,pf,inner[i]);
    scop2past_loop (scop,inner[i]);
//    if (i>0) pstmt[i-1]->next = pstmt[i];
//    pstmt[i]->next = NULL;
  }
  XFREE (inner);
//  past_replace_node (cstmt[0],pstmt[0]);
}


static
void replace_full_tile
  (s_past_node_t * oldft, s_past_node_t * newft)
{
  // expect parent of oldft to be an if or affine guard
  s_past_node_t * parent;
  parent = oldft->parent;
  if (past_node_is_a (parent,past_if))
  {
    s_past_node_t * then;
    PAST_DECLARE_TYPED(if, ifnode, parent);
    s_past_node_t * stmtlist;
    for (stmtlist = ifnode->then_clause;
          stmtlist->next != oldft; stmtlist=stmtlist->next);
    if (stmtlist->next == oldft)
    {
      PAST_DECLARE_TYPED (if,ifft,newft);
      newft = ifft->then_clause;
      newft = past_clone (newft);
      newft->parent = oldft->parent;
      newft->next = oldft->next;
      past_replace_node (oldft,newft);
      stmtlist->next = newft;
      oldft->next = NULL;
      oldft->parent = NULL;
      past_set_parent (newft->parent);
    }

  }
  if (past_node_is_a (parent,past_affineguard))
  {
    s_past_node_t * then;
    PAST_DECLARE_TYPED(affineguard, ifnode, parent);
    s_past_node_t * stmtlist;
    for (stmtlist = ifnode->then_clause;
          stmtlist->next != oldft; stmtlist=stmtlist->next);
    stmtlist->next = newft;
    past_replace_node (oldft,newft);
    newft->parent = oldft->parent;
    newft->next = oldft->next;

  }

}

static
s_past_node_t * get_rhs (s_past_node_t * node)
{
  PAST_DECLARE_TYPED (binary,pb,node);
  //assert (past_node_is_a (pb->rhs,past_varref));
  return pb->rhs;
}


static
void
map_ufactors_to_dimensions (
  s_past_node_t * ftroot,
  s_psimdkzer_options_t * psk_opts)
{
  // assume one inner loop, as we have a full tile
  // climb up the tree and map ufactor by loop bounds name
  int i;
  s_past_node_t ** inner;
  inner = past_inner_loops (ftroot);
  s_past_node_t * current;
  for ( current=inner[0],i=psk_opts->max_dim-1;
        past_node_is_a (current,past_for);
        current=current->parent,--i)
  {
    PAST_DECLARE_TYPED (for,pf,current);
    s_past_node_t * lb;
    s_past_node_t * ub;
    lb = get_rhs (pf->init);
    ub = get_rhs (pf->test);
    PAST_DECLARE_TYPED (varref,lbv,lb);
    PAST_DECLARE_TYPED (varref,ubv,ub);
    sprintf (psk_opts->ufactor_map[i],
      "%s_%s",(char*)(lbv->symbol->data),(char*)(ubv->symbol->data));
//    printf ("Dim %d mapped to %s (%d)\n",i,psk_opts->ufactor_map[i],psk_opts->ufactor[i]);
  }
  XFREE (inner);
}



static
void
remap_ufactors_to_dimensions (
  s_past_node_t * ftroot,
  s_psimdkzer_options_t * psk_opts)
{
  // assume one inner loop, as we have a full tile
  // climb up the tree and map ufactor by loop bounds name
  int i;
  int j;
  int temp[psk_opts->max_dim];
  int tsize[psk_opts->max_dim];
  for (i=0; i<psk_opts->max_dim; ++i)
  {
    temp[i] = psk_opts->ufactor[i];
    tsize[i] = psk_opts->tsize[i];
  }

  s_past_node_t ** inner;
  inner = past_inner_loops (ftroot);
  s_past_node_t * current;
  for ( current=inner[0],i=psk_opts->max_dim-1;
        past_node_is_a (current,past_for);
        current=current->parent,--i)
  {
    PAST_DECLARE_TYPED (for,pf,current);
    s_past_node_t * lb;
    s_past_node_t * ub;
    lb = get_rhs (pf->init);
    ub = get_rhs (pf->test);
    PAST_DECLARE_TYPED (varref,lbv,lb);
    PAST_DECLARE_TYPED (varref,ubv,ub);
    char name[100];
    sprintf (name,
      "%s_%s",(char*)(lbv->symbol->data),(char*)(ubv->symbol->data));
    for (j=0; j<psk_opts->max_dim; ++j)
    {
      if (!strcmp (name,psk_opts->ufactor_map[j]))
      {
//        printf ("Dimension %d mapped to %d\n",j,i);
        psk_opts->ufactor[i] = temp[j];
        psk_opts->tsize[i] = tsize[j];
      }
    }
  }
  XFREE (inner);
}

static
int
check_atomic_bounds (s_past_node_t * ftroot)
{
  s_past_node_t ** loops;
  loops = past_collect_nodetype (ftroot,past_for);
  int i;
  for (i=0; loops && loops[i]; ++i)
  {
    PAST_DECLARE_TYPED (for,pf,loops[i]);
    s_past_node_t * lb;
    s_past_node_t * ub;
    lb = get_rhs (pf->init);
    ub = get_rhs (pf->test);
    if (!past_node_is_a (lb,past_varref) ||
        !past_node_is_a (ub,past_varref))
    {
      XFREE (loops);
      return 0;
    }
    // this is the bad case, we do not want skewing transfos
  }
  XFREE (loops);
  return 1;
}


/* entry point to module */
void pocc_driver_psimdkzer
(
	s_past_node_t  * root,
	scoplib_scop_p scop,
	s_pocc_options_t * poptions,
	s_pocc_utils_options_t * puoptions
//	s_past_node_t ** fundecl )
)
{
  /* // Re-set the options. */
  /*     poptions->psimdkzer = 1; */
  /*     poptions->pluto = 1; */
  /*     poptions->codegen = 1; */
  /*     poptions->use_past = 1; */
  /*     poptions->ptile = 1; */
  /*     poptions->ptile_fts = 1; */
  /*     poptions->pragmatizer = 1; */
  /*     poptions->past_optimize_loop_bounds = 1; */
  /*     // ponos options */
  /*     poptions->ponos = 0; */
  /*     poptions->ponos_build_2d_plus_one = 1; */
  /*     poptions->ponos_objective = PONOS_OBJECTIVES_CODELET; */
  /*     poptions->ponos_noredundancy_solver = 1; */
  /*     poptions->ponos_solver_precond = 1; */
  /*     poptions->ponos_maxscale_solver = 1; */
  /*     poptions->ponos_coef_are_pos = 1; */
  /*     poptions->ponos_quiet = 0; */

  if (! poptions->quiet)
    printf ("[PoCC] Running PSimdKzer\n");

  int i;
  s_psimdkzer_options_t * psk_opts;
  psk_opts = psimdkzer_options_malloc();

  psk_opts->vector_isa = poptions->psimdkzer_vector_isa;
  psk_opts->scalar_datatype = poptions->psimdkzer_scalar_datatype;

  if (poptions->psimdkzer_target == PSIMDKZER_TARGET_PREVECTOR)
    psk_opts->prevector_only = 1;
  else
    psk_opts->prevector_only = 0;

  if (poptions->psimdkzer_target == PSIMDKZER_TARGET_SPIRAL)
    psk_opts->use_spiral = 1;
  else
    psk_opts->use_spiral = 0;

  if (poptions->psimdkzer_target == PSIMDKZER_TARGET_NAIVE)
    psk_opts->use_direct = 1;
  else
    psk_opts->use_direct = 0;

  if (poptions->psimdkzer_target == PSIMDKZER_TARGET_NOVEC)
    psk_opts->novec = 1;
  else
    psk_opts->novec = 0;


  /*
    printf("PSIMDKzer options passed\n");
    printf("Target          : %d\n",poptions->psimdkzer_target);
    printf("Vector ISA 			: %d\n",poptions->psimdkzer_vector_isa);
    printf("Scalar Datatype : %d\n",poptions->psimdkzer_scalar_datatype);
    printf("Scalar reskew   : %d\n",poptions->psimdkzer_reskew);
    printf("\n\n");
  */


  // vector register size in bytes
  switch ( psk_opts->vector_isa )
    {
    case PSIMDKZER_VECTOR_ISA_SSE: psk_opts->regsize = 16; break;
    case PSIMDKZER_VECTOR_ISA_AVX: psk_opts->regsize = 32; break;
    case PSIMDKZER_VECTOR_ISA_BGQ: psk_opts->regsize = 32; break;
    case PSIMDKZER_VECTOR_ISA_MIC: psk_opts->regsize = 64; break;
    }

  //int dtsize = ( strcmp ( datatype, "float") == 0)? 4:8;
  switch ( psk_opts->scalar_datatype )
    {
    case PSIMDKZER_SCALAR_DATATYPE_FLOAT : psk_opts->dtsize = 4;  break;
    case PSIMDKZER_SCALAR_DATATYPE_DOUBLE : psk_opts->dtsize = 8; break;
    }

  // number of elements that fit in a vector length is the register size
  // into the datatype size in bytes
  psk_opts->vlen = psk_opts->regsize / psk_opts->dtsize;


  if ( psk_opts->max_inner_ufactor && psk_opts->vector_pbs > psk_opts->vlen * 3 )
    {
      int max_iuf = 250;
      int new_iuf = ( psk_opts->vector_pbs - 2 * psk_opts->vlen ) / psk_opts->vlen;
      if ( new_iuf < max_iuf )
	max_iuf = new_iuf;
      psk_opts->inner_ufactor = max_iuf;
    }

  // MK: TODO:
  // 1) Add checks for ufactors to be divisors of tile sizes
  // 2) Check that re-skewing factor is at least as big as the biggest ufactor

  char codelet_func[30];
  char proto_func[30];

  psk_opts->scop_id = 0;

  // headers


  /*
    if (psk_opts->prevector_only)
    {
    //char simd_func_debug[40];
    //sprintf (simd_func_debug,"simd_func_%d.prevect.c",psk_opts->scop_id);
    //psk_opts->simd_file_c_debug = 	fopen (simd_func_debug,"w");
    psk_opts->tile_file =  NULL;
    //fprintf ( psk_opts->simd_file_c_debug,"#include <%s>\n",proto_func);
    //psk_opts->debug_mode = 0;
    }

    // generate spiral = ! prevector_onyl
    if (psk_opts->use_spiral)
    {
    sprintf (spiral_func,"simd_func_%d.spiral",psk_opts->scop_id);
    psk_opts->simd_file_spiral = 	fopen (spiral_func,"w");
    }

    if (psk_opts->use_direct)
    {
    sprintf (codelet_func,"simd_func_%d.vector.c",psk_opts->scop_id);
    psk_opts->codelet_file = fopen (spiral_func,"w");
    }

  */


  // read unroll factors from outer to innermost dimension
  FILE * ffac = fopen ("ufactors","r");
  if (ffac)
    {
      for (i=0; 1; ++i)
	{
	  int res = fscanf(ffac,"%d",&psk_opts->ufactor[i]);
	  psk_opts->ufactor_fake[i] = 1;
	  if (! res || res == -1 ) break;
	}
      psk_opts->inner_ufactor = psk_opts->ufactor[i-1];
      psk_opts->ufactor[i] = 0;
      psk_opts->ufactor_fake[i] = 0;
      psk_opts->max_dim = i;
      fclose(ffac);
    }
  else
    {
      for (i=0; i < PSIMDKZER_MAX_DIM - 1; ++i)
	{
	  psk_opts->ufactor[i] = 1;
	  psk_opts->ufactor_fake[i] = 1;
	}
      psk_opts->inner_ufactor = psk_opts->ufactor[i-1];
      psk_opts->ufactor[i] = 0;
      psk_opts->ufactor_fake[i] = 0;
      psk_opts->max_dim = i;
    }

  // read tile sizes from outer to innermost dimension
  ffac = fopen ("tile.sizes","r");
  if (ffac)
    {
      for (i=0; 1; ++i)
	{
	  int res = fscanf(ffac,"%d",&psk_opts->tsize[i]);
	  if (! res || res == -1 ) break;
	}
      psk_opts->tsize[i] = 0;
      fclose(ffac);
    }
  else
    {
      for (i=0; i < PSIMDKZER_MAX_DIM - 1; ++i)
	psk_opts->tsize[i] = 32;
      psk_opts->tsize[i] = 0;
    }

  /// LNP: check ufactor divide tile size.
  int my_error = 0;
  for (i = 0; psk_opts->ufactor[i] != 0; ++i)
    {
      if (psk_opts->tsize[i] == 0)
	break;
      if (psk_opts->tsize[i] % psk_opts->ufactor[i] != 0)
	{
	  fprintf
	    (stderr, "[PSimdKzer][ERROR] unroll factor %d (position %d) does not divide tile size %d\n",
	     psk_opts->ufactor[i], i, psk_opts->tsize[i]);
	  my_error = 1;
	}
    }
  if (my_error)
    {
      psimdkzer_options_free (psk_opts);
      return;
    }

  /*
    if ( psk_opts->tile_type == PSIMDKZER_FULL_TILE )
    psimdkzer_gen_spiral( scop, root, psk_opts );
    else if ( psk_opts->tile_type == PSIMDKZER_PARTIAL_TILE )
    psimdkzer_gen_spiral_partial_tile( scop, root, psk_opts );
  */

  // Process each full tile

  if (! poptions->quiet)
    printf ("[PSimdKzer] Starting vectorization\n");


//  if (!poptions->psimdkzer_post_transform)
    {
      s_past_node_t ** fulltiles;
      fulltiles = collect_all_full_tiles (root,psk_opts->max_dim);
      poptions->ponos = 1;
      int n_proc_fts = 0;
      for (i=0; fulltiles && fulltiles[i]; ++i)
	{
	  scoplib_scop_p ftscop;
	  ftscop = scoptools_past2scop (fulltiles[i], scop);
	  //scoplib_scop_print (stdout, ftscop);
	  if (scop_has_fulltile (ftscop))
	    {
	      sprintf (proto_func,"simd_proto_%d.h",n_proc_fts);
	      psk_opts->proto_file = fopen (proto_func,"w");
	      if (psk_opts->prevector_only)
		sprintf (codelet_func,"codelet_%d.c",n_proc_fts);
	      if (psk_opts->use_direct)
		sprintf (codelet_func,"codelet_%d.c",n_proc_fts);
	      if (psk_opts->use_spiral)
		sprintf (codelet_func,"codelet_%d.spiral",n_proc_fts);
	      psk_opts->codelet_file = fopen(codelet_func,"w");

	      fprintf (psk_opts->proto_file,"#include \"simd_func.h\"\n");

	      fprintf (psk_opts->proto_file,
		       "#ifndef VECTLEN\n#define VECTLEN %d\n#endif\n\n",psk_opts->vlen);

	      fclose (psk_opts->proto_file);

	      fprintf (psk_opts->codelet_file,"#include \"%s\"\n",proto_func);

	      n_proc_fts++;
	      if (! poptions->quiet)
		printf ("[PSimdKzer] Processing full tile %d\n",i);

	      scop2past (ftscop,fulltiles[i]);

	      scoplib_scop_p new_ftscop;
	      new_ftscop = scoptools_past2scop (fulltiles[i],ftscop);
	      //past_pprint (stdout, fulltiles[i]);
	      //scoplib_scop_print (stdout, new_ftscop);

	      map_ufactors_to_dimensions (fulltiles[i],psk_opts);
	      if (pocc_driver_ponos (new_ftscop, poptions, puoptions) == EXIT_FAILURE)
		{
		  exit (EXIT_FAILURE);
		}
	      s_past_node_t *new_ftroot;

	      //scoplib_scop_print (stdout, new_ftscop);

	      //new_ftroot = fulltile_codegen (new_ftscop,poptions,puoptions);
	      //scop2past (new_ftscop,new_ftroot);

	      new_ftroot = scoptools_scop2past (new_ftscop);
	      new_ftscop = scoptools_past2scop (new_ftroot,new_ftscop);
	      new_ftroot = scoptools_scop2past (new_ftscop);

	      scop2past (new_ftscop,new_ftroot);

	      if (!psk_opts->novec)
		{
		  // bounds should remain atomic if no skewing was performed
		  if (check_atomic_bounds (new_ftroot) )
		    {
		      //printf ("driver-psimdkzer: calling driver\n");
		      // potential permutation of dimensions
		      remap_ufactors_to_dimensions (new_ftroot,psk_opts);

		      psimdkzer_pocc_driver (ftscop,new_ftroot,psk_opts);
		    }
		  else
		    {
		      printf ("[PSimdKzer] Cannot proceed with vectorization due to missmatching unrolling factor to dimensions.\n");
		    }
		}
	      new_ftroot->next = fulltiles[i]->next;
	      new_ftroot->parent = fulltiles[i]->parent;
	      replace_full_tile (fulltiles[i],new_ftroot);

	      fclose (psk_opts->codelet_file);
	    }

	}
      XFREE (fulltiles);
      poptions->psimdkzer_proc_fts = n_proc_fts;
    }
/*
  else
    {
      if (! poptions->quiet)
	printf ("[PSimdKzer] Post-transformation only\n");

      int n_proc_fts = 0;

      sprintf (proto_func,"simd_proto_%d.h",n_proc_fts);
      psk_opts->proto_file = fopen (proto_func,"w");
      if (psk_opts->prevector_only)
	sprintf (codelet_func,"codelet_%d.c",n_proc_fts);
      if (psk_opts->use_direct)
	sprintf (codelet_func,"codelet_%d.c",n_proc_fts);
      if (psk_opts->use_spiral)
	sprintf (codelet_func,"codelet_%d.spiral",n_proc_fts);
      psk_opts->codelet_file = fopen(codelet_func,"w");
      fprintf (psk_opts->proto_file,"#include \"simd_func.h\"\n");
      fprintf (psk_opts->proto_file,
	       "#ifndef VECTLEN\n#define VECTLEN %d\n#endif\n\n",psk_opts->vlen);
      fprintf (psk_opts->codelet_file,"#include \"%s\"\n",proto_func);

      //scoplib_scop_print (stdout,scop);
      s_past_node_t * new_root = scoptools_scop2past  (scop);
      //scoplib_scop_p ftscop;
      //ftscop = scoptools_past2scop (root,scop);
      //scoplib_scop_print (stdout,ftscop);

      scop2past (scop,new_root);

      s_past_node_t ** outer = past_outer_loops (new_root);
      s_past_node_t * ag = outer[0]->parent;
      XFREE (outer);
      psimdkzer_pocc_driver (scop,ag,psk_opts);
      PAST_DECLARE_TYPED (if,pif,ag);
      s_past_node_t ** _outer_ = past_outer_loops (root);
      outer = past_outer_loops (ag);
      past_replace_node (_outer_[0],outer[0]);
      //XFREE (_outer_);
      //past_replace_node (pif->then_clause,pf);
      past_set_parent (root);

      n_proc_fts++;
      fclose (psk_opts->codelet_file);
      fclose (psk_opts->proto_file);
      poptions->psimdkzer_proc_fts = n_proc_fts;
    }
    */

  //	fclose ( psk_opts->simd_file_proto );
  //	if ( ! psk_opts->prevector_only )
  //		fclose ( psk_opts->simd_file_spiral );
  //	if ( psk_opts->debug_mode || psk_opts->prevector_only )
  //		fclose ( psk_opts->simd_file_c_debug );
  //	if ( psk_opts->debug_mode )
  //		fclose ( psk_opts->tile_file );


  psimdkzer_options_free (psk_opts);
  if (! poptions->quiet)
    printf ("[PSimdKzer] Vectorization completed\n");

}
