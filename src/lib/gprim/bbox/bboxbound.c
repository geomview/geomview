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

#include "bboxP.h"

BBox *
BBoxBound(bbox, T)
	BBox	*bbox;
	Transform T;
{
	if(bbox == NULL)
		return NULL;

	if(T == NULL || T == TM_IDENTITY)
		return (BBox *) GeomCopy((Geom *)bbox);	/* entirely trivial! */

	return BBoxTransform((BBox *)GeomCopy((Geom *)bbox), T);
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
