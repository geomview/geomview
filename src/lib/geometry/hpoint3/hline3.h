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

#ifndef HLINE3DEF
#define HLINE3DEF

#ifndef HPOINT3DEF
# include "hpoint3.h"
#endif

#define HLN3_POINT_FORM -1
#define HLN3_PLANE_FORM  1
#define HLN3_DUAL_FORM(f) (-(f))

extern HLine3 *HLn3Create();
extern void HLn3Delete( HLine3 *ln );

extern void HLn3Print( HLine3 *ln );
extern void HLn3Copy( HLine3 *ln1, HLine3 *ln2 );
extern int HLn3From2HPt3s( HLine3 *ln, HPoint3 *pt1, HPoint3 *pt2 );
extern int HLn3From2HPl3s( HLine3 *ln, HPlane3 *pl1, HPlane3 *pl2 );

extern int HLn3IntersectHPt3( HLine3 *ln, HPoint3 *pt, HPlane3 *pl );
extern int HLn3IntersectHPl3( HLine3 *ln, HPlane3 *pl, HPoint3 *pt );
extern int HLn3IntersectHLn3( HLine3 *lnpl, HLine3 *lnpt, HPlane3 *pl, HPoint3 *pt );

extern int HLn3Undefined( HLine3 *ln );
extern int HLn3Infinity( HLine3 *ln );
extern int HLn3Compare( HLine3 *ln1, HLine3 *ln2 );

extern int HLn3CoincidentHPt3( HLine3 *ln, HPoint3 *pt );
extern int HLn3CoincidentHLn3( HLine3 *ln1, HLine3 *ln2 );
extern int HLn3CoincidentHPl3( HLine3 *ln, HPlane3 *pl );

extern void HLn3Transform( Transform3 T, HLine3 *ln1, HLine3 *ln2 );

extern void HLn3Dual( HLine3 *ln, HLine3 *lndual );

extern void HLn3Perp( HLine3 *ln, HLine3 *lnperp );

extern int HLn3IntersectHLn3( HLine3 *ln1, HLine3 *ln2, HPlane3 *pl, HPoint3 *pt );


#endif
