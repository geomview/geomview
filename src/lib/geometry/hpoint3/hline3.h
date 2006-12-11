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
**	hline3.h - procedural interface to 3D line geometry
**
**	pat hanrahan
*/

/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef _GV_HLINE3_H_
#define _GV_HLINE3_H_

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

#include <stdio.h>
#include <math.h>
#include "ooglutil.h"
#include "geomtypes.h"

static inline HLine3 *HLn3Create(void);
static inline void HLn3Delete(HLine3 *ln);
static inline void HLn3Print(HLine3 *ln);
static inline void HLn3Copy(HLine3 *ln1, HLine3 *ln2);
static inline int HLn3From2HPt3s(HLine3 *ln, HPoint3 *pt1, HPoint3 *pt2);
static inline int HLn3From2HPl3s(HLine3 *ln, HPlane3 *pl1, HPlane3 *pl2);
static inline int HLn3IntersectHPl3(HLine3 * ln, HPlane3 * pl, HPoint3 * pt);
static inline int HLn3IntersectHPt3(HLine3 * ln, HPoint3 * pt, HPlane3 * pl);
static inline int HLn3IntersectHLn3(HLine3 * ln1, HLine3 * ln2,
				    HPlane3 * pl, HPoint3 * pt);
static inline int HLn3Undefined(HLine3 * ln);
static inline int HLn3Infinity(HLine3 * ln);
static inline int HLn3Compare(HLine3 * ln1, HLine3 * ln2);
static inline int HLn3CoincidentHPt3(HLine3 * ln, HPoint3 * pt);
static inline int HLn3CoincidentHLn3(HLine3 * ln1, HLine3 * ln2);
static inline int HLn3CoincidentHPl3(HLine3 * ln, HPlane3 * pl);
static inline void HLn3Transform(Transform3 T, HLine3 * ln1, HLine3 * ln2);
static inline void HLn3Dual(HLine3 * ln, HLine3 * lndual);
static inline void HLn3Perp(HLine3 * ln, HLine3 * lnperp);

#include "hg4.h"
#include "hpoint3.h"
#include "hplane3.h"
#include "transform3.h"

#define HLN3_POINT_FORM -1
#define HLN3_PLANE_FORM  1
#define HLN3_DUAL_FORM(f) (-(f))

static inline HLine3 *HLn3Create(void)
{
  return OOGLNewE(HLine3, "HLine3");
}

static inline void HLn3Delete(HLine3 *ln)
{
  OOGLFree(ln);
}

static inline void HLn3Print(HLine3 *ln)
{
  printf("ln(%s) \n", ln->type == HLN3_POINT_FORM ? "pt" : "pl");
  Hg4Print2(ln->L);
}

static inline void HLn3Copy(HLine3 *ln1, HLine3 *ln2)
{
  memcpy((char *) ln2, (char *) ln1, sizeof(HLine3));
}

/*
**	Form the anti-symmetric line tensor from two points.
**
**	Note: pt * L = pl.
**	(1) pl is the plane formed from the line and the pt.
**	(2) each col of L is a plane containing the line.
**	(3) if pl is identically 0 then the pt lies on the line.
*/
static inline int HLn3From2HPt3s(HLine3 *ln, HPoint3 *pt1, HPoint3 *pt2)
{
  ln->type = HLN3_POINT_FORM;
  Hg4AntiProductPiQj(ln->L, (Hg4Tensor1Ptr)pt1, (Hg4Tensor1Ptr)pt2);

  return Hg4Undefined2(ln->L);
}

/*
**	Form the anti-symmetric line tensor from two planes.
**
**	Note: K * pl = pt.
**	(1) pt is the point formed from the line and the pl.
**	(2) each row of K is a point on the line.
**	(3) if pt is identically 0 then the pl lies on the line.
*/
static inline int HLn3From2HPl3s(HLine3 *ln, HPlane3 *pl1, HPlane3 *pl2)
{
  ln->type = HLN3_PLANE_FORM;
  Hg4AntiProductPiQj(ln->L, (Hg4Tensor1Ptr)pl1, (Hg4Tensor1Ptr)pl2);

  return Hg4Undefined2(ln->L);
}

