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

#
/*
**	hpoint3.c - procedural interface to 3D point geometry
**
**	pat hanrahan
*/

#include <math.h>
#include "hg4.h"
#include "point3.h"
#include "hpoint3.h"
#include "tolerance.h"

void
HPt3Copy( HPoint3 *pt1, HPoint3 *pt2 )
{
    *pt2 = *pt1;
}

void
HPt3Normalize( HPoint3 *pt1, HPoint3 *pt2 )
{
    HPt3Dehomogenize( pt1, pt2 );
}

/*
 * pt2 = pt1 * T
 */
void
HPt3Transform( Transform3 T, HPoint3 *pt1, HPoint3 *pt2 )
{
    float x = pt1->x, y = pt1->y, z = pt1->z, w = pt1->w;

    pt2->x = x*T[0][0] + y*T[1][0] + z*T[2][0] + w*T[3][0];
    pt2->y = x*T[0][1] + y*T[1][1] + z*T[2][1] + w*T[3][1];
    pt2->z = x*T[0][2] + y*T[1][2] + z*T[2][2] + w*T[3][2];
    pt2->w = x*T[0][3] + y*T[1][3] + z*T[2][3] + w*T[3][3];
}

void
HPt3TransformN( Transform3 T, HPoint3 *pt1, HPoint3 *pt2, int n )
{
    while( --n >= 0 )
	HPt3Transform( T, pt1++, pt2++ );
}

/*
 * Transform and project an HPoint3 onto a plain Point3.
 * Transforms pin . T -> pout,
 * then projects pout.{x,y,z} /= pout.w.
 * Returns pout.w.
 */
HPt3Coord
HPt3TransPt3( Transform3 T, HPoint3 *pin, Point3 *pout )
{
    HPoint3 tp;

    HPt3Transform( T, pin, &tp );
    if(tp.w != 1.0 && tp.w != 0.0) {
	pout->x = tp.x / tp.w;
	pout->y = tp.y / tp.w;
	pout->z = tp.z / tp.w;
    } else {
	*pout = *(Point3 *)(void *)&tp;
    }
    return tp.w;
}

/* 
 * Pt3ToPt4: convert 3-vectors to 4-vectors by padding with 1.0 's.
 *
 * Charlie Gunn
 * Nov 26, 1991: originally written
 */

void
Pt3ToPt4( Point3 *v3, HPoint3 *v4, int n )
{
  int i;
  for (i = 0; i<n; ++i)	{
    *(Point3 *)&v4[i] = v3[i];
    v4[i].w = 1.0;
  }
}

void
HPt3ToPt3( HPoint3 *hp, Point3 *p )
{
  if(hp->w == 1.0 || hp->w == 0.0) {
    *p = *(Point3 *)hp;
  } else {
    p->x = hp->x / hp->w;
    p->y = hp->y / hp->w;
    p->z = hp->z / hp->w;
  }
}

/* also need an in-place dehomogenization routine which
sets the w-coordinate to 1.0, unless the original one is zero */
void 
HPt3Dehomogenize( HPoint3 *hp1, HPoint3 *hp2 )
{
  float inv;
  if (hp1->w == 1.0 || hp1->w == 0.0) {
	if (hp2 != hp1) *hp2 = *hp1; 
	return;
  }
  /* else if ( || hp->w == 0.0) hp->w = .000001;*/	
  inv = 1.0 / hp1->w;
    hp2->x = hp1->x * inv;
    hp2->y = hp1->y * inv;
    hp2->z = hp1->z * inv;
    hp2->w = 1.0;
}

void
HPt3SubPt3(HPoint3 *p1, HPoint3 *p2, Point3 *v)
{
    if(p1->w == p2->w) {
	v->x = p1->x - p2->x; v->y = p1->y - p2->y; v->z = p1->z - p2->z;
    } else if(p1->w == 0) {
	*v = *(Point3 *)p1;
	return;
    } else if(p2->w == 0) {
	*v = *(Point3 *)p2;
    } else {
	float s = p2->w / p1->w;
	v->x = p1->x*s - p2->x; v->y = p1->y*s - p2->y; v->z = p1->z*s - p2->z;
    }
    if(p2->w != 1)
	v->x /= p2->w, v->y /= p2->w, v->z /= p2->w;
}
    
	
