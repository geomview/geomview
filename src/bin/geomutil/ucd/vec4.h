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
#define VSUB3(p0, p1, p2)		\
    { (p2)->x  = (p1)->x - (p0)->x;		\
    (p2)->y  = (p1)->y - (p0)->y;		\
    (p2)->z  = (p1)->z - (p0)->z;  }

#define VDOT3(p0, p1)		\
    ((p0)->x*(p1)->x + (p0)->y*(p1)->y + (p0)->z*(p1)->z)

#define NORMALIZE3(xx)	\
	{float x = sqrt((xx)->x*(xx)->x + (xx)->y*(xx)->y +(xx)->z*(xx)->z); \
	if (x == 0) x = 1.0;	\
	x = 1.0/x;		\
	(xx)->x	*= x;   (xx)->y *= x;   (xx)->z *= x;}	
