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


/* Author: Timothy Rowley */

#ifndef MGBUFRENDER24
#define MGBUFRENDER24

void 
Xmgr_24fullinit(int rmask, int gmask, int bmask);

void
Xmgr_24clear(unsigned char *buf, float *zbuf, int zwidth, 
	     int width, int height, int *color, int flag,
	     int fullclear, int xmin, int ymin, int xmax, int ymax);

void
Xmgr_24poly(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
	CPoint3 *p, int n, int *color);

void
Xmgr_24polyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
	CPoint3 *p, int n, int lwidth, int *color);

void
Xmgr_24line(unsigned char *buf, float *zbuf, int zwidth, int width, int height,
	CPoint3 *p1, CPoint3 *p2, int lwidth, int *color);

void
Xmgr_24Zpoly(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
	CPoint3 *p, int n, int *color);

void
Xmgr_24Zpolyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
	CPoint3 *p, int n, int lwidth, int *color);

void
Xmgr_24GZpolyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
	CPoint3 *p, int n, int lwidth, int *color);

void
Xmgr_24Gpolyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
	CPoint3 *p, int n, int lwidth, int *color);

void
Xmgr_24Zline(unsigned char *buf, float *zbuf, int zwidth,
	     int width, int height,
	     CPoint3 *p1, CPoint3 *p2, int lwidth, int *color);

void
Xmgr_24GZline(unsigned char *buf, float *zbuf, int zwidth,
	     int width, int height,
	     CPoint3 *p1, CPoint3 *p2, int lwidth, int *color);

void
Xmgr_24GZpoly(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
	CPoint3 *p, int n, int *dummy);

void
Xmgr_24Gpoly(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
	CPoint3 *p, int n, int *dummy);

#endif
