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

#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
#include "config.h"
#endif

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";

/*
 * hpoint3a.c: linear algebra routines for hpoints
 */

#include <math.h>
#include <stdio.h>
#include "hg4.h"
#include "hpoint3.h"
#include "transform3.h"
#include "tolerance.h"

/* inner product of R4 */
HPt3Coord
HPt3R40Dot(HPoint3 *a, HPoint3 *b)
{
  return a->x*b->x + a->y*b->y + a->z*b->z + a->w*b->w;
}

/* inner product of R(3,1) */
HPt3Coord
HPt3R31Dot(HPoint3 *a, HPoint3 *b)
{
  return a->x*b->x + a->y*b->y + a->z*b->z - a->w*b->w;
}

/* inner product of R(3,0) */
HPt3Coord
HPt3R30Dot(HPoint3 *a, HPoint3 *b)
{
  double w2 = a->w*b->w;
  if (w2 == 1.0 || w2 == 0.0)
    return a->x*b->x + a->y*b->y + a->z*b->z;
  else
    return (a->x*b->x + a->y*b->y + a->z*b->z) / w2;
}

HPt3Coord
HPt3SpaceDot(HPoint3 *a, HPoint3 *b, int space)
{
  switch (space) {
  case TM_EUCLIDEAN:
  default:
    return HPt3R30Dot(a,b);
    break;
  case TM_HYPERBOLIC:
    return HPt3R31Dot(a,b);
    break;
  case TM_SPHERICAL:
    return HPt3R40Dot(a,b);
    break;
  }
}

/* normalize a point a in R4 so that <a,a> = 1 */
void
HPt3R40Normalize(HPoint3 *a)
{
  float len = sqrt( (double)(HPt3R40Dot(a,a)) );
  if (len > 0) {
    len = 1 / len;
    a->x *= len;
    a->y *= len;
    a->z *= len;
    a->w *= len;
  }
}

/* normalize a point a in R(3,1) so that <a,a> = sign(<a,a>) */
void
HPt3R31Normalize(HPoint3 *a)
{
  float len = sqrt( fabs( (double)(HPt3R31Dot(a,a)) ) );
  if (len > 0) {
    len = 1 / len;
    a->x *= len;
    a->y *= len;
    a->z *= len;
    a->w *= len;
  }
}

/* normalize a point a in R(3,0) so that <a,a> = 1 */
void
HPt3R30Normalize(HPoint3 *a)
{
  float len = sqrt( (double)(HPt3R30Dot(a,a)) );
  if (len > 0) {
    len = 1 / len;
    a->x *= len;
    a->y *= len;
    a->z *= len;
  }
}

void
HPt3SpaceNormalize(HPoint3 *a, int space)
{
  switch (space) {
  case TM_EUCLIDEAN:
  default:
    HPt3R30Normalize(a);
    break;
  case TM_HYPERBOLIC:
    HPt3R31Normalize(a);
    break;
  case TM_SPHERICAL:
    HPt3R40Normalize(a);
    break;
  }
}

HPt3Coord
HPt3HypDistance(HPoint3 *a, HPoint3 *b)
{
  float aa, bb, ab;
  aa = HPt3R31Dot(a,a);
  bb = HPt3R31Dot(b,b);
  ab = HPt3R31Dot(a,b);
  return acosh( fabs(ab / sqrt( aa * bb ) ));
}

HPt3Coord
HPt3Distance( a, b )
    HPoint3 *a, *b;
{
    float dx, dy, dz;
    float w1w2;

    w1w2 = a->w * b->w;
    if( w1w2 == 0. )
	return 0.;

    dx = b->w * a->x - b->x * a->w;
    dy = b->w * a->y - b->y * a->w;
    dz = b->w * a->z - b->z * a->w;

    return (sqrt( dx*dx + dy*dy + dz*dz )) / w1w2;
}

