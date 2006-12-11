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


/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips */

#include <stdio.h>
#include <math.h>
#include "3d.h"
#include "ooglutil.h"

static void PtNormalPlane(Point3 *base, Point3 *normal, HPoint3 *ans);
static void Proj(Point3 *a, Point3 *b, Point3 *ans);
static void Orth(Point3 *a, Point3 *b, Point3 *ans);
static float ParSgSgDistance(Point3 *a1, Point3 *a2, Point3 *adir,
			     Point3 *b1, Point3 *b2);
static float SgPtDistance(Point3 *p, Point3 *a1, Point3 *a2, Point3 *dir);
static void SgPlMinPoint(HPoint3 *pl, Point3 *a, Point3 *b,
			 Point3 *dir, Point3 *ans);
static void TComb(Point3 *a, float t, Point3 *dir, Point3 *b);
static int LnPlIntersect(HPoint3 *pl, Point3 *a, Point3 *dir, float *t);

/*-----------------------------------------------------------------------
 * Function:	SgSgDistance
 * Description:	distance between two line segments
 * Args:	a1,a2: first segment
 *		b1,b2: second segment
 * Returns:	distance between segment [a1,a2] and segment [b1,b2]
 * Author:	mbp
 * Date:	Mon Dec 23 15:17:30 1991
 * Notes:	This procedure is totally general; either or both
 *		segments may in fact be points.
 */
Pt3Coord SgSgDistance(Point3 *a1, Point3 *a2,
		      Point3 *b1, Point3 *b2)
{
  Point3 adir, bdir, amin, bmin, na, nb;
  HPoint3 aplane, bplane;
  Pt3Coord alen, blen, d, cosang;

  Pt3Sub(a2,a1, &adir);
  alen = Pt3Length( &adir );
  Pt3Sub(b2,b1, &bdir);
  blen = Pt3Length( &bdir );

  switch ( ((alen<1.e-12)<<1) | (blen<1.e-12) ) {
  case 1:
    /* b is a point, a is not */
    d = SgPtDistance(b1, a1,a2,&adir);
    return d;
  case 2:
    /* a is a point, b is not */
    d = SgPtDistance(a1, b1,b2,&bdir);
    return d;
  case 3:
    /* both a and b are points */
    d = Pt3Distance(a1,b1);
    return d;
  }

  /*
   * We fall thru the above switch if neither segment is a point.
   * Now check for parallelism.
   */
 
  cosang = Pt3Dot(&adir,&bdir) / ( alen * blen );
  if ( fabs((double)cosang)  > .99 ) {
    /* segments are essentially parallel */
    d = ParSgSgDistance(a1,a2,&adir,b1,b2);
  }
  else {
    /* segments are skew */
    Orth(&adir, &bdir, &na);
    Orth(&bdir, &adir, &nb);
    PtNormalPlane(a1, &na, &aplane);
    PtNormalPlane(b1, &nb, &bplane);
    SgPlMinPoint(&aplane, b1, b2, &bdir, &bmin);
    SgPlMinPoint(&bplane, a1, b2, &adir, &amin);
    d = Pt3Distance(&amin, &bmin);
  }
  return d;
}

/*-----------------------------------------------------------------------
 * Function:	PtNormalPlane
 * Description:	compute homog coords of a plane
 * Args:	*base: point on plane
 *		*normal: normal vector to plane
 *		*ans: homog coords of plane
 * Returns:	nothing
 * Author:	mbp
 * Date:	Mon Dec 23 15:08:46 1991
 */
static void PtNormalPlane(Point3 *base, Point3 *normal, HPoint3 *ans)
{
  ans->x = normal->x;
  ans->y = normal->y;
  ans->z = normal->z;
  ans->w = - Pt3Dot(base, normal);
}


/*-----------------------------------------------------------------------
 * Function:	Proj
 * Description:	projection of a onto b
 * Args IN:	a,b
 *	OUT:	ans
 * Returns:	nothing
 * Author:	mbp
 * Date:	Mon Dec 23 15:09:29 1991
 */
static void Proj(Point3 *a, Point3 *b, Point3 *ans)
{
  Pt3Mul( Pt3Dot(a,b) / Pt3Dot(a,a), a, ans );
}

/*-----------------------------------------------------------------------
 * Function:	Orth
 * Description:	orthogonalization of b wrt a
 * Args IN:	a, b
 *	OUT:	ans
 * Returns:	nothing
 * Author:	mbp
 * Date:	Mon Dec 23 15:09:54 1991
 * Notes:	answer is vector in plane spanned
 *		  by a & b, orthogonal to a.
 */
static void Orth(Point3 *a, Point3 *b, Point3 *ans)
{
  Point3 p;

  Proj(a,b,&p);
  Pt3Sub(b, &p, ans);
}

