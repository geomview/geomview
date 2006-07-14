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
#include "radians.h"
#include "transform3.h"
#include "ooglutil.h"	/* For OOGLError() */

/*
 *
 *     ( cot(fov/2)/aspect       0           0           0	)
 *     (         0          cot(fov/2)       0           0	)
 * T = (         0               0     -(f+n)/(f-n)     -1	)
 *     (         0               0     -2*f*n/(f-n)      0	)
 *
 * Transform to the unit cube. Also flip from rh to lh.
 */
void
Tm3PerspectiveFOV( Transform3 T,
		   float fov, float aspect, float n, float f )
{
    float cotfov;

    Tm3Identity( T );

    if( n == f ) {
	OOGLError(1/*UserError*/, "Tm3Perspective: n and f must be different" );
	return;
    }
    if( fov == 0. ) {
	OOGLError(1/*UserError*/, "Tm3Perspective: fov must not equal 0" );
	return;
    }

    cotfov = tan( RADIANS(fov)/2.0 );
    if( cotfov != 0. )
	cotfov = 1. / cotfov;


    T[TMX][TMX] =  cotfov/aspect;
    T[TMY][TMY] =  cotfov;
    T[TMZ][TMZ] = -2*(f+n)/(f-n);
    T[TMW][TMZ] = -2*f*n/(f-n);
    T[TMX][TMW] = -1.;
    T[TMW][TMW] =  0.;
}
