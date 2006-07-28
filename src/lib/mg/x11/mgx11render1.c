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

#include <stdio.h>
#include "mgP.h"
#include "mgx11P.h"

/* }}} */

/* {{{ dither and masking tables */

static unsigned char dither[65][8] = {
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x80, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00},
	{0x80, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00},
	{0x88, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00},
	{0x88, 0x00, 0x20, 0x00, 0x88, 0x00, 0x00, 0x00},
	{0x88, 0x00, 0x20, 0x00, 0x88, 0x00, 0x02, 0x00},
	{0x88, 0x00, 0x20, 0x00, 0x88, 0x00, 0x22, 0x00},
	{0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00},
	{0x88, 0x00, 0xA2, 0x00, 0x88, 0x00, 0x22, 0x00},
	{0x88, 0x00, 0xA2, 0x00, 0x88, 0x00, 0x2A, 0x00},
	{0x88, 0x00, 0xA2, 0x00, 0x88, 0x00, 0xAA, 0x00},
	{0x88, 0x00, 0xAA, 0x00, 0x88, 0x00, 0xAA, 0x00},
	{0xA8, 0x00, 0xAA, 0x00, 0x88, 0x00, 0xAA, 0x00},
	{0xA8, 0x00, 0xAA, 0x00, 0x8A, 0x00, 0xAA, 0x00},
	{0xA8, 0x00, 0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x00},
	{0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x00},
	{0xAA, 0x40, 0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x00},
	{0xAA, 0x40, 0xAA, 0x00, 0xAA, 0x04, 0xAA, 0x00},
	{0xAA, 0x40, 0xAA, 0x00, 0xAA, 0x44, 0xAA, 0x00},
	{0xAA, 0x44, 0xAA, 0x00, 0xAA, 0x44, 0xAA, 0x00},
	{0xAA, 0x44, 0xAA, 0x10, 0xAA, 0x44, 0xAA, 0x00},
	{0xAA, 0x44, 0xAA, 0x10, 0xAA, 0x44, 0xAA, 0x01},
	{0xAA, 0x44, 0xAA, 0x10, 0xAA, 0x44, 0xAA, 0x11},
	{0xAA, 0x44, 0xAA, 0x11, 0xAA, 0x44, 0xAA, 0x11},
	{0xAA, 0x44, 0xAA, 0x51, 0xAA, 0x44, 0xAA, 0x11},
	{0xAA, 0x44, 0xAA, 0x51, 0xAA, 0x44, 0xAA, 0x15},
	{0xAA, 0x44, 0xAA, 0x51, 0xAA, 0x44, 0xAA, 0x55},
	{0xAA, 0x44, 0xAA, 0x55, 0xAA, 0x44, 0xAA, 0x55},
	{0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x44, 0xAA, 0x55},
	{0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55},
	{0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55},
	{0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55},
	{0xAA, 0xD5, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55},
	{0xAA, 0xD5, 0xAA, 0x55, 0xAA, 0x5D, 0xAA, 0x55},
	{0xAA, 0xD5, 0xAA, 0x55, 0xAA, 0xDD, 0xAA, 0x55},
	{0xAA, 0xDD, 0xAA, 0x55, 0xAA, 0xDD, 0xAA, 0x55},
	{0xAA, 0xDD, 0xAA, 0x75, 0xAA, 0xDD, 0xAA, 0x55},
	{0xAA, 0xDD, 0xAA, 0x75, 0xAA, 0xDD, 0xAA, 0x57},
	{0xAA, 0xDD, 0xAA, 0x75, 0xAA, 0xDD, 0xAA, 0x77},
	{0xAA, 0xDD, 0xAA, 0x77, 0xAA, 0xDD, 0xAA, 0x77},
	{0xAA, 0xDD, 0xAA, 0xF7, 0xAA, 0xDD, 0xAA, 0x77},
	{0xAA, 0xDD, 0xAA, 0xF7, 0xAA, 0xDD, 0xAA, 0x7F},
	{0xAA, 0xDD, 0xAA, 0xF7, 0xAA, 0xDD, 0xAA, 0xFF},
	{0xAA, 0xDD, 0xAA, 0xFF, 0xAA, 0xDD, 0xAA, 0xFF},
	{0xAA, 0xFD, 0xAA, 0xFF, 0xAA, 0xDD, 0xAA, 0xFF},
	{0xAA, 0xFD, 0xAA, 0xFF, 0xAA, 0xDF, 0xAA, 0xFF},
	{0xAA, 0xFD, 0xAA, 0xFF, 0xAA, 0xFF, 0xAA, 0xFF},
	{0xAA, 0xFF, 0xAA, 0xFF, 0xAA, 0xFF, 0xAA, 0xFF},
	{0xEA, 0xFF, 0xAA, 0xFF, 0xAA, 0xFF, 0xAA, 0xFF},
	{0xEA, 0xFF, 0xAA, 0xFF, 0xAE, 0xFF, 0xAA, 0xFF},
	{0xEA, 0xFF, 0xAA, 0xFF, 0xEE, 0xFF, 0xAA, 0xFF},
	{0xEE, 0xFF, 0xAA, 0xFF, 0xEE, 0xFF, 0xAA, 0xFF},
	{0xEE, 0xFF, 0xBA, 0xFF, 0xEE, 0xFF, 0xAA, 0xFF},
	{0xEE, 0xFF, 0xBA, 0xFF, 0xEE, 0xFF, 0xAB, 0xFF},
	{0xEE, 0xFF, 0xBA, 0xFF, 0xEE, 0xFF, 0xBB, 0xFF},
	{0xEE, 0xFF, 0xBB, 0xFF, 0xEE, 0xFF, 0xBB, 0xFF},
	{0xEE, 0xFF, 0xFB, 0xFF, 0xEE, 0xFF, 0xBB, 0xFF},
	{0xEE, 0xFF, 0xFB, 0xFF, 0xEE, 0xFF, 0xBF, 0xFF},
	{0xEE, 0xFF, 0xFB, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF},
	{0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF},
	{0xFE, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF},
	{0xFE, 0xFF, 0xFF, 0xFF, 0xEF, 0xFF, 0xFF, 0xFF},
	{0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
};

