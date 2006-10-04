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
 * ROUTINE DESCRIPTION:  Return the bounding box of a collection of vectors.
 *
 */


#include "skelP.h"

static BBox *NSkelBound(Skel *s, Transform dummy, TransformN *T, int *axes)
{
  BBox *result;
  int n, i, pdim, dim;
  float *v;
  HPointN *min, *max;
  HPointN *clean, *raw, *tmp, *tmp2;
  float *tmp_data;

  n = s->nvert;
  v = s->p;
  pdim = s->dim;
 
  tmp = HPtNCreate(pdim, NULL);
  tmp_data = tmp->v;
  tmp->v = v;

  min = HPtNCreate(pdim, NULL);

  /* all points are (N+1)-vectors */
  if (T) {
    HPtNTransform(T, tmp, min);
  }
  if (!(s->geomflags & VERT_4D)) {
    HPtNDehomogenize( min, min );
    dim = pdim - 1;
  } else {
    dim = pdim;
  }
  max = HPtNCopy(min, NULL);
  
  tmp2 = HPtNCreate(pdim, NULL);  
  while(--n > 0) {

    tmp->v += pdim;

    if (T) {
      raw = tmp2;
      HPtNTransform(T, tmp, raw);
    } else {
      raw = tmp;
    }
    if (!(s->geomflags & VERT_4D)) {
      clean = tmp2;
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
    if (s->geomflags & VERT_4D) {
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
				 CR_4D, (s->geomflags & VERT_4D) != 0,
				 CR_END);
  }

  /* Cleanup */
  tmp->v = tmp_data;
  HPtNDelete(tmp);
  HPtNDelete(min);
  HPtNDelete(max);
  HPtNDelete(tmp2);

  return result;
}

BBox *SkelBound(Skel *s, Transform T, TransformN *TN, int *axes)
{
    int n;
    HPoint3 min, max;
    float *p;
    HPoint3 p0;
    HPoint3 *hp;

    if (s->dim > 4) {
	return NSkelBound(s, T, TN, axes);
    }

    if (T == TM_IDENTITY) {
	T = NULL;
    }
    
    n = s->nvert;
    p = s->p;
    if(n <= 0)
	    return NULL;

    hp = (HPoint3 *)p;
    while(--n >= 0 && !finite(p[0] + p[1] + p[2] + p[3]))
	p += s->dim;
    if(n < 0)
	return NULL;
    if (T) {
	HPt3Transform(T, (HPoint3 *)p, &min);
    }
    if (!(s->geomflags & SKEL_4D)) {
	HPt3Dehomogenize( &min, &min );
    }
    max = min;
    while(--n >= 0) {
	if (T) {
	    HPt3Transform(T, ++hp, &p0);
	}
	if (!(s->geomflags & SKEL_4D)) {
	    HPt3Dehomogenize( &p0, &p0 );
	}
	if(min.x > p0.x) min.x = p0.x;
	else if(max.x < p0.x) max.x = p0.x;
	if(min.y > p0.y) min.y = p0.y;
	else if(max.y < p0.y) max.y = p0.y;
	if(min.z > p0.z) min.z = p0.z;
	else if(max.z < p0.z) max.z = p0.z;
	if (s->geomflags & VERT_4D) {
	    if(min.w > p0.w) min.w = p0.w;
	    else if(max.w < p0.w) max.w = p0.w;
	}
    } 
    if (s->geomflags & VERT_4D) {
	return (BBox *)GeomCreate ("bbox",
				   CR_4MIN, &min, CR_4MAX, &max, NULL );
    } else {
	return (BBox *)GeomCreate ("bbox",
				   CR_MIN, &min, CR_MAX, &max, NULL );
    }
}
