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

static BBox *BBoxTransformN(BBox *bbox, TransformN *TN)
{
  HPointN *p;
  HPtNCoord c = bbox->min->v[0] != 1.0 ? bbox->min->v[0] : 1.0;
  HPtNCoord C = bbox->max->v[0] != 1.0 ? bbox->max->v[0] : 1.0;
  int dim = bbox->pdim-1;
  int numvert = 1 << dim;
  VARARRAY(ptN, HPointN *, numvert);
  int i, j;
  
  if (!TN) {
    return bbox;
  }
  
  for (i = 0; i < numvert; i++) {
    ptN[i] = HPtNCreate(bbox->pdim, NULL);
    for (j = 1; j < bbox->pdim; j++) {
      ptN[i]->v[j] = (i & (1 << j)) ? bbox->min->v[j]/c : bbox->max->v[j]/C;
    }
  }

  p = ptN[0];
  HPtNTransform(TN, p, p);
  HPtNDehomogenize(p, p);
  HPtNCopy(p, bbox->min);
  HPtNCopy(p, bbox->max);
  HPtNDelete(p);

  for (i = 1; i < numvert; i++) {
    p = ptN[i];
    HPtNTransform(TN, p, p);
    HPtNDehomogenize(p, p);

    for (j = 1; j < bbox->pdim; j++) {
      if (p->v[j] < bbox->min->v[j]) bbox->min->v[j] = p->v[j];
      else if (p->v[j] > bbox->max->v[j]) bbox->max->v[j] = p->v[j];
    }
    HPtNDelete(p);
  }
  return bbox;
}

BBox *BBoxTransform(BBox *bbox, Transform T, TransformN *TN)
{
  int dim, numvert, i, j;
  HPointN **ptN, *p;
  HPtNCoord c = bbox->min->v[0] != 1.0 ? bbox->min->v[0] : 1.0;
  HPtNCoord C = bbox->max->v[0] != 1.0 ? bbox->max->v[0] : 1.0;
  
  if ((!T || T == TM_IDENTITY) && !TN)
    return bbox;
  
  if (TN) {
    return BBoxTransformN(bbox, TN);
  }

  dim = bbox->pdim-1;
  numvert = 1 << dim;
  ptN = (HPointN **)alloca(numvert*sizeof(HPointN *));
  
  for (i = 0; i < numvert; i++) {
    ptN[i] = HPtNCreate(bbox->pdim, NULL);
    for (j = 1; j < bbox->pdim; j++) {
      ptN[i]->v[j] = (i & (1 << j)) ? bbox->min->v[j]/c : bbox->max->v[j]/C;
    }
  }

  p = ptN[0];
  HPtNTransform3(T, NULL, p, p);
  HPtNDehomogenize(p, p);
  HPtNCopy(p, bbox->min);
  HPtNCopy(p, bbox->max);
  HPtNDelete(p);

  for (i = 1; i < numvert; i++) {
    p = ptN[i];
    HPtNTransform3(T, NULL, p, p);
    HPtNDehomogenize(p, p);

    for (j = 1; j < bbox->pdim; j++) {
      if (p->v[j] < bbox->min->v[j]) bbox->min->v[j] = p->v[j];
      else if (p->v[j] > bbox->max->v[j]) bbox->max->v[j] = p->v[j];
    }
    HPtNDelete(p);
  }

  return bbox;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
