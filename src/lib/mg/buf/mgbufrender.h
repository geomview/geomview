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

#ifndef MGBUFRENDER
#define MGBUFRENDER

typedef struct
{
    int init;
    int P1x, P1r, P1g, P1b, P2x, P2r, P2g, P2b;
    double P1z, P2z;
} endPoint;

#define ABS(a) (((a)<0) ? -(a) : (a))
#define SGN(a) (((a)<0) ? -1 : 1)

#ifdef MAX
#undef MAX
#endif
#define MAX(a,b) (((a)<(b)) ? b : a)

#ifdef MIN
#undef MIN
#endif
#define MIN(a,b) (((a)<(b)) ? a : b)

void
Xmgr_gradWrapper(unsigned char *buf, float *zbuf, int zwidth, int width,
	int height, CPoint3 *p0, CPoint3 *p1, int lwidth,
	void (*flat)(unsigned char *, float *, int, int, int, CPoint3 *,
			CPoint3 *, int, int *),
	void (*smooth)(unsigned char *, float *, int, int, int, CPoint3 *,
			CPoint3 *, int, int *));

void
oldXmgr_gradWrapper(unsigned char *buf, float *zbuf, int zwidth, int width,
	int height, CPoint3 *p0, CPoint3 *p1, int lwidth,
	void (*flat)(unsigned char *, float *, int, int, int, int, int, float,
			int, int, float, int, int *),
	void (*smooth)(unsigned char *, float *, int, int, int, CPoint3 *,
			CPoint3 *, int));

void
Xmgr_polyscan(unsigned char *buf, float *zbuf, int zwidth, 
	      int width, int height, CPoint3 *p, int n, int *color,
	      endPoint *mug,
	      void (*scanfunc)(unsigned char *, float *, int, int, int, int,
				int, int *, endPoint *));

void
Xmgr_Zpolyscan(unsigned char *buf, float *zbuf, int zwidth, 
	       int width, int height, CPoint3 *p, int n, int *color,
	       endPoint *mug,
	       void (*scanfunc)(unsigned char *, float *, int, int, int, int,
				int, int *, endPoint *));

void
Xmgr_Graypolyscan(unsigned char *buf, float *zbuf, int zwidth, 
		  int width, int height, CPoint3 *p, int n, int *color,
		  endPoint *mug,
		  void (*scanfunc)(unsigned char *, float *, int, int, int, 
				   int, int, int *, endPoint *));


void
Xmgr_GrayZpolyscan(unsigned char *buf, float *zbuf, int zwidth, 
		   int width, int height, CPoint3 *p, int n, int *color,
		   endPoint *mug,
		   void (*scanfunc)(unsigned char *, float *, int, int, int, 
				    int, int, int *, endPoint *));

void
Xmgr_Gpolyscan(unsigned char *buf, float *zbuf, int zwidth, 
	       int width, int height, CPoint3 *p, int n, int *color,
	       endPoint *mug,
	       void (*scanfunc)(unsigned char *, float *, int, int, int, 
				int, int, int *, endPoint *));

void
Xmgr_GZpolyscan(unsigned char *buf, float *zbuf, int zwidth, 
		int width, int height, CPoint3 *p, int n, int *color,
		endPoint *mug,
		void (*scanfunc)(unsigned char *, float *, int, int, int,
				 int, int, int *, endPoint *));

#endif

