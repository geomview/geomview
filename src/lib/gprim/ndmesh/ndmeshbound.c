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

#include "bboxP.h"
#include "ndmeshP.h"

BBox *NDMeshBound(NDMesh *mesh, Transform T, TransformN *TN)
{
  BBox *result;
  int n;
  HPoint3 min, max, tmp, clean;
  HPointN **p;

  if (mesh->meshd > 2)
    return NULL;

  n    = mesh->mdim[0]*mesh->mdim[1]; /* more is not implemented yet */
  p    = mesh->p;
  
  /* First handle the case without transformations, this means that we
   * return an Nd bbox.
   */
  if (!T && !TN) {
    HPointN *min;
    HPointN *max;

    min = HPtNCopy(p[0], NULL);
    HPtNDehomogenize(min, min);
    max = HPtNCopy(min, NULL);
    while(--n > 0) {
      HPtNMinMax(min, max, *(++p));
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
    HPointN *ptN;
    BBox *result;

    minN = HPtNTransform(TN, p[0], NULL);
    HPtNDehomogenize(minN, minN);
    maxN = HPtNCopy(minN, NULL);
    ptN = HPtNCreate(TN->odim, NULL);
    while(--n > 0) {
      HPtNTransform(TN, *(++p), ptN);
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
    /* ordinary 3d transform, we simply operate on the x, y, z sub-space. */
    HPtNToHPt3(*p, NULL, &min);
    HPt3Transform(T, &min, &min);
    HPt3Dehomogenize(&min, &min);
    max = min;
    while(--n > 0) {
      HPtNToHPt3(*(++p), NULL, &tmp);
      HPt3Transform(T, &tmp, &clean);
      HPt3MinMax(&min, &max, &clean);
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
