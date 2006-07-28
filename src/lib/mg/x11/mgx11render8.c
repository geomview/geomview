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

/* Module for 8 bit rendering */

/* {{{ Includes */

#include <stdlib.h>
#include <stdio.h>
#include "mgP.h"
#include "mgx11P.h"

/* }}} */

/* {{{ externs for dithering and dithering code */

extern int colorlevels;
extern int mgx11divN[256];
extern int mgx11modN[256];
extern int mgx11magic[16][16];
extern unsigned long mgx11colors[216];
extern int mgx11multab[256];

#define DMAP(v,x,y)  (mgx11modN[v]>mgx11magic[x][y] ? mgx11divN[v] + 1 : mgx11divN[v])

#ifdef __GNUC__
inline
#endif
static unsigned long
ditherRGB(int x, int y, int *color)
{
  int xdith, ydith;

  xdith = x % 16;
  ydith = y % 16;
  return mgx11colors[DMAP(color[0], xdith, ydith) +
                                mgx11multab[DMAP(color[1], xdith, ydith) +
                                  mgx11multab[DMAP(color[2], xdith, ydith)]]];
}

#ifdef __GNUC__
inline
#endif
static unsigned long
ditherRGBvals(int x, int y, int r, int g, int b)
{
  int xdith, ydith;

  xdith = x % 16;
  ydith = y % 16;
  return mgx11colors[DMAP(r, xdith, ydith) +
              mgx11multab[DMAP(g, xdith, ydith) +
	      mgx11multab[DMAP(b, xdith, ydith)]]];
}

/* }}} */

static endPoint *mug=NULL;
static int mugSize = 0;

void
Xmgr_8clear(unsigned char *buf, float *zbuf, int zwidth, int width, int height,
	    int *color, int flag, int fullclear, int xmin, int ymin, int xmax, int ymax)
{
    int i, length, x, col = ditherRGB(0, 0, color);
    unsigned char *ptr;
    int pos;

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
        memset(buf, col, width*height);
        if (flag) {
            x = zwidth*height;
            for (i=0; i<x; i++)
                zbuf[i] = 1.0;
	}
        return;
    }
    xmin = MAX(xmin,0);
    length = MIN(width-1,xmax)-xmin+1;
    ymin = MAX(ymin,0);
    ymax = MIN(height-1,ymax);
    for (i=ymin, ptr=buf+ymin*width+xmin; i<=ymax; i++, ptr+=width)
	memset(ptr, col, length);
    if (flag) {
	pos = ymin*zwidth+xmin;
	for (i=ymin; i<=ymax; i++, pos += zwidth)
	{
	    for (x=0; x<length; x++)
		zbuf[pos+x] = 1.0;
	}
    }
}

/* {{{ Internal wide line functions */

#define WIDENAME wideline
#define NAME Xmgr_8line
#define PTR_INCR width
#define PTR_TYPE unsigned char
#define PTR_INIT buf+y1*width+x1
#define WIDEYDOPIXEL ptr[i*ptrIncr+x] = col;
#define WIDEXDOPIXEL ptr[y*ptrIncr+i] = col;
#define DOPIXEL *ptr = col;
#define VARIABLES unsigned char col=ditherRGB(0,0,color);
#include "MGRline.h"


#define WIDENAME wideDline
#define NAME Xmgr_8Dline
#define PTR_INCR width
#define PTR_TYPE unsigned char
#define PTR_INIT buf+y1*width+x1
#define WIDEYDOPIXEL ptr[i*ptrIncr+x] =  ditherRGB(x, i, color);
#define WIDEXDOPIXEL ptr[y*ptrIncr+i] =  ditherRGB(i, y, color);
#define DOPIXEL *ptr = ditherRGB(x, y, color);
#include "MGRline.h"


