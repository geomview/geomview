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

#include <stdlib.h>
#include <stdio.h>
#include "fexpr.h"

struct expression *expr_new()
{
  struct expression *op;

  op = malloc (sizeof (struct expression));

  op ->nvars = 0;
  op->varnames = NULL;
  op->varvals = NULL;
  op->nelem = 0;
  op->elems = NULL;

  return op;
}

void expr_free(struct expression *e)
{
  if (!e) return;

  if  (e->varnames) {
    int i;
    for (i=0; i<e->nvars; i++)
      if (e->varnames[i])
	free (e->varnames[i]);
    free (e->varnames);
  }
  if (e->varvals)
    free (e->varvals);
  if (e->elems)
    free (e->elems);
  
  free(e);
}
