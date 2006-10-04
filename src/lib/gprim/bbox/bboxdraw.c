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
#include "mgP.h"	/* need mgP.h (instead of mg.h) for _mgc below */
#include "hpointn.h"
#include <stdlib.h>
#ifndef alloca
#include <alloca.h>
#endif

static void draw_projected_bbox(mgmapfunc NDmap, void *NDinfo,
				BBox *bbox, Appearance *ap)
{
    int i, e, numvert, dim;
    ColorA edgecolor;
    HPointN *ptN;
    HPoint3 *pts3;

    if (bbox->minN == NULL)
	return; /* Draw only Nd objects */

    *(Color *)(void *)&edgecolor = ap->mat->edgecolor;
    edgecolor.a = 1;

    ptN = HPtNCreate(bbox->pdim, NULL);
    if (bbox->geomflags & VERT_4D) {
	dim = bbox->pdim;
    } else {
	dim = bbox->pdim-1;
	HPtNDehomogenize(bbox->minN, bbox->minN);
	HPtNDehomogenize(bbox->maxN, bbox->maxN);
	ptN->v[dim] = 1.0;
    }
    numvert = 1 << dim;
    pts3 = (HPoint3 *)alloca(numvert*sizeof(HPoint3));

    for (i = 0; i < numvert; i++) {
	for (e = 0; e < dim; e++) {
	    ptN->v[e] = (i & (1 << e)) ? bbox->minN->v[e] : bbox->maxN->v[e];
	}
	(*NDmap)(NDinfo, ptN, &pts3[i], NULL);
    }
    HPtNDelete(ptN);

    *(Color *)(void *)&edgecolor = ap->mat->edgecolor;
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

BBox *BBoxDraw(BBox *bbox)
{
    int i, numvert;
    int dimn;
    HPoint3 vert[16];
    ColorA edgecolor;
    Appearance *ap = mggetappearance();

    if(!(ap->flag & APF_EDGEDRAW))
	return bbox;

    if (_mgc->NDinfo) {
	Transform T;
	float focallen;

	mgpushtransform();
	CamGet(_mgc->cam, CAM_FOCUS, &focallen);
	TmTranslate(T, 0., 0., -focallen);
	TmConcat(T, _mgc->C2W, T);
	mgsettransform(T);
	draw_projected_bbox(_mgc->NDmap, _mgc->NDinfo, bbox, ap);
	mgpoptransform();
	return bbox;
    }

    dimn = (bbox->geomflags & VERT_4D) ? 4 : 3;
    if (dimn == 3)	{	/* dehomogenize min, max vals */
	HPt3Dehomogenize(&bbox->min, &bbox->min);
	HPt3Dehomogenize(&bbox->max, &bbox->max);
	}

    /* fill in the vertices of the (hyper) cube */
    for(i = 0; i < (1 << dimn); i++) {
	vert[i].x = i&1 ? bbox->min.x : bbox->max.x;
	vert[i].y = i&2 ? bbox->min.y : bbox->max.y;
	vert[i].z = i&4 ? bbox->min.z : bbox->max.z;
	vert[i].w = i&8 ? bbox->min.w : bbox->max.w;
    }

    numvert = 1 << dimn;

    /* turn on the edge color to draw the bbox */

    *(Color *)(void *)&edgecolor = ap->mat->edgecolor;
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
