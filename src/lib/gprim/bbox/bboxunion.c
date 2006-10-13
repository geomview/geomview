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

#define	BB_MIN(a,b)	( (a) <= (b) ? a : b )
#define	BB_MAX(a,b)	( (a) >= (b) ? a : b )

BBox *
BBoxUnion(BBox *bbox1, BBox *bbox2)
{
    return BBoxUnion3(bbox1, bbox2, NULL);
}

BBox *
BBoxUnion3(BBox *bbox1, BBox *bbox2, BBox *result)
{
  int dim1, dim2 = 0;
  
  /* TAKE CARE OF THE CASE OF EITHER CUBE BEING NULL */
  if (!bbox1) {
    if(!bbox2) {
      static Point Max = { -1e10,-1e10,-1e10, 1 };
      static Point Min = {  1e10, 1e10, 1e10, 1 };
      return (BBox *)GeomCCreate((Geom *)result, BBoxMethods(),
				 CR_MIN, Min, CR_MAX, Max, CR_END);
    }
    bbox1 = bbox2;
    bbox2 = NULL;
  }

  if (bbox2) {
    dim2 = bbox2->pdim;
    if (!(bbox2->geomflags & VERT_4D)) {
      HPtNDehomogenize(bbox2->min, bbox2->min);
      HPtNDehomogenize(bbox2->max, bbox2->max);
      --dim2;
    }
  }
  
  dim1 = bbox1->pdim;
  if (!(bbox1->geomflags & VERT_4D)) {
    HPtNDehomogenize(bbox1->min, bbox1->min);
    HPtNDehomogenize(bbox1->max, bbox1->max);
    --dim1;
  }

  /* Make sure bbox1 is one with the larger dimension */
  if (dim2 > dim1) {
    BBox *bboxswap;
    int dimswap;

    bboxswap = bbox1; bbox1 = bbox2; bbox2 = bboxswap;
    dimswap = dim1; dim1 = dim2; dim2 = dim1;
  }
  
  if (bbox1->pdim == 4) {
    result = (BBox *)GeomCCreate((Geom *)result, BBoxMethods(),
				 CR_4MIN, bbox1->min,
				 CR_4MAX, bbox1->max,
				 CR_4D, (bbox1->geomflags & VERT_4D) != 0,
				 CR_END);
  } else {
    result = (BBox *)GeomCCreate((Geom *)result, BBoxMethods(),
				 CR_NMIN, bbox1->min,
				 CR_NMAX, bbox1->max,
				 CR_4D, (bbox1->geomflags & VERT_4D) != 0,
				 CR_END);
  }

  if (bbox2) {
    int i;

    for (i = 0; i < dim2; i++) {
      if (result->min->v[i] > bbox2->min->v[i]) {
	result->min->v[i] = bbox2->min->v[i];
      }
      if (result->max->v[i] < bbox2->max->v[i]) {
	result->max->v[i] = bbox2->max->v[i];
      }
    }
  }
	
  return result;
}
