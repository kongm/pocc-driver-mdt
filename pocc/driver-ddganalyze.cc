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

#include <map>
#include <set>
#include <vector>

#include <math.h>

#include <pocc/driver-ddganalyze.h>
#include <candl/ddv.h>
#include <candl/candl.h>
#include <candl/dependence.h>
#include <candl/program.h>
#include <candl/piplib-wrapper.h>
#include <pocc/driver-pluto.h>
#include <pocc/driver-codegen.h>
#include <pocc/driver-ponos.h>
#include <pocc/driver-cloog.h>
#include <pocc/error.h>
#include <past/past.h>
#include <past/pprint.h>
#include <past/past_api.h>
#include <irconverter/past2scop.h>
#include <polyfeat/ast_stats.h>

#include <letsee/graph.h>

/// Grrrrr
#ifdef Q
# undef Q
#endif
#include <isl/map.h>
#include <isl/set.h>
#include <isl/space.h>
#include <polyfeat/barvinokinterface.h>

#define MAX_NUM_ENTRIES_DDG 1000000

#define POCC_DDG_OPTYPE_LOAD	1
#define POCC_DDG_OPTYPE_STORE	2
#define POCC_DDG_OPTYPE_FLOAT	3
#define POCC_DDG_OPTYPE_DOUBLE	4
#define POCC_DDG_OPTYPE_INT	5
#define POCC_DDG_OPTYPE_CONTROL	6
#define POCC_DDG_OPTYPE_OTHERS	7

#define STMT_UN(x) ((*program->isl_stmt_name_to_unique_isl_stmt_name)[(x)])


struct s_annotations
{
  char* op_type;
  isl_set* iter_dom;
  char* full_name;
  char* instr_addr;
};
typedef struct s_annotations s_annotations_t;

/// YYYY
struct s_statement
{
  isl_set* iter_domain;
  const char* isl_name;
  char* adjusted_name;
  int scop_id;
  char* type;
  int op_type;
  double nb_points;
  int loop_depth;
  std::vector<isl_map*>* access_func;
};
typedef struct s_statement s_statement_t;

struct s_dependence
{
  isl_map* dependence;
  s_statement_t* src;
  s_statement_t* dst;
  const char* isl_name_src;
  const char* isl_name_dst;
  char* adjusted_name_src;
  char* adjusted_name_dst;
  double nb_points;
};
typedef struct s_dependence s_dependence_t;


struct s_program
{
  std::vector<s_statement_t*>* stmts;
  std::vector<s_dependence_t*>* deps;
  std::map<const char*, s_statement_t*>* isl_name_to_stmt;
  std::map<isl_set*,s_statement_t*>* isl_set_to_stmt;
  std::map<isl_map*,s_dependence_t*>* isl_dep_to_dep;
  std::vector<isl_map*>* access_funcs;
  std::map<const char*, s_statement_t*>* scevname_to_stmt;

  std::map<const char*, char*>* isl_name_to_context;
  std::map<const char*, char*>* isl_name_to_stmt_id;
  std::map<char*, std::vector<const char*> >* context_to_isl_name;
  std::map<char*, std::vector<const char*> >* stmt_id_to_isl_name;

  std::map<const char*, char*>* stmt_id_to_execblock;
  std::map<char*, std::vector<char*> >* execblock_to_stmt_id;

  std::map<const char*, const char*>* isl_stmt_name_to_unique_isl_stmt_name;


  scoplib_scop_p scop;

  int stats_enabled;
  double sum_float;
  double sum_double;
  double sum_int;
  double sum_load;
  double sum_store;
  double sum_control;
  double sum_other;
  double all_ops;
  int max_loop_depth;
};
typedef s_program s_program_t;

static
void
program_print(FILE* stream, s_program_t* program)
{
  for (std::vector<s_statement_t*>::iterator ii = program->stmts->begin();
       ii != program->stmts->end(); ++ii)
    {
      fprintf (stream, "S%d\t%s\t%.0f\t%s\n", (*ii)->scop_id, (*ii)->isl_name, (*ii)->nb_points, (*ii)->type);
    }

  for (std::vector<s_dependence_t*>::iterator ii = program->deps->begin();
       ii != program->deps->end(); ++ii)
    {
      fprintf (stream, "S%d->S%d\t%s->%s\t%.0f\t%s->%s\n", (*ii)->src->scop_id, (*ii)->dst->scop_id, (*ii)->isl_name_src, (*ii)->isl_name_dst, (*ii)->nb_points, (*ii)->src->isl_name, (*ii)->dst->isl_name);
    }

  fprintf (stream, "%d stmts, %d deps\n", program->stmts->size(),
	  program->deps->size());
}


static
void
compute_stmt_to_context_map(s_program_t* program)
{
  int i;

  std::vector<s_statement_t*>::iterator ii;
  std::map<char*, std::vector<const char*> >::iterator jj;
  for (ii = program->stmts->begin (); ii != program->stmts->end (); ++ii)
    {
      const char* isl_stmt_name = (*ii)->isl_name;
      char* s = strdup (isl_stmt_name);
      char* tmp = s;
      tmp += strlen ("STMT_");
      for (i = 0; tmp[i] && tmp[i] != '_'; ++i)
	;
      tmp[i] = '\0';
      char* context = strdup (tmp);
      tmp += i + 1;
      char* stmt_id = strdup (tmp);
      for (jj = program->context_to_isl_name->begin ();
	   jj != program->context_to_isl_name->end (); ++jj)
	if (! strcmp (jj->first, context))
	  {
	    (*program->isl_name_to_context)[isl_stmt_name] = jj->first;
	    (*program->context_to_isl_name)[jj->first].push_back (isl_stmt_name);
	    break;
	  }
      if (jj == program->context_to_isl_name->end ())
	{
	  (*program->isl_name_to_context)[isl_stmt_name] = context;
	  (*program->context_to_isl_name)[context].push_back (isl_stmt_name);
	  XFREE(context);
	}

      for (jj = program->stmt_id_to_isl_name->begin ();
	   jj != program->stmt_id_to_isl_name->end (); ++jj)
	if (! strcmp (jj->first, stmt_id))
	  {
	    (*program->isl_name_to_stmt_id)[isl_stmt_name] = jj->first;
	    (*program->stmt_id_to_isl_name)[jj->first].push_back (isl_stmt_name);
	    break;
	  }
      if (jj == program->stmt_id_to_isl_name->end ())
	{
	  (*program->isl_name_to_stmt_id)[isl_stmt_name] = stmt_id;
	  (*program->stmt_id_to_isl_name)[stmt_id].push_back (isl_stmt_name);
	  XFREE(stmt_id);
	}

      XFREE(s);
    }
}


static
int
dep_stmts_in_ddg(s_program_t* program, s_dependence_t* dep)
{
  int src_ok = 0;
  int dst_ok = 0;
  for (std::vector<s_statement_t*>::iterator ii = program->stmts->begin();
       ii != program->stmts->end(); ++ii)
    {
      if (dep->src == *ii)
	src_ok = 1;
      if (dep->dst == *ii)
	dst_ok = 1;
      if (src_ok && dst_ok)
	break;
    }

  return src_ok && dst_ok;
}



