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

/*
**	hpoint3.h - procedural interface to 3D point geometry
**
**	pat hanrahan
*/

/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef _GV_HPOINT3_H_
#define _GV_HPOINT3_H_

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

#include <math.h>
#include "tolerance.h"

/* for safety we make forward declarations for all functions defined *
 * here, only then we include other header files containing inline
 * functions, they will follow the same scheme and everything will
 * work.
 */
#include "geomtypes.h"

static inline HPoint3 *HPt3Create(void);
static inline void HPt3Delete(HPoint3 *pt);
static inline void HPt3Print(HPoint3 *pt);
static inline void HPt3Copy(HPoint3 *pt1, HPoint3 *pt2);
static inline void HPt3Add(HPoint3 *pt1, HPoint3 *pt2, HPoint3 *pt3);
static inline void
HPt3From(HPoint3 *pt, HPt3Coord x, HPt3Coord y, HPt3Coord z, HPt3Coord w);
static inline int
HPt3From3HPl3s(HPoint3 *pt, HPlane3 *pl1, HPlane3 *pl2, HPlane3 *pl3);
static inline int HPt3From2HLn3s( HPoint3 *pt, HLine3 *ln1, HLine3 *ln2 );
static inline int HPt3IntersectHPt3( HPoint3 *pt1, HPoint3 *pt2, HLine3 *ln );
static inline void
HPt3Pencil(HPt3Coord t1, HPoint3 *pt1,
	   HPt3Coord t2, HPoint3 *pt2, HPoint3 *pt3 );
static inline HPt3Coord HPt3DotHPl3(HPoint3 *pt, HPlane3 *pl);
static inline int HPt3Compare(HPoint3 *pt1, HPoint3 *pt2);
static inline int HPt3Undefined(HPoint3 *pt);
static inline int HPt3Infinity(HPoint3 *pt);
static inline int HPt3CoincidentHPl3(HPoint3 *pt, HPlane3 *pl);
static inline int HPt3CoincidentHLn3(HPoint3 *pt, HLine3 *ln);
static inline int HPt3CoincidentHPt3(HPoint3 *pt1, HPoint3 *pt2);
static inline void HPt3Transform(Transform3 T, HPoint3 *pt1, HPoint3 *pt2);
static inline int
HPt3TransformN(Transform3 T, HPoint3 *pt1, HPoint3 *pt2, int n);
static inline HPt3Coord HPt3TransPt3(Transform3 T, HPoint3 *pin, Point3 *pout);
static inline void Pt3ToHPt3(Point3 *v3, HPoint3 *v4, int n);
static inline void HPt3ToPt3(HPoint3 *hp, Point3 *p);
static inline void Pt4ToHPt3(HPoint3 *pt4, int *axes, HPoint3 *hp3);
static inline void HPt3Dehomogenize(HPoint3 *hp1, HPoint3 *hp2);
static inline void HPt3Dual(HPoint3 *pt, HPlane3 *pl);
static inline void
HPt3LinSum(HPt3Coord scale1, HPoint3 *in1,
	   HPt3Coord scale2, HPoint3 *in2, HPoint3 *out);
static inline void
HPt3LinSumDenorm(HPt3Coord scale1, HPoint3 *in1,
		 HPt3Coord scale2, HPoint3 *in2, HPoint3 *out);
