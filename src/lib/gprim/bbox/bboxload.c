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

#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
#include "config.h"
#endif

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Geometry Technologies, Inc.";


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "bboxP.h"

BBox *
BBoxFLoad(FILE *f, char *fname)
{
    BBox b;
    BBox *bbox;
    char *token = GeomToken(f);
    int dim = 3;

    if(*token == '4') {
	dim = 4;
	token++;
    }
    if(strcmp(token, "BBOX"))
	return NULL;

    b.min.w = b.max.w = 1.0;
    if(fgetnf(f, dim, &b.min.x, 0) != dim ||
       fgetnf(f, dim, &b.max.x, 0) != dim) {
      OOGLSyntax(f, "Reading BBOX from \"%s\": expected %d floats", 
		 fname, dim * 2);
      return NULL;
    }

    bbox = OOGLNewE(BBox, "BBoxFLoad BBox");
    *bbox = b;

    GGeomInit(bbox, BBoxMethods(), BBOXMAGIC, NULL);

    if (dim == 4) bbox->geomflags |= VERT_4D;

    return bbox;
}
