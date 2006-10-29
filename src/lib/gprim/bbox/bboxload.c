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
  BBox *bbox;
  char *token = GeomToken(f);
  int dimn = 3, nd = 0, pdim = 4;
  HPointN *min, *max;
  HPtNCoord *minv, *maxv;

  if(*token == '4') {
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
    ++pdim;
  }

  if (pdim == 4) {
    if (dimn == 4) {
      pdim++;
    }
    min = HPtNCreate(pdim, NULL);
    max = HPtNCreate(pdim, NULL);
    minv = min->v+1;
    maxv = max->v+1;
  } else {
    min = HPtNCreate(pdim, NULL);
    max = HPtNCreate(pdim, NULL);
    if (dimn == 4) {
      minv = min->v;
      maxv = max->v;
    } else {
      minv = min->v+1;
      maxv = max->v+1;
      dimn = pdim-1;
    }
  }
   

  if(iobfgetnf(f, dimn, minv, 0) != dimn ||
     iobfgetnf(f, dimn, maxv, 0) != dimn) {
    OOGLSyntax(f, "Reading BBOX from \"%s\": expected %d floats", 
	       fname, dimn * 2);
    HPtNDelete(min);
    HPtNDelete(max);
    return NULL;
  }

  bbox = (BBox *)GeomCCreate(NULL, BBoxMethods(),
			     CR_NMIN, min, CR_NMAX, max, CR_END); 

  return bbox;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