static
void
stats_original_program(s_program_t* program)
{
  int i;
  // Count the total number of each operation.
  double sum_float = 0;
  double sum_double = 0;
  double sum_int = 0;
  double sum_load = 0;
  double sum_store = 0;
  double sum_control = 0;
  double sum_other = 0;
  int max_loop_depth = 0;
  for (std::vector<s_statement_t*>::iterator ii = program->stmts->begin();
       ii != program->stmts->end(); ++ii)
    {
    switch ((*ii)->op_type)
      {
      case POCC_DDG_OPTYPE_FLOAT: sum_float += (*ii)->nb_points; break;
      case POCC_DDG_OPTYPE_DOUBLE: sum_double += (*ii)->nb_points; break;
      case POCC_DDG_OPTYPE_LOAD: sum_load += (*ii)->nb_points; break;
      case POCC_DDG_OPTYPE_STORE: sum_store += (*ii)->nb_points; break;
      case POCC_DDG_OPTYPE_INT: sum_int += (*ii)->nb_points; break;
      case POCC_DDG_OPTYPE_CONTROL: sum_control += (*ii)->nb_points; break;
      case POCC_DDG_OPTYPE_OTHERS:
      default: sum_other += (*ii)->nb_points; break;
      }
    max_loop_depth = max_loop_depth < (*ii)->loop_depth ? (*ii)->loop_depth :
      max_loop_depth;
    }
  if (max_loop_depth == 0)
    max_loop_depth = 1;
  int num_stmts[max_loop_depth + 1];
  double nb_points[max_loop_depth + 1];
  for (i = 0; i <= max_loop_depth; ++i)
    {
      num_stmts[i] = 0;
      nb_points[i] = 0;
    }
  for (i = 0; i <= max_loop_depth; ++i)
    {
      for (std::vector<s_statement_t*>::iterator ii = program->stmts->begin();
	   ii != program->stmts->end(); ++ii)
	{
	  int dim = (*ii)->loop_depth;
	  if (dim == i)
	    {
	      num_stmts[dim]++;
	      nb_points[dim] += (*ii)->nb_points;
	    }
	}

    }

  double all_ops = sum_float + sum_double + sum_load + sum_store + sum_int + sum_control + sum_other;

  double sum_all_deps = 0;
  for (std::vector<s_dependence_t*>::iterator ii = program->deps->begin();
       ii != program->deps->end(); ++ii)
    sum_all_deps += (*ii)->nb_points;
  printf ("Totals: %.0f ops, %.0f dep points\n", all_ops, sum_all_deps);

  printf ("Total operations: %0.f\n", all_ops);
  double flops = sum_float + sum_double;
  printf ("Total FLOPs: %0.f (%.2f%%)\n", flops, (flops/all_ops)*100.0);
  double mem = sum_load + sum_store;
  printf ("Total load/store: %0.f (%.2f%%)\n", mem, (mem/all_ops)*100.0);
  printf ("Total INT: %0.f (%.2f%%)\n", sum_int, (sum_int/all_ops)*100.0);
  double other = sum_control + sum_other;
  printf ("Total others: %0.f (%.2f%%)\n", other, (other/all_ops)*100.0);
  printf ("Maximal loop depth: %d\n", max_loop_depth);
  for (i = 0; i <= max_loop_depth; ++i)
    printf ("-> depth=%d: %d instructions (%.2f%%) %.0f operations (%.2f%%)\n",
	    i, num_stmts[i], (num_stmts[i]/(program->stmts->size()*1.0))*100.0,
	    nb_points[i], (nb_points[i]/all_ops)*100.0);


  // Backup program stats.
  program->sum_float = sum_float;
  program->sum_double = sum_double;
  program->sum_int = sum_int;
  program->sum_load = sum_load;
  program->sum_store = sum_store;
  program->sum_control = sum_control;
  program->sum_other = sum_other;
  program->all_ops = all_ops;
  program->max_loop_depth = max_loop_depth;
}



static
int
compute_op_type(char* op)
{
  if (! strcmp (op, "LOAD"))
    return POCC_DDG_OPTYPE_LOAD;
  else if (! strcmp (op, "STORE"))
    return POCC_DDG_OPTYPE_STORE;
  else if (! strcmp (op, "FLOAT"))
    return POCC_DDG_OPTYPE_FLOAT;
  else if (! strcmp (op, "DOUBLE"))
    return POCC_DDG_OPTYPE_DOUBLE;
  else if (! strcmp (op, "INT"))
    return POCC_DDG_OPTYPE_INT;
  else if (! strcmp (op, "CONTROL"))
    return POCC_DDG_OPTYPE_CONTROL;
  else
    return POCC_DDG_OPTYPE_OTHERS;
}


static
char*
find_stmt_name_from_instr_addr(char* instr_name, char** stmt_names)
{
  int i;
  for (i = 0; stmt_names && stmt_names[i]; ++i)
    {
      char* tmp = stmt_names[i];
      while (*tmp && *tmp != '_')
	++tmp;
      char buffer[256];
      int pos = 0;
      if (*tmp)
	{
	  ++tmp;
	  while (*tmp)
	    buffer[pos++] = *(tmp++);
	  buffer[pos] = '\0';
	  if (! strcmp (buffer, instr_name))
	    return stmt_names[i];
	}
    }

  return NULL;
}

static
char* read_line(FILE* stream, size_t* buffer_size, char** buffer)
{
  if (*buffer == NULL)
    *buffer = XMALLOC(char, *buffer_size);
  char* ret = NULL;
  do
    {
      ret = fgets (*buffer, *buffer_size, stream);
      if (ret == NULL)
	{
	  *buffer_size *= 2;
	  *buffer = XREALLOC(char, *buffer, *buffer_size);
	}
    }
  while (ret == NULL && ! feof (stream));
  return ret;
}


static
void
annotation_loader(char* filename, s_program_t* program)
{

  FILE* input_file = fopen (filename, "r");
  if (! input_file)
    {
      printf ("Error: cannot open annotation file %s\n", filename);
      exit (1);
    }

  int i;
  char* buffer = NULL;
  size_t buffer_size = 32768;
  int idx = 0;
  while (read_line (input_file, &buffer_size, &buffer))
    {
      if (*buffer == '\n' || *buffer == '\0')
	continue;
      char instr_name[256];
      char instr_type[256];
      char* ptr = (char*) instr_name;
      char* tmp = buffer;
      while (tmp && *tmp && *tmp != ' ')
	*(ptr++) = *(tmp++);
      *ptr = '\0';
      ++tmp;
      ptr = (char*) instr_type;
      while (tmp && *tmp && *tmp != ' ' && *tmp != '\n')
	*(ptr++) = *(tmp++);
      *ptr = '\0';

      for (std::vector<s_statement_t*>::iterator jj = program->stmts->begin();
	   jj != program->stmts->end(); ++jj)
	{
	  char* tmp = (*jj)->adjusted_name;
	  /// LNP: new inst to comply with new statement name in ddg.annots
	  tmp += strlen ("STMT_");
	  while (*tmp && *tmp != '_')
	    ++tmp;
	  char buffer[256];
	  int pos = 0;
	  if (*tmp)
	    {
	      ++tmp;
	      while (*tmp)
		buffer[pos++] = *(tmp++);
	      buffer[pos] = '\0';
	      if (! strcmp (buffer, instr_name))
		{
		  (*jj)->type = strdup (instr_type);
		  (*jj)->op_type = compute_op_type ((*jj)->type);
		}
	    }
	}
    }
  XFREE(buffer);
}



static
void
annotation_execblock_loader(char* filename, s_program_t* program)
{

  FILE* input_file = fopen (filename, "r");
  if (! input_file)
    {
      printf ("Error: cannot open annotation file %s\n", filename);
      exit (1);
    }

  int i;
  char* buffer = NULL;
  size_t buffer_size = 32768;
  int idx = 0;
  while (read_line (input_file, &buffer_size, &buffer))
    {
      if (*buffer == '\n' || *buffer == '\0')
	continue;
      char instr_name[256];
      char exec_block[256];
      if (sscanf (buffer, "%s %s", instr_name, exec_block) < 2)
	continue;

      char* stmt_id = strdup (instr_name);
      char* eblock = strdup (exec_block);
      std::map<char*, std::vector<char*> >::iterator ii;
      std::map<char*, std::vector<const char*> >::iterator jj;
      for (ii = program->execblock_to_stmt_id->begin ();
	   ii != program->execblock_to_stmt_id->end (); ++ii)
	{
	  if (! strcmp (ii->first, exec_block))
	    {
	      XFREE(eblock);
	      eblock = ii->first;
	      break;
	    }
	}
      for (jj = program->stmt_id_to_isl_name->begin ();
	   jj != program->stmt_id_to_isl_name->end (); ++jj)
	{
	  if (! strcmp (jj->first, stmt_id))
	    {
	      XFREE(stmt_id);
	      stmt_id = jj->first;
	      break;
	    }
	}
      (*program->execblock_to_stmt_id)[eblock].push_back (stmt_id);
      (*program->stmt_id_to_execblock)[stmt_id] = eblock;
    }
  XFREE(buffer);
}



static
char* adjust_names(char* str)
{
  int i;
  char* ret = str;
  while (str && *str)
    {
      if (*str == '"')
	{
	  // replace first " by S.
	  *str = 'S';
	  // replace all , by _ for till the closing "
	  while (*str != '"')
	    {
	      if (*str == ',')
		*str = '_';
	      ++str;
	    }
	  // replace last " by space.
	  *str = ' ';
	}
      ++str;
    }

  return ret;
}

