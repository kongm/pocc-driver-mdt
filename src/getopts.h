/*
 * getopts.h: this file is part of the PoCC project.
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
#ifndef POCC_GETOPTS_H
# define POCC_GETOPTS_H

# include <pocc/common.h>

BEGIN_C_DECLS

/*
** use short_opt to put the short descriptor of the option
** use long_opt to put the long descriptor of the option
** set expect_arg to 1 if the option expects an argument, 0 otherwise
** use description to put the desciption shown with -h option
*/
struct			s_opt
{
  char			short_opt;
  char			*long_opt;
  int			expect_arg;
  char			*description;
  char			*status;
};

struct			s_option
{
  const struct s_opt	*opts;
  char			*errors[3];
  int			nb_opts;
  int			error_print_active;
};

/*
** Here is a sample use for getopts use:
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
** static const struct s_opt	opts[4] =
**  {
**    { 'n',	"no-write",	0,	"Don't write output file" },
**    { 'h',	"help",		0,	"Print this help" },
**    { 'c',	"color",	0,	"Enable colored output" },
**    { 'v',	"verbose",	0,	"Enable verbosed messages" }
**  };
**
** static const struct s_option	option =
**  {
**    opts,
**    {"Unknown argument: -", "unknown argument: --",
**     "Expected argument for option: "},
**    4,
**    1
**  };
** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/



int		get_cmdline_opts(const struct s_option	*option,
				 int			start,
				 int			argc,
				 char			**argv,
				 char			**opt_tab);


END_C_DECLS


#endif // POCC_GETOPTS_H
