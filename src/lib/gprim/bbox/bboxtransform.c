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

BBox           *
BBoxTransform(bbox, T)
	register BBox	*bbox;
	Transform T;
{
	register int i;
	register HPoint3 *p;
	HPoint3 vert[8];

	for(i = 0, p = &vert[0]; i < 8; i++, p++) {
		*p = bbox->min;
		if(i&1) p->x = bbox->max.x;	/* Neat huh */
		if(i&2) p->y = bbox->max.y;	/* Binary encoding of verts */
		if(i&4) p->z = bbox->max.z;
	}
	

	p = &vert[0];
	HPt3Transform(T, p, p);
	HPt3Normalize(p, p);
	bbox->min = *p;
	bbox->max = bbox->min;
	i = 8-1;
	do {
	    p++;
	    HPt3Transform(T, p, p);
	    HPt3Normalize(p, p);
	    if(bbox->min.x > p->x) bbox->min.x = p->x;
	    else if(bbox->max.x < p->x) bbox->max.x = p->x;
	    if(bbox->min.y > p->y) bbox->min.y = p->y;
	    else if(bbox->max.y < p->y) bbox->max.y = p->y;
	    if(bbox->min.z > p->z) bbox->min.z = p->z;
	    else if(bbox->max.z < p->z) bbox->max.z = p->z;
	} while(--i > 0);
	return (bbox);
}
