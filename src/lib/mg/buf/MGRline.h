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
#if defined(ZBUFFER)
#define ZCODE(x) x
#else
#define ZCODE(x)
#endif

#ifdef GOURAUD
#ifdef COLOR
#define GCODE(x)
#define CCODE(x) x
#else
#define GCODE(x) x
#define CCODE(x)
#endif
#define CGCODE(x) x
#else
#define CCODE(x)
#define GCODE(x)
#define CGCODE(x)
#endif

static void
WIDENAME (unsigned char *buf, float *zbuf, int zwidth, int width, int height, 
	  int x1, int y1,
#ifdef ZBUFFER
	  double z1,
#endif
#ifdef GOURAUD
	  int r1,
#ifdef COLOR
	  int g1, int b1,
#endif
#endif
	  int x2, int y2,
#ifdef ZBUFFER
	  double z2,
#endif
#ifdef GOURAUD
	  int r2,
#ifdef COLOR
	  int g2, int b2, 
#endif
#endif
	  int lwidth, int *color)
{
    int d, x, y, ax, ay, sx, dx, dy;
    int i, end;
#ifdef ZBUFFER
    double delta=0, z;
#endif
#ifdef GOURAUD
    double r, rdelta;
#ifdef COLOR
    double g, b, gdelta, bdelta;
#endif
#endif
#if defined(ZBUFFER) || defined(GOURAUD)
    int total;
#endif
#ifdef PTR_INIT
    PTR_TYPE *ptr = (PTR_TYPE *)buf;
    int ptrIncr = PTR_INCR;
#endif
#ifdef VARIABLES
    VARIABLES
#endif

    dx = x2-x1;
    dy = y2-y1;

    ax = ABS(dx)<<1;
    ay = ABS(dy)<<1;

    sx = SGN(dx);

    x = x1;
    y = y1;
    ZCODE(z = z1;)
    CGCODE(r = r1;)
    CCODE(g = g1; b = b1;)
#if defined(ZBUFFER) || defined(GOURAUD)       
    total = ABS(dx)+ABS(dy);
    if(total == 0) total = 1;
#endif
    ZCODE(delta = (z2-z1)/total;)
    CGCODE(rdelta = (r2-r1)/(double)total;)
    CCODE(gdelta = (g2-g1)/(double)total;
	  bdelta = (b2-b1)/(double)total;)
    if (ax>ay)
    {		/* x dominant */
	d = ay-(ax>>1);
	for (;;)
	{
		for (i=MAX(0,y-lwidth/2), end=MIN(height,y-lwidth/2+lwidth); i<end; i++)
		{
#ifdef ZBUFFER
		    if (z<zbuf[i*zwidth+x])
		    {
			WIDEYDOPIXEL
			zbuf[i*zwidth+x] = z;
		    } 
#else
		    WIDEYDOPIXEL
#endif
		}
	    if (x==x2) break;
	    if (d>=0)
	    {
		y++;
		ZCODE(z += delta;)
		CGCODE(r += rdelta;)
		CCODE(g += gdelta; b += bdelta;)
		d -= ax;
	    }
	    x += sx;
	    ZCODE(z += delta;)
	    CGCODE(r += rdelta;)
	    CCODE(g += gdelta; b += bdelta;)
	    d += ay;
	}
    }
    else 
    {			/* y dominant */
	d = ax-(ay>>1);
	for (;;) 
	{
		for (i=MAX(0,x-lwidth/2), end=MIN(zwidth,x-lwidth/2+lwidth); i<end; i++)
		{
#ifdef ZBUFFER
		    if (z<zbuf[y*zwidth+i])
		    {
		    	WIDEXDOPIXEL
		    	zbuf[y*zwidth+i] = z;
		    }
#else
		    WIDEXDOPIXEL
#endif
		}
	    if (y==y2) break;
	    if (d>=0)
	    {
		x += sx;
		ZCODE(z += delta;)
		CGCODE(r += rdelta;)
		CCODE(g += gdelta; b += bdelta;)
		d -= ay;
	    }
	    y++;
	    ZCODE(z += delta;)
	    CGCODE(r += rdelta;)
	    CCODE(g += gdelta; b += bdelta;)
	    d += ax;
	}
    }
}

