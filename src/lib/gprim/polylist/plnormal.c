/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
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
Copyright (C) 1998-2000 Geometry Technologies, Inc.";


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

/*
 * Compute the normals to each surface in a polylist.
 */


#include "polylistP.h"

PolyList *
PolyListComputeNormals( PolyList *polylist )
{
	register int	i, n;
	float           len;
	register Poly	*p;
	Vertex		**vp;
	register Vertex	*v, *v2, *v3;
	register float	x, y, z;

	if (!polylist) return NULL;

	if(!(polylist->flags & PL_HASPN)) {
	    /* Create per-polygon normals */
	    for (i = polylist->n_polys, p = polylist->p; --i >= 0; p++) {
		x = y = z = 0.0;
		if((n = p->n_vertices) >= 3) {
		    v = p->v[n-2];
		    v2 = p->v[n-1];
		    vp = p->v;

#define ANTI(P,Q)  (v2->pt.P - v->pt.P) * (v3->pt.Q - v->pt.Q) - \
		   (v2->pt.Q - v->pt.Q) * (v3->pt.P - v->pt.P)

		    do {
			v3 = *vp;
			x += ANTI(y,z);
			y += ANTI(z,x);
			z += ANTI(x,y);
			v = v2;
			v2 = v3;
			vp++;
		    } while(--n > 0);
		    len = sqrt(x*x + y*y + z*z);
		    if(len > 0.0) {
			if(polylist->flags & PL_EVNORM)
			    len = -len;
			len = 1.0/len;
			x *= len;
			y *= len;
			z *= len;
		    }
		    p->pn.x = x;
		    p->pn.y = y;
		    p->pn.z = z;
		}
	    }
	    polylist->flags |= PL_HASPN;
	}

	if(!(polylist->flags & PL_HASVN)) {
	    
	    for(i = polylist->n_verts, v = polylist->vl; --i >= 0; v++) {
		v->vn.x = v->vn.y = v->vn.z = 0.0;
	    }
	    for(i = polylist->n_polys, p = polylist->p; --i >= 0; p++) {
		for(n = p->n_vertices, vp = p->v; --n >= 0; vp++) {
		    v = *vp;
		    v->vn.x += p->pn.x;
		    v->vn.y += p->pn.y;
		    v->vn.z += p->pn.z;
		}
	    }
	    for(i = polylist->n_verts, v = polylist->vl; --i >= 0; v++) {
		len = sqrt(v->vn.x*v->vn.x + v->vn.y*v->vn.y + v->vn.z*v->vn.z);
		if(len > 0) {
		    if(polylist->flags & PL_EVNORM)
			len = -len;
		    len = 1.0/len;
		    v->vn.x *= len;
		    v->vn.y *= len;
		    v->vn.z *= len;
		}
	    }
	    polylist->flags |= PL_HASVN;
	}

	return polylist;
}
