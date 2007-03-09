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

/* An extra function for the pure ND case */
static BBox *NSkelBound(Skel *s, Transform T, TransformN *TN)
{
  BBox *result;
  int n, pdim;
  HPoint3 min, max, tmp, clean;
  HPointN ptN[1];

  n = s->nvert;
  pdim = s->pdim;
  ptN->v = s->p;
  ptN->dim = pdim;

  /* First handle the case without transformations, this means that we
   * return an Nd bbox.
   */
  if (!T && !TN) {
    HPointN *min;
    HPointN *max;
    
    min = HPtNCopy(ptN, NULL);
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
    clean = HPtNCreate(s->pdim, NULL);
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
      HPt3MinMax(&min, &max, &clean);
    }
  
    /* At this point we are ready to generate a 3d bounding box */
    return (BBox *)GeomCCreate(NULL, BBoxMethods(),
			       CR_4MIN, &min, CR_4MAX, &max, CR_END);
  }

  return NULL;
}

BBox *SkelBound(Skel *s, Transform T, TransformN *TN)
{
  int n;
  HPoint3 min, max, clean, tmp, *p;

  if (s->pdim > 4) {
    /* extra function for ND case. */
    return NSkelBound(s, T, TN);
  }

  n = s->nvert;
  p = (HPoint3 *)s->p;

  while(--n >= 0 && !finite(p->x + p->y + p->z + p->w))
    ++p;
  if(n < 0)
    return NULL;

  min = *p;

  /* First handle the case without transformations, this means that we
     return a 3d bbox for 3d ss, and a 4d bboy for 4d ss.
   */
  if ((T == TM_IDENTITY || !T) && !TN) {
    if (!T && (s->geomflags & VERT_4D)) {
      max = min;
      while(--n >= 0) {
	Pt4MinMax(&min, &max, ++p);
      }
      return (BBox *)GeomCCreate(NULL, BBoxMethods(),
				 CR_4MIN, &min, CR_4MAX, &max,
				 CR_4D, 1, CR_END);
    } else {
      HPt3Dehomogenize(&min, &min);
      max = min;
      while(--n >= 0) {
	HPt3MinMax(&min, &max, ++p);
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

    if (s->geomflags & VERT_4D) {
      Pt4ToHPtN(&min, ptN);
    } else {
      HPt3ToHPtN(&min, NULL, ptN);
    }
    minN = HPtNTransform(TN, ptN, NULL);
    HPtNDehomogenize(minN, minN);
    maxN = HPtNCopy(minN, NULL);
    while(--n >= 0) {
      if (s->geomflags & VERT_4D) {
	Pt4ToHPtN(++p, ptN);
      } else {
	HPt3ToHPtN(++p, NULL, ptN);
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

    /* ordinary 3d transform */

    if (false && (s->geomflags & VERT_4D)) {
      /* We operate on the 3x3 x,y,z space */
      min.w = 1.0;
      HPt3Transform(T, &min, &min);
      HPt3Dehomogenize(&min, &min);
      max = min;
      while(--n >= 0) {
	tmp = *(++p);
	tmp.w = 1.0;
	HPt3Transform(T, &tmp, &clean);
	HPt3MinMax(&min, &max, &clean);
      }
    } else {
      /* ordinary 3d object */
      HPt3Transform(T, &min, &min);
      HPt3Dehomogenize(&min, &min);
      max = min;
      while(--n >= 0) {
	tmp = *(++p);
	HPt3Transform(T, &tmp, &clean);
	Pt3MinMax(&min, &max, &clean);
      }
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
