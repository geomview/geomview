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

/* {{{ Includes */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "mgP.h"
#include "mgbufP.h"

/* }}} */

static endPoint *mug=NULL;
static int mugSize = 0;

static int rdiv, gdiv, bdiv, rshift, gshift, bshift;

#ifdef __GNUC__
inline
#endif
static int
RGBtoVal(int r, int g, int b)
{
    return (((r>>rdiv) << rshift) | 
	    ((g>>gdiv) << gshift) | 
	    ((b>>bdiv) << bshift));
}

static void
decompose(int mask, int *div, int *shift)
{
    int i;
    for (i=0; ((mask>>1)<<1) == mask; i++, mask >>= 1)
	;
    *shift = i;
    for (i=1; mask>>1; i++, mask >>= 1)
	;
    *div = 8-i;
}


void
Xmgr_16fullinit(int rmask, int gmask, int bmask)
{
    decompose(rmask, &rdiv, &rshift);
    decompose(gmask, &gdiv, &gshift);
    decompose(bmask, &bdiv, &bshift);
}

/*
#ifdef __GNUC__
inline
#endif
static short
RGBtoVal(int r, int g, int b)
{
    return ((r>>3)<<11) | ((g>>2)<<5) | (b>>3);
}

void
Xmgr_16fullinit(int rmask, int gmask, int bmask)
{
    redmask = rmask;
    greenmask = gmask;
    bluemask = bmask;
}
*/


/* {{{ clearing routine */

void
Xmgr_16clear(unsigned char *buf, float *zbuf, int zwidth, 
	     int width, int height, int *color, int flag,
	     int fullclear, int xmin, int ymin, int xmax, int ymax)
{
    short *ptr = (short *)buf;
    int i, end, x, length, pos;
    short fill;
    fill = RGBtoVal(color[0], color[1], color[2]);

    if (mug==NULL)
    {
	mug = (endPoint *)malloc(sizeof(endPoint)*height);
	mugSize = height;
    }
    if (height>mugSize)
    {
	mug = (endPoint *)realloc(mug, sizeof(endPoint)*height);
	mugSize = height;
    }

    if (fullclear)
    {
	end = (width*height)/2;
	for (i=0; i<end; i++)
	    ptr[i] = fill;
	
	if (flag)
	    for (i=0; i<zwidth*height; i++)
		zbuf[i] = 1.0;
    }

    xmin = MAX(xmin,0);
    length = (MIN(zwidth-1,xmax)-xmin+1);
    ymin = MAX(ymin,0);
    ymax = MIN(height-1,ymax);
    for (i=ymin; i<=ymax; i++)
    {
	ptr = (short *)(buf+width*i+xmin*2);
	for (x=0; x<length; x++)
	    ptr[x] = fill;
    }
    length = MIN(zwidth-1,xmax)-xmin+1; 
    if (flag)
	for (i=ymin; i<=ymax; i++)
	{
	    pos = i*zwidth+xmin;
	    for (x=0; x<length; x++)
	        zbuf[pos+x] = 1.0;
	}
}

/* }}} */

/* {{{ single lines */

#define WIDENAME wideline
#define NAME Xmgr_16line
#define PTR_INCR width>>1
#define PTR_TYPE short
#define PTR_INIT (short *)(buf+y1*width+2*x1)
#define WIDEYDOPIXEL ptr[i*ptrIncr+x] = col;
#define WIDEXDOPIXEL ptr[y*ptrIncr+i] = col;
#define DOPIXEL *ptr = col;
#define VARIABLES short col = RGBtoVal(color[0], color[1], color[2]);
#include "MGRline.h"


#define WIDENAME wideZline
#define NAME Xmgr_16Zline
#define ZBUFFER
#define PTR_INCR width>>1
#define PTR_TYPE short
#define PTR_INIT (short *)(buf+y1*width+2*x1)
#define WIDEYDOPIXEL ptr[i*ptrIncr+x] = col;
#define WIDEXDOPIXEL ptr[y*ptrIncr+i] = col;
#define DOPIXEL *ptr = col;
#define VARIABLES short col = RGBtoVal(color[0], color[1], color[2]);
#include "MGRline.h"


#define WIDENAME wideGline
#define NAME Xmgr_16Gline
#define GOURAUD
#define COLOR
#define PTR_INCR width>>1
#define PTR_TYPE short
#define PTR_INIT (short *)(buf+y1*width+2*x1)
#define WIDEYDOPIXEL ptr[i*ptrIncr+x] = RGBtoVal(r,g,b);
#define WIDEXDOPIXEL ptr[y*ptrIncr+i] = RGBtoVal(r,g,b);
#define DOPIXEL *ptr = RGBtoVal(r,g,b);
#include "MGRline.h"


#define WIDENAME wideGZline
#define NAME Xmgr_16GZline
#define ZBUFFER
#define GOURAUD
#define COLOR
#define PTR_INCR width>>1
#define PTR_TYPE short
#define PTR_INIT (short *)(buf+y1*width+2*x1)
#define WIDEYDOPIXEL ptr[i*ptrIncr+x] = RGBtoVal(r,g,b);
#define WIDEXDOPIXEL ptr[y*ptrIncr+i] = RGBtoVal(r,g,b);
#define DOPIXEL *ptr = RGBtoVal(r,g,b);
#include "MGRline.h"

