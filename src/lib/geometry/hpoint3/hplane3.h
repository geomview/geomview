/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
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


/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef HPLANE3DEF
#define HPLANE3DEF

#ifndef HPOINT3DEF
# include "hpoint3.h"
#endif

extern HPlane3 *HPl3Create();
extern void HPl3Delete( HPlane3 *pl );

extern void HPl3Print( HPlane3 *pl );
extern void HPl3Copy( HPlane3 *pl1, HPlane3 *pl2 );
extern void HPl3From( HPlane3 *pl, HPl3Coord a, HPl3Coord b, HPl3Coord c, HPl3Coord d );
extern int HPl3From3HPt3s( HPlane3 *pl, HPoint3 *pt1, HPoint3 *pt2, HPoint3 *pt3 );
extern int HPl3From2HLn3s( HPlane3 *pl, HLine3 *ln1, HLine3 *ln2 );

extern int HPl3IntersectHPl3( HPlane3 *pl1, HPlane3 *pl2, HLine3 *ln );
extern void HPl3Pencil( HPl3Coord t1, HPlane3 *pl1, HPl3Coord t2, HPlane3 *pl2, HPlane3 *pl );

extern float HPl3DotHPt3( HPlane3 *pl, HPoint3 *pt );

/* Not used?  mbp Tue May 16 16:51:17 2000 */
/*extern void HPl3Normalize( HPlane3 *pl1, HPlane3 *pl2 );*/

extern int HPl3Undefined( HPlane3 *pl );
extern int HPl3Infinity( HPlane3 *pl );
extern int HPl3Compare( HPlane3 *pl1, HPlane3 *pl2 );

extern int HPl3CoincidentHPt3( HPlane3 *pl, HPoint3 *pt );
extern int HPl3CoincidentHLn3( HPlane3 *pl, HLine3 *ln );
extern int HPl3CoincidentHPl3( HPlane3 *pl1, HPlane3 *pl2 );

extern void HPl3Transform( Transform3 T, HPlane3 *pl1, HPlane3 *pl2 );
extern void HPl3TransformN( Transform3 T, HPlane3 *pl1, HPlane3 *pl2, int n );

extern void HPl3Dual( HPlane3 *pl, HPoint3 *pt );

/* Identify ideal plane */
extern HPlane3 HPl3Ideal;
extern void HPl3Perp( HPlane3 *pl, HPoint3 *pt );
extern HPl3Coord HPl3Angle( HPlane3 *pl1, HPlane3 *pl2 );

#endif