/*-----------------------------------------------------------------------
 * Function:	ParSgSgDistance
 * Description:	compute distance between two line segments on
 *		  parallel lines
 * Args:	a1, a2: first segment
 *		*adir: a2 - a1
 *		b1,b2: second segment
 * Returns:	distance between segments [a1,a2] and [b1,b2]
 * Author:	mbp
 * Date:	Mon Dec 23 15:11:38 1991
 * Notes:	segments must lie on parallel lines in order for
 *		  this to work.
 *		adir must be exactly a2 - a1 upon input.  It appears
 *		  as an argument to this procedure because we assume
 *		  the caller has already computed it, and we need it
 *		  here, so why recompute it?!!
 */
static Pt3Coord ParSgSgDistance(Point3 *a1, Point3 *a2, Point3 *adir,
			     Point3 *b1, Point3 *b2)
{
  Point3 b1p,b2p;
  HPoint3 b1plane, b2plane;
  Pt3Coord d,t1 = 0.0 ,t2 = 0.0;

  Pt3Sub(a2,a1,adir);
  PtNormalPlane(b1, adir, &b1plane);
  LnPlIntersect(&b1plane, a1, adir, &t1);
  TComb(a1, t1, adir, &b1p);
  d = Pt3Distance(b1, &b1p);
  if ( (t1>=0) && (t1<=1) )
    return d;
  PtNormalPlane(b2, adir, &b2plane);
  LnPlIntersect(&b2plane, a1, adir, &t2);
  TComb(a1, t2, adir, &b2p);
  if ( (t2>=0) && (t2<=1) )
    return d;
  if ( t2 > t1 ) {
    if ( t1 > 1 )
      d = Pt3Distance(a2,b1);
    else
      d = Pt3Distance(a1,b2);
  }
  else {
    if ( t2 > 1 )
      d = Pt3Distance(a2,b2);
    else
      d = Pt3Distance(a1,b1);
  }
  return d;
}

/*-----------------------------------------------------------------------
 * Function:	SgPtDistance
 * Description:	distance from a segment to a point
 * Args:	p: the point
 *		a1,a2: the segment
 *		*dir: a2 - a1
 * Returns:	the distance from p to segment [a1,a2]
 * Author:	mbp
 * Date:	Mon Dec 23 15:15:19 1991
 * Notes:	dir must be a2 - a1 upon input
 */
static Pt3Coord SgPtDistance(Point3 *p, Point3 *a1, Point3 *a2, Point3 *dir)
{
  HPoint3 pl;
  Point3 min;
  Pt3Coord d;

  PtNormalPlane(p, dir, &pl);
  SgPlMinPoint(&pl, a1, a2, dir, &min);
  d = Pt3Distance(p, &min);
  return d;
}


/*-----------------------------------------------------------------------
 * Function:	SgPlMinPoint
 * Description:	find the point of a segment closest to a plane
 * Args:	pl: homog coords of the plane
 *		a,b: the segment
 *		dir: b - a
 *		ans: the point of segment [a,b] closest to plane
 * Returns:	nothing
 * Author:	mbp
 * Date:	Mon Dec 23 15:19:02 1991
 * Notes:	dir must be b - a upon input
 */
static void SgPlMinPoint(HPoint3 *pl, Point3 *a, Point3 *b,
			 Point3 *dir, Point3 *ans)
{
  Pt3Coord t = 0.0;

  LnPlIntersect(pl, a, dir, &t);
  if (t <= 0)
    *ans = *a;
  else if (t >= 1)
    *ans = *b;
  else
    TComb(a, t, dir, ans);
}

/*-----------------------------------------------------------------------
 * Function:	TComb
 * Description:	form a t combination: b = a + t * dir
 * Args	IN:	a,dir,t
 *	OUT:	b: a + t * dir
 * Returns:	nothing
 * Author:	mbp
 * Date:	Mon Dec 23 15:20:43 1991
 */
static void TComb(Point3 *a, Pt3Coord t, Point3 *dir, Point3 *b)
{
  b->x = a->x + t * dir->x;
  b->y = a->y + t * dir->y;
  b->z = a->z + t * dir->z;
}

/*-----------------------------------------------------------------------
 * Function:	LnPlIntersect
 * Description:	intersect a plane with a line
 * Arg	IN:	pl: the plane
 *		a: base point of line
 *		dir: direction vector of line
 *	OUT:	t: t value such that a + t * dir lies on plane
 * Returns:	1 if successful, 0 if not
 * Author:	mbp
 * Date:	Mon Dec 23 15:22:25 1991
 * Notes:	return value of 0 means line is parallel to plane
 */
static int LnPlIntersect(HPoint3 *pl, Point3 *a, Point3 *dir, Pt3Coord *t)
{
  Pt3Coord d;

  d = pl->x*dir->x + pl->y*dir->y + pl->z*dir->z;
  if (d == 0) return 0;
  *t = - ( a->x * pl->x + a->y * pl->y + a->z * pl->z + pl->w ) / d;
  return 1;
}
