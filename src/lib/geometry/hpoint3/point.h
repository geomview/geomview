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

#ifndef POINTDEFS
#define POINTDEFS

#include "hpoint3.h"

typedef HPoint3 Point;

#define PtCreate HPt3Create
#define PtDelete HPt3Delete

#define PtPrint HPt3Print
#define PtCopy  HPt3Copy

#define PtFrom  HPt3From
#define PtFrom3Pls HPt3From3HPl3s
#define PtFrom2Lns HPt3From2HLn3s

#define PtIntersectPt HPt3IntersectHPt3
#define PtPencil HPt3Pencil

#define PtDotPl HPt3DotHPl3

#define PtNormalize HPt3Normalize

#define PtUndefined HPt3Undefined
#define PtInfinity HPt3Infinity
#define PtCompare HPt3Compare

#define PtCoincidentPt HPt3CoincidentHPt3
#define PtCoincidentLn HPt3CoincidentHLn3
#define PtCoincidentPl HPt3CoincidentHPl3

#define PtTransform HPt3Transform
#define PtTransformN HPt3TransformN

#define PtDistance HPt3Distance

#define PtDual HPt3Dual

#define PtLinSum HPt3LinSum
#define PtSizeOne HPt3SizeOne

#endif
