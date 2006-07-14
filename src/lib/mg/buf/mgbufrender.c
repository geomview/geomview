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



/* Author: Timothy Rowley */

#include <limits.h>
#include "mgP.h"
#include "mgbufP.h"

void
Xmgr_gradWrapper(unsigned char *buf, float *zbuf, int zwidth, int width,
	int height, CPoint3 *p0, CPoint3 *p1, int lwidth,
	void (*flat)(unsigned char *, float *, int, int, int, CPoint3 *,
			CPoint3 *, int, int *),
	void (*smooth)(unsigned char *, float *, int, int, int, CPoint3 *,
			CPoint3 *, int, int *))
{
    int color[3];
    
    if ((p0->vcol.r == p1->vcol.r) && (p0->vcol.g == p1->vcol.g) &&
	(p0->vcol.b == p1->vcol.b))
    {
	color[0] = 255*p0->vcol.r;
	color[1] = 255*p0->vcol.g;
	color[2] = 255*p0->vcol.b;
	flat(buf, zbuf, zwidth, width, height, p0, p1, lwidth, color);
    }
    else
	smooth(buf, zbuf, zwidth, width, height, p0, p1, lwidth, color);
}

void
oldXmgr_gradWrapper(unsigned char *buf, float *zbuf, int zwidth, int width,
	int height, CPoint3 *p0, CPoint3 *p1, int lwidth,
	void (*flat)(unsigned char *, float *, int, int, int, int, int, float,
			int, int, float, int, int *),
	void (*smooth)(unsigned char *, float *, int, int, int, CPoint3 *,
			CPoint3 *, int))
{
    int color[3];
    
    if ((p0->vcol.r == p1->vcol.r) && (p0->vcol.g == p1->vcol.g) &&
	(p0->vcol.b == p1->vcol.b))
    {
	color[0] = 255*p0->vcol.r;
	color[1] = 255*p0->vcol.g;
	color[2] = 255*p0->vcol.b;
	flat(buf, zbuf, zwidth, width, height, p0->x, p0->y, p0->z,
	     p1->x, p1->y, p1->z, lwidth, color);
    }
    else
	smooth(buf, zbuf, zwidth, width, height, p0, p1, lwidth);
}

#define NAME Xmgr_polyscan
#include "MGRpolyscan.h"

#define NAME Xmgr_Zpolyscan
#define ZBUFFER
#include "MGRpolyscan.h"

#define NAME Xmgr_Graypolyscan
#define GOURAUD
#include "MGRpolyscan.h"

#define NAME Xmgr_GrayZpolyscan
#define ZBUFFER
#define GOURAUD
#include "MGRpolyscan.h"

#define NAME Xmgr_Gpolyscan
#define GOURAUD
#define COLOR
#include "MGRpolyscan.h"

#define NAME Xmgr_GZpolyscan
#define GOURAUD
#define COLOR
#define ZBUFFER
#include "MGRpolyscan.h"