void
NAME (unsigned char *buf, float *zbuf, int zwidth, 
      int width, int height, CPoint3 *p1, CPoint3 *p2, int lwidth, int *color)
{
    int d, x, y, ax, ay, sx, dx, dy;
    int x1, y1, x2, y2;
#ifdef PTR_INIT
    PTR_TYPE *ptr;
    int ptrIncr = PTR_INCR;
#endif
#ifdef ZBUFFER
    float *zptr;
    double delta=0, z, z1, z2;
#endif
#ifdef GOURAUD
    int r1, r2;
    double r, rdelta;
#ifdef COLOR
    int g1, g2, b1, b2;
    double g, b, gdelta, bdelta;
#endif
#endif
#if defined(ZBUFFER) || defined(GOURAUD)
    int total;
#endif
#ifdef VARIABLES
    VARIABLES
#endif

    if (p2->y<p1->y)
    {
	x1 = p2->x; y1 = p2->y;
	x2 = p1->x; y2 = p1->y;
	ZCODE(z1 = p2->z - _mgc->zfnudge; z2 = p1->z - _mgc->zfnudge;)
	CGCODE(r1 = 255*p2->vcol.r; r2 = 255*p1->vcol.r;)
	CCODE(g1 = 255*p2->vcol.g; b1 = 255*p2->vcol.b;
	      g2 = 255*p1->vcol.g; b2 = 255*p1->vcol.b;)
    }
    else
    {
	x1 = p1->x; y1 = p1->y;
	x2 = p2->x; y2 = p2->y;
	ZCODE(z1 = p1->z - _mgc->zfnudge; z2 = p2->z - _mgc->zfnudge;)
	CGCODE(r1 = 255*p1->vcol.r; r2 = 255*p2->vcol.r;)
	CCODE(g1 = 255*p1->vcol.g; b1 = 255*p1->vcol.b;
	      g2 = 255*p2->vcol.g; b2 = 255*p2->vcol.b;)
    }

    if (lwidth > 1)
    {
	WIDENAME (buf, zbuf, zwidth, width, height, x1, y1, 
#ifdef ZBUFFER
		  z1,
#endif
#ifdef GOURAUD
		  r1,
#ifdef COLOR
		  g1, b1,
#endif
#endif
		  x2, y2,
#ifdef ZBUFFER
		  z2,
#endif
#ifdef GOURAUD
		  r2,
#ifdef COLOR
		  g2, b2,
#endif
#endif
		  lwidth, color);
	return;
    }
#ifdef PTR_INIT
    ptr = PTR_INIT;
#endif
    ZCODE(zptr = zbuf+y1*zwidth+x1;)

    dx = x2-x1;
    dy = y2-y1;

    ax = ABS(dx)<<1;
    ay = ABS(dy)<<1;

    sx = SGN(dx);

    x = x1;
    y = y1;
    ZCODE(z = z1;)
    CGCODE(r = r1;)
    CCODE(g = g1; b = b1;)
#if defined(ZBUFFER) || defined(GOURAUD)
    total = ABS(dx)+ABS(dy);
    if(total == 0) total = 1;
#endif
    ZCODE(delta = (z2-z1)/total;)
    CGCODE(rdelta = (r2-r1)/(double)total;)
    CCODE(gdelta = (g2-g1)/(double)total;
	  bdelta = (b2-b1)/(double)total;)

    if (ax>ay)
    {		/* x dominant */
	d = ay-(ax>>1);
	for (;;)
	{
#ifdef ZBUFFER
	    if (z<*zptr)
	    {
		DOPIXEL
		*zptr = z;
	    }
#else
	    DOPIXEL
#endif
	    if (x==x2) break;
	    if (d>=0)
	    {
		y++;
		ZCODE(z += delta;)
		CGCODE(r += rdelta;)
		CCODE(g += gdelta; b += bdelta;)
#ifdef PTR_INIT
		ptr += ptrIncr;
#endif
		ZCODE(zptr += zwidth;)
		d -= ax;
	    }
	    x += sx;
	    ZCODE(z += delta;)
	    CGCODE(r += rdelta;)
	    CCODE(g += gdelta; b += bdelta;)
#ifdef PTR_INIT
	    ptr += sx;
#endif
	    ZCODE(zptr += sx;)
	    d += ay;
	}
    }
    else 
    {			/* y dominant */
	d = ax-(ay>>1);
	for (;;) 
	{
#ifdef ZBUFFER
	    if (z<*zptr)
	    {
		DOPIXEL
		ZCODE(*zptr = z;)
	    }
#else
	    DOPIXEL
#endif
	    if (y==y2) break;
	    if (d>=0)
	    {
		x += sx;
		ZCODE(z += delta;)
		CGCODE(r += rdelta;)
		CCODE(g += gdelta; b += bdelta;)
#ifdef PTR_INIT
		ptr += sx;
#endif
		ZCODE(zptr += sx;)
		d -= ay;
	    }
	    y++;
	    ZCODE(z += delta;)
	    CGCODE(r += rdelta;)
	    CCODE(g += gdelta; b += bdelta;)
#ifdef PTR_INIT
	    ptr += ptrIncr;
#endif
	    ZCODE(zptr += zwidth;)
	    d += ax;
	}
    }
}

#undef ZCODE
#undef CCODE
#undef GCODE
#undef CGCODE

#undef WIDENAME
#undef NAME
#undef ZBUFFER
#undef GOURAUD
#undef COLOR
#undef PTR_TYPE
#undef PTR_INIT
#undef PTR_INCR
#undef VARIABLES
#undef DOPIXEL
#undef WIDEXDOPIXEL
#undef WIDEYDOPIXEL