static inline void HPt3SizeOne(HPoint3 *pt, HPoint3 *out);
static inline HPt3Coord HPt3R40Dot(HPoint3 *a, HPoint3 *b);
static inline HPt3Coord HPt3R31Dot(HPoint3 *a, HPoint3 *b);
static inline HPt3Coord HPt3R30Dot(HPoint3 *a, HPoint3 *b);
static inline HPt3Coord HPt3SpaceDot(HPoint3 *a, HPoint3 *b, int space);
static inline HPt3Coord HPt3DotPt3(HPoint3 *a, Point3 *b);
static inline void HPt3R40Normalize(HPoint3 *a);
static inline void HPt3R31Normalize(HPoint3 *a);
static inline void HPt3R30Normalize(HPoint3 *a);
static inline void HPt3SpaceNormalize(HPoint3 *a, int space);
static inline HPt3Coord HPt3HypDistance(HPoint3 *a, HPoint3 *b);
static inline HPt3Coord HPt3Distance(HPoint3 *a, HPoint3 *b);
static inline HPt3Coord HPt3SphDistance(HPoint3 *a, HPoint3 *b);
static inline HPt3Coord HPt3SpaceDistance(HPoint3 *a, HPoint3 *b, int space);
static inline void HPt3Sub(HPoint3 *a, HPoint3 *b, HPoint3 *aminusb);
static inline void HPt3Scale(HPt3Coord s, HPoint3 *a, HPoint3 *sa);
static inline void HPt3GramSchmidt(HPoint3 *base, HPoint3 *v);
static inline void HPt3HypGramSchmidt(HPoint3 *base, HPoint3 *v);
static inline void HPt3SphGramSchmidt(HPoint3 *base, HPoint3 *v);
static inline void HPt3SpaceGramSchmidt(HPoint3 *base, HPoint3 *v, int space);
#if 0
static inline HPt3Coord
HPt3Angle(HPoint3 *base, HPoint3 *v1, HPoint3 *v2);
static inline HPt3Coord
HPt3HypAngle(HPoint3 *base, HPoint3 *v1, HPoint3 *v2);
static inline HPt3Coord
HPt3SphAngle(HPoint3 *base, HPoint3 *v1, HPoint3 *v2);
static inline HPt3Coord
HPt3SpaceAngle(HPoint3 *base, HPoint3 *v1, HPoint3 *v2, int space);
#endif
static inline void HPt3SubPt3(HPoint3 *p1, HPoint3 *p2, Point3 *v);
static inline void HPt3MinMax(HPoint3 *min, HPoint3 *max, HPoint3 *other);
static inline void Pt3MinMax(HPoint3 *min, HPoint3 *max, HPoint3 *other);
static inline void Pt4MinMax(HPoint3 *min, HPoint3 *max, HPoint3 *other);

#include "hg4.h"
#include "hline3.h"
#include "hplane3.h"
#include "point3.h"

static inline HPoint3 *HPt3Create(void)
{
  return (HPoint3 *) Hg4Create();
}

static inline void
HPt3Delete( HPoint3 *pt )
{
  Hg4Delete( (Hg4Tensor1Ptr) pt );
}

static inline void
HPt3Print( HPoint3 *pt )
{
  Hg4Print( (Hg4Tensor1Ptr) pt );
}

static inline void HPt3Copy(HPoint3 *pt1, HPoint3 *pt2)
{
  *pt2 = *pt1;
}

static inline void
HPt3Add(HPoint3 *pt1, HPoint3 *pt2, HPoint3 *pt3)
{
  Hg4Add( (Hg4Tensor1Ptr) pt1, (Hg4Tensor1Ptr) pt2 , (Hg4Tensor1Ptr) pt3);
}

static inline void
HPt3From(HPoint3 *pt, HPt3Coord x, HPt3Coord y, HPt3Coord z, HPt3Coord w)
{
  Hg4From( (Hg4Tensor1Ptr)pt, x, y, z, w );
}

static inline int
HPt3From3HPl3s( HPoint3 *pt, HPlane3 *pl1, HPlane3 *pl2, HPlane3 *pl3 )
{
  return Hg4Intersect3( 
		       (Hg4Tensor1Ptr)pl1, (Hg4Tensor1Ptr)pl2, (Hg4Tensor1Ptr)pl3,
		       (Hg4Tensor1Ptr)pt, 1 );
}

static inline int
HPt3From2HLn3s( HPoint3 *pt, HLine3 *ln1, HLine3 *ln2 )
{
  HPlane3 pl;

  return HLn3IntersectHLn3( ln1, ln2, &pl, pt );
}

static inline int
HPt3IntersectHPt3( HPoint3 *pt1, HPoint3 *pt2, HLine3 *ln )
{
  return HLn3From2HPt3s( ln, pt1, pt2 );
}

static inline void
HPt3Pencil( HPt3Coord t1, HPoint3 *pt1, HPt3Coord t2, HPoint3 *pt2, HPoint3 *pt3 )
{
  Hg4Pencil( t1, (Hg4Tensor1Ptr)pt1,
	     t2, (Hg4Tensor1Ptr)pt2, (Hg4Tensor1Ptr)pt3);
}

static inline HPt3Coord
HPt3DotHPl3( HPoint3 *pt, HPlane3 *pl )
{
  return Hg4ContractPiQi( (Hg4Tensor1Ptr)pt, (Hg4Tensor1Ptr)pl );
}

static inline int
HPt3Compare( HPoint3 *pt1, HPoint3 *pt2 )
{
  return Hg4Compare( (Hg4Tensor1Ptr)pt1, (Hg4Tensor1Ptr)pt2 );
}

static inline int
HPt3Undefined( HPoint3 *pt )
{
  return Hg4Undefined( (Hg4Tensor1Ptr)pt );
}

static inline int
HPt3Infinity( HPoint3 *pt )
{
  return Hg4Infinity( (Hg4Tensor1Ptr)pt, 0 );
}

