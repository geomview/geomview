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
#include "transform3.h"
#include "ooglutil.h"	/* For OOGLError */

/*
 *     (  2*n/(r-l)      0             0          0 )
 *     (     0        2*n/(t-b)        0          0 )
 * T = ( (r+l)/(r-l) (t+b)/(t-b)  (f+n)/(n-f)    -1 )
 *     (     0		 0	  2*f*n/(n-f)     0 )
 *
 * Transform a row vector {xw,yw,zw,w} * T => {X,Y,Z,-z}
 * mapping the given viewing frustum into the cube -1 <= {X,Y,Z} <= 1,
 * with Z = -1 at near plane (z = -n), +1 at far plane (z = -f)
 * since the camera is looking in its -Z direction.
 * Here l and r are the x coordinates, and b and t the y coordinates,
 * of the edges of the viewing frustum at the near plane, z = -n.
 *
 * Note that n and f should be the positive distances to the near & far planes,
 * not the negative Z coordinates of those planes.
 */
void
Tm3Perspective( Transform3 T, float l, float r, float b, float t, float n, float f )
{
    Tm3Identity( T );

    if( l == r ) {
	OOGLError(1/*UserError*/, "Tm3Perspective: l and r must be different." );
	return;
    }
    if( b == t ) {
	OOGLError(1/*UserError*/, "Tm3Perspective: b and t must be different." );
	return;
    }
    if( n == f ) {
	OOGLError(1/*UserError*/, "Tm3Perspective: n and f must be different." );
	return;
    }

    T[TMX][TMX] = 2*n/(r-l);
    T[TMY][TMY] = 2*n/(t-b);
    T[TMZ][TMZ] = -(f+n)/(f-n);
    T[TMW][TMW] = 0.;
    T[TMZ][TMW] = -1;
    T[TMZ][TMX] = (r+l)/(r-l);
    T[TMZ][TMY] = (t+b)/(t-b);
    T[TMW][TMZ] = 2*n*f/(n-f);
}
