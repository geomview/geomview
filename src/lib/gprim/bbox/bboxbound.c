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

BBox *BBoxBound(BBox *bbox, Transform T, TransformN *TN)
{
  HPoint3 min, max;
  HPt3Coord tmp;
  
  if (bbox == NULL)
    return NULL;

  if (bbox->pdim < 4)
    return NULL; /* should not happen */

  if (!T && !TN) {
    return (BBox *)GeomCopy((Geom *)bbox);
  }

  if (TN) { /* real ND bbox */
    int i, dim;
    HPointN *minN;
    HPointN *maxN;
    BBox *result;

    if (bbox->pdim == 4 && (bbox->geomflags & VERT_4D)) {
      /* generate a real ND bounding box without VERT_4D */
      HPointN tmp;

      tmp.flags = 0;
      tmp.dim = bbox->pdim+1;
      tmp.v   = alloca(tmp.dim*sizeof(HPtNCoord));
      tmp.v[tmp.dim-1] = 1.0;
      for (i = 0; i < bbox->pdim; i++) {
	tmp.v[i] = bbox->min->v[i];
      }
      minN = HPtNTransform(TN, &tmp, NULL);
      for (i = 0; i < bbox->pdim; i++) {
	tmp.v[i] = bbox->max->v[i];
      }
      maxN = HPtNTransform(TN, &tmp, NULL);
    } else {
      minN = HPtNTransform(TN, bbox->min, NULL);
      maxN = HPtNTransform(TN, bbox->max, NULL);
    }
     
    if (!(bbox->geomflags & VERT_4D)) {
      HPtNDehomogenize(minN, minN);
      HPtNDehomogenize(maxN, maxN);
      dim = TN->odim-1;
    } else {
      dim = TN->odim;
    }

    /* and now swap coordinates between min and max as necessary */
    for (i = 0; i < dim; i++) {
      if (minN->v[i] > maxN->v[i]) {
	tmp = maxN->v[i]; maxN->v[i] = minN->v[i]; minN->v[i] = tmp;
      }
    }

    /* At this point we are ready to generate an Nd bounding box */
    result = (BBox *)GeomCCreate(NULL, BBoxMethods(),
				 CR_NMIN, minN, CR_NMAX, maxN, CR_END);
    HPtNDelete(minN);
    HPtNDelete(maxN);
    return result;
  }

  if (T) {
    if (bbox->pdim > 4) {
      HPtNToHPt3(bbox->min, &min, NULL);
      HPtNToHPt3(bbox->max, &max, NULL);
    }
    HPt3Transform(T, (HPoint3 *)bbox->min->v, &min);
    HPt3Transform(T, (HPoint3 *)bbox->max->v, &max);
    HPt3Dehomogenize(&min, &min);
    HPt3Dehomogenize(&max, &max);
      
    /* and now swap coordinates between min and max as necessary */
    if (min.x > max.x) {
      tmp = max.x; max.x = min.x; min.x = tmp;
    }
    if (min.y > max.y) {
      tmp = max.y; max.y = min.y; min.y = tmp;
    }
    if (min.z > max.z) {
      tmp = max.z; max.z = min.z; min.z = tmp;
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
