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

#include <math.h>
#include "radians.h"
#include "transform3.h"

/* 
 * Pre-multiply a matrix by a rotate about the x axis.
 *
 *       (  1  0  0  0 )      
 *       (  0  c  s  0 )      
 * [a] = (  0 -s  c  0 )  * [a]
 *       (  0  0  0  1 )      
 */
void
Ctm3RotateX( Transform3 T, float angle )
{
    int i;
    double t,s,c;

    s=sin(angle);
    c=cos(angle);
    for (i=0; i<4; ++i) {
      t = T[1][i] * c + T[2][i] * s;
      T[2][i] = T[2][i] * c - T[1][i] * s;
      T[1][i] = t;
    }
}

/* 
 * Pre-multiply a matrix by a rotate about the y axis.
 *
 *       (  c  0  s  0 )      
 *       (  0  1  0  0 )      
 * [a] = ( -s  0  c  0 ) * [a]
 *       (  0  0  0  1 )      
 */
void
Ctm3RotateY( Transform3 T, float angle )
{
    int i;
    double t,s,c;

    s=sin(angle);
    c=cos(angle);
    for (i=0; i<4; ++i) {
      t = T[2][i] * c - T[0][i] * s;
      T[0][i] = T[0][i] * c + T[2][i] * s;
      T[2][i] = t;
    }
}

/* 
 * Pre-multiply a matrix by a rotate about the z axis.
 *
 *       (  c  s  0  0 )      
 *       ( -s  c  0  0 )      
 * [a] = (  0  0  1  0 ) * [a]
 *       (  0  0  0  1 )      
 */
void
Ctm3RotateZ( Transform3 T, float angle )
{
    int i;
    double t,s,c;

    s=sin(angle);
    c=cos(angle);
    for (i=0; i<4; ++i) {
      t = T[0][i] * c + T[1][i] * s;
      T[1][i] = T[1][i] * c - T[0][i] * s;
      T[0][i] = t;
    }
}

/* 
 * Pre-multiply a matrix by a rotate about an arbitrary axis
 *
 * [a] = [ rotation] * [a]
 *             
 */
void
Ctm3Rotate( Transform3 T, float angle, Point3 *axis )
{
    if(      axis == &TM3_XAXIS ) Ctm3RotateX( T, angle );
    else if( axis == &TM3_YAXIS ) Ctm3RotateY( T, angle );
    else if( axis == &TM3_ZAXIS ) Ctm3RotateZ( T, angle );
    else {
	Transform3 Ta;

	Tm3Rotate( Ta, angle, axis );
	Tm3Concat(Ta, T, T);
    }
}
