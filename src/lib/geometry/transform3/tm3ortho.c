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
#include "transform3.h"

/*
 *     (     2/(r-l)          0             0        0 )
 *     (        0          2/(t-b)          0        0 )
 * T = (        0             0         -2/(f-n)     0 )
 *     (  -(r+l)/(r-l)  -(t+b)/(t-b)  -(f+n)/(f-n)   1 )
 *
 * Transform to the unit cube! Also flips from rh to lh. 
 */
void
Tm3Orthographic( Transform3 T, float l, float r, float b, float t, float n, float f )
{
    Tm3Identity( T );

    if( l == r ) {
	fprintf( stderr, "Tm3Orthographic: l and r must be different.\n" );
	return;
    }
    if( b == t ) {
	fprintf( stderr, "Tm3Orthographic: b and t must be different.\n" );
	return;
    }
    if( n == f ) {
	fprintf( stderr, "Tm3Orthographic: n and f must be different.\n" );
	return;
    }

    T[TMX][TMX] =  2/(r-l);
    T[TMY][TMY] =  2/(t-b);
    T[TMZ][TMZ] = -2/(f-n);
    T[TMW][TMX] = -(r+l)/(r-l);
    T[TMW][TMY] = -(t+b)/(t-b);
    T[TMW][TMZ] = -(f+n)/(f-n);
}
