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

/*
 * Geometry Routines
 *
 * Geometry Supercomputer Project
 *
 * ROUTINE DESCRIPTION:  Return the bounding box of a collection of vectors.
 *
 */


#include "skelP.h"


BBox *
SkelBound(s, T)
	register Skel *s;
	Transform T;
{
    register int n;
    Point3 min, max;
    float *p;
    HPoint3 p0;

    n = s->nvert;
    p = s->p;
    if(n <= 0)
	    return NULL;

    if(s->dim == 4 && !(s->geomflags & SKEL_4D)) {
	HPoint3 *hp = (HPoint3 *)p;
	while(--n >= 0 && !finite(p[0] + p[1] + p[2]))
		p += s->dim;
	if(n < 0)
		return NULL;
	HPt3TransPt3(T, (HPoint3 *)p, (Point3 *)&min);
	max = min;
	while(--n >= 0) {
		HPt3TransPt3(T, ++hp, (Point3 *)&p0);
		if(min.x > p0.x) min.x = p0.x;
		else if(max.x < p0.x) max.x = p0.x;
		if(min.y > p0.y) min.y = p0.y;
		else if(max.y < p0.y) max.y = p0.y;
		if(min.z > p0.z) min.z = p0.z;
		else if(max.z < p0.z) max.z = p0.z;
	} 
	return (BBox *) GeomCreate ("bbox", CR_MIN, &min, CR_MAX, &max, NULL );
    }

    /* Otherwise, it's an N-D skel.  Do what we can. */
#ifdef BBOX_ND_HACK
    return BBox_ND_hack(NULL, p, n*s->dim);
#else
    return NULL;
#endif
}