static
scoplib_matrix_list_p
convert_islset_to_domain(isl_ctx* ctxt, isl_set* s)
{
  const char* tempfilename = "___tmp_isl_mats.txt";
  FILE* f = fopen (tempfilename, "w+");
  if (! f)
    {
      printf ("Error: cannot create temporary file %s\n", tempfilename);
      exit (1);
    }
  isl_printer* pr = isl_printer_to_file (ctxt, f);
  pr = isl_printer_set_output_format(pr, ISL_FORMAT_POLYLIB);
  isl_printer_print_set (pr, s);
  /* isl_printer* pr_dbg = isl_printer_to_file (ctxt, stdout); */
  /* printf ("Translating domain:\n"); */
  /* isl_printer_print_set (pr_dbg, s); */

  rewind (f);
  scoplib_matrix_p m = scoplib_matrix_read (f);
  /* scoplib_matrix_print (stdout, m); */
  fclose (f);
  scoplib_matrix_list_p ret = scoplib_matrix_list_malloc ();
  ret->elt = m;

  return ret;
}

static
CandlMatrix*
convert_isldep_to_candldep(isl_map* m, CandlDependence* dep,
			   isl_set* iter_src, isl_set* iter_dst)
{
  const char* tmpfilename = "____candl_mat_convert.txt";
  FILE* f = fopen (tmpfilename, "w+");
  if (! f)
    {
      printf ("Error: cannot create file %s\n", tmpfilename);
      exit (1);
    }
  isl_ctx* ctxt = isl_ctx_alloc ();
  isl_printer* pr = isl_printer_to_file (ctxt, f);
  pr = isl_printer_set_output_format (pr, ISL_FORMAT_POLYLIB);
  isl_printer_print_map (pr, m);
  rewind (f);
  CandlMatrix* depdomain = candl_matrix_read (f);
  fclose (f);

  /* isl_printer* pr_dbg = isl_printer_to_file (ctxt, stdout); */
  /* printf ("Translating dep:\n"); */
  /* isl_printer_print_map (pr_dbg, m); */
  /* printf ("\n into:\n"); */
  /* candl_matrix_print (stdout, depdomain); */

  // Permute the src and tgt columns.
  int i, j;
  int val;
  isl_space* space_src = isl_set_get_space (iter_src);
  int nb_iters_src = isl_space_dim (space_src, isl_dim_set);
  isl_space* space_dst = isl_set_get_space (iter_dst);
  int nb_iters_dst = isl_space_dim (space_dst, isl_dim_set);

  int is_self_dep = 0;
  if (iter_src == iter_dst)
    is_self_dep = 1;

  int nbrows = depdomain->NbRows;
  if (is_self_dep)
    nbrows += nb_iters_src;
  CandlMatrix* newm = candl_matrix_malloc (nbrows,
					   depdomain->NbColumns);

  for (i = 0; i < depdomain->NbRows; ++i)
    {
      val = CANDL_get_si(depdomain->p[i][0]);
      CANDL_set_si(newm->p[i][0], val);
      for (j = 0; j < nb_iters_dst; ++j)
	{
	  val = CANDL_get_si(depdomain->p[i][1 + j]);
	  CANDL_set_si(newm->p[i][1 + nb_iters_src + j], val);
	}
      for (j = 0; j < nb_iters_src; ++j)
	{
	  val = CANDL_get_si(depdomain->p[i][1 + nb_iters_dst + j]);
	  CANDL_set_si(newm->p[i][1 + j], val);
	}
      for (j = 1 + nb_iters_src + nb_iters_dst; j < newm->NbColumns; ++j)
	{
	  val = CANDL_get_si(depdomain->p[i][j]);
	  CANDL_set_si(newm->p[i][j], val);
	}
    }

  if (is_self_dep)
    {
      // Encode the dep is src <lec tgt, otherwise it is not
      // schedulable.
      int pos;
      for (; i < newm->NbRows - 1; ++i)
	{
	  pos = i - depdomain->NbRows + 1;
	  CANDL_set_si(newm->p[i][0], 0);
	  CANDL_set_si(newm->p[i][pos], 1);
	  CANDL_set_si(newm->p[i][pos + nb_iters_src], -1);
	}
      CANDL_set_si(newm->p[i][0], 0);
      CANDL_set_si(newm->p[i][pos], -1);
      CANDL_set_si(newm->p[i][pos + nb_iters_src], 1);
      CANDL_set_si(newm->p[i][newm->NbColumns - 1], 1);

      if (! pip_has_rational_point (newm, NULL, 1))
	{
	  candl_matrix_free (newm);
	  newm = NULL;
	}
    }
  candl_matrix_free (depdomain);

  return newm;
}


static
CandlDependence*
convert_isl_deps_oldversion(scoplib_scop_p scop, CandlProgram* cprogram,
		 isl_map** dependences, isl_set** iter_domains)
{
  int i, j;
  /* candl_program_print (stdout, cprogram); */
  int num_stmt = scoplib_statement_number (scop->statement);
  const char* stmt_names[num_stmt + 1];
  CandlStatement* candl_stmts[num_stmt + 1];
  for (i = 0; i < num_stmt; ++i)
    {
      isl_space* space = isl_set_get_space (iter_domains[i]);
      stmt_names[i] = isl_space_get_tuple_name (space, isl_dim_set);
      candl_stmts[i] = cprogram->statement[i];
    }
  stmt_names[i] = NULL;
  candl_stmts[i] = NULL;

  CandlDependence* ret = NULL;
  CandlDependence** curr = &ret;
  for (i = 0; dependences && dependences[i]; ++i)
    {
      isl_map* m = dependences[i];
      // Get the source and target stmt id.
      isl_space* space = isl_map_get_space (m);
      const char* src = isl_space_get_tuple_name (space, isl_dim_in);
      const char* dst = isl_space_get_tuple_name (space, isl_dim_out);
      for (j = 0; stmt_names[j]; ++j)
	if (! strcmp (src, stmt_names[j]))
	  break;
      if (stmt_names[j] == NULL)
	{
	  printf ("Error: cannot find statement %s in the iteration domains\n",
		  src);
	  printf ("Problematic dependence:\n");
	  isl_ctx* ctxt = isl_ctx_alloc ();
	  isl_printer* pr_dbg = isl_printer_to_file (ctxt, stdout);
	  isl_printer_print_map (pr_dbg, m);
	  exit (1);
	}
      int stmt_src_id = j;
      for (j = 0; stmt_names[j]; ++j)
	if (! strcmp (dst, stmt_names[j]))
	  break;
      if (stmt_names[j] == NULL)
	{
	  printf ("Error: cannot find statement %s in the iteration domains\n",
		  dst);
	  printf ("Problematic dependence:\n");
	  isl_ctx* ctxt = isl_ctx_alloc ();
	  isl_printer* pr_dbg = isl_printer_to_file (ctxt, stdout);
	  isl_printer_print_map (pr_dbg, m);
	  exit (1);
	}
      int stmt_dst_id = j;
      CandlDependence* dep = candl_dependence_malloc ();
      dep->source = candl_stmts[stmt_src_id];
      dep->target = candl_stmts[stmt_dst_id];
      /* dep->type = CANDL_RAW; */
      dep->depth = dep->source->depth;
      dep->domain = convert_isldep_to_candldep
      	(m, dep, iter_domains[stmt_src_id], iter_domains[stmt_dst_id]);
      if (dep->domain == NULL)
	XFREE(dep);
      else
	{
	  if (ret == NULL)
	    ret = *curr = dep;
	  else
	    *curr = dep;
	  curr = &((*curr)->next);
	}
    }

  return ret;
}



static
CandlDependence*
convert_isl_deps(scoplib_scop_p scop, CandlProgram* cprogram,
		 s_program_t* program)
{
  int i, j;
  /* candl_program_print (stdout, cprogram); */
  int num_stmt = program->stmts->size();
  CandlStatement* candl_stmts[num_stmt + 1];
  for (i = 0; i < num_stmt; ++i)
    candl_stmts[i] = cprogram->statement[i];
  candl_stmts[i] = NULL;

  CandlDependence* ret = NULL;
  CandlDependence** curr = &ret;
  for (std::vector<s_dependence_t*>::iterator ii = program->deps->begin();
       ii != program->deps->end(); ++ii)
    {
      isl_map* m = (*ii)->dependence;
      s_statement_t* src = (*ii)->src;
      s_statement_t* dst = (*ii)->dst;
      if (! dep_stmts_in_ddg (program, *ii))
	{
	  printf ("Error: cannot find statement %s/%s in the iteration domains\n",
		  (*ii)->isl_name_src, (*ii)->isl_name_dst);
	  printf ("Problematic dependence:\n");
	  isl_ctx* ctxt = isl_ctx_alloc ();
	  isl_printer* pr_dbg = isl_printer_to_file (ctxt, stdout);
	  isl_printer_print_map (pr_dbg, m);
	  exit (1);
	}
      CandlDependence* dep = candl_dependence_malloc ();
      dep->source = candl_stmts[(*ii)->src->scop_id];
      dep->target = candl_stmts[(*ii)->dst->scop_id];
      /* dep->type = CANDL_RAW; */
      dep->depth = dep->source->depth;
      dep->domain = convert_isldep_to_candldep
      	(m, dep, (*ii)->src->iter_domain, (*ii)->dst->iter_domain);
      if (dep->domain == NULL)
	XFREE(dep);
      else
	{
	  if (ret == NULL)
	    ret = *curr = dep;
	  else
	    *curr = dep;
	  curr = &((*curr)->next);
	}
    }

  return ret;
}


