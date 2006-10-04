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

BBox *BBoxBound(BBox *bbox, Transform T, TransformN *TN, int *axes)
{
    if(bbox == NULL)
	return NULL;

    bbox = (BBox *)GeomCopy((Geom *)bbox);
    if((T == NULL || T == TM_IDENTITY) &&
       (TN == NULL || TN == (void *)TM_IDENTITY))
	return bbox;

    bbox = BBoxTransform(bbox, T, TN);

    if (TN && axes && bbox->pdim > 4) {
	int i;
	float *min = (float *)&bbox->min;
	float *max = (float *)&bbox->max;

	for (i = 0; i < 3; i++) {
	    min[i] = bbox->minN->v[axes[i]];
	    max[i] = bbox->maxN->v[axes[i]];
	}
	if (bbox->geomflags & VERT_4D) {
	    min[3] = bbox->minN->v[axes[3]];
	    max[3] = bbox->maxN->v[axes[3]];
	} else {
	    min[3] = 1.0;
	    max[3] = 1.0;
	}
	HPtNDelete(bbox->minN);
	HPtNDelete(bbox->maxN);
	bbox->minN = bbox->maxN = NULL;
	bbox->pdim = 4;
    }

    return bbox;
}

BBox *
BBox_ND_hack(BBox *b, float *p, int nfloats)
{
    float vmin, vmax;

    if(nfloats <= 0)
	return b;
    if(b == NULL) {
	b = (BBox *)GeomCCreate(NULL, BBoxMethods(), CR_END);
	vmin = vmax = *p++;
	nfloats--;
    } else {
	vmin = b->min.x;
	vmax = b->max.x;
    }
    for( ; --nfloats >= 0; p++) {
	if(vmin > *p) vmin = *p;
	if(vmax < *p) vmax = *p;
    }
    b->min.x = b->min.y = b->min.z = vmin;
    b->max.x = b->max.y = b->max.z = vmax;
    b->min.w = b->max.w = 1;
    return b;
}