static inline int
HPt3CoincidentHPl3( HPoint3 *pt, HPlane3 *pl )
{
  return fzero(HPt3DotHPl3(pt,pl));
}

static inline int
HPt3CoincidentHLn3( HPoint3 *pt, HLine3 *ln )
{
  HPlane3 pl;

  return HLn3IntersectHPt3( ln, pt, &pl );
}

static inline int
HPt3CoincidentHPt3( HPoint3 *pt1, HPoint3 *pt2 )
{
  return Hg4Coincident( (Hg4Tensor1Ptr)pt1, (Hg4Tensor1Ptr)pt2 );
}

/*
 * pt2 = pt1 * T
 */
static inline void
HPt3Transform(Transform3 T, HPoint3 *pt1, HPoint3 *pt2)
{
  HPt3Coord x = pt1->x, y = pt1->y, z = pt1->z, w = pt1->w;

  pt2->x = x*T[0][0] + y*T[1][0] + z*T[2][0] + w*T[3][0];
  pt2->y = x*T[0][1] + y*T[1][1] + z*T[2][1] + w*T[3][1];
  pt2->z = x*T[0][2] + y*T[1][2] + z*T[2][2] + w*T[3][2];
  pt2->w = x*T[0][3] + y*T[1][3] + z*T[2][3] + w*T[3][3];
}

static inline int
HPt3TransformN(Transform3 T, HPoint3 *pt1, HPoint3 *pt2, int n)
{
  int res = 0;
  
  while (--n >= 0) {
    HPt3Transform(T, pt1++, pt2);
    if ((pt2++)->w != 1.0) {
      res = 1;
    }
  }
  return res;
}

/*
 * Transform and project an HPoint3 onto a plain Point3.
 * Transforms pin . T -> pout,
 * then projects pout.{x,y,z} /= pout.w.
 * Returns pout.w.
 */
static inline HPt3Coord
HPt3TransPt3( Transform3 T, HPoint3 *pin, Point3 *pout )
{
  HPoint3 tp;

  HPt3Transform( T, pin, &tp );
  if(tp.w != 1.0 && tp.w != 0.0) {
    pout->x = tp.x / tp.w;
    pout->y = tp.y / tp.w;
    pout->z = tp.z / tp.w;
  } else {
    pout->x = tp.x;
    pout->y = tp.y;
    pout->z = tp.z;
  }
  return tp.w;
}

/* 
 * Pt3ToHPt4: convert 3-vectors to 4-vectors by padding with 1.0 's.
 *
 * Charlie Gunn
 * Nov 26, 1991: originally written
 */
static inline void
Pt3ToHPt3(Point3 *v3, HPoint3 *v4, int n)
{
  int i;
  for (i = 0; i < n; ++i) {
    v4[i].x = v3[i].x;
    v4[i].y = v3[i].y;
    v4[i].z = v3[i].z;
    v4[i].w = 1.0;
  }
}

static inline void
HPt3ToPt3( HPoint3 *hp, Point3 *p )
{
  if(hp->w == 1.0 || hp->w == 0.0) {
    memcpy(p, hp, sizeof(Point3));
  } else {
    p->x = hp->x / hp->w;
    p->y = hp->y / hp->w;
    p->z = hp->z / hp->w;
  }
}

/* Transform a 4-point to a 3-point according to the mapping defined
 * in "axes"
 */
static inline void
Pt4ToHPt3(HPoint3 *pt4, int *axes, HPoint3 *hp3)
{
  HPt3Coord *from = (HPt3Coord *)pt4, *to = (HPt3Coord *)hp3;
  HPoint3 tmp;
  int i;

  if (!axes) {
    if (pt4 != hp3) {
      hp3->x = pt4->x;
      hp3->y = pt4->y;
      hp3->z = pt4->z;
    }
    hp3->w = 1.0;
    return;
  }
  
  if (pt4 == hp3) {
    tmp = *pt4;
    from = (HPt3Coord *)&tmp;
  }

  for (i = 0; i < 3; i++) {
    if (axes[i] > 3) {
      to[i] = 0.0;
    } else if (axes[i] != -1) {
      to[i] = from[axes[i]];
    }
  }
  hp3->w = 1.0;
}

/* also need an in-place dehomogenization routine which
   sets the w-coordinate to 1.0, unless the original one is zero */
