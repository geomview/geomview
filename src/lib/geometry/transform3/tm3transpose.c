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

/*-----------------------------------------------------------------------
 * Function:	Tm3Transpose
 * Description:	transpose a matrix
 * Args:	T: the matrix to transpose (INPUT)
 *		Ttrans: the transposed matrix (OUTPUT)
 * Returns:	nothing
 * Author:	hanrahan, mbp
 * Date:	Thu Aug  8 14:41:24 1991
 * Notes:	
 */
void
Tm3Transpose( T, Ttrans )
    Transform3 T, Ttrans;
{
    int i, j;
    double t;

    if( T != Ttrans ) {
        for( i=0; i<4; i++ )
            for( j=0; j<4; j++ )
                Ttrans[i][j] = T[j][i];
    }
    else {
        for( i=0; i<4; i++ ) 
            for( j=0; j<i; j++ ) {
                t = T[i][j];
                T[i][j] = T[j][i];
                T[j][i] = t;
            }
    }
}