#define WIDENAME wideZline
#define NAME Xmgr_8Zline
#define ZBUFFER
#define PTR_INCR width
#define PTR_TYPE unsigned char
#define PTR_INIT buf+y1*width+x1
#define WIDEYDOPIXEL ptr[i*ptrIncr+x] = col;
#define WIDEXDOPIXEL ptr[y*ptrIncr+i] = col;
#define DOPIXEL *ptr = col;
#define VARIABLES unsigned char col=ditherRGB(0,0,color);
#include "MGRline.h"


#define WIDENAME wideDZline
#define NAME Xmgr_8DZline
#define ZBUFFER
#define PTR_INCR width
#define PTR_TYPE unsigned char
#define PTR_INIT buf+y1*width+x1
#define WIDEYDOPIXEL ptr[i*ptrIncr+x] =  ditherRGB(x, i, color);
#define WIDEXDOPIXEL ptr[y*ptrIncr+i] =  ditherRGB(i, y, color);
#define DOPIXEL *ptr = ditherRGB(x, y, color);
#include "MGRline.h"


#define WIDENAME wideDGline
#define NAME Xmgr_8DGline
#define GOURAUD
#define COLOR
#define PTR_INCR width
#define PTR_TYPE unsigned char
#define PTR_INIT buf+y1*width+x1
#define WIDEYDOPIXEL ptr[i*ptrIncr+x] =  ditherRGBvals(x, i, r, g, b);
#define WIDEXDOPIXEL ptr[y*ptrIncr+i] =  ditherRGBvals(i, y, r, g, b);
#define DOPIXEL *ptr = ditherRGBvals(x, y, r, g, b);
#include "MGRline.h"


#define WIDENAME wideDGZline
#define NAME Xmgr_8DGZline
#define ZBUFFER
#define GOURAUD
#define COLOR
#define PTR_INCR width
#define PTR_TYPE unsigned char
#define PTR_INIT buf+y1*width+x1
#define WIDEYDOPIXEL ptr[i*ptrIncr+x] =  ditherRGBvals(x, i, r, g, b);
#define WIDEXDOPIXEL ptr[y*ptrIncr+i] =  ditherRGBvals(i, y, r, g, b);
#define DOPIXEL *ptr = ditherRGBvals(x, y, r, g, b);
#include "MGRline.h"

/* }}} */

/* {{{ Polygon scan-conversion functions */

#define NAME Xmgr_doLines
#define PTR_TYPE unsigned char
#define PTR_INIT buf+width*y+x1
#define VARIABLES unsigned char col=ditherRGB(0,0,color);
#define DOPIXEL	*ptr = col;
#include "MGRdolines.h"


#define NAME Xmgr_DdoLines
#define PTR_TYPE unsigned char
#define PTR_INIT buf+width*y+x1
#define VARIABLES int rmod, gmod, bmod, rdiv, gdiv, bdiv, \
    rdiv1, gdiv1, bdiv1, xdith, *magic;
#define INITCODE \
    rmod = mgx11modN[color[0]]; rdiv = mgx11divN[color[0]]; rdiv1 = rdiv+1; \
    gmod = mgx11modN[color[1]]; gdiv = mgx11divN[color[1]]; gdiv1 = gdiv+1; \
    bmod = mgx11modN[color[2]]; bdiv = mgx11divN[color[2]]; bdiv1 = bdiv+1;
#define YCODE magic = mgx11magic[y & 0x0F];
#define DOPIXEL	    xdith = magic[i & 0x0F]; \
		    *ptr = mgx11colors[((rmod>xdith)?rdiv1:rdiv) + \
		    mgx11multab[((gmod>xdith)?gdiv1:gdiv) + \
		    mgx11multab[(bmod>xdith)?bdiv1:bdiv]]];
#include "MGRdolines.h"


#define NAME Xmgr_ZdoLines
#define ZBUFFER
#define PTR_TYPE unsigned char
#define PTR_INIT buf+width*y+x1
#define VARIABLES unsigned char col=ditherRGB(0,0,color);
#define DOPIXEL	*ptr = col;
#include "MGRdolines.h"


