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
/* needed stuff:
   NAME: name of function
   GOURAUD: defined for smooth shade
   ZBUFFER: defined for z-buffer code
   COLOR: define for color
 */

#define RLUM 0.299
#define GLUM 0.587
#define BLUM 0.144

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

void
NAME (unsigned char *buf, float *zbuf, int zwidth, 
      int width, int height, CPoint3 *p, int n, int *color, endPoint *mug,
      void (*scanfunc)(unsigned char *, float *, int, int, int, 
				 int, int, int *, endPoint *))
{
    int x,y,dx,dy,d;                         /* bresenham engine */
    int miny = INT_MAX, maxy = INT_MIN;    /* y-bounds         */
    int i,j,x1,x2,y1,y2;                  /* indices          */
    int sf, si, sr, inc, dec;
#ifdef ZBUFFER
    double z, z1, z2, delta;
#endif

#ifdef GOURAUD
    double r, dr;
    int r1, r2;
#ifdef COLOR
    double g, b, dg, db;
    int g1, g2, b1, b2;
#endif
#endif

    for (i=0; i<n; i++)
    {
	miny = MIN(p[i].y, miny);
	maxy = MAX(p[i].y, maxy);
    }

    for (i=miny; i<=maxy; i++)
	mug[i].init = 0;

    for (i=0; i<n; i++)
    {
	if ((j = i+1)==n)
	    j=0;
	x1 = p[i].x; y1 = p[i].y;
	x2 = p[j].x; y2 = p[j].y;

	ZCODE(z1 = p[i].z; z2 = p[j].z;)
	GCODE(r1 = 64*(RLUM*p[i].vcol.r+GLUM*p[i].vcol.g+BLUM*p[i].vcol.b);
	      r2 = 64*(RLUM*p[j].vcol.r+GLUM*p[j].vcol.g+BLUM*p[j].vcol.b);)
	CCODE(r1 = 255*p[i].vcol.r; g1 = 255*p[i].vcol.g; 
	      b1 = 255*p[i].vcol.b; r2 = 255*p[j].vcol.r; 
	      g2 = 255*p[j].vcol.g; b2 = 255*p[j].vcol.b;)

	if (y2<y1)
	{
	    d = y2; y2=y1; y1=d;
	    d = x2; x2=x1; x1=d;
	    CGCODE(d = r2; r2=r1; r1=d;)
	    CCODE(d = g2; g2=g1; g1=d; 
	          d = b2; b2=b1; b1=d;)
	    ZCODE(z = z2; z2=z1; z1=z;)
	}
#if 0
	CCODE(fprintf(stderr, 
		"Edge %d:%d (%3d %3d)->(%3d %3d) (%d,%d,%d)->(%d,%d,%d)\n",
		i, j, x1, y1, x2, y2, r1, g1, b1, r2, g2, b2);)
#endif
	dx = x2-x1;
	dy = y2-y1;

	if (dy == 0)
	{
	    CGCODE(dr = 0;) CCODE(dg = db = 0;) ZCODE(delta = 0);
	}
	else
	{
	   CGCODE(dr = (r2-r1)/(double)(dy);)
	   CCODE(dg = (g2-g1)/(double)(dy); db = (b2-b1)/(double)(dy);)
	   ZCODE(delta = (z2-z1)/(double)(dy);)
	}

	x = x1;	y = y1;

	CGCODE(r = r1+dr;)
	CCODE(g = g1+dg; b = b1+db;)
	ZCODE(z = z1+delta;)

	if (dy)
	{
	    if (dx >= 0)
		si = dx/dy;
	    else
		si = (dx/dy) + (((dx % dy) == 0) ? 0 : -1);
	}
	else
	    si = 0;
	x = x1+si;
	sf = dx - si*dy;
	sr = 2*sf-dy;
	inc = sf;
	dec = sf-dy;

        for (y=y1+1; y<=y2; y++)
	{
#if 0
	    CCODE(fprintf(stderr, "\t%3d %3d (%lf %lf %lf)\n", x, y, r, g, b);)
#endif
	    if(mug[y].init)
	    {
		if (x < mug[y].P1x)
		{
		    mug[y].P1x = x;
		    ZCODE(mug[y].P1z = z;)
		    CGCODE(mug[y].P1r = r;)
		    CCODE(mug[y].P1g = g; mug[y].P1b = b;)
		}
		else if (x > mug[y].P2x)
		{
		    mug[y].P2x = x;
		    ZCODE(mug[y].P2z = z;)
		    CGCODE(mug[y].P2r = r;)
		    CCODE(mug[y].P2g = g; mug[y].P2b = b;)
	        }
	    }
	    else
	    {
		mug[y].init = 1;
		mug[y].P1x = x; mug[y].P2x = x;
		ZCODE(mug[y].P1z = z; mug[y].P2z = z;)
		CGCODE(mug[y].P1r = r; mug[y].P2r = r;)
		CCODE(mug[y].P1g = g; mug[y].P1b = b;
		      mug[y].P2g = g; mug[y].P2b = b;)
	    }
	    ZCODE(z += delta;)
	    CGCODE(r += dr;)
	    CCODE(g += dg; b += db;)

	    if (sr >= 0)
	    {
		x += si+1;
		sr += dec;
	    }
	    else
	    {
		x += si;
		sr += inc;
	    }
	}
    }

    miny++;

    for (i=miny; (i<=maxy) && ( mug[i].P2x == mug[i].P1x); i++)
	miny++;

    for (i=maxy; (i>=miny) && ( mug[i].P2x == mug[i].P1x); i--)
	maxy--;

    for (i=miny; i<=maxy; i++)
	mug[i].P1x++;

    scanfunc(buf, zbuf, zwidth, width, height, miny, maxy, color, mug);
}

/* Now clear out all the definitions */

#undef COLOR
#undef ZBUFFER
#undef NAME
#undef GOURAUD

#undef ZCODE
#undef CGCODE
#undef GCODE
#undef CCODE
