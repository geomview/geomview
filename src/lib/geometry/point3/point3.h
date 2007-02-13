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


/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips, Celeste Fowler */

#ifndef _GV_POINT3_H_
#define _GV_POINT3_H_

/*
**	point3.c - 3D vector arithmetic module.
**
**	pat hanrahan
**
** cH: now as inline functions for efficiency
**
*/
#include <stdio.h>
#include <math.h>
#include "tolerance.h"
#include "geomtypes.h"

#define Pt3X(pt) (pt)->x
#define Pt3Y(pt) (pt)->y
#define Pt3Z(pt) (pt)->z

extern
Pt3Coord SgSgDistance(Point3 *a1, Point3 *a2,
		      Point3 *b1, Point3 *b2);

static const Point3 Pt3Origin = { 0., 0., 0. };
static inline void Pt3Print(const Point3 *v);
static inline void Pt3From(Point3 *v, double x, double y, double z);
static inline void Pt3Copy(const Point3 *v1, Point3 *v2);
static inline void Pt3Add(const Point3 *v1, const Point3 *v2, Point3 *v3);
static inline void Pt3Sub(const Point3 *v1, const Point3 *v2, Point3 *v3);
static inline void Pt3Mul(double s, const Point3 *v1, Point3 *v2);
static inline Pt3Coord Pt3Dot(const Point3 *v1, const Point3 *v2);
static inline void Pt3Cross(const Point3 *v1, const Point3 *v2, Point3 *v3);
static inline Pt3Coord Pt3TripleDot(const Point3 *v1, const Point3 *v2,
				    const Point3 *v3);
static inline void Pt3TripleCross(const Point3 *v1, const Point3 *v2,
				  const Point3 *v3, Point3 *v4);
static inline Pt3Coord Pt3Length(const Point3 *v);
static inline Pt3Coord Pt3Distance(const Point3 *v1, const Point3 *v2);
static inline Pt3Coord Pt3Angle(const Point3 *v1, const Point3 *v2);
static inline void Pt3Unit(Point3 *v);
static inline void Pt3Lerp(double t,
			   const Point3 *v1, const Point3 *v2,  Point3 *v3);
static inline void Pt3Comb(double t1, const Point3 *v1,
			   double t2, const Point3 *v2, Point3 *v3);
static inline int Pt3Equal(const Point3 *v1, const Point3 *v2);
static inline int Pt3Parallel(const Point3 *v1, const Point3 *v2);
static inline int Pt3Perpendicular(const Point3 *v1, const Point3 *v2);
static inline void Pt3Transform(Transform3 T, Point3 *p1, Point3 *p2);
static inline void Pt3TransformN(Transform3 T, Point3 *p1, Point3 *p2, int n);
static inline void NormalTransform(Transform3 T, Point3 *a, Point3 *b);
static inline void NormalTransformN(Transform3 T, Point3 *a, Point3 *b, int n);

#include "transform3.h"

/* print vector */
static inline
void Pt3Print(const Point3 *v)
{
  /* Only works if Pt3Coord is a float */
  printf("[%g %g %g]\n", v->x, v->y, v->z);
}

static inline
void Pt3From(Point3 *v, double x, double y, double z)
{
  v->x = x;
  v->y = y;
  v->z = z;
}

/* v2 = v1 */
static inline
void Pt3Copy( const Point3 *v1, Point3 *v2 )
{
  v2->x = v1->x;
  v2->y = v1->y;
  v2->z = v1->z;
}

/* v3 = v1 + v2 */
static inline
void Pt3Add( const Point3 *v1, const Point3 *v2, Point3 *v3 )
{
  v3->x = v1->x + v2->x;
  v3->y = v1->y + v2->y;
  v3->z = v1->z + v2->z;
}

/* v3 = v1 - v2 */
static inline
void Pt3Sub( const Point3 *v1, const Point3 *v2, Point3 *v3)
{
  v3->x = v1->x - v2->x;
  v3->y = v1->y - v2->y;
  v3->z = v1->z - v2->z;
}

/* v2 = s * v1  */
static inline
void Pt3Mul( double s, const Point3 *v1, Point3 *v2 )
{
  v2->x = s*v1->x;
  v2->y = s*v1->y;
  v2->z = s*v1->z;
}


/* v1 . v2 */
static inline
Pt3Coord Pt3Dot( const Point3 *v1, const Point3 *v2 )
{
  return v1->x*v2->x+v1->y*v2->y+v1->z*v2->z;
}

/* v3 = v1 x v2 */
static inline
void Pt3Cross( const Point3 *v1, const Point3 *v2, Point3 *v3 )
{
  v3->x = v1->y*v2->z-v1->z*v2->y; 
  v3->y = v1->z*v2->x-v1->x*v2->z; 
  v3->z = v1->x*v2->y-v1->y*v2->x;
}

/* v1 . (v2 x v3) */
static inline
Pt3Coord Pt3TripleDot( const Point3 *v1, const Point3 *v2, const Point3 *v3 )
{
  return v1->x*(v2->y*v3->z-v2->z*v3->y) 
    + v1->y*(v2->z*v3->x-v2->x*v3->z) 
    + v1->z*(v2->x*v3->y-v2->y*v3->x);
}

