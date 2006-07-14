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


/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips, Nathaniel Thurston */

#include "transform.h"
#include <math.h>
#include <stdio.h>

void Tm3Translate( Transform3 T, Tm3Coord tx, Tm3Coord ty, Tm3Coord tz )
{
  HPoint3 pt;
  pt.x = tx;
  pt.y = ty;
  pt.z = tz;
  pt.w = 1;
  Tm3TranslateOrigin(T, &pt);
}

void Tm3TranslateOrigin( Transform3 T, HPoint3 *pt )
{
  Tm3Identity(T);
  T[3][0] = pt->x / pt->w;
  T[3][1] = pt->y / pt->w;
  T[3][2] = pt->z / pt->w;
}

/************************************************************************/

void Tm3HypTranslate( Transform3 T, Tm3Coord tx, Tm3Coord ty, Tm3Coord tz )
{
  HPoint3 pt;
  float t = sqrt( tx*tx + ty*ty + tz*tz );

  if(t > 0) {
    pt.x = sinh(t) * tx / t;
    pt.y = sinh(t) * ty / t;
    pt.z = sinh(t) * tz / t;
    pt.w = cosh(t);
    Tm3HypTranslateOrigin(T, &pt);
  } else {
    Tm3Identity(T);
  }
}

static void minkowski_normalize( HPoint3 *pt )
{
  float f = sqrt( pt->w*pt->w  - pt->x*pt->x - pt->y*pt->y - pt->z*pt->z );
  pt->x /= f;
  pt->y /= f;
  pt->z /= f;
  pt->w /= f;
}

void Tm3HypTranslateOrigin( Transform3 T, HPoint3 *pt )
{
  Transform R, Rinv;
  minkowski_normalize(pt);
  Tm3Identity(T);
  T[2][3] = T[3][2] = sqrt(pt->x*pt->x + pt->y*pt->y + pt->z*pt->z);
  T[2][2] = T[3][3] = pt->w;
  Tm3RotateTowardZ(R, pt);
  Tm3Invert(R, Rinv);
  Tm3Concat(R, T, T);
  Tm3Concat(T, Rinv, T);
}

/***********************************************************************/

void Tm3SphTranslate( Transform3 T, Tm3Coord tx, Tm3Coord ty, Tm3Coord tz )
{
  HPoint3 pt;
  float t = sqrt( tx*tx + ty*ty + tz*tz );

  if(t > 0) {
    pt.x = sin(t) * tx / t;
    pt.y = sin(t) * ty / t;
    pt.z = sin(t) * tz / t;
    pt.w = cos(t);
    Tm3SphTranslateOrigin(T, &pt);
  } else {
    Tm3Identity(T);
  }
}

static void sph_normalize(HPoint3 *pt)
{
  /* the following line originally did not have sqrt; but I think it
     should.  Right?  -- mbp Thu Dec 10 11:02:46 1992 */
  float t = sqrt( pt->x*pt->x + pt->y*pt->y + pt->z*pt->z + pt->w*pt->w );
  if(t > 0) {
    pt->x /= t;
    pt->y /= t;
    pt->z /= t;
    pt->w /= t;
  }
}

void Tm3SphTranslateOrigin( Transform3 T, HPoint3 *pt )
{
  Transform R, Rinv;
  sph_normalize(pt);
  Tm3Identity(T);
  T[2][3] = -(T[3][2] = sqrt(pt->x*pt->x + pt->y*pt->y + pt->z*pt->z));
  T[2][2] = T[3][3] = pt->w;
  Tm3RotateTowardZ(R, pt);
  Tm3Invert(R, Rinv);
  Tm3Concat(R, T, T);
  Tm3Concat(T, Rinv, T);
}

void Tm3SpaceTranslate( Transform3 T, Tm3Coord tx, Tm3Coord ty, Tm3Coord tz, int space)
{
  switch (TM_SPACE(space)) {
  case TM_EUCLIDEAN:
  default:
    Tm3Translate(T, tx, ty, tz);
    break;
  case TM_HYPERBOLIC:
    Tm3HypTranslate(T, tx, ty, tz);
    break;
  case TM_SPHERICAL:
    Tm3SphTranslate(T, tx, ty, tz);
    break;
  }
}

void Tm3SpaceTranslateOrigin( Transform3 T, HPoint3 *pt, int space)
{
  switch (TM_SPACE(space)) {
  case TM_EUCLIDEAN:
  default:
    Tm3TranslateOrigin(T, pt);
    break;
  case TM_HYPERBOLIC:
    Tm3HypTranslateOrigin(T, pt);
    break;
  case TM_SPHERICAL:
    Tm3SphTranslateOrigin(T, pt);
    break;
  }
}

