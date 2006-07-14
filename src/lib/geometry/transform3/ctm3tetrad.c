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
 * Function:	Ctm3Tetrad
 * Description:	concatenate a transform with given columns to the right
 *			of another transform
 * Args:	T: the transform to modify
 *		x: column 1 of right factor
 *		y: column 2 of right factor
 *		z: column 3 of right factor
 *		w: column 4 of right factor
 * Returns:	nothing
 * Author:	hanrahan, mbp
 * Date:	Thu Aug  8 14:54:56 1991
 * Notes:	sets T to T * S, where S is the transform with
 *		  columns x, y, z, w.
 */
void
Ctm3Tetrad( T, x, y, z, w )
    Transform3 T;
    HPoint3 *x, *y, *z, *w;
{
    Transform3 Ta;

    Tm3Tetrad( Ta, x, y, z, w );
    Tm3Concat(Ta, T, T);
}
