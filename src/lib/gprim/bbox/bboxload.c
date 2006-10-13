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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "bboxP.h"

BBox *
BBoxFLoad(IOBFILE *f, char *fname)
{
  BBox b = { 0 };
  BBox *bbox;
  char *token = GeomToken(f);
  int dimn = 3, nd = 0, pdim = 4;

  b.geomflags = 0;

  if(*token == '4') {
    b.geomflags = VERT_4D;
    dimn = 4;
    token++;
  }
  if(*token == 'n') {
    nd = 1;
    token++;
  }
  if(strcmp(token, "BBOX"))
    return NULL;

  if(nd) {
    if(iobfgetni(f, 1, &pdim, 0) <= 0) {
      OOGLSyntax(f, "Reading nBBOX from \"%s\": Expected dimension", fname);
      return NULL;
    }
    if (dimn == 4) {
      dimn = pdim;
    } else {
      dimn = pdim++;
    }
    b.geomflags = VERT_ND;
  }

  b.pdim = pdim;

  b.min = HPtNCreate(pdim, NULL);
  b.max = HPtNCreate(pdim, NULL);

  if(iobfgetnf(f, dimn, b.min->v, 0) != dimn ||
     iobfgetnf(f, dimn, b.max->v, 0) != dimn) {
    OOGLSyntax(f, "Reading BBOX from \"%s\": expected %d floats", 
	       fname, dimn * 2);
    HPtNDelete(b.min);
    HPtNDelete(b.max);
    return NULL;
  }

  bbox = OOGLNewE(BBox, "BBoxFLoad BBox");
  *bbox = b;

  GGeomInit(bbox, BBoxMethods(), BBOXMAGIC, NULL);

  if (pdim == dimn + 1) {
    bbox->min->v[dimn] = 1.0;
    bbox->max->v[dimn] = 1.0;
  }

  bbox->geomflags |= b.geomflags;

  return bbox;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
