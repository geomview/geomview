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

/* $Header: /home/mbp/geomview-git/geomview-cvs/geomview/src/lib/gprim/polylist/plevert.c,v 1.2 2000/09/01 22:38:13 mphillips Exp $ */
/* $Source: /home/mbp/geomview-git/geomview-cvs/geomview/src/lib/gprim/polylist/plevert.c,v $ */


/*
 * Evert all the normals in a polylist.
 */

#include	"polylistP.h"

PolyList *
PolyListEvert( register PolyList *polylist )
{
	register Poly	*p;
	register Vertex	*v;
	register int	i;

	polylist->flags ^= PL_EVNORM;

	if ((polylist->flags & (PL_HASVN|PL_HASPN)) == 0)
	{
		PolyListComputeNormals(polylist);
	} else {
	    i = polylist->n_polys;
	    for (p = polylist->p; --i >= 0; p++)
	    {
		p->pn.x = -p->pn.x;
		p->pn.y = -p->pn.y;
		p->pn.z = -p->pn.z;
	    }
	    i = polylist->n_verts;
	    for (v = polylist->vl; --i >= 0; v++) {
		v->vn.x = -v->vn.x;
		v->vn.y = -v->vn.y;
		v->vn.z = -v->vn.z;
	    }
	}
	return (polylist);
}