HPt3Coord
HPt3SphDistance(HPoint3 *a, HPoint3 *b)
{
  return acos( HPt3R40Dot(a,b) / sqrt( HPt3R40Dot(a,a) * HPt3R40Dot(b,b) ) );
}

HPt3Coord
HPt3SpaceDistance(HPoint3 *a, HPoint3 *b, int space)
{
  switch (space) {
  case TM_EUCLIDEAN:
  default:
    return HPt3Distance(a,b);
    break;
  case TM_HYPERBOLIC:
    return HPt3HypDistance(a,b);
    break;
  case TM_SPHERICAL:
    return HPt3SphDistance(a,b);
    break;
  }
}

void
HPt3Sub(HPoint3 *a, HPoint3 *b, HPoint3 *aminusb)
{
  aminusb->x = a->x - b->x;
  aminusb->y = a->y - b->y;
  aminusb->z = a->z - b->z;
  aminusb->w = a->w - b->w;
}

void
HPt3Scale(HPt3Coord s, HPoint3 *a, HPoint3 *sa)
{
  sa->x = s * a->x;
  sa->y = s * a->y;
  sa->z = s * a->z;
  sa->w = s * a->w;
}

void
HPt3GramSchmidt(HPoint3 *base, HPoint3 *v)
{
  HPt3SpaceGramSchmidt(base, v, TM_EUCLIDEAN);
}

void
HPt3HypGramSchmidt(HPoint3 *base, HPoint3 *v)
{
  HPt3SpaceGramSchmidt(base, v, TM_HYPERBOLIC);
}

void
HPt3SphGramSchmidt(HPoint3 *base, HPoint3 *v)
{
  HPt3SpaceGramSchmidt(base, v, TM_SPHERICAL);
}

/* Modifies v to arrange that <base,v> = 0,
   i.e. v is a tangent vector based at base */
void
HPt3SpaceGramSchmidt(HPoint3 *base, HPoint3 *v, int space)
{
  HPt3Coord d,e;
  HPoint3 tmp;

  d = HPt3SpaceDot(base,v,space);
  e = HPt3SpaceDot(base,base,space);
  if (e == 0.0) {
    fprintf(stderr, "GramSchmidt: invalid base point.\n");
    e = 1.0;
  }
  HPt3Scale((HPt3Coord)(d / e), base, &tmp);
  HPt3Sub(v, &tmp, v);
}


#if 0

/* This stuff doesn't compile yet; in progress: */


HPt3Coord
HPt3Angle(HPoint3 *base, HPoint3 *v1, HPoint3 *v2)
{
  /*
   * I'm not sure that this will work in the euclidean case!!! mbp
   */
  return HPt3SpaceAngle(base, v1, v2, TM_EUCLIDEAN);
}

HPt3Coord
HPt3HypAngle(HPoint3 *base, HPoint3 *v1, HPoint3 *v2)
{
  return HPt3SpaceAngle(base, v1, v2, TM_HYPERBOLIC);
}

HPt3Coord
HPt3SphAngle(HPoint3 *base, HPoint3 *v1, HPoint3 *v2)
{
  return HPt3SpaceAngle(base, v1, v2, TM_SPHERICAL);
}

HPt3Coord
HPt3SpaceAngle(HPoint3 *base, HPoint3 *v1, HPoint3 *v2, int space)
{
  double d, n;
  HPoint3 v1n = *v1;
  HPoint3 v2n = *v2;
  HPt3SpaceGramSchmidt(base, &v1n, space);
  HPt3SpaceGramSchmidt(base, &v2n, space);
  d = HPt3SpaceDot(&v1n,&v1n,space) * HPt3SpaceDot(&v2n,&v2n,space);
  if (d <= 0.0) {
    fprintf(stderr,"HPt3SpaceAngle: invalid denominator\n");
    return 0.0;
  }
  n = HPt3SpaceDot(&v1n, &v2n, space);
  return acos( n / sqrt(d) );
}

#endif
