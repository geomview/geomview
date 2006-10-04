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

#define	BB_MIN(a,b)	( (a) <= (b) ? a : b )
#define	BB_MAX(a,b)	( (a) >= (b) ? a : b )

BBox *
BBoxUnion(BBox *bbox1, BBox *bbox2)
{
    return BBoxUnion3(bbox1, bbox2, NULL);
}

BBox *
BBoxUnion3(BBox *bbox1, BBox *bbox2, BBox *result)
{
    /* TAKE CARE OF THE CASE OF EITHER CUBE BEING NULL */
    if (!bbox1) {
	if(!bbox2) {
	    if(result) {
		static Point Max = { -1e10,-1e10,-1e10, 1 };
		static Point Min = {  1e10, 1e10, 1e10, 1 };
		result->min = Min;
		result->max = Max;
	    }
	    return result;
	}
	bbox1 = bbox2;
	bbox2 = NULL;
    }

    if (bbox1->pdim != bbox2->pdim) {
	OOGLError (0, "BBoxUnion3: dimensions do not match: %d/%d",
		   bbox1->pdim, bbox2->pdim);
    }

    if (bbox1->minN == NULL) { /* ordinary case */
	HPoint3	min, max;

	min = bbox1->min;
	max = bbox1->max;
	if(bbox2) {
	    if(min.x > bbox2->min.x) min.x = bbox2->min.x;
	    if(max.x < bbox2->max.x) max.x = bbox2->max.x;
	    if(min.y > bbox2->min.y) min.y = bbox2->min.y;
	    if(max.y < bbox2->max.y) max.y = bbox2->max.y;
	    if(min.z > bbox2->min.z) min.z = bbox2->min.z;
	    if(max.z < bbox2->max.z) max.z = bbox2->max.z;
	    if(min.w > bbox2->min.w) min.w = bbox2->min.w;
	    if(max.w < bbox2->max.w) max.w = bbox2->max.w;
	}

	if (bbox1->geomflags & VERT_4D) {
	    return (BBox *)GeomCCreate((Geom *)result, BBoxMethods(),
				       CR_4MIN, &min, CR_4MAX, &max, CR_END);
	} else {
	    return (BBox *)GeomCCreate((Geom *)result, BBoxMethods(),
				       CR_MIN, &min, CR_MAX, &max, CR_END);
	}
    } else {
	int dim;

	if (bbox1->geomflags & VERT_4D) {
	    result = (BBox *)GeomCCreate((Geom *)result, BBoxMethods(),
					 CR_NMIN, bbox1->minN,
					 CR_NMAX, bbox1->maxN,
					 CR_4D, 1,
					 CR_END);
	    dim = bbox1->pdim;
	} else {
	    HPtNDehomogenize(bbox1->minN, bbox1->minN);
	    HPtNDehomogenize(bbox1->minN, bbox1->maxN);

	    result = (BBox *)GeomCCreate((Geom *)result, BBoxMethods(),
					 CR_NMIN, bbox1->minN,
					 CR_NMAX, bbox1->maxN,
					 CR_4D, 1,
					 CR_END);
	    dim = bbox1->pdim-1;
	}

	if (bbox2) {
	    int i;

	    if (!(bbox1->geomflags & VERT_4D)) {
		HPtNDehomogenize(bbox2->minN, bbox2->minN);
		HPtNDehomogenize(bbox2->minN, bbox2->maxN);
	    }

	    for (i = 0; i < dim; i++) {
		if (result->minN->v[i] > bbox2->minN->v[i]) {
		    result->minN->v[i] = bbox2->minN->v[i];
		}
		if (result->maxN->v[i] < bbox2->maxN->v[i]) {
		    result->maxN->v[i] = bbox2->maxN->v[i];
		}
	    }
	}
	
	return result;
    }
}
