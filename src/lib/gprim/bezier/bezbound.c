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
  HPoint3 min, max, tmp, clean;

  p = bezier->CtrlPnts;
  n = (bezier->degree_u + 1) * (bezier->degree_v + 1);
  min = *(HPoint3 *)p;

  /* First handle the case without transformations, this means that we
     return a 3d bbox for 3d beziers, and a 4d bboy for 4d beziers.
   */
  if (!T && !TN) {
    if (bezier->geomflags & VERT_4D) {
      max = min;
      while(--n > 0) {
	p += 4;
	HPt3MinMax(&min, &max, (HPoint3 *)p);
      }
      return (BBox *)GeomCCreate(NULL, BBoxMethods(),
				 CR_4MIN, &min, CR_4MAX, &max, CR_END);
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
    int stride = (bezier->geomflags & VERT_4D) ? 4 : 3;
    HPointN *ptN;
    HPointN *minN;
    HPointN *maxN;
    BBox *result;

    ptN = HPtNCreate(5, NULL);

    if (!(bezier->geomflags & VERT_4D)) {
      min.w = 1.0;
    }
    *(HPoint3 *)ptN->v = min;
    minN = HPtNTransform(TN, ptN, NULL);
    HPtNDehomogenize(minN, minN);
    maxN = HPtNCopy(minN, NULL);
    while(--n > 0) {
      p += stride;
      *(HPoint3 *)ptN->v = *(HPoint3 *)p;
      if (!(bezier->geomflags & VERT_4D)) {
	ptN->v[3] = 1.0;
      }
      HPtNTransform(TN, ptN, ptN);
      HPtNDehomogenize(ptN, ptN);
      HPtNMinMax(minN, maxN, ptN, TN->odim-1);
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
    int stride = (bezier->geomflags & VERT_4D) ? 4 : 3;
    
    /* ordinary 3d transform, act on the x,y,z sub-space */

    min.w = 1.0;
    HPt3Transform(T, &min, &min);
    HPt3Dehomogenize(&min, &min);
    max = min;
    while(--n > 0) {
      p += stride;
      tmp.x = p[0];
      tmp.y = p[1];
      tmp.z = p[2];
      tmp.w = 1.0;
      HPt3Transform(T, &tmp, &clean);
      HPt3Dehomogenize(&clean, &clean);
      Pt3MinMax(&min, &max, &clean);
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
