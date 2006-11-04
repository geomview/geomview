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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips,
 * Celeste Fowler */

#include "listP.h"
#include "ooglutil.h"
#include "pickP.h"

Geom *
ListPick(List *list, Pick *p, Appearance *ap,
	 Transform T, TransformN *TN, int *axes)
{
  int elem = 0, pathInd;
  List *l;
  Geom *v = NULL;

  pathInd = VVCOUNT(p->gcur);
  vvneeds(&p->gcur, pathInd + 1);
  VVCOUNT(p->gcur)++;
  for(l = list; l != NULL; l = l->cdr) {
    *VVINDEX(p->gcur, int, pathInd) = elem;
    if(l->car) 
      if(GeomPick(l->car, p, ap, T, TN, axes))
	v = (Geom *)list;
    elem++;
  }
  VVCOUNT(p->gcur)--;
  return v;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
