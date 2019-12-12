/*
 * options.h: this file is part of the PoCC project.
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
#ifndef POCC_SRC_OPTIONS_H
# define POCC_SRC_OPTIONS_H

# include <stdio.h>

# if HAVE_CONFIG_H
#  include <pocc-utils/config.h>
# endif

# include <pocc/options.h>
# include "getopts.h"

# ifdef POCC_DEVEL_MODE
#  define POCC_NB_OPTS				137
# else
//#  define POCC_NB_OPTS				109
#  define POCC_NB_OPTS				(109+21)
# endif

# define POCC_OPT_HELP				0
# define POCC_OPT_VERSION			1
# define POCC_OPT_OUTFILE			2
# define POCC_OPT_OUTFILE_SCOP			3
# define POCC_OPT_CLOOGIFY_SCHED       		4
# define POCC_OPT_BOUNDED_CTXT	       		5
# define POCC_OPT_DEFAULT_CTXT	       		6
# define POCC_OPT_USER_CTXT	       		7
# define POCC_OPT_INSCOP_FAKEARRAY		8
# define POCC_OPT_READ_SCOP_FILE		9

# define POCC_OPT_NO_CANDL			10
# define POCC_OPT_CANDL_DEP_ISL_SIMP		11
# define POCC_OPT_CANDL_DEP_PRUNE_DUPS		12

# define POCC_OPT_POLYFEAT			13
# define POCC_OPT_POLYFEAT_LEGACY    		14
# define POCC_OPT_POLYFEAT_RAR			15
# define POCC_OPT_POLYFEAT_LINESIZE    		16
# define POCC_OPT_POLYFEAT_CACHESIZE   		17
# define POCC_OPT_POLYFEAT_CACHE_IS_PRIV  	18
# define POCC_OPT_POLYFEAT_MULTI_PARAMS   	19

# define POCC_OPT_TRASH				20
# define POCC_OPT_VERBOSE			21
# define POCC_OPT_QUIET				22

# define POCC_OPT_LETSEE			23
# define POCC_OPT_LETSEE_SEARCHSPACE		24
# define POCC_OPT_LETSEE_TRAVERSAL		25
# define POCC_OPT_LETSEE_DRY_RUN          	26
# define POCC_OPT_LETSEE_NORMSPACE		27
# define POCC_OPT_LETSEE_BOUNDS			28
# define POCC_OPT_LETSEE_SCHEME_M1		29
# define POCC_OPT_LETSEE_RTRIES			30
# define POCC_OPT_LETSEE_PRUNE_PRECUT		31
# define POCC_OPT_LETSEE_BACKTRACK_MULTI	32

# define POCC_OPT_PLUTO				33
# define POCC_OPT_PLUTO_PARALLEL		34
# define POCC_OPT_PLUTO_TILE			35
# define POCC_OPT_PLUTO_L2TILE			36
# define POCC_OPT_PLUTO_FUSE			37
# define POCC_OPT_PLUTO_UNROLL			38
# define POCC_OPT_PLUTO_UFACTOR			39
# define POCC_OPT_PLUTO_POLYUNROLL		40
# define POCC_OPT_PLUTO_PREVECTOR		41
# define POCC_OPT_PLUTO_MULTIPIPE		42
# define POCC_OPT_PLUTO_RAR			43
# define POCC_OPT_PLUTO_RAR_CF			44
# define POCC_OPT_PLUTO_LASTWRITER		45
# define POCC_OPT_PLUTO_SCALPRIV		46
# define POCC_OPT_PLUTO_BEE			47
# define POCC_OPT_PLUTO_QUIET			48
# define POCC_OPT_PLUTO_FT			49
# define POCC_OPT_PLUTO_LT			50
# define POCC_OPT_PLUTO_EXTERNAL_CANDL	       	51
# define POCC_OPT_PLUTO_TILING_IN_SCATT	       	52
# define POCC_OPT_PLUTO_BOUND_COEF	       	53

# define POCC_OPT_NOCODEGEN			54
# define POCC_OPT_CLOOG_F			55
# define POCC_OPT_CLOOG_L			56
# define POCC_OPT_PRINT_CLOOG_FILE		57
# define POCC_OPT_NO_PAST			58
# define POCC_OPT_PAST_OPTIMIZE_LOOP_BOUND	59
# define POCC_OPT_PRAGMATIZER			60

# define POCC_OPT_PTILE				61
# define POCC_OPT_PTILE_FTS    			62
# define POCC_OPT_PTILE_LEVEL			63
# define POCC_OPT_PTILE_EVOLVETILE    		64

# define POCC_OPT_PUNROLL			65
# define POCC_OPT_PUNROLL_AND_JAM      		66
# define POCC_OPT_PUNROLL_SIZE      		67

# define POCC_OPT_VECTORIZER			68
# define POCC_OPT_VECTORIZER_SIMDIZE   		69

# define POCC_OPT_CODEGEN_TIMERCODE		70
# define POCC_OPT_CODEGEN_TIMER_ASM		71
# define POCC_OPT_CODEGEN_TIMER_PAPI		72
# define POCC_OPT_CODEGEN_DINERO		73


# define POCC_OPT_COMPILE			74
# define POCC_OPT_COMPILE_CMD			75
# define POCC_OPT_RUN_CMD_ARGS          	76
# define POCC_OPT_PROGRAM_TIMEOUT        	77

# define POCC_OPT_PONOS				78
# define POCC_OPT_PONOS_QUIET	        	79
# define POCC_OPT_PONOS_DEBUG	        	80
# define POCC_OPT_PONOS_SCHED_DIMENSION		81
# define POCC_OPT_PONOS_COEF_ARE_POS	        82
# define POCC_OPT_PONOS_BUILD_2DP1	        83
# define POCC_OPT_PONOS_SOLVER_TYPE	        84
# define POCC_OPT_PONOS_SOLVER_PRECOND	        85
# define POCC_OPT_PONOS_MAXSCALE_SOLVER		86
# define POCC_OPT_PONOS_NOREDUNDANCY_SOLVER	87
# define POCC_OPT_PONOS_LEGALITY_CONSTANT_K	88
# define POCC_OPT_PONOS_SCHED_COEF_BOUND	89
# define POCC_OPT_PONOS_OBJECTIVE		90
# define POCC_OPT_PONOS_OBJECTIVE_LIST		91
# define POCC_OPT_PONOS_PIPSOLVE_LP		92
# define POCC_OPT_PONOS_PIPSOLVE_GMP		93

# define POCC_OPT_PAST_SUPER_OPT_LOOP_BOUND	94
# define POCC_OPT_READ_CLOOG_FILE		95

# define POCC_OPT_PSIMDKZER			96
# define POCC_OPT_PSIMDKZER_TARGET		97
# define POCC_OPT_PSIMDKZER_VECTOR_ISA		98
# define POCC_OPT_PSIMDKZER_SCALAR_DATATYPE	99
# define POCC_OPT_PSIMDKZER_RESKEW		100
# define POCC_OPT_PSIMDKZER_GEN_POLYBENCH_SCRIPT	101

# define POCC_OPT_SIMD_LENGTH			102
# define POCC_OPT_SIMD_FMA			103
# define POCC_OPT_ELEMENT_SIZE_IN_BYTES		104
# define POCC_OPT_NUM_OMP_THREADS		105

# define POCC_OPT_OUTPUT_APPROXAST_SCOP       	106
# define POCC_OPT_ASTER_OUTPUT		       	107
# define POCC_OPT_INTERPRETER		       	108

# define POCC_OPT_CHUNKED_START 97
# define POCC_OPT_PONOS_CHUNKED               (POCC_OPT_CHUNKED_START+12)
# define POCC_OPT_PONOS_CHUNKED_MAX_FUSION    (POCC_OPT_CHUNKED_START+13)
# define POCC_OPT_PONOS_CHUNKED_MIN_FUSION    (POCC_OPT_CHUNKED_START+14)
# define POCC_OPT_PONOS_CHUNKED_LOOP_MAX_STMT (POCC_OPT_CHUNKED_START+15)
# define POCC_OPT_PONOS_CHUNKED_LOOP_MAX_REF  (POCC_OPT_CHUNKED_START+16)
# define POCC_OPT_PONOS_CHUNKED_LOOP_MAX_LAT  (POCC_OPT_CHUNKED_START+17)
# define POCC_OPT_PONOS_CHUNKED_ARCH          (POCC_OPT_CHUNKED_START+18)
# define POCC_OPT_PONOS_CHUNKED_ARCH_FILE     (POCC_OPT_CHUNKED_START+19)
# define POCC_OPT_PONOS_CHUNKED_FP_PRECISION  (POCC_OPT_CHUNKED_START+20)
# define POCC_OPT_PONOS_CHUNKED_AUTO          (POCC_OPT_CHUNKED_START+21)
# define POCC_OPT_PONOS_CHUNKED_READ_OLIST    (POCC_OPT_CHUNKED_START+22)
# define POCC_OPT_PONOS_PIPES_DF_PROGRAM      (POCC_OPT_CHUNKED_START+23)
# define POCC_OPT_PONOS_PIPES_C_PROGRAM       (POCC_OPT_CHUNKED_START+24)
# define POCC_OPT_PONOS_PIPES_C_HARNESS       (POCC_OPT_CHUNKED_START+25)
# define POCC_OPT_PONOS_PIPES_T_SCHEDULE      (POCC_OPT_CHUNKED_START+26)
# define POCC_OPT_PONOS_PIPES_P_SCHEDULE      (POCC_OPT_CHUNKED_START+27)
# define POCC_OPT_PONOS_MO_SPACE              (POCC_OPT_CHUNKED_START+28)
# define POCC_OPT_PONOS_CHUNKED_UNROLL        (POCC_OPT_CHUNKED_START+29)
# define POCC_OPT_PONOS_CHUNKED_ANNOTATE_UNROLL        (POCC_OPT_CHUNKED_START+30)
# define POCC_OPT_PONOS_CHUNKED_ADAPTIVE_ASSEMBLY     (POCC_OPT_CHUNKED_START+31)
# define POCC_OPT_PONOS_CHUNKED_MDT_DB_FILENAME   (POCC_OPT_CHUNKED_START+32)

# ifdef POCC_DEVEL_MODE
#  define POCC_OPT_STD_LAST_ID 108
/* Devel-only options. */
#  define POCC_OPT_VECT_MARK_PAR_LOOPS    	(POCC_OPT_STD_LAST_ID+1)
#  define POCC_OPT_VECT_NO_KEEP_OUTER_PAR_LOOPS (POCC_OPT_STD_LAST_ID+2)
#  define POCC_OPT_VECT_SINK_ALL_LOOPS		(POCC_OPT_STD_LAST_ID+3)
#  define POCC_OPT_STORCOMPACT			(POCC_OPT_STD_LAST_ID+4)
#  define POCC_OPT_AC_KEEP_OUTERPAR		(POCC_OPT_STD_LAST_ID+5)
#  define POCC_OPT_AC_KEEP_VECTORIZED		(POCC_OPT_STD_LAST_ID+6)
#  define POCC_OPT_CDSCGR			(POCC_OPT_STD_LAST_ID+7)
#  define POCC_OPT_DDGANALYZE			(POCC_OPT_STD_LAST_ID+8)
#  define POCC_OPT_DDGANALYZE_PARAMETERIZE	(POCC_OPT_STD_LAST_ID+9)
#  define POCC_OPT_DDGANALYZE_PRUNE		(POCC_OPT_STD_LAST_ID+10)
#  define POCC_OPT_DDGANALYZE_STATS		(POCC_OPT_STD_LAST_ID+11)
#endif

BEGIN_C_DECLS

extern
int
pocc_getopts(s_pocc_options_t* options, int argc, char** argv);

END_C_DECLS

#endif // POCC_SRC_OPTIONS_H