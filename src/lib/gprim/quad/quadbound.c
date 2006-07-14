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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "quadP.h"

BBox *
QuadBound( Quad *q, Transform T )
{
    register int v;
    HPoint3 min, max;
    register HPoint3 *p;
    Point3 p0;

    p = q->p[0];

#ifdef BBOX_ND_HACK
    if(q->flags & QUAD_4D)
	return BBox_ND_hack(NULL, p, 4*q->maxquad);
#endif

    if (T != TM_IDENTITY)
	    HPt3TransPt3(T, p, (Point3 *)(void *)&min);
    else
	HPt3Normalize(p, &min);
    max = min;
    for( v = 4 * q->maxquad; --v > 0; ) {
        p++;
        if (T != TM_IDENTITY)
	    HPt3TransPt3(T, p, (Point3 *)&p0);
	else {
	    p0 = *(Point3 *)p;
	    if(p->w != 1.0 && p->w != 0.0)
		p0.x /= p->w, p0.y /= p->w, p0.z /= p->w;
	}
        if(min.x > p0.x) min.x = p0.x;
        else if(max.x < p0.x) max.x = p0.x;
        if(min.y > p0.y) min.y = p0.y;
        else if(max.y < p0.y) max.y = p0.y;
        if(min.z > p0.z) min.z = p0.z;
        else if(max.z < p0.z) max.z = p0.z;
    }
    
    return (BBox *) GeomCCreate (NULL, BBoxMethods(),
		CR_MIN, &min, CR_MAX, &max, CR_END);
}