/* static */
/* void */
/* heurist_ddg_prune_1(isl_set** iter_domains, double* card_doms, */
/* 		    isl_map** dependences, double* card_deps, */
/* 		    float cutoff) */
/* { */
/*   if (! iter_domains || ! dependences || ! card_doms || ! card_deps */
/*       || cutoff == 1.0) */
/*     return; */
/*   int i, j, k; */

/*   // 0. Prep. */
/*   int num_deps; */
/*   for (num_deps = 0; dependences[num_deps]; ++num_deps) */
/*     ; */
/*   int num_doms; */
/*   for (num_doms = 0; iter_domains[num_doms]; ++num_doms) */
/*     ; */
/*   const char** stmt_names = XMALLOC(char*, num_doms + 1); */
/*   for (i = 0; i < num_doms; ++i) */
/*     stmt_names[i] = isl_space_get_tuple_name */
/*       (isl_set_get_space (iter_domains[i]), isl_dim_set); */
/*   stmt_names[i] = NULL; */

/*   // 1. Find all deps with > cutoff points. */
/*   isl_map** bigdeps = XMALLOC(isl_map*, num_deps + 1); */
/*   int pos = 0; */
/*   for (i = 0; dependences[i]; ++i) */
/*     if (card_deps[i] > cutoff) */
/*       bigdeps[pos++] = dependences[i]; */
/*   bigdeps[pos] = NULL; */

/*   // 2. Find all statements involved in large deps. */
/*   const char** stmts = XMALLOC(char*, num_doms + 1); */
/*   int pos = 0; */
/*   for (i = 0; bigdeps[i]; ++i) */
/*     { */
/*       isl_space* space = isl_map_get_space (bigdeps[i]); */
/*       const char* src = isl_space_get_tuple_name (space, isl_dim_in); */
/*       const char* dst = isl_space_get_tuple_name (space, isl_dim_out); */
/*       for (j = 0; j < pos; ++j) */
/* 	if (! strcmp (src, stmts[j])) */
/* 	  break; */
/*       if (j == pos) */
/* 	stmts[pos++] = src; */
/*       for (j = 0; j < pos; ++j) */
/* 	if (! strcmp (dst, stmts[j])) */
/* 	  break; */
/*       if (j == pos) */
/* 	stmts[pos++] = dst; */
/*     } */

/*   // 3.  */

/* } */



static
void
heurist_ddg_prune_2(isl_set** iter_domains, double* card_doms,
		    isl_map** dependences, double* card_deps,
		    float cutoff)
{
  printf ("Start pruning heuristic\n");
  if (! iter_domains || ! dependences || ! card_doms || ! card_deps
      || cutoff == 1.0)
    return;
  int i, j, k;

  // 0. Prep.
  int num_deps;
  for (num_deps = 0; dependences[num_deps]; ++num_deps)
    ;
  int num_doms;
  for (num_doms = 0; iter_domains[num_doms]; ++num_doms)
    ;
  char** stmt_names = XMALLOC(char*, num_doms + 1);
  for (i = 0; i < num_doms; ++i)
    stmt_names[i] = (char*)isl_space_get_tuple_name
      (isl_set_get_space (iter_domains[i]), isl_dim_set);
  stmt_names[i] = NULL;

  // 1. Find all deps with > cutoff points.
  isl_map** bigdeps = XMALLOC(isl_map*, num_deps + 1);
  int pos = 0;
  for (i = 0; dependences[i]; ++i)
    if (card_deps[i] > cutoff)
      bigdeps[pos++] = dependences[i];
  bigdeps[pos] = NULL;

  // 2. Find all statements involved in large deps.
  char** stmts = XMALLOC(char*, num_doms + 1);
  pos = 0;
  for (i = 0; bigdeps[i]; ++i)
    {
      isl_space* space = isl_map_get_space (bigdeps[i]);
      char* src = (char*)isl_space_get_tuple_name (space, isl_dim_in);
      char* dst = (char*)isl_space_get_tuple_name (space, isl_dim_out);
      for (j = 0; j < pos; ++j)
	if (! strcmp (src, stmts[j]))
	  break;
      if (j == pos)
	stmts[pos++] = src;
      for (j = 0; j < pos; ++j)
	if (! strcmp (dst, stmts[j]))
	  break;
      if (j == pos)
	stmts[pos++] = dst;
    }

  // 3.


  /////////////////////////////////////////////////

  // Build a graph with all statements/dependences.
  s_graph_t* g = ls_graph_alloc ();
  std::map<char*,s_vertex_t*> name_to_vertex = std::map<char*,s_vertex_t*>();
  std::map<s_vertex_t*,char*> vertex_to_name = std::map<s_vertex_t*,char*>();
  std::map<s_vertex_t*,int> vertex_to_uid = std::map<s_vertex_t*,int>();
  std::map<isl_set*,s_vertex_t*> dom_to_vertex = std::map<isl_set*,s_vertex_t*>();
  std::map<isl_map*,s_edge_t*> dependence_to_edge =
    std::map<isl_map*,s_edge_t*> ();

  // Create all nodes.
  for (i = 0; iter_domains[i]; ++i)
    {
      s_vertex_t* v = ls_graph_create_vertex (g, iter_domains[i], i);
      name_to_vertex[stmt_names[i]] = v;
      vertex_to_name[v] = stmt_names[i];
      vertex_to_uid[v] = i;
      dom_to_vertex[iter_domains[i]] = v;
    }

  // Create all vertices.
  for (i = 0; dependences[i]; ++i)
    {
      isl_space* space = isl_map_get_space (dependences[i]);
      char* src = (char*)isl_space_get_tuple_name (space, isl_dim_in);
      char* dst = (char*)isl_space_get_tuple_name (space, isl_dim_out);
      s_edge_t* e = ls_graph_create_edge (g, name_to_vertex[src],
					  name_to_vertex[dst], dependences[i]);
      dependence_to_edge[dependences[i]] = e;
    }

  // ls_graph_print (stdout, g);
  printf ("start scc computation\n");

  // int nb_sccs = ls_graph_compute_scc (g);
  // printf ("print the result\n");
  // ls_graph_print_scc (g, nb_sccs);
  // printf ("print the graph\n");
  // ls_graph_print (stdout, g);

  // First, separate all vertices that are of size > cutoff, and small ones.
  isl_set** bigdoms = XMALLOC(isl_set*, num_doms + 1);
  isl_set** smalldoms = XMALLOC(isl_set*, num_doms + 1);
  int posbig = 0;
  int possmall = 0;
  for (i = 0; iter_domains[i]; ++i)
    if (card_doms[i] > cutoff)
      bigdoms[posbig++] = iter_domains[i];
  else
      smalldoms[possmall++] = iter_domains[i];
  bigdoms[posbig] = NULL;
  smalldoms[possmall] = NULL;

  printf ("number of big vs. small: %d vs. %d\n", posbig, possmall);
  exit (43);
  //
  for (i = 0; smalldoms[i]; ++i)
    {
      s_vertex_t* v = dom_to_vertex[smalldoms[i]];
      // if (v->d_out == 1)
      // 	continue;
      printf ("small: name=%s, size=%.0f\n", vertex_to_name[v],
	      card_doms[vertex_to_uid[v]]);
    }

  exit (42);
/* s_edge_t* */
/* ls_graph_create_edge (s_graph_t* g, */
/* 		      s_vertex_t* src, */
/* 		      s_vertex_t* dst, */
/* 		      void* data); */


  ls_graph_free (g);

}


