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

#ifndef POINT3DEFS
#define POINT3DEFS
/*
** Definitions for 3D float points:
*/

typedef float Pt3Coord;
typedef struct { Pt3Coord x, y, z; } Point3;

void Pt3Print(const Point3 *p);
void Pt3From(Point3 *p, double x, double y, double z);
void Pt3Copy(const Point3 *a, Point3 *b);

void Pt3Add(const Point3 *a, const Point3 *b, Point3 *c);
void Pt3Sub(const Point3 *a, const Point3 *b, Point3 *c);
void Pt3Neg(const Point3 *a, Point3 *b);
void Pt3Mul(double s, const Point3 *a, Point3 *b);
void Pt3Comb(double sa, const Point3 *a, 
	     double sb, const Point3 *b, Point3 *c);
void Pt3Lerp(double t, const Point3 *a, const Point3 *b, Point3 *c);

Pt3Coord Pt3Dot(const Point3 *a, const Point3 *b);
void Pt3Cross(const Point3 *a, const Point3 *b, Point3 *c);
Pt3Coord Pt3TripleDot(const Point3 *a, const Point3 *b, const Point3 *c);
void Pt3TripleCross(const Point3 *a, const Point3 *b, const Point3 *c, 
		    Point3 *d);

Pt3Coord Pt3Distance(const Point3 *a, const Point3 *b);
Pt3Coord Pt3Length(const Point3 *p);
Pt3Coord Pt3Angle(const Point3 *a, const Point3 *b);

int Pt3Equal(const Point3 *a, const Point3 *b);
int Pt3Parallel(const Point3 *a, const Point3 *b);
int Pt3Perpendicular(const Point3 *a, const Point3 *b);

/* Note -- this was incorrect -- there is only one arg
   Tamara Munzner 7/17/90 */
void Pt3Unit(Point3 *a);

/* these should be in transform3.h, not here --- Oliver 11/20/92 */

void Pt3Transform(/* Transform3 T, const Point3 *a, Point3 *b */);
void Pt3TransformN(/* Transform3 T, const Point3 *a, Point3 *b, int n */);

void NormalTransform(/* Transform3 T, const Point3 *a, Point3 *b */);
void NormalTransformN(/* Transform3 T, const Point3 *a, Point3 *b, int n */);

extern Point3 Pt3Origin;

#define Pt3X(pt) (pt)->x
#define Pt3Y(pt) (pt)->y
#define Pt3Z(pt) (pt)->z

float SgSgDistance(Point3 *a1, Point3 *a2,
		   Point3 *b1, Point3 *b2);


#endif