static unsigned char bits[8] = {
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

#if 0
static unsigned char startBits[8] = {
    0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01};

static unsigned char endBits[8] = {
    0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF};
#endif

/* }}} */

static endPoint *mug=NULL;
static int mugSize = 0;
static int flipped = 0;

static int
RGB2gray(int *color)
{
    int n;
    n = (64.0*(color[0]*0.299 + color[1]*0.587 + color[2]*0.114))/255.0;
    if (n>64)
	return 64;
    else
	return n;
}

#if 0
static int
RGB2gray2(float r, float g, float b)
{
    int n;
    n = 64.0*(r*0.299 + g*0.587 + b*0.114);
    if (n>64)
	return 64;
    else
	return n;
}
#endif

static inline void
setZpixel(unsigned char *buf, float *zbuf, int zwidth, int width, int height,
	  CPoint3 *p, int *color)
{
    int x, y;
    x = p->x;
    y = p->y;
    if (p->z < zbuf[y*zwidth+x])
	buf[y*width+(x>>3)] = 
	    (buf[y*width+(x>>3)] & ~bits[x&0x07]) | 
	    (bits[x&0x07] & dither[RGB2gray(color)][y&0x07]);
}

#ifdef __GNUC__
inline
#endif
static void
setpixel(unsigned char *buf, int zwidth, int width, int height,
	  CPoint3 *p, int *color)
{
    int x, y;
    x = p->x;
    y = p->y;
    buf[y*width+(x>>3)] = 
	(buf[y*width+(x>>3)] & ~bits[x&0x07]) | 
	(bits[x&0x07] & dither[RGB2gray(color)][y&0x07]);
}

#ifdef __GNUC__
inline
#endif
static void
setPixel(unsigned char *buf, int zwidth, int width, int height, int x, int y,
	 int color)
{
    buf[y*width+(x>>3)] = 
	(buf[y*width+(x>>3)] & ~bits[x&0x07]) | 
	(bits[x&0x07] & dither[color][y&0x07]);
}

void
Xmgr_1init(int blackPixel)
{
    int col, i;

    if (blackPixel && (!flipped))
    {
	for (col=0; col<65; col++)
	    for (i=0; i<8; i++)
		dither[col][i] = ~dither[col][i];
	flipped = 1;
    }
}

void
Xmgr_1clear(unsigned char *buf, float *zbuf, int zwidth, 
	     int width, int height, int *color, int flag,
	    int fullclear, int xmin, int ymin, int xmax, int ymax)
{
    int i, length, pos, x;
    int col = RGB2gray(color);

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
        for (i=0; i<height; i++)
            memset(buf+i*width, dither[col][i&0x07], width);

        if (flag)
            for (i=0; i<zwidth*height; i++)
                zbuf[i] = 1.0;
	return;
    }
    xmin = MAX(xmin,0) >> 3;
    length = (MIN(zwidth-1,xmax)-xmin+8) >> 3;
    ymin = MAX(ymin,0);
    ymax = MIN(height-1,ymax);

    for (i=ymin; i<=ymax; i++)
	memset(buf+i*width+xmin, dither[col][i&0x07], length);

    length = MIN(zwidth-1,xmax)-xmin+1; 
    if (flag)
	for (i=ymin; i<=ymax; i++)
	{
	    pos = i*zwidth+xmin;
	    for (x=0; x<length; x++)
	        zbuf[pos+x] = 1.0;
	}
}

/* {{{ single lines */

#define WIDENAME wideDline
#define NAME Xmgr_1Dline
#define WIDEYDOPIXEL setPixel(buf, zwidth, width, height, x, y, col);
#define WIDEXDOPIXEL setPixel(buf, zwidth, width, height, x, y, col);
#define DOPIXEL setPixel(buf, zwidth, width, height, x, y, col);
#define VARIABLES int col=RGB2gray(color);
#include "MGRline.h"