static
void
heurist_prune_x1(s_program_t* program, int cutoff)
{
  printf ("[PoCC-DDG] DDG before pruning: %d stmts, %d deps\n",
	  program->stmts->size(), program->deps->size());
  //program_print (stdout, program);
  // 1. Remove all statements < cutoff.
  std::vector<s_statement_t*>* prunedstmts = new std::vector<s_statement_t*>();
  std::set<s_statement_t*> deletedstmts;
  int scop_id = 0;
  for (std::vector<s_statement_t*>::iterator ii = program->stmts->begin();
       ii != program->stmts->end(); ++ii)
    {
      if ((*ii)->nb_points > cutoff)
	{
	  (*ii)->scop_id = scop_id++;
	  prunedstmts->push_back (*ii);
	}
      else
	deletedstmts.insert (*ii);
    }
  delete program->stmts;
  program->stmts = prunedstmts;

  // 2. Remove any dependence pointing to a deleted statement, or any
  // too small dependence.
  std::vector<s_dependence_t*>* pruneddeps = new std::vector<s_dependence_t*>();
  for (std::vector<s_dependence_t*>::iterator ii = program->deps->begin();
       ii != program->deps->end(); ++ii)
    {
      if ((*ii)->nb_points < cutoff)
	continue;
      s_statement_t* src = (*program->isl_name_to_stmt)[(*ii)->isl_name_src];
      s_statement_t* dst = (*program->isl_name_to_stmt)[(*ii)->isl_name_dst];
      const char* src_name = src->isl_name;
      const char* dst_name = dst->isl_name;
      int src_ok = 1;
      int dst_ok = 1;
      for (std::set<s_statement_t*>::iterator jj = deletedstmts.begin();
	   jj != deletedstmts.end(); ++jj)
	{
	  // if (! strcmp ((*jj)->isl_name, src_name))
	  if ((*jj)->isl_name == src_name)
	    src_ok = 0;
	  // if (! strcmp ((*jj)->isl_name, dst_name))
	  if ((*jj)->isl_name == dst_name)
	    dst_ok = 0;
	  if (!src_ok || !dst_ok)
	    break;
	}

      // if (deletedstmts.find (src) != deletedstmts.end() &&
      // 	  deletedstmts.find (dst) != deletedstmts.end())
      if (src_ok && dst_ok)
	pruneddeps->push_back (*ii);
    }
  delete program->deps;
  program->deps = pruneddeps;

  // 3. Re-adjust the list of statements from the updated dep. list.
  std::set<s_statement_t*> usedstmts;
  prunedstmts = new std::vector<s_statement_t*>();
  scop_id = 0;
  for (std::vector<s_dependence_t*>::iterator ii = program->deps->begin();
       ii != program->deps->end(); ++ii)
    {
      usedstmts.insert ((*ii)->src);
      usedstmts.insert ((*ii)->dst);
    }
  for (std::vector<s_statement_t*>::iterator ii = program->stmts->begin();
       ii != program->stmts->end(); ++ii)
    {
      if (usedstmts.find (*ii) != usedstmts.end())
	{
	  (*ii)->scop_id = scop_id++;
	  prunedstmts->push_back (*ii);
	}
    }
  delete program->stmts;
  program->stmts = prunedstmts;

  printf ("[PoCC-DDG] DDG after pruning: %d stmts, %d deps\n",
	  program->stmts->size(), program->deps->size());
  //program_print (stdout, program);
}


static
void
prune_ddg(s_program_t* program, int cutoff)
{
  if (! program)
    return;
  printf ("[PoCC-DDG] Start pruning.\n");

  printf ("[PoCC-DDG] Program stats (before pruning):\n");
  //program_print (stdout, program);
  stats_original_program (program);
  printf ("%d statements, %d dependences\n", program->stmts->size(),
	  program->deps->size());

  heurist_prune_x1 (program, cutoff);

  printf ("[PoCC-DDG] Program stats (after pruning):\n");
  stats_original_program (program);
  printf ("%d statements, %d dependences\n", program->stmts->size(),
	  program->deps->size());

   // exit (1);

  // // 2. Apply filtering. Remove a dependence if it represents less
  // // than x points.
  // float cut_threshold = 10.0;
  // int pos = 0;
  // int removed_deps = 0;
  // for (i = 0; dependences[i]; ++i)
  //   {
  //     if (card_deps[i] > cut_threshold || card_deps[i] == 0)
  // 	deps[pos++] = dependences[i];
  //     else
  // 	removed_deps++;
  //   }
  // deps[pos] = NULL;
  // printf ("[PoCC-DDG] Input has %d dependences, %d have less than %d points and are removed.\n", num_deps, removed_deps, cut_threshold);

  // // 3. Compute all stmt names, remove statements not linked by a dep.
  // const char* stmt_names[num_doms + 1];
  // for (i = 0; i < num_doms; ++i)
  //   {
  //     isl_space* space = isl_set_get_space (iter_domains[i]);
  //     stmt_names[i] = isl_space_get_tuple_name (space, isl_dim_set);
  //     doms[i] = NULL;
  //   }
  // stmt_names[i] = NULL;
  // doms[i] == NULL;

  // for (i = 0; deps[i]; ++i)
  //   {
  //     isl_space* space = isl_map_get_space (deps[i]);
  //     const char* src = isl_space_get_tuple_name (space, isl_dim_in);
  //     const char* dst = isl_space_get_tuple_name (space, isl_dim_out);
  //     for (j = 0; stmt_names[j]; ++j)
  // 	if (! strcmp (src, stmt_names[j]))
  // 	  break;
  //     int src_id = j;
  //     for (j = 0; stmt_names[j]; ++j)
  // 	if (! strcmp (dst, stmt_names[j]))
  // 	  break;
  //     int tgt_id = j;
  //     if (src_id == num_doms || tgt_id == num_doms)
  // 	{
  // 	  fprintf (stderr, "Error. This dependence has no matching statement:\n");
  // 	  isl_ctx* ctxt = isl_ctx_alloc();
  // 	  isl_printer* pr = isl_printer_to_file (ctxt, stdout);
  // 	  isl_printer_print_map (pr, deps[i]);
  // 	  printf ("\n");
  // 	  if (src_id == num_doms)
  // 	    fprintf (stderr, "Cannot locate src statement: %s\n", src);
  // 	  if (tgt_id == num_doms)
  // 	    fprintf (stderr, "Cannot locate dst statement: %s\n", dst);
  // 	  exit (1);
  // 	}
  //     doms[src_id] = iter_domains[src_id];
  //     doms[tgt_id] = iter_domains[tgt_id];
  //   }


  // // 4. Finalize.
  // int pos_doms = 0;
  // for (i = 0; i <= num_doms; ++i)
  //   iter_domains[i] = NULL;
  // for (i = 0; i <= num_deps; ++i)
  //   dependences[i] = NULL;
  // for (i = 0; i < num_doms; ++i)
  //   if (doms[i])
  //     iter_domains[pos_doms++] = doms[i];
  // for (i = 0; deps[i]; ++i)
  //   dependences[i] = deps[i];

  // printf ("[PoCC-DDG] Input has %d statements, %d have no dependence and are removed.\n", num_doms, num_doms - pos_doms);
}



static
void
count_sizes_polyhedra(s_program_t* program)
{
  if (! program)
    return;
  printf ("[PoCC-DDG] Count all polyhedra.\n");

  for (std::vector<s_statement_t*>::iterator ii = program->stmts->begin();
       ii != program->stmts->end(); ++ii)
    {
      (*ii)->nb_points = polyfeat_compute_card_isl_set ((*ii)->iter_domain);
    }
  for (std::vector<s_dependence_t*>::iterator ii = program->deps->begin();
       ii != program->deps->end(); ++ii)
    {
      (*ii)->nb_points = polyfeat_compute_card_isl_map ((*ii)->dependence);
      if ((*ii)->nb_points == 0)
	{
	  printf ("Warning: this dependence has 0/no finite number of points:\n");
	  isl_ctx* ctxt = isl_ctx_alloc();
	  isl_printer* pr = isl_printer_to_file (ctxt, stdout);
	  isl_printer_print_map (pr, (*ii)->dependence);
	}
    }
  program->stats_enabled = 1;

  return;
}