/* v4 = (v1 x v2) x v3 */
static inline
void Pt3TripleCross( const Point3 *v1, const Point3 *v2, const Point3 *v3, Point3 *v4 )
{
  Point3 v;
 
  Pt3Cross( v1, v2, &v );
  Pt3Cross( &v, v3, v4 );
}

static inline
Pt3Coord Pt3Length( const Point3 *v )
{
  return sqrt( v->x*v->x + v->y*v->y + v->z*v->z );
}

static inline
Pt3Coord Pt3Distance( const Point3 *v1, const Point3 *v2 )
{
  Point3 v12;
	
  Pt3Sub(v1,v2,&v12);

  return Pt3Length(&v12);
}

static inline
Pt3Coord Pt3Angle( const Point3 *v1, const Point3 *v2 )
{

  return(acos(Pt3Dot(v1, v2) / (Pt3Length(v1) * Pt3Length(v2))));

}

static inline
void Pt3Unit( Point3 *v )
{
  Pt3Coord len;

  len = Pt3Length(v);
  if( len != 0. && len != 1. )
    Pt3Mul( 1./len, v, v );
}


/*
 * lerp - linear interpolation
 *
 * v3 = (1-t)*v1 + t*v2
 */
static inline
void Pt3Lerp( double t, const Point3 *v1, const Point3 *v2,  Point3 *v3 )
{
  v3->x = (1.-t)*v1->x + t*v2->x;
  v3->y = (1.-t)*v1->y + t*v2->y;
  v3->z = (1.-t)*v1->z + t*v2->z;
}

static inline
void Pt3Comb( double t1, const Point3 *v1, double t2, const Point3 *v2, Point3 *v3 )
{
  v3->x = t1*v1->x + t2*v2->x;
  v3->y = t1*v1->y + t2*v2->y;
  v3->z = t1*v1->z + t2*v2->z;
}


/* vectors equal */
static inline
int Pt3Equal( const Point3 *v1, const Point3 *v2 )
{
  Pt3Coord v;
	
  v = Pt3Distance(v1, v2);

  return fz(v);
}

/* parallel vectors */
static inline
int Pt3Parallel( const Point3 *v1, const Point3 *v2 )
{
  Point3 v3;
  Pt3Coord v;
	
  Pt3Cross(v1,v2,&v3);
  v=Pt3Length(&v3);

  return fz(v);
}

/* perpendicular vectors */
static inline
int Pt3Perpendicular( const Point3 *v1, const Point3 *v2 )
{
  Pt3Coord v;

  v=Pt3Dot(v1,v2);

  return fz(v);
}

static inline
void Pt3Transform( Transform3 T, Point3 *p1, Point3 *p2 )
{
  Pt3Coord x, y, z, w;

  x = p1->x;
  y = p1->y;
  z = p1->z;
  w = (T[TMX][TMW]*x + T[TMY][TMW]*y + T[TMZ][TMW]*z + T[TMW][TMW]);
  if(w != 1.0) {
    w = 1.0 / w;
    p2->x = w * (x*T[TMX][TMX] + y*T[TMY][TMX] + z*T[TMZ][TMX] + T[TMW][TMX]);
    p2->y = w * (x*T[TMX][TMY] + y*T[TMY][TMY] + z*T[TMZ][TMY] + T[TMW][TMY]);
    p2->z = w * (x*T[TMX][TMZ] + y*T[TMY][TMZ] + z*T[TMZ][TMZ] + T[TMW][TMZ]);
  } else {
    p2->x = x*T[TMX][TMX] + y*T[TMY][TMX] + z*T[TMZ][TMX] + T[TMW][TMX];
    p2->y = x*T[TMX][TMY] + y*T[TMY][TMY] + z*T[TMZ][TMY] + T[TMW][TMY];
    p2->z = x*T[TMX][TMZ] + y*T[TMY][TMZ] + z*T[TMZ][TMZ] + T[TMW][TMZ];
  }
}

static inline
void Pt3TransformN( Transform3 T, Point3 *p1, Point3 *p2, int n )
{
  while( n-- ) 
    Pt3Transform( T, p1++, p2++ );
}

/* Behold: if T is not orthogonal, then this function has to be called
 * with T^{-tr}, or the adj(T)^{tr}, otherwise the result will be
 * wrong.
 */
static inline
void NormalTransform(Transform3 Tdual, Point3 *a, Point3 *b)
{
#if 0
  Pt3Transform(T, a, b);
  b->x -= T[TMW][TMX];
  b->y -= T[TMW][TMY];
  b->z -= T[TMW][TMZ];
#else
  Pt3Coord x, y, z;

  x = a->x;
  y = a->y;
  z = a->z;
  b->x = x*Tdual[TMX][TMX] + y*Tdual[TMY][TMX] + z*Tdual[TMZ][TMX];
  b->y = x*Tdual[TMX][TMY] + y*Tdual[TMY][TMY] + z*Tdual[TMZ][TMY];
  b->z = x*Tdual[TMX][TMZ] + y*Tdual[TMY][TMZ] + z*Tdual[TMZ][TMZ];
#endif
  Pt3Unit(b);
}

static inline
void NormalTransformN( Transform3 T, Point3 *a, Point3 *b, int n )
{
  while ( n-- )
    NormalTransform( T, a++, b++ );
}


#endif /* _GV_POINT3_H_ */

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