#define NAME Xmgr_DZdoLines
#define ZBUFFER
#define PTR_TYPE unsigned char
#define PTR_INIT buf+width*y+x1
#define VARIABLES int rmod, gmod, bmod, rdiv, gdiv, bdiv, \
    rdiv1, gdiv1, bdiv1, xdith, *magic;
#define INITCODE \
    rmod = mgx11modN[color[0]]; rdiv = mgx11divN[color[0]]; rdiv1 = rdiv+1; \
    gmod = mgx11modN[color[1]]; gdiv = mgx11divN[color[1]]; gdiv1 = gdiv+1; \
    bmod = mgx11modN[color[2]]; bdiv = mgx11divN[color[2]]; bdiv1 = bdiv+1;
#define YCODE magic = mgx11magic[y & 0x0F];
#define DOPIXEL	    xdith = magic[i & 0x0F]; \
		    *ptr = mgx11colors[((rmod>xdith)?rdiv1:rdiv) + \
		    mgx11multab[((gmod>xdith)?gdiv1:gdiv) + \
		    mgx11multab[(bmod>xdith)?bdiv1:bdiv]]];
#include "MGRdolines.h"


#define NAME Xmgr_DGdoLines
#define GOURAUD
#define COLOR
#define PTR_TYPE unsigned char
#define PTR_INIT buf+width*y+x1
#define VARIABLES int *magic, xdith;
#define YCODE magic = mgx11magic[y & 0x0F];
#define DOPIXEL	    xdith = magic[i & 0x0F]; \
	*ptr = mgx11colors[ \
	((mgx11modN[r]>xdith)?(mgx11divN[r]+1):mgx11divN[r]) + \
	mgx11multab[((mgx11modN[g]>xdith)?(mgx11divN[g]+1):mgx11divN[g])+ \
	mgx11multab[(mgx11modN[b]>xdith)?(mgx11divN[b]+1):mgx11divN[b]]]];
#include "MGRdolines.h"


#define NAME Xmgr_DGZdoLines
#define ZBUFFER
#define GOURAUD
#define COLOR
#define PTR_TYPE unsigned char
#define PTR_INIT buf+width*y+x1
#define VARIABLES int *magic, xdith;
#define YCODE magic = mgx11magic[y & 0x0F];
#define DOPIXEL	    xdith = magic[i & 0x0F]; \
	*ptr = mgx11colors[ \
	((mgx11modN[r]>xdith)?(mgx11divN[r]+1):mgx11divN[r]) + \
	mgx11multab[((mgx11modN[g]>xdith)?(mgx11divN[g]+1):mgx11divN[g])+ \
	mgx11multab[(mgx11modN[b]>xdith)?(mgx11divN[b]+1):mgx11divN[b]]]];
#include "MGRdolines.h"


void
Xmgr_8DZpoly(unsigned char *buf, float *zbuf, int zwidth, 
	     int width, int height, CPoint3 *p, int n, int *color)
{
    Xmgr_Zpolyscan(buf, zbuf, zwidth, width, height, p, n, color, mug,
		   Xmgr_DZdoLines);
}

void
Xmgr_8Zpoly(unsigned char *buf, float *zbuf, int zwidth, 
	    int width, int height, CPoint3 *p, int n, int *color)
{
    Xmgr_Zpolyscan(buf, zbuf, zwidth, width, height, p, n, color, mug,
		   Xmgr_ZdoLines);
}

void
Xmgr_8Dpoly(unsigned char *buf, float *zbuf, int zwidth, 
	    int width, int height, CPoint3 *p, int n, int *color)
{
    Xmgr_polyscan(buf, zbuf, zwidth, width, height, p, n, color, mug,
		  Xmgr_DdoLines);
}