static
void
stats_transformed_program(s_program_t* program)
{
  // Read the .aster file, extract statistics.
  FILE* f = fopen (".pocc.aster", "r");
  if (! f)
    {
      printf ("Warning: the .aster file is not generated, cannot compute stats\n");
      return;
    }




  fclose (f);





  // printf ("Total operations: %0.f\n", all_ops);
  // double flops = sum_float + sum_double;
  // printf ("Total FLOPs: %0.f (%.2f%%)\n", flops, (flops/all_ops)*100.0);
  // double mem = sum_load + sum_store;
  // printf ("Total load/store: %0.f (%.2f%%)\n", mem, (mem/all_ops)*100.0);
  // printf ("Total INT: %0.f (%.2f%%)\n", sum_int, (sum_int/all_ops)*100.0);
  // double other = sum_control + sum_other;
  // printf ("Total others: %0.f (%.2f%%)\n", other, (other/all_ops)*100.0);


  //
}


static
scoplib_matrix_p
convert_scevs_to_access_func (scoplib_statement_p s, s_statement_t* pstmt)
{

  // Discard scevs attached to non load/store ops.
  if (pstmt->op_type != POCC_DDG_OPTYPE_LOAD &&
      pstmt->op_type != POCC_DDG_OPTYPE_STORE)
    return scoplib_matrix_malloc (0, s->domain->elt->NbColumns);

  int i, j;

  // Count the number of rows.
  std::vector<isl_map*>::iterator ii;
  int nb_rows = 0;
  for (ii = pstmt->access_func->begin (); ii != pstmt->access_func->end ();
       ++ii)
    {
      isl_map* m = *ii;
      isl_set* s = isl_map_domain (isl_map_copy (m));
      isl_space* space = isl_set_get_space (s);
      int dimensionality = isl_space_dim (space, isl_dim_set);
      nb_rows += dimensionality;
    }

  // Allocate output matrix, and populate.
  scoplib_matrix_p ret =
    scoplib_matrix_malloc (nb_rows, s->domain->elt->NbColumns);

  int row_offset = 0;
  for (ii = pstmt->access_func->begin (); ii != pstmt->access_func->end ();
       ++ii)
    {
      isl_map* m = *ii;
      const char* tmpfilename = "____isl_mat_convert.txt";
      FILE* f = fopen (tmpfilename, "w+");
      if (! f)
	{
	  printf ("Error: cannot create file %s\n", tmpfilename);
	  exit (1);
	}
      isl_ctx* ctxt = isl_ctx_alloc ();
      isl_printer* pr = isl_printer_to_file (ctxt, f);
      pr = isl_printer_set_output_format (pr, ISL_FORMAT_POLYLIB);
      isl_printer_print_map (pr, m);
      rewind (f);
      scoplib_matrix_p mat = scoplib_matrix_read (f);
      fclose (f);

      isl_space* space = isl_map_get_space (m);
      /// LNP: beware!!!! This should always return 1, as we view
      /// accesses as linearized.
      int offset_mat = isl_space_dim (space, isl_dim_out);
      if (row_offset + mat->NbRows >= ret->NbRows)
	{
	  scoplib_matrix_p tmp =
	    scoplib_matrix_malloc (row_offset + mat->NbRows, ret->NbColumns);
	  scoplib_matrix_replace_matrix (tmp, ret, 0);
	  scoplib_matrix_free (ret);
	  ret = tmp;
	}
      SCOPVAL_set_si(ret->p[0 + row_offset][0], row_offset + 1);
      for (i = 0; i < mat->NbRows; ++i)
	{
	  // Skip inequalities.
	  if (SCOPVAL_get_si(mat->p[i][0]) != 0)
	    continue;
	  for (j = 1 + offset_mat; j < mat->NbColumns - 1; ++j)
	    SCOPVAL_assign(ret->p[i + row_offset][j - offset_mat],
			   mat->p[i][j]);
	  SCOPVAL_assign(ret->p[i + row_offset][ret->NbColumns - 1],
			 mat->p[i][j]);
	}
      row_offset += offset_mat;
    }

  return ret;
}


static
void
merge_statements_by_context_and_execblock (s_program_t* program)
{
  int i, j;
  std::map<char*, std::vector<const char* > >::iterator ii;
  std::vector<const char*>::iterator jj;
  std::map<char*, const char*> execblock_to_unique_isl_name;
  std::vector<s_statement_t*> stmts_unique;
  for (ii = program->context_to_isl_name->begin ();
       ii != program->context_to_isl_name->end (); ++ii)
    {
      execblock_to_unique_isl_name.clear ();
      // 1. Populate a map from context/execblock to unique isl_stmt_name
      char* context = ii->first;
      for (jj = ii->second.begin (); jj != ii->second.end (); ++jj)
	{
	  char* stmt_id = (*program->isl_name_to_stmt_id)[*jj];
	  char* execblock = (*program->stmt_id_to_execblock)[stmt_id];
	  if (execblock_to_unique_isl_name.find (execblock) ==
	      execblock_to_unique_isl_name.end ())
	    {
	      execblock_to_unique_isl_name[execblock] = *jj;
	      stmts_unique.push_back
		((*program->isl_name_to_stmt)[*jj]);
	    }
	}
      // 2. Populate the map isl_stmt_name_to_unique_name.
      for (jj = ii->second.begin (); jj != ii->second.end (); ++jj)
	{
	  char* stmt_id = (*program->isl_name_to_stmt_id)[*jj];
	  char* execblock = (*program->stmt_id_to_execblock)[stmt_id];
	  (*program->isl_stmt_name_to_unique_isl_stmt_name)[*jj] =
	    execblock_to_unique_isl_name[execblock];
	}
    }

  // Remove all statements that are useless.
  std::vector<s_statement_t*>* newstmt = new std::vector<s_statement_t*>();
  for (i = 0; i < program->stmts->size (); ++i)
    {
      for (j = 0; j < stmts_unique.size (); ++j)
	if (stmts_unique[j] == (*program->stmts)[i])
	  break;
      if (j < stmts_unique.size ())
	newstmt->push_back ((*program->stmts)[i]);
    }
  delete program->stmts;
  program->stmts = newstmt;
}



