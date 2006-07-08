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
BBoxUnion3(register BBox *bbox1, register BBox *bbox2, register BBox *result)
{
	HPoint3	min, max;


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
	min = bbox1->min;
	max = bbox1->max;
	if(bbox2) {
	    if(min.x > bbox2->min.x) min.x = bbox2->min.x;
	    if(max.x < bbox2->max.x) max.x = bbox2->max.x;
	    if(min.y > bbox2->min.y) min.y = bbox2->min.y;
	    if(max.y < bbox2->max.y) max.y = bbox2->max.y;
	    if(min.z > bbox2->min.z) min.z = bbox2->min.z;
	    if(max.z < bbox2->max.z) max.z = bbox2->max.z;
	}
	/* this needs to be fleshed out for true 4D */
	min.w = max.w = 1.0;
	if(result == NULL)
	    return (BBox *)GeomCCreate(NULL, BBoxMethods(),
				CR_MIN, &min, CR_MAX, &max, CR_END);
	result->min = min;
	result->max = max;
	return result;
}
