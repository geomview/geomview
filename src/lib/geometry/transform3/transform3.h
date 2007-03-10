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


/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef _GV_TRANSFORM3_H_
#define _GV_TRANSFORM3_H_

#include <stdio.h>
#include "geomtypes.h"

/*
 * Space & model flags; used to specify what space we're in, and
 * a model for hyperbolic space.  The actual value may be a logical
 * OR of a space and a model value.
 */
#define TM_HYPERBOLIC	0x0001
#define TM_EUCLIDEAN	0x0002
#define TM_SPHERICAL	0x0004

#define TM_SPACE(x) (x & 0x00ff)
#define TM_MODEL(x) (x & 0xff00)

#define TM_VIRTUAL		0x0100
#define TM_PROJECTIVE		0x0200
#define TM_CONFORMAL_BALL	0x0400

#define TMNULL  ((TransformPtr)NULL)     /* How to pass a NULL Transform3 ptr */

#define TMX 0
#define TMY 1
#define TMZ 2
#define TMW 3

extern Point3 TM3_XAXIS;
extern Point3 TM3_YAXIS;
extern Point3 TM3_ZAXIS;
extern Transform3 TM3_IDENTITY;

extern void Tm3Adjoint( Transform3, Transform3 );
extern float Tm3Invert( Transform3 T, Transform3 Tinv );
extern float Tm3Determinant( Transform3 T );
extern void Tm3Dual( Transform3 T, Transform3 Tdual );
extern void Tm3Transpose( Transform3 Ta, Transform3 Tb );
extern void Tm3PostConcat( Transform3 Ta, Transform3 Tb );
extern void Tm3PreConcat( Transform3 Ta, Transform3 Tb );
extern void Tm3Concat( Transform3 Ta, Transform3 Tb, Transform3 Tc );
extern void Tm3PolarDecomp( Transform3, Transform3 );

extern void Tm3Print(FILE *f, Transform3 T);
extern void Tm3Copy( Transform3 Tsrc, Transform3 Tdst );
extern int  Tm3Compare( Transform3 T1, Transform3 T2, float tol );
extern void Tm3Push( Transform3 T );
extern void Tm3Pop( Transform3 T );

extern void Tm3Conjugate( Transform3 T, Transform3 Tcon, Transform3 Tres );

extern void Tm3Identity( Transform3 T );
extern void Tm3Tetrad( Transform3 T, HPoint3 *x,HPoint3 *y,HPoint3 *z,HPoint3 *w );
extern void Tm3Tetrad3( Transform3 T, Point3 *x,Point3 *y,Point3 *z,Point3 *w );

extern void Tm3Translate( Transform3 T, Tm3Coord tx, Tm3Coord ty, Tm3Coord tz );
extern void Tm3TranslateOrigin( Transform3 T, HPoint3 *pt );

extern void Tm3HypTranslate( Transform3 T, Tm3Coord tx, Tm3Coord ty, Tm3Coord tz );
extern void Tm3HypTranslateOrigin( Transform3 T, HPoint3 *pt );

extern void Tm3SphTranslate( Transform3 T, Tm3Coord tx, Tm3Coord ty, Tm3Coord tz );
extern void Tm3SphTranslateOrigin( Transform3 T, HPoint3 *pt );
extern void Tm3SpaceTranslate( Transform3 T,
              Tm3Coord tx, Tm3Coord ty, Tm3Coord tz, int space );
extern void Tm3SpaceTranslateOrigin( Transform3 T, HPoint3 *pt, int space );

extern void Tm3RotateX( Transform3 T, float angle );
extern void Tm3RotateY( Transform3 T, float angle );
extern void Tm3RotateZ( Transform3 T, float angle );
extern void Tm3Rotate( Transform3 T, float angle, Point3 *axis );
extern void Tm3RotateBetween( Transform3 T, Point3 *from, Point3 *to );
extern void Tm3CarefulRotateTowardZ( Transform3 T, HPoint3 *pos );
extern void Tm3RotateTowardZ( Transform3 T, HPoint3 *pos );
extern void Tm3AlignX( Transform3 T, Point3 *axis );
extern void Tm3AlignY( Transform3 T, Point3 *axis );
extern void Tm3AlignZ( Transform3 T, Point3 *axis );
extern void Tm3Align( Transform3 T, Point3 *axis, Point3 *newaxis );
extern void Tm3Scale( Transform3 T, float sx, float sy, float sz );
extern void Tm3Skew( Transform3 T, float angle, Point3 *axis1, Point3 *axis2 );
extern void Tm3Shear( Transform3 T, Point3 *pt, HPlane3 *pl );
extern void Tm3Reflect( Transform3 T, HPlane3 *pl );
extern int  Tm3Refract( Transform3 T, HPlane3 *pl, float eta );
extern int  Tm3Refract2( Transform3 T, HPlane3 *pl, float eta );
extern void Tm3Project( Transform3 T, HPlane3 *pl, Point3 *pt );

extern void Tm3Orthographic( Transform3 T,
    float l, float r, float b, float t, float n, float f );
extern void Tm3Perspective( Transform3 T,
    float l, float r, float b, float t, float n, float f );
extern void Tm3Window( Transform3 T, float l, float r, float b, float t );

extern void Tm3PerspectiveFOV( Transform3 T, float fov, float aspect, float n, float f );


extern void Ctm3Translate( Transform3 T, float tx, float ty, float tz );
extern void Ctm3RotateX( Transform3 T, float angle );
extern void Ctm3RotateY( Transform3 T, float angle );
extern void Ctm3RotateZ( Transform3 T, float angle );
extern void Ctm3Rotate( Transform3 T, float angle, Point3 *axis );
extern void Ctm3AlignX( Transform3 T, Point3 *axis );
extern void Ctm3AlignY( Transform3 T, Point3 *axis );
extern void Ctm3AlignZ( Transform3 T, Point3 *axis );
extern void Ctm3Align( Transform3 T, Point3 *axis, Point3 *newaxis );
extern void Ctm3Scale( Transform3 T, float sx, float sy, float sz );
extern void Ctm3Skew( Transform3 T, float angle, Point3 *axis1, Point3 *axis2 );
extern void Ctm3Shear( Transform3 T, Point3 *pt, HPlane3 *pl );
extern void Ctm3Reflect( Transform3 T, HPlane3 *pl );
extern int  Ctm3Refract( Transform3 T, HPlane3 *pl, float eta );
extern int  Ctm3Refract2( Transform3 T, HPlane3 *pl, float eta );
extern void Ctm3Project( Transform3 T, HPlane3 *pl, Point3 *pt );

extern void Ctm3Orthographic( Transform3 T,
    float l, float r, float b, float t, float n, float f );
extern void Ctm3Perspective( Transform3 T,
    float l, float r, float b, float t, float n, float f );
extern void Ctm3Window( Transform3 T, float l, float r, float b, float t );

/* include it after defining all prototypes */

#include "point3.h"
#include "hpoint3.h"

#endif /* _GV_TRANSFORM3_H_ */
