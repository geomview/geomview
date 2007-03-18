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

#include "meshP.h"

Mesh *
MeshCopy(Mesh *obj)
{
    Mesh  *m;
    Mesh  *om = obj;
    int    n;

    if (om == NULL) return (NULL);

    if ( (m = GeomNew(Mesh)) == NULL ) {
	GeomError(0,"Can't allocate space for mesh");
	return (NULL);
    }

    *m = *om; /* copy scalar fields */
    n = m->nu * m->nv;
    if ((m->p = GeomNewN(HPoint3, n)) == NULL) {
	GeomError(0,"Can't allocate space for mesh vertices");
	return (NULL);
    }
    memcpy(m->p, om->p, n * sizeof(HPoint3));

    if (m->geomflags & MESH_N) {
	if ((m->n = GeomNewN(Point3, n)) == NULL) {
	    GeomError(0,"Can't allocate space for mesh normals");
	    return(NULL);
	}
	memcpy(m->n, om->n, n * sizeof(Point3));
    } else
	m->n = NULL;

    if (m->geomflags & MESH_C) {
	if ((m->c = GeomNewN(ColorA, n)) == NULL) {
	    GeomError(0,"Can't allocate space for mesh colors");
	    return(NULL);
	}
	memcpy(m->c, om->c, n * sizeof(ColorA));
    } else {
	m->c = NULL;
    }

    if (m->geomflags & MESH_U) {
	if ((m->u = GeomNewN(TxST, n)) == NULL) {
	    GeomError(0,"Can't allocate space for mesh texture");
	    return(NULL);
	}
	memcpy(m->u, om->u, n * sizeof(TxST));
    } else
	m->u = NULL;

    return m;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
