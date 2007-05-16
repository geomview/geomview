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

#include "bezierP.h"

BBox *BezierBound(Bezier *bezier, Transform T, TransformN *TN)
{
  float *p;
  int n;
  HPoint3 min, max, clean;

  p = bezier->CtrlPnts;
  n = (bezier->degree_u + 1) * (bezier->degree_v + 1);

  /* First handle the case without transformations, this means that we
     return a 3d bbox for 3d beziers, and a 4d bbox for 4d beziers.
   */
  if ((T == TM_IDENTITY || !T) && !TN) {
    min = *(HPoint3 *)p;
    if (bezier->dimn == 4) {
      if (!T) {
	max = min;
	while(--n > 0) {
	  p += 4;
	  Pt4MinMax(&min, &max, (HPoint3 *)p);
	}
      } else {
	HPt3Dehomogenize(&min, &min);
	max = min;
	while(--n > 0) {
	  p += 4;
	  HPt3MinMax(&min, &max, (HPoint3 *)p);
	}
      }
      return (BBox *)GeomCCreate(NULL, BBoxMethods(),
				 CR_4MIN, &min, CR_4MAX, &max,
				 CR_4D, 1, CR_END);
    } else {
      min.w = 1.0;
      max = min;
      while(--n > 0) {
	p += 3;
	Pt3MinMax(&min, &max, (HPoint3 *)p);
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

    if (bezier->dimn == 3) {
      Pt3ToHPtN((Point3 *)p, ptN);
    } else if (!(bezier->geomflags & VERT_4D)) {
      HPt3ToHPtN((HPoint3 *)p, NULL, ptN);
    } else {
      Pt4ToHPtN((HPoint3 *)p, ptN);
    }

    minN = HPtNTransform(TN, ptN, NULL);
    HPtNDehomogenize(minN, minN);
    maxN = HPtNCopy(minN, NULL);
    while(--n > 0) {
      if (bezier->dimn == 3) {
	p += 3;
	Pt3ToHPtN((Point3 *)p, ptN);
      } else {
	p += 4;
	if (!(bezier->geomflags & VERT_4D)) {
	  HPt3ToHPtN((HPoint3 *)p, NULL, ptN);
	} else {
	  Pt4ToHPtN((HPoint3 *)p, ptN);
	}
      }
      HPtNTransform(TN, ptN, ptN);
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
    int stride = bezier->dimn;
    
    /* ordinary 3d transform, treat 4d co-ordinates as homogeneous
     * co-ordinates.
     */
    if (stride == 3) {
      Pt3ToHPt3((Point3 *)p, &min, 1);
    } else {
      min = *(HPoint3 *)p;
    }
    HPt3Transform(T, &min, &min);
    HPt3Dehomogenize(&min, &min);
    max = min;
    while(--n > 0) {
      p += stride;
      if (stride == 3) {
	Pt3ToHPt3((Point3 *)p, &clean, 1);
      } else {
	clean = *(HPoint3 *)p;
      }
      HPt3Transform(T, &clean, &clean);
      HPt3MinMax(&min, &max, &clean);
    }
    /* At this point we are ready to generate a 3d bounding box */
    return (BBox *)GeomCCreate(NULL, BBoxMethods(),
			       CR_4MIN, &min, CR_4MAX, &max,
			       CR_END);
  }

  return NULL;  
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
