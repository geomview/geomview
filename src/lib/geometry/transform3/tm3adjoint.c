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

#include "transform3.h"

#define det( a1, a2, a3, b1, b2, b3, c1, c2, c3 ) \
    (a1*(b2*c3-c2*b3) - a2*(b1*c3-c1*b3) + a3*(b1*c2-c1*b2))

static float cofactor(Transform3 T, int x, int y)
{
    static Tm3Coord mat3x3[3][3];
    Tm3Coord *dst = mat3x3[0];
    Tm3Coord *src = T[0];
    int i;

    for( i=0; i<4; i++, src += 4 ) {
	if(i == x)
	    continue;
	if(y != 0) *dst++ = src[0];
	if(y != 1) *dst++ = src[1];
	if(y != 2) *dst++ = src[2];
	if(y != 3) *dst++ = src[3];
    }
    return det( mat3x3[0][0], mat3x3[0][1], mat3x3[0][2],
                mat3x3[1][0], mat3x3[1][1], mat3x3[1][2],
                mat3x3[2][0], mat3x3[2][1], mat3x3[2][2] );
}


static void adjoint(Transform3 T, Transform3 Tadj)
{
    int x, y;
    float cof;

    for( x=0; x<4; x++ )
	for( y=0; y<4; y++ ) {
	    cof = cofactor( T, y, x );
	    Tadj[x][y] = ((x+y)&1) ? -cof : cof;
	}
}

/*-----------------------------------------------------------------------
 * Function:	Tm3Adjoint
 * Description:	compute the adjoint of a transform
 * Args:	T: the transform (INPUT)
 *		Tadj: the adjoint of T (OUTPUT)
 * Returns:	nothing
 * Author:	hanrahan, mbp
 * Date:	Thu Aug  8 15:38:56 1991
 * Notes:	
 */
void
Tm3Adjoint(Transform3 T, Transform3 Tadj)
{
    if( T == Tadj ) {
	Transform3 Ttmp;

	adjoint( T, Ttmp );
	Tm3Copy( Ttmp, Tadj );
    } else {
	adjoint( T, Tadj );
    }
}

/*-----------------------------------------------------------------------
 * Function:	determinant
 * Description:	compute the determinant of a transform, using an already
 *		  computed adjoint transform
 * Args:	T: the transform (INPUT)
 *		Tadj: T's adjoint transform (INPUT)
 * Returns:	det(T)
 * Author:	hanrahan, mbp
 * Date:	Thu Aug  8 15:23:53 1991
 * Notes:	
 */
static
float
determinant( T, Tadj )
    Transform3 T, Tadj;
{
    return T[0][0]*Tadj[0][0] 
        + T[0][1]*Tadj[1][0] 
        + T[0][2]*Tadj[2][0] 
        + T[0][3]*Tadj[3][0];
}

/*-----------------------------------------------------------------------
 * Function:	Tm3Determinant
 * Description:	compute the determinant of a transform
 * Args:	T: the transform (INPUT)
 * Returns:	det(T)
 * Author:	mbp
 * Date:	Thu Aug  8 15:27:52 1991
 * Notes:	
 */
float
Tm3Determinant(Transform3 T)
{
    Transform3 Tadj;

    Tm3Adjoint( T, Tadj );

    return determinant( T, Tadj );
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
