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
#include "mg.h"
#include "hpointn.h"
#include <stdlib.h>

static void
draw_projected_bbox(mgNDctx *NDctx, BBox *bbox, const Appearance *ap)
{
  int i, e, numvert, dim;
  ColorA edgecolor;
  HPointN *ptN;
  mgNDmapfunc mapHPtN = NDctx->mapHPtN;

  edgecolor.r = ap->mat->edgecolor.r;
  edgecolor.g = ap->mat->edgecolor.g;
  edgecolor.b = ap->mat->edgecolor.b;
  edgecolor.a = 1;

  dim = bbox->pdim-1;
  HPtNDehomogenize(bbox->min, bbox->min);
  HPtNDehomogenize(bbox->max, bbox->max);

  ptN = HPtNCreate(dim+1, NULL);
  numvert = 1 << dim;
  {
    VARARRAY(pts3, HPoint3, numvert);

    for (i = 0; i < numvert; i++) {
      int mask;
      for (mask = 1, e = 1; e < dim+1; e++, mask <<= 1) {
	ptN->v[e] = (i & mask) ? bbox->min->v[e] : bbox->max->v[e];
      }
      mapHPtN(NDctx, ptN, &pts3[i], NULL);
    }
    HPtNDelete(ptN);

    edgecolor.r = ap->mat->edgecolor.r;
    edgecolor.g = ap->mat->edgecolor.g;
    edgecolor.b = ap->mat->edgecolor.b;
    edgecolor.a = 1;

    for(i = 0; i < numvert; i++) {
      int j, incr;
      HPoint3 edge[2];

      for(j = 0; j < dim; j ++) {
	/* connect this vertex to its nearest neighbors if they
	 * follow it in lexicographical order */
	incr = 1 << j;
	/* is the j_th bit a zero? */
	if ( ! (i & incr) )	{
	  /* if so, draw the edge to the vertex whose number is
	   * gotten from i by making the j_th bit a one */
	  edge[0] = pts3[i];
	  edge[1] = pts3[i + incr];
	  mgpolyline(2, edge, 1,  &edgecolor, 0) ;
	}
      }
    }
  }
}

BBox *BBoxDraw(BBox *bbox)
{
  int i, numvert;
  const int dimn = 3;
  HPoint3 vert[16];
  HPoint3 min, max;
  ColorA edgecolor;
  const Appearance *ap = mggetappearance();
  mgNDctx *NDctx = NULL;

  if(!(ap->flag & APF_EDGEDRAW))
    return bbox;

  mgctxget(MG_NDCTX, &NDctx);
  
  if (NDctx) {
    draw_projected_bbox(NDctx, bbox, ap);
    return bbox;
  }

  HPtNToHPt3(bbox->min, NULL, &min);
  HPtNToHPt3(bbox->max, NULL, &max);
  /* dehomogenize min, max vals */
  HPt3Dehomogenize(&min, &min);
  HPt3Dehomogenize(&max, &max);

  /* fill in the vertices of the (hyper) cube */
  for(i = 0; i < (1 << dimn); i++) {
    vert[i].x = i&1 ? min.x : max.x;
    vert[i].y = i&2 ? min.y : max.y;
    vert[i].z = i&4 ? min.z : max.z;
    vert[i].w = 1.0;
  }

  numvert = 1 << dimn;

  /* turn on the edge color to draw the bbox */

  edgecolor.r = ap->mat->edgecolor.r;
  edgecolor.g = ap->mat->edgecolor.g;
  edgecolor.b = ap->mat->edgecolor.b;
  edgecolor.a = 1;

  for(i = 0; i < numvert; i++) {
    int j, incr;
    HPoint3 edge[2];

    for(j = 0; j < dimn; j ++) {
      /* connect this vertex to its nearest neighbors if they
       * follow it in lexicographical order */
      incr = 1 << j;
      /* is the j_th bit a zero? */
      if ( ! (i & incr) )	{
	/* if so, draw the edge to the vertex whose number is
	 * gotten from i by making the j_th bit a one */
	edge[0] = vert[i];
	edge[1] = vert[i + incr];
	mgpolyline(2, edge, 1,  &edgecolor, 0) ;
      }
    }
  }
  return bbox;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
