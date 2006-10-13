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

static BBox *BBoxTransformN(BBox *bbox, Transform dummy, TransformN *T)
{
  int dim, numvert, i, j;
  HPointN **ptN, *p;

  if (!T)
    return bbox;

  if (bbox->geomflags & VERT_4D) {
    dim = bbox->pdim;
  } else {
    dim = bbox->pdim-1;
    HPtNDehomogenize(bbox->min, bbox->min);
    HPtNDehomogenize(bbox->max, bbox->max);
  }
  numvert = 1 << dim;
  ptN = (HPointN **)alloca(numvert*sizeof(HPointN *));
  
  for (i = 0; i < numvert; i++) {
    ptN[i] = HPtNCreate(bbox->pdim, NULL);
    for (j = 0; j < dim; j++) {
      ptN[i]->v[j] = (i & (1 << j)) ? bbox->min->v[j] : bbox->max->v[j];
    }
    if (!(bbox->geomflags & VERT_4D)) {
      ptN[i]->v[dim] = 1.0;
    }
  }

  p = ptN[0];
  HPtNTransform(T, p, p);
  if (!(bbox->geomflags & VERT_4D)) {
    HPtNDehomogenize(p, p);
  }
  HPtNCopy(p, bbox->min);
  HPtNCopy(p, bbox->max);
  HPtNDelete(p);

  for (i = 1; i < numvert; i++) {
    p = ptN[i];
    HPtNTransform(T, p, p);
    if (!(bbox->geomflags & VERT_4D)) {
      HPtNDehomogenize(p, p);
    }

    for (j = 0; j < dim; j++) {
      if (p->v[j] < bbox->min->v[j]) bbox->min->v[j] = p->v[j];
      else if (p->v[j] > bbox->max->v[j]) bbox->max->v[j] = p->v[j];
    }

    HPtNDelete(p);
  }
  return bbox;
}

BBox *BBoxTransform(BBox *bbox, Transform T, TransformN *TN)
{
  int i;
  HPoint3 *p;
  HPoint3 vert[8];
  
  if (bbox->pdim > 4) {
    return BBoxTransformN(bbox, T, TN);
  }

  if (!T)
    return bbox;

  for(i = 0, p = &vert[0]; i < 8; i++, p++) {
    *p = *(HPoint3 *)bbox->min->v;
    if(i&1) p->x = bbox->max->v[0];	/* Neat huh */
    if(i&2) p->y = bbox->max->v[1];	/* Binary encoding of verts */
    if(i&4) p->z = bbox->max->v[2];
  }
	
  p = &vert[0];
  HPt3Transform(T, p, p);
  if (!(bbox->geomflags & VERT_4D)) {
    HPt3Dehomogenize(p, p);
  }
  *(HPoint3 *)bbox->min->v = *p;
  *(HPoint3 *)bbox->max->v = *p;
  i = 8-1;
  do {
    p++;
    HPt3Transform(T, p, p);
    HPt3Normalize(p, p);
    if(bbox->min->v[0] > p->x) bbox->min->v[0] = p->x;
    else if(bbox->max->v[0] < p->x) bbox->max->v[0] = p->x;
    if(bbox->min->v[1] > p->y) bbox->min->v[1] = p->y;
    else if(bbox->max->v[1] < p->y) bbox->max->v[1] = p->y;
    if(bbox->min->v[2] > p->z) bbox->min->v[2] = p->z;
    else if(bbox->max->v[2] < p->z) bbox->max->v[2] = p->z;
    if (bbox->geomflags & VERT_4D) {
      if(bbox->min->v[3] > p->w) bbox->min->v[3] = p->w;
      else if(bbox->max->v[3] < p->w) bbox->max->v[3] = p->w;
    }
  } while(--i > 0);
  return (bbox);
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
