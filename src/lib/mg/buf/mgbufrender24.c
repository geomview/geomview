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
#include "mgP.h"
#include "mgbufP.h"

/* }}} */

static endPoint *mug=NULL;
static int mugSize = 0;

static int rshift, bshift, gshift;

#ifdef __GNUC__
inline
#endif
static int
RGBtoVal(int r, int g, int b)
{
    return ((r << rshift) | (g << gshift) | (b << bshift));
}

static int
maskShift(int mask)
{
    switch (mask)
    {
    case 0xFF000000U:
	return 24;
    case 0xFF0000U:
	return 16;
    case 0xFF00U:
	return 8;
    case 0xFFU:
	return 0;
    default:
	return 32;
    }
}

void
Xmgr_24fullinit(int rmask, int gmask, int bmask)
{
    rshift = maskShift(rmask);
    gshift = maskShift(gmask);
    bshift = maskShift(bmask);
}

/* {{{ clearing routine */

void
Xmgr_24clear(unsigned char *buf, float *zbuf, int zwidth, 
	     int width, int height, int *color, int flag,
	     int fullclear, int xmin, int ymin, int xmax, int ymax)
{
    int *ptr = (int *)buf;
    int i, fill, end, x, length, pos;
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
	end = (width*height)/4;
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
	ptr = (int *)(buf+width*i+xmin*4);
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
#define NAME Xmgr_24line
#define PTR_INCR width>>2
#define PTR_TYPE int
#define PTR_INIT (int *)(buf+y1*width+4*x1)
#define WIDEYDOPIXEL ptr[i*ptrIncr+x] = col;
#define WIDEXDOPIXEL ptr[y*ptrIncr+i] = col;
#define DOPIXEL *ptr = col;
#define VARIABLES int col = RGBtoVal(color[0], color[1], color[2]);
#include "MGRline.h"


#define WIDENAME wideZline
#define NAME Xmgr_24Zline
#define PTR_INCR width>>2
#define ZBUFFER
#define PTR_TYPE int
#define PTR_INIT (int *)(buf+y1*width+4*x1)
#define WIDEYDOPIXEL ptr[i*ptrIncr+x] = col;
#define WIDEXDOPIXEL ptr[y*ptrIncr+i] = col;
#define DOPIXEL *ptr = col;
#define VARIABLES int col = RGBtoVal(color[0], color[1], color[2]);
#include "MGRline.h"


#define WIDENAME wideGline
#define NAME Xmgr_24Gline
#define PTR_INCR width>>2
#define GOURAUD
#define COLOR
#define PTR_TYPE int
#define PTR_INIT (int *)(buf+y1*width+4*x1)
#define WIDEYDOPIXEL ptr[i*ptrIncr+x] =  RGBtoVal(r, g, b);
#define WIDEXDOPIXEL ptr[y*ptrIncr+i] =  RGBtoVal(r, g, b);
#define DOPIXEL *ptr =  RGBtoVal(r, g, b);
#include "MGRline.h"


#define WIDENAME wideGZline
#define NAME Xmgr_24GZline
#define PTR_INCR width>>2
#define ZBUFFER
#define GOURAUD
#define COLOR
#define PTR_TYPE int
#define PTR_INIT (int *)(buf+y1*width+4*x1)
#define WIDEYDOPIXEL ptr[i*ptrIncr+x] =  RGBtoVal(r, g, b);
#define WIDEXDOPIXEL ptr[y*ptrIncr+i] =  RGBtoVal(r, g, b);
#define DOPIXEL *ptr =  RGBtoVal(r, g, b);
#include "MGRline.h"

/* }}} */

/* {{{ polygon scan conversion */

#define INIT (int *)(buf+width*y+x1*4)

#define NAME Xmgr_doLines
#define PTR_TYPE int
#define PTR_INIT INIT
#define VARIABLES int col=RGBtoVal(color[0], color[1], color[2]);
#define DOPIXEL *ptr = col;
#include "MGRdolines.h"

#define NAME Xmgr_ZdoLines
#define ZBUFFER
#define PTR_TYPE int
#define PTR_INIT INIT
#define VARIABLES int col=RGBtoVal(color[0], color[1], color[2]);
#define DOPIXEL *ptr = col;
#include "MGRdolines.h"

#define NAME Xmgr_GdoLines
#define PTR_TYPE int
#define PTR_INIT INIT
#define DOPIXEL *ptr = RGBtoVal(r,g,b);
#define GOURAUD
#define COLOR
#include "MGRdolines.h"

#define NAME Xmgr_GZdoLines
#define ZBUFFER
#define PTR_TYPE int
#define PTR_INIT INIT
#define DOPIXEL *ptr = RGBtoVal(r,g,b);
#define GOURAUD
#define COLOR
#include "MGRdolines.h"

void
Xmgr_24Zpoly(unsigned char *buf, float *zbuf, int zwidth,
	     int width, int height, CPoint3 *p, int n, int *color)
{
    Xmgr_Zpolyscan(buf, zbuf, zwidth, width, height, p, n, color, mug,
		   Xmgr_ZdoLines);
}

void
Xmgr_24poly(unsigned char *buf, float *zbuf, int zwidth, 
	    int width, int height, CPoint3 *p, int n, int *color)
{
    Xmgr_polyscan(buf, zbuf, zwidth, width, height, p, n, color, mug,
		  Xmgr_doLines);
}

void
Xmgr_24GZpoly(unsigned char *buf, float *zbuf, int zwidth, int width, int height,
	     CPoint3 *p, int n, int *dummy)
{
    Xmgr_GZpolyscan(buf, zbuf, zwidth, width, height, p, n, dummy, mug,
		    Xmgr_GZdoLines);
}

void
Xmgr_24Gpoly(unsigned char *buf, float *zbuf, int zwidth, int width, int height,
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
	((int *)buf)[(int)(p->y)*(width/4)+(int)p->x] = 
	    RGBtoVal(color[0], color[1], color[2]);
}

#ifdef __GNUC__
inline
#endif
static void
setpixel(unsigned char *buf, int zwidth, 
	  int width, int height, CPoint3 *p, int *color)
{
    ((int *)buf)[(int)(p->y)*(width/4)+(int)p->x] = 
	RGBtoVal(color[0], color[1], color[2]);
}

void
Xmgr_24polyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
		CPoint3 *p, int n, int lwidth, int *color)
{
    int i;
    CPoint3 *cp;

    if (n == 1)
    {
	setpixel(buf, zwidth, width, height, p, color);
	return;
    }

    for (i=0, cp=p; i<n-1; i++, cp++)
	if (cp->drawnext)
	    Xmgr_24line(buf, zbuf, zwidth, width, height, cp, cp+1,
			 lwidth, color);
}


