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
 * T     TN    AXES  BBOX
 * NULL  NULL  NULL  return the objects "native" bounding box
 * !NULL NULL  NULL  return a transformed bounding box, Nd objects
 *                   interprete T as acting on the x,y,z sub-space.
 * NULL  !NULL NULL  return a "native" transformed bounding box, Nd objects
 *                   just let TN act on their coordinates, 3d objects are
 *                   implicitly padded with zeros
 * NULL  !NULL !NULL return a transformed and projected 3d bounding box.
 *                   3d objects are implicitly padded with zeroes before
 *                   the projection is applied.
 *
 * All other combinations are unsupported.
 */

#include "polylistP.h"

BBox *PolyListBound(PolyList *polylist, Transform T, TransformN *TN)
{
  int n;
  Vertex  *v;
  HPoint3 min, max;
  HPoint3 clean, tmp;

  n = polylist->n_verts;
  v = polylist->vl;
  min = v->pt;

  /* We assume all the vertices in vl[] are actually used... */
  while(--n >= 0 && !finite(v->pt.x + v->pt.y + v->pt.z + v->pt.w))
    v++;
  if(n <= 0)
    return NULL;	/* No finite points */

  /* First handle the case without transformations, this means that we
     return a 3d bbox for 3d polylists, and a 4d bboy for 4d polylists.
   */
  if ((T == TM_IDENTITY || !T) && !TN) {
    if (!T && (polylist->geomflags & VERT_4D)) {
      max = min;
      while(--n >= 0) {
	Pt4MinMax(&min, &max, &(++v)->pt);
      }
      return (BBox *)GeomCCreate(NULL, BBoxMethods(),
				 CR_4MIN, &min, CR_4MAX, &max,
				 CR_4D, 1, CR_END);
    } else {
      HPt3Dehomogenize(&min, &min);
      max = min;
      while(--n >= 0) {
	HPt3MinMax(&min, &max, &(++v)->pt);
      }
      return (BBox *)GeomCCreate(NULL, BBoxMethods(),
				 CR_4MIN, &min, CR_4MAX, &max, CR_END);
    }
  }
  
  if (TN) {
    /* Nd bounding box is requested, with transformation. */
    HPointN *ptN;
    HPointN *minN;
    HPointN *maxN;
    BBox *result;

    ptN = HPtNCreate(5, NULL);

    if (polylist->geomflags & VERT_4D) {
      minN = Pt4NTransform(TN, &min, NULL);
    } else {
      minN = HPt3NTransform(TN, &min, NULL);
    }
    HPtNDehomogenize(minN, minN);
    maxN = HPtNCopy(minN, NULL);
    while(--n >= 0) {
      if (polylist->geomflags & VERT_4D) {
	Pt4NTransform(TN, &(++v)->pt, ptN);
      } else {
	HPt3NTransform(TN, &(++v)->pt, ptN);
      }
      HPtNMinMax(minN, maxN, ptN);
    }
    result = (BBox *)GeomCCreate(NULL, BBoxMethods(),
				 CR_NMIN, minN, CR_NMAX, maxN, CR_END);

    HPtNDelete(ptN);
    HPtNDelete(minN);
    HPtNDelete(maxN);

    return result;
  }

  /* A 3d bbox is requested, with transformations */

  if (T) {
    /* ordinary 3d transform */
    if (false && polylist->geomflags & VERT_4D) {
      /* We operate on the 3x3 x,y,z space, this means that we just
       * have to omit the "w" coordinate.
       */
      min.w = 1.0;
      HPt3Transform(T, &min, &min);
      HPt3Dehomogenize(&min, &min);
      max = min;
      while(--n >= 0) {
	tmp = (++v)->pt;
	tmp.w = 1.0;
	HPt3Transform(T, &tmp, &clean);
	HPt3MinMax(&min, &max, &clean);
      }
    } else {
      /* ordinary 3d object */
      HPt3Transform(T, &min, &min);
      HPt3Dehomogenize(&min, &min);
      max = min;
      while(--n >= 0) {
	tmp = (++v)->pt;
	HPt3Transform(T, &tmp, &clean);
	HPt3MinMax(&min, &max, &clean);
      }
    }

    /* At this point we are ready to generate a 3d bounding box */
    return (BBox *)GeomCCreate(NULL, BBoxMethods(),
			       CR_4MIN, &min, CR_4MAX, &max, CR_END);
  }

  return NULL;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
