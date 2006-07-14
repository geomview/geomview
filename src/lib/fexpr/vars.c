/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 *
 * This file is part of Geomview.
 * 
 * Geomview is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * Geomview is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with Geomview; see the file COPYING.  If not, write
 * to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 * USA, or visit http://www.gnu.org.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

#include "fexpr.h"
#include <stdlib.h>
#include <string.h>

expr_var expr_create_variable(struct expression *e, char *name, double val)
{
  int i;

  if (e->varnames)
    for (i=0; i<e->nvars; i++)
      if (!strcmp(e->varnames[i], name)) {
	e->varvals[i].real = val;
	return i;
      }
  
  if (e->nvars) {
    e->varnames = realloc(e->varnames, (1+e->nvars)*sizeof(char *));
    e->varvals = realloc (e->varvals, (1+e->nvars)*sizeof(fcomplex));
  } else {
    e->varnames = malloc ((e->nvars+1)*sizeof(char *));
    e->varvals = malloc ((e->nvars+1) * sizeof(fcomplex));
  }
  e->varnames[e->nvars] = malloc(strlen(name)+1);
  strcpy (e->varnames[e->nvars],name);
  e->varvals[e->nvars].real = val;
  e->varvals[e->nvars].imag = 0;
  
  return e->nvars++;
}

void expr_set_variable(struct expression *e, expr_var v, double val)
{
  e->varvals[v].real = val;
}

double expr_get_variable(struct expression *e, expr_var v)
{
  return e->varvals[v].real;
}
void expr_set_variable_complex(struct expression *e, expr_var v, fcomplex*val)
{
  e->varvals[v] = *val;
}

void expr_get_variable_complex(struct expression *e, expr_var v,fcomplex *op)
{
  *op = e->varvals[v];
}