void
Xmgr_24Zpolyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
		 CPoint3 *p, int n, int lwidth, int *color)
{
    int i;
    CPoint3 *cp;

    if (n == 1)
    {
	setZpixel(buf, zbuf, zwidth, width, height, p, color);
	return;
    }

    for (i=0, cp=p; i<n-1; i++)
	if (cp->drawnext)
	    Xmgr_24Zline(buf, zbuf, zwidth, width, height, cp, cp+1,
			 lwidth, color);
}


void
Xmgr_24Gpolyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
		 CPoint3 *p, int n, int lwidth, int *color)
{
    int i;
    CPoint3 *cp;

    if (n == 1)
    {
	setpixel(buf, zwidth, width, height, p, color);
	return;
    }

    for (i=0, cp=p; i<n-1; i++, cp++)
	if (cp->drawnext)
	    Xmgr_gradWrapper(buf, zbuf, zwidth, width, height, cp, cp+1,
			     lwidth, Xmgr_24line, Xmgr_24Gline);
}


void
Xmgr_24GZpolyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
		 CPoint3 *p, int n, int lwidth, int *color)
{
    int i;
    CPoint3 *cp;

    if (n == 1)
    {
	setZpixel(buf, zbuf, zwidth, width, height, p, color);
	return;
    }

    for (i=0, cp=p; i<n-1; i++, cp++)
	if (cp->drawnext)
	    Xmgr_gradWrapper(buf, zbuf, zwidth, width, height, cp, cp+1,
			     lwidth, Xmgr_24Zline, Xmgr_24GZline);
}

/* }}} */

/*
Local variables:
folded-file: t
*/