static inline void 
HPt3Dehomogenize(HPoint3 *hp1, HPoint3 *hp2)
{
  HPt3Coord inv;
  if (hp1->w == 1.0 || hp1->w == 0.0) {
    if (true || hp2 != hp1) *hp2 = *hp1; 
    return;
  }
  /* else if ( || hp->w == 0.0) hp->w = .000001;*/	
  inv = 1.0 / hp1->w;
  hp2->x = hp1->x * inv;
  hp2->y = hp1->y * inv;
  hp2->z = hp1->z * inv;
  hp2->w = 1.0;
}

/* Fishy procedure */
static inline void
HPt3Dual( HPoint3 *pt, HPlane3 *pl )
{
  pl->a = pt->x;
  pl->b = pt->y;
  pl->c = pt->z;
  pl->d = pt->w;
}

static inline void
HPt3LinSum (HPt3Coord scale1, HPoint3 *in1, HPt3Coord scale2, HPoint3 *in2, HPoint3 *out)
{
  if ((in1->w == 0) || (in2->w == 0)) {
    out->w = 0;
    out->x = scale1 * in1->x + scale2 * in2->x;
    out->y = scale1 * in1->y + scale2 * in2->y;
    out->z = scale1 * in1->z + scale2 * in2->z;
    return;
  }
  out->w = 1;
  out->x = scale1 * (in1->x/in1->w) + scale2 * (in2->x/in2->w);
  out->y = scale1 * (in1->y/in1->w) + scale2 * (in2->y/in2->w);
  out->z = scale1 * (in1->z/in1->w) + scale2 * (in2->z/in2->w);
}

static inline void
HPt3LinSumDenorm(HPt3Coord scale1, HPoint3 *in1, HPt3Coord scale2, HPoint3 *in2, HPoint3 *out)
{
  if ((in1->w == 0) || (in2->w == 0)) {
    out->w = 0;
    out->x = scale1 * in1->x + scale2 * in2->x;
    out->y = scale1 * in1->y + scale2 * in2->y;
    out->z = scale1 * in1->z + scale2 * in2->z;
    return;
  }
  out->w = scale1 * in1->w + scale2 * in2->w;
  scale1 = scale1 * out->w / in1->w;
  scale2 = scale2 * out->w / in2->w;
  out->x = scale1 * in1->x + scale2 * in2->x;
  out->y = scale1 * in1->y + scale2 * in2->y;
  out->z = scale1 * in1->z + scale2 * in2->z;
}

static inline void
HPt3SizeOne ( HPoint3 *pt, HPoint3 *out )
{
  HPt3Coord size;

  size = sqrt (pt->x * pt->x + pt->y * pt->y + pt->z * pt->z);
  if (size == 0) return;
  out->x = pt->x / size;
  out->y = pt->y / size;
  out->z = pt->z / size;
  out->w = 1.;
}

/* inner product of R4 */
static inline HPt3Coord
HPt3R40Dot(HPoint3 *a, HPoint3 *b)
{
  return a->x*b->x + a->y*b->y + a->z*b->z + a->w*b->w;
}

/* inner product of R(3,1) */
static inline HPt3Coord
HPt3R31Dot(HPoint3 *a, HPoint3 *b)
{
  return a->x*b->x + a->y*b->y + a->z*b->z - a->w*b->w;
}

/* inner product of R(3,0) */
static inline HPt3Coord
HPt3R30Dot(HPoint3 *a, HPoint3 *b)
{
  double w2 = a->w*b->w;
  if (w2 == 1.0 || w2 == 0.0)
    return a->x*b->x + a->y*b->y + a->z*b->z;
  else
    return (a->x*b->x + a->y*b->y + a->z*b->z) / w2;
}

static inline HPt3Coord
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

static inline HPt3Coord
HPt3DotPt3(HPoint3 *a, Point3 *b)
{
  HPt3Coord scp;
  
  scp = a->x*b->x + a->y*b->y + a->z*b->z;
  if (a->w != 0 && a->w != 1.0) {
    return scp / a->w;
  } else {
    return scp;
  }
}

/* normalize a point a in R4 so that <a,a> = 1 */
static inline void
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
static inline void
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
static inline void
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

static inline void
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

static inline HPt3Coord
HPt3HypDistance(HPoint3 *a, HPoint3 *b)
{
  float aa, bb, ab;
  aa = HPt3R31Dot(a,a);
  bb = HPt3R31Dot(b,b);
  ab = HPt3R31Dot(a,b);
  return acosh( fabs(ab / sqrt( aa * bb ) ));
}

static inline HPt3Coord
HPt3Distance(HPoint3 *a, HPoint3 *b)
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

static inline HPt3Coord
HPt3SphDistance(HPoint3 *a, HPoint3 *b)
{
  return acos( HPt3R40Dot(a,b) / sqrt( HPt3R40Dot(a,a) * HPt3R40Dot(b,b) ) );
}