static
s_program_t*
build_scoplib_from_islrep(FILE* input_file, char* input_filename,
			  int cutoff, int enable_stats,
			  int enable_merge_stmt)
{
  s_program_t* program = XMALLOC(s_program_t, 1);
  program->stmts = new std::vector<s_statement_t*>();
  program->deps = new std::vector<s_dependence_t*>();
  program->isl_name_to_stmt = new std::map<const char*,s_statement_t*>();
  program->isl_set_to_stmt = new std::map<isl_set*,s_statement_t*>();
  program->isl_dep_to_dep = new std::map<isl_map*,s_dependence_t*>();
  program->access_funcs = new std::vector<isl_map*>();
  program->scevname_to_stmt = new std::map<const char*,s_statement_t*>();
  program->stats_enabled = 0;

  program->isl_name_to_context = new std::map<const char*, char*>();
  program->isl_name_to_stmt_id = new std::map<const char*, char*>();
  program->context_to_isl_name =
    new std::map<char*, std::vector<const char*> >();
  program->stmt_id_to_isl_name =
    new std::map<char*, std::vector<const char*> >();
  program->stmt_id_to_execblock = new std::map<const char*, char*>();
  program->execblock_to_stmt_id = new std::map<char*, std::vector<char*> >();
  program->isl_stmt_name_to_unique_isl_stmt_name =
    new std::map<const char*, const char*>();


  //printf ("input filename=%s\n", input_filename);
  char* filename_annot = XMALLOC(char, 32792);
  isl_ctx* ctxt = isl_ctx_alloc();
  isl_map** maps = XMALLOC(isl_map*, MAX_NUM_ENTRIES_DDG);
  char* buffer = NULL;
  size_t buffer_size = 32768;
  int idx = 0;
  while (read_line (input_file, &buffer_size, &buffer))
    {
      // Adjust statement names: , -> _, " ->
      adjust_names (buffer);
      if (*buffer == '\n' || *buffer == '\0')
	continue;
      isl_map* m = isl_map_read_from_str (ctxt, buffer);
      if (m == NULL)
	{
	  printf ("Error: incorrect ISL formatting for entry:\n%s\n", buffer);
	  exit (1);
	}
      if (idx < MAX_NUM_ENTRIES_DDG)
	maps[idx++] = m;
      else
	{
	  printf ("Error in driver-ddganalyze.c: too many maps (max: MAX_NUM_ENTRIES_DDG)\n");
	  exit (1);
	}
    }


  // Maps have been read. Count how many statements, i.e., domains,
  // i.e., maps with empty domain.
  int i, j;
  int stmt_num = 0;
  int dep_num = 0;
  isl_set** iter_domains = XMALLOC(isl_set*, MAX_NUM_ENTRIES_DDG);
  isl_map** dependences = XMALLOC(isl_map*, MAX_NUM_ENTRIES_DDG);
  int nb_parameters = -1;
  int dimensionality = 0;
  isl_map* m;
  isl_set* s;
  isl_space* space;
  for (i = 0; i < idx; ++i)
    {
      m = maps[i];
      s = isl_map_domain (isl_map_copy (m));
      space = isl_set_get_space (s);
      dimensionality = isl_space_dim (space, isl_dim_set);
      if (nb_parameters == -1)
	nb_parameters = isl_space_dim (space, isl_dim_param);
      if (dimensionality == 0)
	{
	  ///XXX
	  iter_domains[stmt_num++] = isl_map_range (isl_map_copy (m));

	  /// New.
	  s_statement_t* stmt = XMALLOC(s_statement_t, 1);
	  stmt->iter_domain = isl_map_range (isl_map_copy (m));
	  isl_space* sp = isl_set_get_space (stmt->iter_domain);
	  stmt->isl_name = isl_space_get_tuple_name (sp, isl_dim_set);
	  stmt->adjusted_name = adjust_names (strdup (stmt->isl_name));
	  stmt->scop_id = program->stmts->size ();
	  stmt->loop_depth = isl_space_dim (sp, isl_dim_set);
	  stmt->type = NULL; stmt->op_type = -1; stmt->nb_points = -1;
	  stmt->access_func = new std::vector<isl_map*>();
	  program->stmts->push_back (stmt);
	  (*(program->isl_name_to_stmt))[stmt->isl_name] = stmt;
	  (*(program->isl_set_to_stmt))[stmt->iter_domain] = stmt;
	}
    }

  if (enable_merge_stmt)
    {
      // Compute some useful maps, prior to pruning/coalescing statements.
      compute_stmt_to_context_map (program);

      // Load statement type annotation files.
      strcpy (filename_annot, input_filename);
      strcat (filename_annot, ".annots.execblocks");
      annotation_execblock_loader (filename_annot, program);

      // Coalesce statements by context + execution block.
      merge_statements_by_context_and_execblock (program);
    }
  else
    {
      // Still create the unique map, but with a one-to-one
      // correspondance.
      for (std::vector<s_statement_t*>::iterator kk =
	     program->stmts->begin (); kk != program->stmts->end (); ++kk)
	(*program->isl_stmt_name_to_unique_isl_stmt_name)[(*kk)->isl_name] =
	  (*kk)->isl_name;
    }

  // Now process dependences.
  for (i = 0; i < idx; ++i)
    {
      m = maps[i];
      s = isl_map_domain (isl_map_copy (m));
      space = isl_set_get_space (s);
      dimensionality = isl_space_dim (space, isl_dim_set);
      if (nb_parameters == -1)
	nb_parameters = isl_space_dim (space, isl_dim_param);

      if (dimensionality == 0)
	continue;
      // Check if it is a SCEV node.
      const char* src_name =
	isl_space_get_tuple_name (isl_map_get_space (m), isl_dim_in);
      const char* dst_name =
	isl_space_get_tuple_name (isl_map_get_space (m), isl_dim_out);
      if (! strncmp (src_name, "SCEV_", 5))
	{
	  // printf ("discard scev node:\n");
	  // isl_ctx* ctxt = isl_ctx_alloc ();
	  // isl_printer* pr = isl_printer_to_file (ctxt, stdout);
	  // isl_printer_print_map (pr, m);
	  // printf ("\n");

	  if (dst_name != NULL)
	    {
	      // This is the scev -> statement info.
	      if (program->isl_name_to_stmt->find (dst_name) !=
		  program->isl_name_to_stmt->end ())
		(*program->scevname_to_stmt)[src_name] =
		  (*program->isl_name_to_stmt)[dst_name];
	    }
	  else
	    {
	      program->access_funcs->push_back (isl_map_copy (m));
	    }
	  continue;
	}

      dependences[dep_num++] = isl_map_copy (m);
      /// New.
      s_dependence_t* dep = XMALLOC(s_dependence_t, 1);
      dep->dependence = isl_map_copy (m);
      isl_space* sp = isl_map_get_space (dep->dependence);
      dep->isl_name_src = STMT_UN(isl_space_get_tuple_name (sp, isl_dim_in));
      dep->isl_name_dst = STMT_UN(isl_space_get_tuple_name (sp, isl_dim_out));
      dep->adjusted_name_src = dep->adjusted_name_dst = NULL;
      dep->nb_points = -1;
      program->deps->push_back (dep);
      (*(program->isl_dep_to_dep))[dep->dependence] = dep;
    }

  XFREE(maps);
  iter_domains[stmt_num] = NULL;
  dependences[dep_num] = NULL;

  // Update the access function info.
  for (std::vector<isl_map*>::iterator ii = program->access_funcs->begin ();
       ii != program->access_funcs->end (); ++ii)
    {
      isl_map* m = *ii;
      const char* src_name =
	isl_space_get_tuple_name (isl_map_get_space (m), isl_dim_in);
      const char* dst_name =
	isl_space_get_tuple_name (isl_map_get_space (m), isl_dim_out);

      if (program->scevname_to_stmt->find (src_name) !=
	  program->scevname_to_stmt->end ())
	(*program->scevname_to_stmt)[src_name]->access_func->push_back (isl_map_copy (m));
      else
	{
	  printf ("Error! Attempting to locate scev -> statement id relation for %s[...] -> %s[...] but no such relation with %s[...] as output!\n",
		  src_name, dst_name, src_name);
	  // printf ("ERROR! (map is of size %d)\n",
	  // 	  program->scevname_to_stmt->size ());
	  exit (1);
	}
    }

  // Update the dep. info.
  for (std::vector<s_dependence_t*>::iterator ii = program->deps->begin();
       ii != program->deps->end(); ++ii)
    {
      (*ii)->src = (*(program->isl_name_to_stmt))[STMT_UN((*ii)->isl_name_src)];
      (*ii)->dst = (*(program->isl_name_to_stmt))[STMT_UN((*ii)->isl_name_dst)];
    }

  // Load statement type annotation files.
  strcpy (filename_annot, input_filename);
  strcat (filename_annot, ".annots.clean");
  annotation_loader (filename_annot, program);
  XFREE(filename_annot);

  if (enable_stats || cutoff > 1)
    count_sizes_polyhedra (program);

  if (cutoff > 1)
    // prune_ddg (iter_domains, dependences, program, cutoff);
    prune_ddg (program, cutoff);

  stmt_num = program->stmts->size();
  dep_num = program->deps->size();

  // We are good, now build the scoplib.
  scoplib_scop_p scop = scoplib_scop_malloc ();
  scop->context = scoplib_matrix_malloc (0, 2 + nb_parameters);
  scop->nb_parameters = nb_parameters;
  for (i = 0; i < nb_parameters; ++i)
    {
      // Assume we have 1 iteration domain at least...
      if (stmt_num == 0)
	break;
      isl_space* space = isl_set_get_space (iter_domains[0]);
      scop->parameters = XMALLOC(char*, nb_parameters);
      for (j = 0; j < nb_parameters; ++j)
	scop->parameters[j] =
	  strdup (isl_space_get_dim_name (space, isl_dim_param, j));
    }
  // Build all statements.
  scoplib_statement_p stmt = NULL;
  scoplib_statement_p curr_elt = NULL;
  scoplib_statement_p* curr = &curr_elt;

  for (i = 0; i < stmt_num; ++i)
    {
      isl_set* iter_domain = (*program->stmts)[i]->iter_domain;
      scoplib_statement_p s = scoplib_statement_malloc ();
      s->domain = convert_islset_to_domain (ctxt, iter_domain);
      isl_space* space = isl_set_get_space (iter_domain);
      s->nb_iterators = isl_space_dim (space, isl_dim_set);
      s->iterators = XMALLOC(char*, s->nb_iterators);
      for (j = 0; j < s->nb_iterators; ++j)
	s->iterators[j] =
	  strdup (isl_space_get_dim_name (space, isl_dim_set, j));
      const char* stmt_name = isl_space_get_tuple_name (space, isl_dim_set);
      sprintf (buffer, "%s(", stmt_name);
      char buff_args[256];
      for (j = 0; j < s->nb_iterators; ++j)
	{
	  sprintf (buff_args, "%s", s->iterators[j]);
	  strcat (buffer, buff_args);
	  if (j < s->nb_iterators - 1)
	    strcat (buffer, ",");
	}
      strcat (buffer, "); // @");
      s_statement_t* pstmt = (*(program->isl_name_to_stmt))[STMT_UN(stmt_name)];
      if (pstmt)
	{
	  if (pstmt->type)
	    strcat (buffer, pstmt->type);
	  else
	    strcat (buffer, "NOTYPE");
	}
      strcat (buffer, "@ ");
      s->body = strdup (buffer);
      s->usr = pstmt;
      s->read = convert_scevs_to_access_func (s, pstmt);

      if (stmt == NULL)
	*curr = stmt = s;
      else
	{
	  (*curr)->next = s;
	  curr = &((*curr)->next);
	}
    }
  scop->statement = stmt;

  // Build all dependences.
  int* indices[stmt_num];
  int unique_loop_index = 1;
  curr_elt = stmt;
  for (i = 0; i < stmt_num; ++i)
    {
      indices[i] = XMALLOC(int, curr_elt->nb_iterators);
      for (j = 0; j < curr_elt->nb_iterators; ++j)
	indices[i][j] = unique_loop_index++;
      curr_elt = curr_elt->next;
    }
  CandlProgram* cprogram = candl_program_convert_scop (scop, (int**)indices);
  CandlDependence* deps =
    convert_isl_deps (scop, cprogram, program);

  // 3. Embed the dependences in the scop.
  candl_dependence_update_scop_with_deps (scop, deps);

  // Be clean.
  for (i = 0; i < stmt_num; ++i)
    XFREE(indices[i]);
  XFREE(dependences);
  XFREE(iter_domains);

  program->scop = scop;

  return program;
}


