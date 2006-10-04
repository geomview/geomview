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

BBox *NDMeshBound(NDMesh *m, Transform dummy, TransformN *T, int *axes)
{
  BBox *result;
  int n, i, pdim, dim;
  HPointN *min, *max;
  HPointN *clean, *raw, **p, *tmp;

  if ((void *)T == (void *)TM_IDENTITY) {
    T = NULL;
  }

  n    = m->mdim[0]*m->mdim[1];
  p    = m->p;
  pdim = m->pdim;
 
  min = HPtNCreate(pdim, NULL);

  /* all points are (N+1)-vectors */
  if (T) {
    HPtNTransform(T, *p, min);
  }
  if (!(m->geomflags & VERT_4D)) {
    HPtNDehomogenize( min, min );
    dim = pdim - 1;
  } else {
    dim = pdim;
  }
  max = HPtNCopy(min, NULL);
  
  tmp = HPtNCreate(pdim, NULL);

  for (i = 1; i < n; i++) {
    if (T) {
      raw = tmp;
      HPtNTransform(T, *p, raw);
    } else {
      raw = *p;
    }
    if (!(m->geomflags & VERT_4D)) {
      clean = tmp;
      HPtNDehomogenize(raw, clean);
    } else {
      clean = raw;
    }
    for (i = 0; i < dim; i++) {
      if (min->v[i] > clean->v[i]) {
	min->v[i] = clean->v[i];
      } else if (max->v[i] < clean->v[i]) {
	max->v[i] = clean->v[i];
      }
    }
  }

  if (axes) {
    HPt3Coord min4[4], max4[4];

    for (i = 0; i < 3; i++) {
      min4[i] = min->v[axes[i]];
      max4[i] = max->v[axes[i]];
    }
    if (m->geomflags & VERT_4D) {
      min4[3] = min->v[axes[3]];
      min4[3] = max->v[axes[3]];
      result = (BBox *)GeomCCreate(NULL, BBoxMethods(),
				   CR_4MIN, min4, CR_4MAX, max4, CR_END);
    } else {
      min4[3] = 1.0;
      max4[3] = 1.0;
      result = (BBox *)GeomCCreate(NULL, BBoxMethods(),
				   CR_MIN, min4, CR_MAX, max4, CR_END);
    }
  } else {
    result = (BBox *)GeomCCreate(NULL, BBoxMethods(),
				 CR_NMIN, min, CR_NMAX, max,
				 CR_4D, (m->geomflags & VERT_4D) != 0,
				 CR_END);
  }

  /* Cleanup */
  HPtNDelete(min);
  HPtNDelete(max);
  HPtNDelete(tmp);

  return result;
}

