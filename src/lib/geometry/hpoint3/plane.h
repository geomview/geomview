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

#ifndef PLANEDEFS
#define PLANEDEFS

#include "hplane3.h"

typedef HPlane3 Plane;

#define PlCreate HPl3Create
#define PlDelete HPl3Delete

#define PlPrint HPl3Print
#define PlCopy  HPl3Copy

#define PlFrom  HPl3From
#define PlFrom3Pls HPl3From3HPl3s
#define PlFrom2Lns HPl3From2HLn3s

#define PlIntersectPl HPl3IntersectHPl3
#define PlPencil HPl3Pencil

#define PlDotPt HPl3DotHPt3

/* Not used? mbp Tue May 16 16:51:40 2000 */
/*#define PlNormalize HPl3Normalize*/

#define PlUndefined HPl3Undefined
#define PlInfinity HPl3Infinity
#define PlCompare HPl3Compare

#define PlCoincidentPl HPl3CoincidentHPl3
#define PlCoincidentLn HPl3CoincidentHLn3
#define PlCoincidentPl HPl3CoincidentHPl3

#define PlTransform HPl3Transform
#define PlTransformN HPl3TransformN


#define PlDual HPl3Dual

#define PlIdeal HPl3Ideal
#define PlPerp HPl3Perp
#define PlAngle HPl3Angle


#endif
