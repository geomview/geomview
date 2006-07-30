/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
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
/*
 * ColormapDefs.h
 * author: Brian T. Luense
 * date: August 18, 1994
 */

#ifndef COLORMAPDEFS_H
#define COLORMAPDEFS_H

#include 	"color.h"

/*Following are the data definitions that I used to store Geomview colormaps
in the routines I wrote dealing with those entities.*/

typedef struct			
{	float height;		/*height of mark along its direction*/
	ColorA c;		/*color of mark*/
} mark;

typedef struct
{	int dim;		/*dimension of the direction coloring along*/
	int np;			/*number of marks along the direction*/
	float *direction;	/*array of floats giving the direction*/
	mark **p;		/*array of pointer to the marks*/
} dir;

typedef struct
{	char **coordsys;	/*array of strings of length numdirs giving the
				  name of the coordinate system in which the 
				 colormap is specified*/
	int numdirs;		/*number of directions along which to color*/
	dir **plex;		/*array of pointers to dirs of length numdirs*/
} colormap;

#endif