void
Xmgr_8poly(unsigned char *buf, float *zbuf, int zwidth, 
	   int width, int height, CPoint3 *p, int n, int *color)
{
    Xmgr_polyscan(buf, zbuf, zwidth, width, height, p, n, color, mug,
		  Xmgr_doLines);
}

void
Xmgr_8DGZpoly(unsigned char *buf, float *zbuf, int zwidth, 
	      int width, int height, CPoint3 *p, int n, int *dummy)
{
    Xmgr_GZpolyscan(buf, zbuf, zwidth, width, height, p, n, dummy, mug,
		    Xmgr_DGZdoLines);
}

void
Xmgr_8DGpoly(unsigned char *buf, float *zbuf, int zwidth, 
	      int width, int height, CPoint3 *p, int n, int *dummy)
{
    Xmgr_Gpolyscan(buf, zbuf, zwidth, width, height, p, n, dummy, mug,
		   Xmgr_DGdoLines);
}

/* }}} */




/* {{{ Multiple line scan-conversion functions */

#ifdef __GNUC__
inline
#endif
static void
setZpixel(unsigned char *buf, float *zbuf, int zwidth, 
	  int width, int height, CPoint3 *p, int *color)
{
    if (p->z < zbuf[(int)(p->y)*zwidth+(int)p->x])
	buf[(int)(p->y)*width+(int)p->x] = ditherRGB(0, 0, color);
}

#ifdef __GNUC__
inline
#endif
static void
setpixel(unsigned char *buf, int zwidth, 
	 int width, int height, CPoint3 *p, int *color)
{
    buf[(int)(p->y)*width+(int)p->x] = ditherRGB(0, 0, color);
}

void
Xmgr_8polyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, CPoint3 *p, int n, int lwidth, int *color)
{
    int i;

    if (n == 1)
    {
	setpixel(buf, zwidth, width, height, p, color);
	return;
    }

    for (i=0; i<n-1; i++)
	if (p[i].drawnext)
	    Xmgr_8line(buf, zbuf, zwidth, width, height, p+i, p+i+1, 
			 lwidth, color);
}

void
Xmgr_8Dpolyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, CPoint3 *p, int n, int lwidth, int *color)
{
    int i;

    if (n == 1)
    {
	setpixel(buf, zwidth, width, height, p, color);
	return;
    }

    for (i=0; i<n-1; i++)
	if (p[i].drawnext)
	    Xmgr_8Dline(buf, zbuf, zwidth, width, height, p+i, p+i+1, 
			 lwidth, color);
}

void
Xmgr_8Zpolyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, CPoint3 *p, int n, int lwidth, int *color)
{
    int i;

    if (n == 1)
    {
	setZpixel(buf, zbuf, zwidth, width, height, p, color);
	return;
    }

    for (i=0; i<n-1; i++)
	if (p[i].drawnext)
	    Xmgr_8Zline(buf, zbuf, zwidth, width, height, p+i, p+i+1, 
			 lwidth, color);
}

void
Xmgr_8DZpolyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, CPoint3 *p, int n, int lwidth, int *color)
{
    int i;

    if (n == 1)
    {
	setZpixel(buf, zbuf, zwidth, width, height, p, color);
	return;
    }

    for (i=0; i<n-1; i++)
	if (p[i].drawnext)
	    Xmgr_8DZline(buf, zbuf, zwidth, width, height, p+i, p+i+1, 
			 lwidth, color);
}


void
Xmgr_8DGZpolyline(unsigned char *buf, float *zbuf, int zwidth, 
		  int width, int height, CPoint3 *p, int n, 
		  int lwidth, int *color)
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
			     lwidth, Xmgr_8DZline, Xmgr_8DGZline);
}

void
Xmgr_8DGpolyline(unsigned char *buf, float *zbuf, int zwidth, 
		 int width, int height, CPoint3 *p, int n, 
		 int lwidth, int *color)
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
			     lwidth, Xmgr_8Dline, Xmgr_8DGline);
}

/* }}} */

/*
Local variables:
folded-file: t
End:
*/

