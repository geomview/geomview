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

#include "meshP.h"
static void mnorm();

Mesh *
MeshComputeNormals(Mesh *m)
{
	register HPoint3 *normp = NULL, *normptr, *pptr;
	int i;
	if(m->n) {
		GeomFree(m->n);
	}
	m->n = GeomNewN(Point3, m->nu * m->nv);
	m->flag |= MESH_N;
	/* if this a '4D' mesh, we need to dehomogenize (temporarily)
	before computing the normals */
	if (m->geomflags & VERT_4D)	{
	    normp = GeomNewN(HPoint3, m->nu * m->nv);
	    for (normptr = normp, pptr = m->p, i=0; 
		i < m->nu * m->nv; 
		++i, ++pptr, ++normptr) 
		    HPt3Normalize(pptr, normptr);
	    }
	mnorm(m->geomflags & VERT_4D ? normp : m->p, m->n, m->nu, m->nv,
	     m->flag & MESH_UWRAP, m->flag & MESH_VWRAP, m->flag & MESH_EVERT);
	if (m->geomflags & VERT_4D) GeomFree(normp);
	return m;
}


static void
mnorm(ap, an, nu, nv, uwrap, vwrap, evert)
	HPoint3	*ap;
	Point3	*an;
	int	nu, nv;
	int	uwrap, vwrap;
	int	evert;
{
	register HPoint3 *prev, *next;
	register Point3 *n;
	register int k;
	int u, v;
	float x,y,z, norm;
	float unit;

	/*
	 * We set the normal at each point to be the mean of the
	 * cross products at the four adjacent points.  I.e.:
	 *   n		Given mesh point p and its four orthogonal neighbors
	 * w p e	e, n, w, s we define displacements E=e-p, N=n-p, ...
	 *   s		and compute normal(p) = ExN + NxW + WxS + SxE
	 * This turns out to be equal to (e-w) x (n-s) -- independent of p!
	 *
	 * Since the index arithmetic gets a bit messy at the boundaries
	 * we make two passes.  Pass 1 computes e-w and holds it in
	 * the array to be filled with normals.
	 * Pass 2 computes n-s, takes the cross product and normalizes.
	 */

	unit = evert ? -1.0 : 1.0;

	for(u = 0; u < nu; u++) {
	    if(u == 0) {
		prev = &ap[uwrap ? nu-1 : 0];
		next = &ap[u+1];
	    } else if(u == nu-1) {
		prev = &ap[u-1];
		next = &ap[uwrap ? 0 : u];
	    } else {
		prev = &ap[u-1];
		next = &ap[u+1];
	    }
	    n = &an[u];
	    k = nv;
	    do {
		n->x = next->x - prev->x;	/* e - w */
		n->y = next->y - prev->y;
		n->z = next->z - prev->z;
		n += nu;			/* advance to next v row */
		prev += nu;
		next += nu;
	    } while(--k > 0);
	}

	for(v = 0; v < nv; v++) {
	    if(v == 0) {
		prev = &ap[vwrap ? nu*(nv-1) : 0];
		next = &ap[nu*1];
	    } else if(v == nv-1) {
		prev = &ap[nu*(v-1)];
		next = &ap[vwrap ? 0 : nu*v];
	    } else {
		prev = &ap[nu*(v-1)];
		next = &ap[nu*(v+1)];
	    }
	    n = &an[nu*v];
	    k = nu;
	    do {
		Point3 t;

		t.x = next->x - prev->x;
		t.y = next->y - prev->y;
		t.z = next->z - prev->z;
		x = n->y * t.z - n->z * t.y;
		y = n->z * t.x - n->x * t.z;
		z = n->x * t.y - n->y * t.x;
		norm = x*x + y*y + z*z;
		if(norm == 0.0) {
		    /*
		     * Oh no, degenerate norm.
		     * Let's hope it happened because (part of) a row of
		     * mesh points coincided -- maybe even though N=S,
		     * NE != SE or NW != SW.
		     */
		    if(t.x == 0.0 && t.y == 0.0 && t.z == 0.0) {
			if(k > 1) {
			   t.x = (next+1)->x - (prev+1)->x;
			   t.y = (next+1)->y - (prev+1)->y;
			   t.z = (next+1)->z - (prev+1)->z;
			}
			if(t.x == 0.0 && t.y == 0.0 && t.z == 0.0 && k < nu) {
			   t.x = (next-1)->x - (prev-1)->x;
			   t.y = (next-1)->y - (prev-1)->y;
			   t.z = (next-1)->z - (prev-1)->z;
			}
		    }
		    if(n->x == 0.0 && n->y == 0.0 && n->z == 0.0) {
			/* Do likewise in E-W direction. */
			register HPoint3 *cur = &ap[nu*(v+1) - k];

			if(k == 1) cur--;
			else if(k == nu) cur++;
			if(v > 0) {
			    cur -= nu;		/* SE-SW */
			    n->x = (cur+1)->x - (cur-1)->x;
			    n->y = (cur+1)->y - (cur-1)->y;
			    n->z = (cur+1)->z - (cur-1)->z;
			}
			if(n->x==0.0 && n->y==0.0 && n->z==0.0 && v < nv-1) {
			    cur += 2*nu;	/* NE-NW */
			    n->x = (cur+1)->x - (cur-1)->x;
			    n->y = (cur+1)->y - (cur-1)->y;
			    n->z = (cur+1)->z - (cur-1)->z;
			}
		    }
		    x = n->y * t.z - n->z * t.y;
		    y = n->z * t.x - n->x * t.z;
		    z = n->x * t.y - n->y * t.x;
		    norm = x*x + y*y + z*z;
		    if(norm == 0.0) {
			/* Oh well. */
			n->x = unit;
			norm = 1.0;
		    }
		}
		norm = unit / sqrt(norm);
		n->x = x*norm;
		n->y = y*norm;
		n->z = z*norm;

		n++;			/* Next u column */
		prev++;
		next++;
	    } while(--k > 0);
	}
}
