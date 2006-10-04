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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

/*
 * Geometry Routines
 *
 * Geometry Supercomputer Project
 *
 * ROUTINE DESCRIPTION:  Return the bounding box of a polylist.
 *
 */


#include "polylistP.h"

BBox *PolyListBound(PolyList *polylist, Transform T, TransformN *TN, int *axes)
{
  int n;
  Vertex  *v;
  HPoint3 min, max;
  HPoint3 *raw, *clean, tmp;

  if (T == TM_IDENTITY)
    T = NULL;

  n = polylist->n_verts;
  v = polylist->vl;

  /* We assume all the vertices in vl[] are actually used... */
  while(--n >= 0 && !finite(v->pt.x + v->pt.y + v->pt.z + v->pt.w))
    v++;
  if(n <= 0)
    return NULL;	/* No finite points */

#if 0 && defined(BBOX_ND_HACK)
  if(polylist->geomflags & VERT_4D) {
    BBox *b;
    for(b = NULL; --n >= 0; )
      b = BBox_ND_hack(b, &v->pt.x, 4);
    return b;
  }
#endif

  min = v->pt;
  /* all points are 4-vectors, but we've marked whether they
   * have true 4-dimensional nature */
  if (T) 	{
    HPt3Transform(T, &min, &min);
  }
  if (!(polylist->geomflags & VERT_4D)) {
    HPt3Dehomogenize( &min, &min );
  }
  max = min;
  while(--n >= 0) {
    v++;
    if (T) 	{
      raw = &tmp;
      HPt3Transform(T, &v->pt, raw);
    } else {
      raw = &v->pt;
    }
    if (!(polylist->geomflags & VERT_4D)) {
      clean = &tmp;
      HPt3Normalize( raw, clean );
    } else {
      clean = raw;
    }
    if(min.x > clean->x) min.x = clean->x;
    else if(max.x < clean->x) max.x = clean->x;
    if(min.y > clean->y) min.y = clean->y;
    else if(max.y < clean->y) max.y = clean->y;
    if(min.z > clean->z) min.z = clean->z;
    else if(max.z < clean->z) max.z = clean->z;
    if (polylist->geomflags & VERT_4D) {
      if(min.w > clean->w) min.w = clean->w;
      else if(max.w < clean->w) max.w = clean->w;
    }
  }
  if (polylist->geomflags & VERT_4D) {
    return (BBox *) GeomCCreate(NULL, BBoxMethods(),
				CR_4MIN, &min, CR_4MAX, &max, CR_END);
  } else {
    return (BBox *) GeomCCreate(NULL, BBoxMethods(),
				CR_MIN, &min, CR_MAX, &max, CR_END);
  }
}