/* }}} */

/* {{{ polygon scan conversion */

#define NAME Xmgr_doLines
#define PTR_TYPE short
#define PTR_INIT (short *)(buf+width*y+x1*2)
#define VARIABLES short col=RGBtoVal(color[0], color[1], color[2]);
#define DOPIXEL *ptr = col;
#include "MGRdolines.h"


#define NAME Xmgr_ZdoLines
#define ZBUFFER
#define PTR_TYPE short
#define PTR_INIT (short *)(buf+width*y+x1*2)
#define VARIABLES short col=RGBtoVal(color[0], color[1], color[2]);
#define DOPIXEL *ptr = col;
#include "MGRdolines.h"


#define NAME Xmgr_GdoLines
#define GOURAUD
#define COLOR
#define PTR_TYPE short
#define PTR_INIT (short *)(buf+width*y+x1*2)
#define DOPIXEL *ptr = RGBtoVal(r,g,b);
#include "MGRdolines.h"


#define NAME Xmgr_GZdoLines
#define ZBUFFER
#define GOURAUD
#define COLOR
#define PTR_TYPE short
#define PTR_INIT (short *)(buf+width*y+x1*2)
#define DOPIXEL *ptr = RGBtoVal(r,g,b);
#include "MGRdolines.h"

void
Xmgr_16Zpoly(unsigned char *buf, float *zbuf, int zwidth,
	     int width, int height, CPoint3 *p, int n, int *color)
{
    Xmgr_Zpolyscan(buf, zbuf, zwidth, width, height, p, n, color, mug,
		   Xmgr_ZdoLines);
}

void
Xmgr_16poly(unsigned char *buf, float *zbuf, int zwidth, 
	    int width, int height, CPoint3 *p, int n, int *color)
{
    Xmgr_polyscan(buf, zbuf, zwidth, width, height, p, n, color, mug,
		  Xmgr_doLines);
}

void
Xmgr_16GZpoly(unsigned char *buf, float *zbuf, int zwidth, int width, int height,
	     CPoint3 *p, int n, int *dummy)
{
    Xmgr_GZpolyscan(buf, zbuf, zwidth, width, height, p, n, dummy, mug,
		    Xmgr_GZdoLines);
}

void
Xmgr_16Gpoly(unsigned char *buf, float *zbuf, int zwidth, int width, int height,
	     CPoint3 *p, int n, int *dummy)
{
    Xmgr_Gpolyscan(buf, zbuf, zwidth, width, height, p, n, dummy, mug,
		    Xmgr_GdoLines);
}

/* }}} */

/* {{{ multi-line scan conversion */

#ifdef __GNUC__
inline
#endif
static void
setZpixel(unsigned char *buf, float *zbuf, int zwidth, 
	  int width, int height, CPoint3 *p, int *color)
{
    if (p->z < zbuf[(int)(p->y)*zwidth+(int)p->x])
	((short *)buf)[(int)(p->y)*(width/2)+(int)p->x] = 
	    RGBtoVal(color[0], color[1], color[2]);
}

#ifdef __GNUC__
inline
#endif
static void
setpixel(unsigned char *buf, int zwidth, 
	 int width, int height, CPoint3 *p, int *color)
{
    ((short *)buf)[(int)(p->y)*(width/2)+(int)p->x] = 
	RGBtoVal(color[0], color[1], color[2]);
}

void
Xmgr_16polyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
		CPoint3 *p, int n, int lwidth, int *color)
{
    int i;

    if (n == 1)
    {
	setpixel(buf, zwidth, width, height, p, color);
	return;
    }

    for (i=0; i<n-1; i++)
	if (p[i].drawnext)
	    Xmgr_16line(buf, zbuf, zwidth, width, height, p+i, p+i+1, 
			 lwidth, color);
}


void
Xmgr_16Zpolyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
		 CPoint3 *p, int n, int lwidth, int *color)
{
    int i;

    if (n == 1)
    {
	setZpixel(buf, zbuf, zwidth, width, height, p, color);
	return;
    }

    for (i=0; i<n-1; i++)
	if (p[i].drawnext)
	    Xmgr_16Zline(buf, zbuf, zwidth, width, height, p+i, p+i+1, 
			 lwidth, color);
}

void
Xmgr_16Gpolyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
		 CPoint3 *p, int n, int lwidth, int *color)
{
    int i;

    if (n == 1)
    {
	setpixel(buf, zwidth, width, height, p, color);
	return;
    }

    for (i=0; i<n-1; i++)
	if (p[i].drawnext)
	    Xmgr_gradWrapper(buf, zbuf, zwidth, width, height, p+i, p+i+1, 
			     lwidth, Xmgr_16line, Xmgr_16Gline);
}

void
Xmgr_16GZpolyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
		 CPoint3 *p, int n, int lwidth, int *color)
{
    int i;

    if (n == 1)
    {
	setZpixel(buf, zbuf, zwidth, width, height, p, color);
	return;
    }

    for (i=0; i<n-1; i++)
	if (p[i].drawnext)
	    Xmgr_gradWrapper(buf, zbuf, zwidth, width, height, p+i, p+i+1, 
			     lwidth, Xmgr_16Zline, Xmgr_16GZline);
}

/* }}} */

/*
 * Local variables: ***
 * folded-file: t ***
 * End: ***
 */