static inline HPt3Coord
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

static inline void
HPt3Sub(HPoint3 *a, HPoint3 *b, HPoint3 *aminusb)
{
  aminusb->x = a->x - b->x;
  aminusb->y = a->y - b->y;
  aminusb->z = a->z - b->z;
  aminusb->w = a->w - b->w;
}

static inline void
HPt3Scale(HPt3Coord s, HPoint3 *a, HPoint3 *sa)
{
  sa->x = s * a->x;
  sa->y = s * a->y;
  sa->z = s * a->z;
  sa->w = s * a->w;
}

static inline void
HPt3GramSchmidt(HPoint3 *base, HPoint3 *v)
{
  HPt3SpaceGramSchmidt(base, v, TM_EUCLIDEAN);
}

static inline void
HPt3HypGramSchmidt(HPoint3 *base, HPoint3 *v)
{
  HPt3SpaceGramSchmidt(base, v, TM_HYPERBOLIC);
}

static inline void
HPt3SphGramSchmidt(HPoint3 *base, HPoint3 *v)
{
  HPt3SpaceGramSchmidt(base, v, TM_SPHERICAL);
}

/* Modifies v to arrange that <base,v> = 0,
   i.e. v is a tangent vector based at base */
static inline void
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

static inline HPt3Coord
HPt3Angle(HPoint3 *base, HPoint3 *v1, HPoint3 *v2)
{
  /*
   * I'm not sure that this will work in the euclidean case!!! mbp
   */
  return HPt3SpaceAngle(base, v1, v2, TM_EUCLIDEAN);
}

static inline HPt3Coord
HPt3HypAngle(HPoint3 *base, HPoint3 *v1, HPoint3 *v2)
{
  return HPt3SpaceAngle(base, v1, v2, TM_HYPERBOLIC);
}

static inline HPt3Coord
HPt3SphAngle(HPoint3 *base, HPoint3 *v1, HPoint3 *v2)
{
  return HPt3SpaceAngle(base, v1, v2, TM_SPHERICAL);
}

static inline HPt3Coord
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

static inline void
HPt3SubPt3(HPoint3 *p1, HPoint3 *p2, Point3 *v)
{
  if (p1->w == p2->w) {
    v->x = p1->x - p2->x; v->y = p1->y - p2->y; v->z = p1->z - p2->z;
  } else if (p1->w == 0) {
    *v = *(Point3 *)p1;
    return;
  } else if (p2->w == 0) {
    *v = *HPoint3Point3(p2);
    Pt3Mul(-1.0, v, v);
    return;
  } else {
    HPt3Coord s = p2->w / p1->w;
    v->x = p1->x*s - p2->x; v->y = p1->y*s - p2->y; v->z = p1->z*s - p2->z;
  }
  if(p2->w != 1.0 && p2->w != 0.0)
    v->x /= p2->w, v->y /= p2->w, v->z /= p2->w;
}

/* assume that min and max are  dehomogenized */
static inline void HPt3MinMax(HPoint3 *min, HPoint3 *max, HPoint3 *other)
{
  HPt3Coord oc = other->w;

  if (oc == 0.0) {
    oc = 1.0;
  }
  if(oc * min->x > other->x) min->x = other->x / oc;
  else if(oc * max->x < other->x) max->x = other->x / oc;
  if(oc * min->y > other->y) min->y = other->y / oc;
  else if(oc * max->y < other->y) max->y = other->y / oc;
  if(oc * min->z > other->z) min->z = other->z / oc;
  else if(oc * max->z < other->z) max->z = other->z / oc;
}

static inline void Pt3MinMax(HPoint3 *min, HPoint3 *max, HPoint3 *other)
{
  if(min->x > other->x) min->x = other->x;
  else if(max->x < other->x) max->x = other->x;
  if(min->y > other->y) min->y = other->y;
  else if(max->y < other->y) max->y = other->y;
  if(min->z > other->z) min->z = other->z;
  else if(max->z < other->z) max->z = other->z;
}

static inline void Pt4MinMax(HPoint3 *min, HPoint3 *max, HPoint3 *other)
{
  if(min->x > other->x) min->x = other->x;
  else if(max->x < other->x) max->x = other->x;
  if(min->y > other->y) min->y = other->y;
  else if(max->y < other->y) max->y = other->y;
  if(min->z > other->z) min->z = other->z;
  else if(max->z < other->z) max->z = other->z;
  if(min->w > other->w) min->w = other->w;
  else if(max->w < other->w) max->w = other->w;
}

#endif /* _GV_HPOINT3_H_ */

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
