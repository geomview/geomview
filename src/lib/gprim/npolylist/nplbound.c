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
 * if T != NULL then we act only on the 3d sub-space of the first 3
 * components.
 */


#include "npolylistP.h"

BBox *NPolyListBound(NPolyList *np, Transform T, TransformN *TN)
{
  BBox *result;
  int n, pdim;
  HPoint3 min, max, tmp, clean;
  HPointN ptN[1];

  n = np->n_verts;
  pdim = np->pdim;
  ptN->v = np->v;
  ptN->dim = pdim;

  /* First handle the case without transformations, this means that we
   * return an Nd bbox.
   */
  if (!T && !TN) {
    HPointN *min = HPtNCreate(pdim, np->v);
    HPointN *max;
    
    HPtNDehomogenize(min, min);
    max = HPtNCopy(min, NULL);
    while(--n > 0) {
      ptN->v += pdim;
      HPtNMinMax(min, max, ptN);
    }
    result = (BBox *)GeomCCreate(NULL, BBoxMethods(),
				 CR_NMIN, min, CR_NMAX, max, CR_END);
    HPtNDelete(min);
    HPtNDelete(max);

    return result;
  }

  if (TN) {
    /* Nd bounding box is requested, with transformation. */
    HPointN *minN;
    HPointN *maxN;
    HPointN *clean;
    BBox *result;

    minN = HPtNTransform(TN, ptN, NULL);
    HPtNDehomogenize(minN, minN);
    maxN = HPtNCopy(minN, NULL);

    clean = HPtNCreate(np->pdim, NULL);
    while(--n > 0) {
      ptN->v += pdim;
      HPtNTransform(TN, ptN, clean);
      HPtNMinMax(minN, maxN, clean);
    }
    result = (BBox *)GeomCCreate(NULL, BBoxMethods(),
				 CR_NMIN, minN, CR_NMAX, maxN, CR_END);

    HPtNDelete(minN);
    HPtNDelete(maxN);
    HPtNDelete(clean);

    return result;
  }

  /* A 3d bbox is requested, with transformations */

  if (T) {
    /* ordinary 3d transform */
    HPtNToHPt3(ptN, NULL, &min);
    HPt3Transform(T, &min, &min);
    HPt3Dehomogenize(&min, &min);
    max = min;
    while(--n > 0) {
      ptN->v += pdim;
      HPtNToHPt3(ptN, NULL, &tmp);
      HPt3Transform(T, &tmp, &clean);
      HPt3Dehomogenize(&clean, &clean);
      Pt3MinMax(&min, &max, &clean);
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