static
void
parameterize_scop(scoplib_scop_p scop, int slack)
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
      // Iteration domain.
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

      // Access functions.
      scoplib_matrix_p newread =
	scoplib_matrix_malloc (stm->read->NbRows,
			       stm->read->NbColumns + num_params);
      for (i = 0; i < stm->read->NbRows; ++i)
	{
	  for (j = 0; j < stm->read->NbColumns - 1; ++j)
	    SCOPVAL_assign(newread->p[i][j], stm->read->p[i][j]);
	  SCOPVAL_assign(newread->p[i][newread->NbColumns - 1],
			 stm->read->p[i][j]);
	}
      scoplib_matrix_free (stm->read);
      stm->read = newread;
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
      printf ("[PoCC][ddg-analyze] Create parameter N%d=%d\n", i, parameters[i]);
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

static
int
driver_ponos (scoplib_scop_p program,
	      s_pocc_options_t* poptions,
	      s_pocc_utils_options_t* puoptions)
{
  if (! poptions->quiet)
    printf ("[PoCC] Running Ponos\n");

  s_ponos_options_t* popts = ponos_options_malloc ();
  popts->debug = poptions->ponos_debug;
  popts->build_2d_plus_one = poptions->ponos_build_2d_plus_one;
  popts->maxscale_solver = poptions->ponos_maxscale_solver;
  popts->noredundancy_solver = poptions->ponos_noredundancy_solver;
  popts->legality_constant = poptions->ponos_legality_constant_K;
  popts->schedule_bound = poptions->ponos_schedule_bound;
  popts->schedule_size = poptions->ponos_schedule_dim;
  popts->solver = poptions->ponos_solver_type;
  popts->solver_precond = poptions->ponos_solver_precond;
  popts->quiet = poptions->ponos_quiet;
  popts->schedule_coefs_are_pos = poptions->ponos_coef_are_pos;
  popts->objective = poptions->ponos_objective;
  popts->pipsolve_lp = poptions->ponos_pipsolve_lp;
  popts->pipsolve_gmp = poptions->ponos_pipsolve_gmp;
  popts->output_file_template = strdup (poptions->output_file_name);
  popts->legal_space_normalization = poptions->ponos_solver_precond;

  if (popts->output_file_template)
    {
      int len = strlen (popts->output_file_template);
      if (len >= 2)
	popts->output_file_template[len - 2] = '\0';
    }
  int i, j;
  for (i = 0; poptions->ponos_objective_list[i] != -1; ++i)
    popts->objective_list[i] = poptions->ponos_objective_list[i];
  popts->objective_list[i] = -1;
  // Candl options: candl is called in ponos for the moment.
  /// FIXME: enable reading of external deps.
  popts->candl_deps_isl_simplify = poptions->candl_deps_isl_simplify;
  popts->candl_deps_prune_transcover = poptions->candl_deps_prune_transcover;


  // Prepare candl data structures.
  int num_stmt = scoplib_statement_number (program->statement);
  candl_program_p candl_program;
  int* indices[num_stmt];
  int unique_loop_index = 1;
  scoplib_statement_p curr_elt = program->statement;
  for (i = 0; i < num_stmt; ++i)
    {
      indices[i] = XMALLOC(int, curr_elt->nb_iterators);
      for (j = 0; j < curr_elt->nb_iterators; ++j)
	indices[i][j] = unique_loop_index++;
      curr_elt = curr_elt->next;
    }
  candl_program = candl_program_convert_scop (program, (int**)indices);
  for (i = 0; i < num_stmt; ++i)
    free (indices[i]);
  CandlDependence* candl_deps =
    candl_dependence_read_from_scop (program, candl_program);

  // Call the scheduler.
  ponos_scheduler_from_deplist (program, candl_program, candl_deps, popts);


  // Dump the scop file, if needed.
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

  // Be clean.
  ponos_options_free (popts);
  candl_dependence_free (candl_deps);
  candl_program_free (candl_program);

  return EXIT_SUCCESS;
}



/**
 * Entry point.
 *
 *
 */
void
pocc_driver_ddganalyze (FILE* input_file,
			s_pocc_options_t* poptions,
			s_pocc_utils_options_t* puoptions)
{
  int debug = 0;
  int i;
  printf ("[PoCC] Start DDG-analyze pass\n");
  if (input_file == NULL)
    pocc_error ("Invalid input file (cannot be opened)\n");

  // 1. Read the scop.
  s_program_t* program = build_scoplib_from_islrep
    (input_file, poptions->input_file_name,
     poptions->ddg_analyze_prune_cutoff, poptions->ddg_analyze_stats,
     poptions->ddg_merge_statements);
  scoplib_scop_p scop = program->scop;
  // 2. Parameterize it.
  parameterize_scop (scop, poptions->ddg_analyze_parameterize_slack);

  // Pretty-print the scop, for debugging.
  if (debug)
    scoplib_scop_print_dot_scop (stdout, scop);

  // Pretty-print the dependences, for debugging.
  if (debug)
    {
      char* strdeps = scoplib_scop_tag_content(scop, "<dependence-polyhedra>",
					       "</dependence-polyhedra>");
      printf ("DEPS: \n%s\n", strdeps);
      free (strdeps);
    }

  // Compute stats on original program.
  if (poptions->ddg_analyze_stats)
    stats_original_program (program);

  // Call pluto, instructing to use dependences embedded in the
  // scop file.
  poptions->pluto_external_candl = 1;
  if (poptions->pluto)
    pocc_driver_pluto (scop, poptions, puoptions);
  else if (poptions->ponos)
    driver_ponos (scop, poptions, puoptions);
  else
    printf ("No scheduler selected, no transfo performed.\n");

  // Populate the program info structure w/ iteration domain sizes.
  s_polyfeat_program_info_t* progstats =
    polyfeat_program_info_malloc ();
  progstats->domains_size = XMALLOC(double, program->stmts->size ());
  progstats->total_operations = 0;
  progstats->scop = scop;
  for (i = 0; i < program->stmts->size (); ++i)
    {
      s_statement_t* s = (*(program->stmts))[i];
      progstats->domains_size[i] = s->nb_points;
      progstats->total_operations += s->nb_points;
    }
  progstats->total_load_ops = program->sum_load;
  progstats->total_store_ops = program->sum_store;

  /// Call stats on the produced schedule.
  if (poptions->ddg_analyze_stats)
    polyfeat_ast_stats_ddg (progstats, poptions->verbose);

  // Generate code using CLooG.
  /* scoplib_scop_print (stdout, scop); */
  if (poptions->codegen)
    pocc_driver_codegen (scop, poptions, puoptions);

  // Be clean.
  scoplib_scop_free (scop);
  /* candl_program_free (cprogram); */

  printf ("[PoCC] DDG-analyze pass: done\n");
}
