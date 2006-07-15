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
NAME (unsigned char *buf, float *zbuf, int zwidth, int width, 
		int height, int miny, int maxy, int *color, endPoint *mug)
{
#ifdef PTR_INIT
    PTR_TYPE *ptr;           /* pixel pointers   */
#endif
    int i, x2;           /* dithering junk   */
    int y, x1;                          /* current line     */
#ifdef ZBUFFER
    float *zptr;                 /* zbuff pointers   */
    double z, z2, zdelta;              /* z-buffer stuff   */
#endif
#if defined(ZBUFFER) || defined(GOURAUD)
    int dx;
#endif
#ifdef GOURAUD
    int sr, er, dr, r;
#ifdef COLOR
    int sg, sb, eg, eb, dg, db, g, b;
#endif
#endif
#ifdef VARIABLES
    VARIABLES
#endif

#ifdef INITCODE
    INITCODE
#endif
    for(y = miny; y <= maxy; y++)
    {
	x1 = mug[y].P1x;
	ZCODE(z = mug[y].P1z;)
	CGCODE(r = mug[y].P1r;)
	CCODE(g = mug[y].P1g; b = mug[y].P1b;)

	x2 = mug[y].P2x;
	ZCODE(z2 = mug[y].P2z;)
	CGCODE(dr = mug[y].P2r-r;)
	CCODE(dg = mug[y].P2g-g; db = mug[y].P2b-b;)

#if defined(ZBUFFER) || defined(GOURAUD)
	dx = x2-x1;
#endif
	ZCODE(zdelta = dx ? (z2-z)/dx : 0;)
	CGCODE(er = (dr << 1) - dx;
	       sr = SGN(dr); dr = abs(dr)<<1;)
	CCODE(eg = (dg << 1) - dx;
	      eb = (db << 1) - dx;
	      sg = SGN(dg); dg = abs(dg)<<1;
	      sb = SGN(db); db = abs(db)<<1;)
	CGCODE(dx <<= 1;)

#ifdef YCODE
	YCODE
#endif

#ifdef PTR_INIT
	ptr = PTR_INIT;
#endif
	ZCODE(zptr = zbuf+zwidth*y+x1;)

	for (i=x1; i<=x2; i++
#ifdef ZBUFFER
	     , z+=zdelta, zptr++
#endif
#ifdef PTR_INIT
             , ptr++
#endif
            )
	{
#ifdef ZBUFFER	    
	    if (z < *zptr)
	    {
		DOPIXEL
		*zptr = z;
	    }
#else
	    DOPIXEL
#endif

#ifdef GOURAUD
	    if (dx) 
	    {
		while (er>0)
		{
		    r += sr;
		    er -= dx;
		}
#ifdef COLOR
		while (eg>0)
		{
		    g += sg;
		    eg -= dx;
		}
		while (eb>0)
		{
		    b += sb;
		    eb -= dx;
		}
#endif
	    }
#endif

	    CGCODE(er += dr;)
	    CCODE(eg += dg; eb += db;)
	}
    }
}

#undef ZCODE
#undef CCODE
#undef GCODE
#undef CGCODE

#undef NAME
#undef ZBUFFER
#undef GOURAUD
#undef COLOR
#undef PTR_TYPE
#undef PTR_INIT
#undef VARIABLES
#undef INITCODE
#undef YCODE
#undef DOPIXEL
