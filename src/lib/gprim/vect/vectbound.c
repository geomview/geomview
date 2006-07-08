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

/*
 * Geometry Routines
 *
 * Geometry Supercomputer Project
 *
 * ROUTINE DESCRIPTION:  Return the bounding box of a collection of vectors.
 *
 */


#include "vectP.h"
#include "bboxP.h"


BBox *
VectBound(v, T)
	register Vect *v;
	Transform T;
{
	register int n;
	register HPoint3 *p;
	HPoint3 min, max;
	HPoint3	*raw, *clean, p0, p1;

	n = v->nvert;
	p = v->p;
	while(--n >= 0 && !finite(p->x + p->y + p->z))
		p++;
	if(n <= 0)
		return NULL;	/* No (finite) points! */

#ifdef BBOX_ND_HACK
	if(v->geomflags & VERT_4D)
	    return BBox_ND_hack(NULL, (float *)p, n*4);
#endif

	min = *p;
    	if (T && T != TM_IDENTITY) 	{
    		HPt3Transform(T, &min, &min);
		HPt3Normalize( &min, &min );
    		}
	max = min;
	clean = &p1;
	while(--n >= 0) {
		p++;
            	if (T && T != TM_IDENTITY) 	{
			HPt3Transform(T, p, &p0);
			raw = &p0;
			}
		else raw = p;
		HPt3Normalize( raw , clean );
		if(min.x > clean->x) min.x = clean->x;
		else if(max.x < clean->x) max.x = clean->x;
		if(min.y > clean->y) min.y = clean->y;
		else if(max.y < clean->y) max.y = clean->y;
		if(min.z > clean->z) min.z = clean->z;
		else if(max.z < clean->z) max.z = clean->z;
		} 
    return (BBox *) GeomCreate ("bbox", CR_MIN, &min, CR_MAX, &max, NULL );
}
