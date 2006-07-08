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

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

#include <math.h>
#include <stdio.h>
#include "transform.h"
#include "hpoint3.h"
#include "vec4.h"

int make_rotation(HPoint3 *v1, HPoint3 *v2, Transform m);

int make_tform(HPoint3 *p1, HPoint3 *p2, HPoint3 *p3, Transform m)
/* 	Generate a Euclidean isometry which moves (0,0,0) -> p1.
   Furthermore, 
	the vector (1,0,0) will go a vector tx based at p1 and ending at p2.
	the vector (0,1,0) will go a vector ty based at p1, perpendicular to tx,
   and lying in the plane of p1-p2-p3, in the "direction" of p3.	
	the vector (0,0,1) will go a vector tz based at p1, perpendicular to tx and ty.
	The rotational part is computed by make_rotation. See comments there */
{
    HPoint3 nv1, nv2;
    Transform trans;

    VSUB3(p2, p1, &nv1);
    VSUB3(p3, p2, &nv2);
    nv1.w = 1.0; nv2.w = 1.0;
    NORMALIZE3(&nv1);
    NORMALIZE3(&nv2);
    if ( !make_rotation(&nv1, &nv2, m))	/* if collinear, use most recent rotation */
	{
#ifdef DEBUG
	fprintf(stderr,"Non-independent vectors: make_rotation\n");
#endif
	return(-1);
	}
    /* compute translation to take p1->(0,0,0) */
    TmTranslate(trans, p1->x, p1->y, p1->z);
    /* first do the rotation, then the translation */
    TmConcat(m, trans, m);
    return(0);
    }

/* make rotation that takes (1,0,0) to v1; and (0,1,0) to the orthog. proj of v2 onto
   the perpendicular subspace of v1.  Return (0) if v1 and v2 are collinear. 
*/
int make_rotation(v1, v2, m)
HPoint3 *v1, *v2;
Transform m;
{
    double a;
    HPoint3  t1, t2;

    a = VDOT3(v1, v2);
    if (a > .9999 || a < -.9999)  return(0); 
    t1.x = v1->x * a;
    t1.y = v1->y * a;
    t1.z = v1->z * a;
    VSUB3(v2, &t1, &t2) 		/* now t2 is orthogonal to v1 */
    NORMALIZE3(&t2);
    TmIdentity(m);
    m[0][0] = v1->x;	m[0][1] = v1->y; 	m[0][2] = v1->z;
    m[1][0] = t2.x;	m[1][1] = t2.y; 	m[1][2] = t2.z;
    /* last row is cross product of the first two rows */
    m[2][0] = v1->y*t2.z - v1->z*t2.y;
    m[2][1] = v1->z*t2.x - v1->x*t2.z;
    m[2][2] = v1->x*t2.y - v1->y*t2.x;

    return(1);

}
