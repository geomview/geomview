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


/* Authors:  Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips,
   Celeste Fowler */

#include "point3.h"
#include "polylistP.h"

PolyList *PolyListTransform(PolyList *p, Transform T, TransformN *TN)
{
  int i;

  (void)TN;

  if (!T) {
    return p;
  }

  for (i = 0; i < p->n_verts; i++) {
    HPt3Transform(T, &p->vl[i].pt, &p->vl[i].pt);
  }
  if (p->geomflags & (PL_HASVN|PL_HASPN)) {
    Transform Tit;

    TmDual(T, Tit);
    if (p->geomflags & PL_HASVN) {
      for (i = 0; i < p->n_verts; i++) {
	NormalTransform(Tit, &p->vl[i].vn, &p->vl[i].vn);
      }
    }
    if (p->geomflags & PL_HASPN) {
      for (i = 0; i < p->n_polys; i++) {
	NormalTransform(T, &p->p[i].pn, &p->p[i].pn);
      }
    }
  }

  return p;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
