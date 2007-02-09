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

Quad *
QuadComputeNormals( Quad *q )
{
    int i;
    Point3 nor;
    float len;
    HPoint3 *p;
    Point3 *n;

    /*
     * Only compute if not present!
     */
    if( !(q->geomflags & QUAD_N) ) {
	if(q->n == NULL)
	    q->n = OOGLNewNE(QuadN, q->maxquad, "QuadComputeNormals normals");
	p = &q->p[0][0];
	n = &q->n[0][0];
	for( i = q->maxquad; --i >= 0; p += 4) {

#define  TERM(S, T)	(p[0].S - p[1].S) * (p[1].T + p[0].T) + \
			(p[1].S - p[2].S) * (p[2].T + p[1].T) + \
			(p[2].S - p[3].S) * (p[3].T + p[2].T) + \
			(p[3].S - p[0].S) * (p[0].T + p[3].T)

	    nor.x = TERM(y, z);
	    nor.y = TERM(z, x);
	    nor.z = TERM(x, y);
	    len = nor.x*nor.x + nor.y*nor.y + nor.z*nor.z;
	    if( len != 0.0 ) {
		len = 1.0 / sqrt(len);
		nor.x *= len;
		nor.y *= len;
		nor.z *= len;
	    }
	    *n++ = nor;
	    *n++ = nor;
	    *n++ = nor;
	    *n++ = nor;
	}
	q->geomflags |= QUAD_N;
    }
    return q;
}
