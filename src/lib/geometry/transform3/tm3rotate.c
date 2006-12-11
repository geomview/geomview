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


/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner,
   Mark Phillips, Nathaniel Thurston */

#include <math.h>
#include "radians.h"
#include "transform3.h"
#include "point3.h"

Point3 TM3_XAXIS = { 1.0, 0.0, 0.0 };
Point3 TM3_YAXIS = { 0.0, 1.0, 0.0 };
Point3 TM3_ZAXIS = { 0.0, 0.0, 1.0 };

void
Tm3RotateX( Transform3 T, float angle )
{
    Tm3Identity( T );
    Ctm3RotateX( T, angle );
}

void
Tm3RotateY( Transform3 T, float angle )
{
    Tm3Identity( T );
    Ctm3RotateY( T, angle );
}

void
Tm3RotateZ( Transform3 T, float angle )
{
    Tm3Identity( T );
    Ctm3RotateZ( T, angle );
}

/*
 * Construct the matrix for a rotation about an axis.
 */
void
Tm3Rotate( Transform3 T, float angle, Point3 *axis )
{
    Point3 Vu;
    float sinA, cosA, versA;
	
    if(      axis == &TM3_XAXIS ) Tm3RotateX( T, angle );
    else if( axis == &TM3_YAXIS ) Tm3RotateY( T, angle );
    else if( axis == &TM3_ZAXIS ) Tm3RotateZ( T, angle );
    else {
	Pt3Copy( axis, (Point3 *)&Vu );
	Pt3Unit( &Vu );

	sinA = sin(angle); 
	cosA = cos(angle); 
	versA = 1 - cosA;

	Tm3Identity( T );
	T[TMX][TMX] = Vu.x*Vu.x*versA + cosA;
	T[TMY][TMX] = Vu.x*Vu.y*versA - Vu.z*sinA;
	T[TMZ][TMX] = Vu.x*Vu.z*versA + Vu.y*sinA;

	T[TMX][TMY] = Vu.y*Vu.x*versA + Vu.z*sinA;
	T[TMY][TMY] = Vu.y*Vu.y*versA + cosA;
	T[TMZ][TMY] = Vu.y*Vu.z*versA - Vu.x*sinA;

	T[TMX][TMZ] = Vu.x*Vu.z*versA - Vu.y*sinA;
	T[TMY][TMZ] = Vu.y*Vu.z*versA + Vu.x*sinA;
	T[TMZ][TMZ] = Vu.z*Vu.z*versA + cosA;
    }
}

/*
 * Construct the matrix for the geodesic rotation between two vectors.
 */
void
Tm3RotateBetween( Transform3 T, Point3 *vfrom, Point3 *vto )
{
	Point3 Vu;
	float len, sinA, cosA, versA;

	Tm3Identity( T );
	
	len = sqrt(Pt3Dot(vfrom,vfrom) * Pt3Dot(vto,vto));
	if(len == 0) return;

	cosA = Pt3Dot(vfrom,vto) / len;
	versA = 1 - cosA;

	Pt3Cross( vfrom, vto, &Vu );
	sinA = Pt3Length( &Vu ) / len;
	if(sinA == 0) return;

	Pt3Mul( 1/(len*sinA), &Vu, &Vu );	/* Normalize Vu */

	T[TMX][TMX] = Vu.x*Vu.x*versA + cosA;
	T[TMY][TMX] = Vu.x*Vu.y*versA - Vu.z*sinA;
	T[TMZ][TMX] = Vu.x*Vu.z*versA + Vu.y*sinA;

	T[TMX][TMY] = Vu.y*Vu.x*versA + Vu.z*sinA;
	T[TMY][TMY] = Vu.y*Vu.y*versA + cosA;
	T[TMZ][TMY] = Vu.y*Vu.z*versA - Vu.x*sinA;

	T[TMX][TMZ] = Vu.x*Vu.z*versA - Vu.y*sinA;
	T[TMY][TMZ] = Vu.y*Vu.z*versA + Vu.x*sinA;
	T[TMZ][TMZ] = Vu.z*Vu.z*versA + cosA;
}

/*-----------------------------------------------------------------------
 * Function:	Tm3RotateTowardZ
 * Description:	Rotation of 3-space moving pt to positive z-axis
 * Args:	T: output matrix
 *		pt: input point
 * Returns:	nothing
 * Author:	njt (comments by mbp)
 * Date:	(before) Tue Aug 18 23:31:58 1992
 */
void Tm3RotateTowardZ( Transform3 T, HPoint3 *pt )
{
  Transform3 S;
  float r = pt->z;
  /* Construct T = rotation about x-axis moving pt into x-z plane */
  Tm3Identity(T);
  r = sqrt(pt->y*pt->y + r*r);
  if (r > 0) {
    T[2][1] = -(T[1][2] = pt->y/r);
    T[2][2] = T[1][1] = pt->z/r;
  }
  /* Construct S = rotation about y axis moving T(pt) into y-z plane */
  Tm3Identity(S);
  r = sqrt(pt->x*pt->x + r*r);
  if (r > 0) {
    S[2][0] = -(S[0][2] = pt->x/r);
    S[2][2] = S[0][0] = sqrt(pt->z*pt->z + pt->y*pt->y)/r;
  }
  /* Desired transform is then S * T */
  Tm3Concat(T, S, T);
}

/* Tm3CarefulRotateTowardZ gives a matrix which rotates the world
 * about an axis perpendicular to both pos and the z axis, which moves
 * the -Z axis to pos (so [0,0,-1,0] * T = pos).
 * Unlike Tm3RotateTowardZ, the "twist" is well-defined
 * provided that it is not asked to rotate the negative z axis toward
 * the positive z axis.
 */
void Tm3CarefulRotateTowardZ( Transform3 T, HPoint3 *pos )
{
  Transform3 S;
  Transform3 Sinv;
  static HPoint3 minusZ = { 0,0,-1,0 };
  HPoint3 perp, axis, posxy;
  double dist, c, s;
  /* first, find a rotation takes both pos and the z axis into the xy plane */
  perp.x = -pos->y; perp.y = pos->x; perp.z = 0; perp.w = 1;
  Tm3RotateTowardZ(S, &perp);
   
  /* now, rotate pos and the -Z axis to this plane */
  HPt3Transform(S, &minusZ, &axis);
  HPt3Transform(S, pos, &posxy);

  /* find the rotation matrix for the transformed points */
  c = axis.x * posxy.x + axis.y * posxy.y;
  s = axis.x * posxy.y - axis.y * posxy.x;
  dist = sqrt(c*c+s*s);
  Tm3Identity(T);
  if (dist > 0) {
    c /= dist;
    s /= dist;
    T[0][0] =  c; T[0][1] = s;
    T[1][0] = -s; T[1][1] = c;
  } else if(pos->z > 0) {
    T[1][1] = T[2][2] = -1;	/* Singular rotation: arbitrarily flip YZ */
  } /* else no rotation */

  /* Finally, conjugate the result back to the original coordinate system */
  Tm3Invert(S, Sinv);
  Tm3Concat(S, T, T);
  Tm3Concat(T, Sinv, T);
}
