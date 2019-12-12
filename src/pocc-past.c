#include <pocc/pocc-driver-past.h>
#include <past/pprint.h>



static
s_past_node_t* test1 ()
{
  // symbols to be used:
  s_symbol_t* i;
  s_symbol_t* j;
  s_symbol_t* k;
  s_symbol_t* A;
  s_symbol_t* B;
  s_symbol_t* C;
  s_symbol_t* N;
  s_symbol_t* M;
  s_symbol_t* typeint;

  // Constants to be used.
  s_past_node_t* mone;
  s_past_node_t* zero;
  s_past_node_t* one;

  s_symbol_table_t* symt = symbol_table_malloc ();
  i = symbol_add_from_char (symt, "i");
  j = symbol_add_from_char (symt, "j");
  k = symbol_add_from_char (symt, "k");
  A = symbol_add_from_char (symt, "A");
  B = symbol_add_from_char (symt, "B");
  C = symbol_add_from_char (symt, "C");
  N = symbol_add_from_char (symt, "N");
  M = symbol_add_from_char (symt, "M");
  typeint = symbol_add_from_char (symt, "int");
  mone = past_node_value_create_from_int (-1);
  zero = past_node_value_create_from_int (0);
  one = past_node_value_create_from_int (1);

  // build this:
  /* for (i = 0; i < N; ++i) { */
  /*   for (j = 1; j < M; ++j) */
  /*     A[i][j] += B[i]; */
  /*   for (j = 1; j < M; ++j) */
  /*     A[i-1][j] += B[i+M]; */
  /* } */

  // Build B[i]
  s_past_node_t* arrefB =
    past_node_binary_create (past_arrayref,
			     past_node_varref_create (B),
			     past_node_varref_create (i));
  // Build A[i][j]
  s_past_node_t* arrefA =
    past_node_binary_create
    (past_arrayref,
     past_node_binary_create
     (past_arrayref,
      past_node_varref_create (A),
      past_node_varref_create (i)),
     past_node_varref_create (j));

  // Build A[i][j] += B[i];
  s_past_node_t* stmt =
    past_node_statement_create
    (past_node_binary_create (past_addassign, arrefA, arrefB));

  // Build B[i+M]
  s_past_node_t* arrefB2 =
    past_node_binary_create (past_arrayref,
			     past_node_varref_create (B),
			     past_node_binary_create
			     (past_add,
			      past_node_varref_create (i),
			      past_node_varref_create (M)));
  // Build A[i-1][j]
  s_past_node_t* arrefA2 =
    past_node_binary_create
    (past_arrayref,
     past_node_binary_create
     (past_arrayref,
      past_node_varref_create (A),
      past_node_binary_create
      (past_sub,
       past_node_varref_create (i),
       past_node_value_create_from_int (1))),
     past_node_varref_create (j));

  // Build A[i-1][j] += B[i+M];
  s_past_node_t* stmt2 =
    past_node_statement_create
    (past_node_binary_create (past_addassign, arrefA2, arrefB2));


  // Build ++i
  s_past_node_t* inc =
    past_node_unary_create (past_inc_before,
			    past_node_varref_create (i));

  // Build i < N
  s_past_node_t* test =
    past_node_binary_create (past_lt,
			     past_node_varref_create (i),
			     past_node_varref_create (N));

  // Build i = 0
  s_past_node_t* init =
    past_node_binary_create (past_assign,
			     past_node_varref_create (i),
			     past_clone (zero));


  // Build ++j
  s_past_node_t* inc2 =
    past_node_unary_create (past_inc_before,
			    past_node_varref_create (j));

  // Build j < M
  s_past_node_t* test2 =
    past_node_binary_create (past_lt,
			     past_node_varref_create (j),
			     past_node_varref_create (M));

  // Build j = 1
  s_past_node_t* init2 =
    past_node_binary_create (past_assign,
			     past_node_varref_create (j),
			     past_clone (one));


  // Build program, encapsulate things in basic blocks.
  s_past_node_t* bb = past_node_block_create (stmt);
  s_past_node_t* forloopj =
    past_node_for_create (init2, test2, j,
			  inc2, bb);
  s_past_node_t* forloopj2 =
    past_node_for_create (past_clone (init2), past_clone (test2), j,
			  past_clone (inc2),
			  past_node_block_create (stmt2));
  forloopj->next = forloopj2;

  s_past_node_t* forloopi =
    past_node_for_create (init, test, i,
			  inc, forloopj);
  s_past_node_t* prog =
    past_node_block_create (forloopi);

  // Encapsulate in a fundef.
  s_past_node_t* fundef =
    past_node_fundecl_create
    (past_node_type_create (past_node_varref_create (typeint)),
     past_node_varref_create (symbol_add_from_char (symt, "foo")),
     NULL,
     prog);

  s_past_node_t* root = past_node_root_create (symt, fundef);

  return root;
}


int main() {
  printf ("[START TEST]\n");
  s_past_node_t* input = test1 ();
  past_pprint (stdout, input);
  s_pocc_options_t* options = pocc_options_malloc ();
  options->verbose = 1;
  options->pluto = 1;
  options->pluto_tile = 1;
  options->cloog_f = 100;
  s_past_node_t* ret = pocc_past2past (input, options);
  past_pprint (stdout, ret);
  past_deep_free (ret);
  printf ("[DONE TEST]\n");
}
