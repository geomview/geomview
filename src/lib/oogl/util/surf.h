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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "material.h"
#include "3d.h"

/***********************************************************************
 * a surf is a primitive bit of surface, conveys the geometrical
 * information about an intersection
 ***********************************************************************/

#define SURF_NEEDS_SET(p, n, s)	((n)&(p) && !((s)&(p)))
#define SURF_SET(p, f)		((f) |= (p))

#define	SURF_NONE	0x0
#define SURF_P  	0x1
#define SURF_N  	0x2
#define SURF_NG		0x4
#define SURF_DPDU  	0x8
#define SURF_DPDV  	0x10
#define SURF_UV  	0x20
#define SURF_DUDV  	0x40
#define SURF_ST  	0x80
#define SURF_I  	0x100
#define SURF_CS  	0x200
#define SURF_OS  	0x400

typedef struct surf {
	int	flag ;		/* copied mostly from Renderman */
	Point3	P ;		/* location of the point 	*/
	Point3	dPdu ;		/* derivatives 			*/
	Point3	dPdv ;
	Point3	N ;		/* shading normal vector	*/
	Point3	Ng ;		/* geometric normal		*/
	Point2	uv ;		/* location on the surface	*/
	Point2	dudv ;		/* derivatives of u, v		*/
	Point2	st ;		/* texture coordinates 		*/
	Point3	I ;		/* incident ray direction	*/
	Material mat;
	Transform3 T;
	Geom     *geom;
} Surf ;
