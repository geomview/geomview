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

/* $Header: /home/mbp/geomview-git/geomview-cvs/geomview/src/lib/gprim/npolylist/nplcopy.c,v 1.1 2000/08/15 16:33:19 mphillips Exp $ */

/*
 * Geometry object routines
 *
 * Copy an NPolyList.
 */


#include "npolylistP.h"

NPolyList *
NPolyListCopy(NPolyList *p)
{
	NPolyList *newpl;
	NPoly *newp;
	float *newv;
	int *newvi;
	ColorA *newvc;
	register int i, j;

	if(p == NULL) return NULL;

	newv = OOGLNewNE(float, p->pdim*p->n_verts, "NPolyList verts");
	newp = OOGLNewNE(NPoly, p->n_polys, "NPolyList polygons");
	newvc = p->vcol ? OOGLNewNE(ColorA, p->n_verts, "NPolyList vcolors") : NULL;
	newvi = OOGLNewNE(int, p->nvi, "NPolyList vert indices");
	newpl = OOGLNewE(NPolyList, "NPolyList");
	*newpl = *p;
	newpl->p = newp;
	newpl->v = newv;
	newpl->vcol = newvc;
	memcpy(newv, p->v, p->n_verts * p->pdim * sizeof(float));
	memcpy(newp, p->p, p->n_polys * sizeof(NPoly));
	memcpy(newvi, p->vi, p->nvi * sizeof(int));
	if(newvc) memcpy(newvc, p->vcol, p->n_verts * sizeof(ColorA));

	return (newpl);
}
