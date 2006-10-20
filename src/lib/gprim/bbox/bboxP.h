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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef	BBOXPDEF
#define BBOXPDEF

#include "geomclass.h"
#include "bbox.h"
#include "hpointn.h"

/*
 * The vert[] array lists vertices, indexed by binary order of initial axes,
 * where X has weight 1, Y weight 2, Z weight 4.
 * Initially "min" corresponds to 0, "max" to 1.
 * Specifically GeomCreate("bbox", CR_MIN, min, CR_MAX, max) makes a bbox with:
 *  vert[0] = (min.x, min.y, min.z)
 *  vert[1] = (max.x, min.y, min.z)
 *  vert[2] = (min.x, max.y, min.z) ...
 *  vert[7] = (max.x, max.y, max.z).
 * Note this property isn't necessarily preserved if the bbox is transformed.
 */

/* We always use projective coordinates, i.e. min and max are vectors
 * of length pdim
 */
struct BBox {
  GEOMFIELDS;
  HPointN *center;
  HPointN *min, *max;
};

#endif /* ! BBOXPDEF */

