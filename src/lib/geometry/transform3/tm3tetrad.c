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
 * Function:	Tm3Tetrad
 * Description:	create a matrix with given vectors as rows
 * Args:	T: the created matrix (OUTPUT)
 *		x: row 1 (INPUT)
 *		y: row 2 (INPUT)
 *		z: row 3 (INPUT)
 *		w: row 4 (INPUT)
 * Returns:	nothing
 * Author:	hanrahan, mbp
 * Date:	Thu Aug  8 13:06:48 1991
 * Notes:	
 */
void
Tm3Tetrad( Transform3 T, HPoint3 *x, HPoint3 *y, HPoint3 *z, HPoint3 *w )
{
    T[TMX][TMX] = x->x;
    T[TMX][TMY] = x->y;
    T[TMX][TMZ] = x->z;
    T[TMX][TMW] = x->w;

    T[TMY][TMX] = y->x;
    T[TMY][TMY] = y->y;
    T[TMY][TMZ] = y->z;
    T[TMY][TMW] = y->w;

    T[TMZ][TMX] = z->x;
    T[TMZ][TMY] = z->y;
    T[TMZ][TMZ] = z->z;
    T[TMZ][TMW] = z->w;

    T[TMW][TMX] = w->x;
    T[TMW][TMY] = w->y;
    T[TMW][TMZ] = w->z;
    T[TMW][TMW] = w->w;
}

void
Tm3Tetrad3( Transform3 T, Point3 *x, Point3 *y, Point3 *z, Point3 *w )
{
    T[TMX][TMX] = x->x;
    T[TMX][TMY] = x->y;
    T[TMX][TMZ] = x->z;
    T[TMX][TMW] = 0;

    T[TMY][TMX] = y->x;
    T[TMY][TMY] = y->y;
    T[TMY][TMZ] = y->z;
    T[TMY][TMW] = 0;

    T[TMZ][TMX] = z->x;
    T[TMZ][TMY] = z->y;
    T[TMZ][TMZ] = z->z;
    T[TMZ][TMW] = 0;

    T[TMW][TMX] = w->x;
    T[TMW][TMY] = w->y;
    T[TMW][TMZ] = w->z;
    T[TMW][TMW] = 1;
}