static inline int
HLn3IntersectHPl3(HLine3 * ln, HPlane3 * pl, HPoint3 * pt)
{
  HLine3 aln;

  if (ln->type == HLN3_PLANE_FORM) {
    HLn3Dual(ln, &aln);
    ln = &aln;
  }
  return Hg4Intersect2(ln->L, (Hg4Tensor1Ptr)pl, (Hg4Tensor1Ptr)pt);
}

static inline int
HLn3IntersectHPt3(HLine3 * ln, HPoint3 * pt, HPlane3 * pl)
{
  HLine3 aln;

  if (ln->type == HLN3_POINT_FORM) {
    HLn3Dual(ln, &aln);
    ln = &aln;
  }
  return Hg4Intersect2(ln->L, (Hg4Tensor1Ptr)pt, (Hg4Tensor1Ptr)pl);
}


/*
**	predicate which tests for 3d line intersection and
**	if an intersection is found returns the point at which th
**	two lines cross and the plane in which the two lines lie.
**	
**	Note: One of the lines should be in the "plane-form" and the
**	other in the "point-form."
*/
static inline int
HLn3IntersectHLn3(HLine3 * ln1, HLine3 * ln2, HPlane3 * pl, HPoint3 * pt)
{
  HLine3 ln;

  if (ln1->type == ln2->type) {
    Hg4Dual((Hg4Tensor2Ptr)(void *)ln2, (Hg4Tensor2Ptr)(void *)&ln);
    ln2 = &ln;
  }

  if (ln1->type == HLN3_POINT_FORM)
    return Hg4Intersect4(ln1->L, ln2->L, (Hg4Tensor1Ptr)pt, (Hg4Tensor1Ptr)pl);
  else
    return Hg4Intersect4(ln1->L, ln2->L, (Hg4Tensor1Ptr)pl, (Hg4Tensor1Ptr)pt);
}

static inline int HLn3Undefined(HLine3 * ln)
{
  return Hg4Undefined2(ln->L);
}

static inline int HLn3Infinity(HLine3 * ln)
{
  return Hg4Infinity2(ln->L, ln->type == HLN3_PLANE_FORM);
}

static inline int HLn3Compare(HLine3 * ln1, HLine3 * ln2)
{
  /* Do the types have to agree? */
  return Hg4Compare2(ln1->L, ln2->L);
}

static inline int HLn3CoincidentHPt3(HLine3 * ln, HPoint3 * pt)
{
  HPlane3 pl;

  return HLn3IntersectHPt3(ln, pt, &pl);
}

static inline int HLn3CoincidentHLn3(HLine3 * ln1, HLine3 * ln2)
{
  HPlane3 pl;
  HPoint3 pt;

  if (HLn3IntersectHLn3(ln1, ln2, &pl, &pt))
    return HPl3Undefined(&pl);
  return 0;
}

static inline int HLn3CoincidentHPl3(HLine3 * ln, HPlane3 * pl)
{
  HPoint3 pt;

  return HLn3IntersectHPl3(ln, pl, &pt);
}

static inline void HLn3Transform(Transform3 T, HLine3 * ln1, HLine3 * ln2)
{
  /* Assume T is a point transform */
  if (ln1->type == HLN3_PLANE_FORM) {
    HLn3Dual(ln1, ln2);
    Hg4Transform2(T, ln2->L, ln2->L);
    HLn3Dual(ln2, ln2);
  } else {
    Hg4Transform2(T, ln1->L, ln2->L);
    ln2->type = ln1->type;
  }
}

/*
**	Convert the matrix formed from planes/points to that formed
**	from points/planes.
**
**	Note:
**	(1) pu-qt+sr=0.
**	(2) (s,q,p,0) is parallel to the line.
**	(3) (-r,t,-u,0) is a plane containing the line and the origin.
**	(4) K*L = 0.
*/
static inline void HLn3Dual(HLine3 * ln, HLine3 * lndual)
{
  Hg4Dual(ln->L, lndual->L);
  lndual->type = HLN3_DUAL_FORM(ln->type);
}

static inline void HLn3Perp(HLine3 * ln, HLine3 * lnperp)
{
  HPoint3 pt;
  HPlane3 pl;

  /* LnIntersectPl( ln, &HPl3Ideal, &pt ); */
  HPt3Copy((HPoint3 *) ln->L[3], &pt);
  HPt3Dual(&pt, &pl);
  HLn3From2HPl3s(lnperp, &pl, (HPlane3 *)&HPl3Ideal);
}

#endif /* _GV_HLINE3_H_ */

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