#define WIDENAME wideDZline
#define NAME Xmgr_1DZline
#define ZBUFFER
#define WIDEYDOPIXEL setPixel(buf, zwidth, width, height, x, y, col);
#define WIDEXDOPIXEL setPixel(buf, zwidth, width, height, x, y, col);
#define DOPIXEL setPixel(buf, zwidth, width, height, x, y, col);
#define VARIABLES int col=RGB2gray(color);
#include "MGRline.h"


#define WIDENAME wideDGZline
#define NAME Xmgr_1DGZline
#define ZBUFFER
#define GOURAUD
#define WIDEYDOPIXEL setPixel(buf, zwidth, width, height, x, y, r);
#define WIDEXDOPIXEL setPixel(buf, zwidth, width, height, x, y, r);
#define DOPIXEL setPixel(buf, zwidth, width, height, x, y, r);
#include "MGRline.h"


#define WIDENAME wideDGline
#define NAME Xmgr_1DGline
#define GOURAUD
#define WIDEYDOPIXEL setPixel(buf, zwidth, width, height, x, y, r);
#define WIDEXDOPIXEL setPixel(buf, zwidth, width, height, x, y, r);
#define DOPIXEL setPixel(buf, zwidth, width, height, x, y, r);
#include "MGRline.h"

/* }}} */

/* {{{ polygon scan convers */

#define NAME Xmgr_DdoLines
#define YCODE pattern = dith[y&0x07]; ptr=buf+y*width;
#define DOPIXEL	\
    ptr[i>>3] = (ptr[i>>3] & ~bits[i&0x07]) | (bits[i&0x07] & pattern);
#define VARIABLES    unsigned char *dith = dither[RGB2gray(color)]; \
                     unsigned char pattern; \
		     unsigned char *ptr;
#include "MGRdolines.h"

#define NAME Xmgr_DZdoLines
#define ZBUFFER
#define YCODE pattern = dith[y&0x07]; ptr=buf+y*width;
#define DOPIXEL	\
    ptr[i>>3] = (ptr[i>>3] & ~bits[i&0x07]) | (bits[i&0x07] & pattern);
#define VARIABLES    unsigned char *dith = dither[RGB2gray(color)]; \
                     unsigned char pattern; \
		     unsigned char *ptr;
#include "MGRdolines.h"


#define NAME Xmgr_DGdoLines
#define GOURAUD
#define DOPIXEL	setPixel(buf, zwidth, width, height, i, y, r);
#include "MGRdolines.h"


#define NAME Xmgr_DGZdoLines
#define ZBUFFER
#define GOURAUD
#define DOPIXEL	setPixel(buf, zwidth, width, height, i, y, r);
#include "MGRdolines.h"


void
Xmgr_1Dpoly(unsigned char *buf, float *zbuf, int zwidth, int width, int height,
	    CPoint3 *p, int n, int *color)
{
    Xmgr_polyscan(buf, zbuf, zwidth, width, height, p, n, color, mug,
		  Xmgr_DdoLines);
}

void
Xmgr_1DZpoly(unsigned char *buf, float *zbuf, int zwidth, 
	     int width, int height, CPoint3 *p, int n, int *color)
{
    Xmgr_Zpolyscan(buf, zbuf, zwidth, width, height, p, n, color, mug,
		  Xmgr_DZdoLines);
}

void
Xmgr_1DGpoly(unsigned char *buf, float *zbuf, int zwidth, 
	     int width, int height, CPoint3 *p, int n, int *color)
{
    Xmgr_Graypolyscan(buf, zbuf, zwidth, width, height, p, n, color, mug,
		       Xmgr_DGdoLines);
}

void
Xmgr_1DGZpoly(unsigned char *buf, float *zbuf, int zwidth, 
	     int width, int height, CPoint3 *p, int n, int *color)
{
    Xmgr_GrayZpolyscan(buf, zbuf, zwidth, width, height, p, n, color, mug,
		       Xmgr_DGZdoLines);
}

/* }}} */

/* {{{ multi-line scan conversion */

void
Xmgr_1Dpolyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
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
	    Xmgr_1Dline(buf, zbuf, zwidth, width, height, p+i, p+i+1, 
			 lwidth, color);
}


void
Xmgr_1DZpolyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
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
	    Xmgr_1DZline(buf, zbuf, zwidth, width, height, p+i, p+i+1, 
			 lwidth, color);
}

void
Xmgr_1DGpolyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
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
	    Xmgr_gradWrapper(buf, zbuf, zwidth, width, height, p+1, p+i+1,
			     lwidth, Xmgr_1Dline, Xmgr_1DGline);
}

void
Xmgr_1DGZpolyline(unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
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
	    Xmgr_gradWrapper(buf, zbuf, zwidth, width, height, p+1, p+i+1,
			     lwidth, Xmgr_1DZline, Xmgr_1DGZline);
}

/* }}} */

/*
Local variables:
folded-file: t
End:
*/


