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

#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
#include "config.h"
#endif

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "mg.h"
#include "bboxP.h"

BBox *
BBoxDraw(bbox)
     register BBox *bbox;
{
    register int i, k, numvert;
    int dimn;
    HPoint3 vert[16];
    ColorA edgecolor;
    Appearance *ap = mggetappearance();

    if(!(ap->flag & APF_EDGEDRAW))
	return bbox;

    dimn = (bbox->geomflags & VERT_4D) ? 4 : 3;
    if (dimn == 3)	{	/* dehomogenize min, max vals */
	HPt3Normalize(&bbox->min, &bbox->min);
	HPt3Normalize(&bbox->max, &bbox->max);
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

    *(Color *)&edgecolor = ap->mat->edgecolor;
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
