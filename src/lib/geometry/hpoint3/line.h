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


/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef LINEDEFS
#define LINEDEFS

#include "hline3.h"

typedef HLine3 Line;

#define LnCreate HLn3Create
#define LnDelete HLn3Delete

#define LnPrint HLn3Print
#define LnCopy HLn3Copy
#define LnFrom2Pts HLn3From2HPt3s
#define LnFrom2Pls HLn3From2HPl3s

#define LnUndefined HLn3Undefined
#define LnInfinity HLn3Infinity
#define LnCompare HLn3Compare

#define LnCoincidentPt HLn3CoincidentHPt3
#define LnCoincidentLn HLn3CoincidentHLn3
#define LnCoincidentPl HLn3CoincidentHPl3

#define LnTransform HLn3Transform

#define LnIntersectPt HLn3IntersectHPt3
#define LnIntersectPl HLn3IntersectHPl3
#define LnIntersectLn HLn3IntersectHLn3

#define LnDual HLn3Dual
#define LnPerp HLn3Perp

#endif
