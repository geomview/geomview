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

#ifndef MGX11RENDER8
#define MGX11RENDER8

void
Xmgr_8clear(unsigned char *buf, float *zbuf, int zwidth, int width, int height,
	    int *color, int flag, int fullclear, int xmin, int ymin, int xmax, int ymax);

void
Xmgr_8poly(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
	   CPoint3 *p, int n, int *color);

void
Xmgr_8polyline(unsigned char *buf, float *zbuf, int zwidth,
	       int width, int height, CPoint3 *p, int n, int lwidth, int *color);

void
Xmgr_8line(unsigned char *buf, float *zbuf, int zwidth, int width, int height,
	   CPoint3 *p0, CPoint3 *p1, int lwidth, int *color);

void
Xmgr_8Zpoly(unsigned char *buf, float *zbuf, int zwidth, 
	    int width, int height, CPoint3 *p, int n, int *color);

void
Xmgr_8Zpolyline(unsigned char *buf, float *zbuf, int zwidth,
		int width, int height, CPoint3 *p, int n, int lwidth, int *color);

void
Xmgr_8Zline(unsigned char *buf, float *zbuf, int zwidth, int width, int height,
	CPoint3 *p0, CPoint3 *p1, int lwidth, int *color);

void
Xmgr_8Dpoly(unsigned char *buf, float *zbuf, int zwidth, 
	    int width, int height, CPoint3 *p, int n, int *color);

void
Xmgr_8Dpolyline(unsigned char *buf, float *zbuf, int zwidth,
		int width, int height, CPoint3 *p, int n, int lwidth, int *color);

void
Xmgr_8Dline(unsigned char *buf, float *zbuf, int zwidth, int width, int height,
	CPoint3 *p0, CPoint3 *p1, int lwidth, int *color);

void
Xmgr_8DZpoly(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
	CPoint3 *p, int n, int *color);

void
Xmgr_8DZpolyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
	CPoint3 *p, int n, int lwidth, int *color);

void
Xmgr_8DGpolyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
	CPoint3 *p, int n, int lwidth, int *color);

void
Xmgr_8DGZpolyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
	CPoint3 *p, int n, int lwidth, int *color);

void
Xmgr_8DZline(unsigned char *buf, float *zbuf, int zwidth, int width, int height,
	CPoint3 *p0, CPoint3 *p1, int lwidth, int *color);

void
Xmgr_8DGZpoly(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
	CPoint3 *p, int n, int *dummy);

void
Xmgr_8DGpoly(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
	CPoint3 *p, int n, int *dummy);

#endif
