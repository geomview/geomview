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

#include "geomclass.h"

/* this needs to be here so that the shared memory code can work */
Geom *
GGeomCopy(Geom *g, Geom *object)
{
    RefInit((Ref *)g, object->magic);
    g->Class = object->Class;
    g->ap = object->ap;
    if(g->ap != NULL) {
	g->ap = ApCopy(g->ap, NULL);
    }
    g->aphandle = NULL;
    DblListInit(&g->handles);
    DblListInit(&g->pernode);
    return g;
}

Geom *
GeomCopy(Geom *object)
{
    Geom *g;

    /* this is a little tricky */
    if( object == NULL )
	return NULL;
    if( object->Class->copy ) {
	g = (*object->Class->copy)(object);
	if(g != NULL) {
	    GGeomCopy(g, object);
	}
    } else {
	GeomError(1/*Warning-unimpl*/,"GeomCopy: no copy method for %s: %x",
		GeomName(object), object);
	RefIncr((Ref *)object);	/* Copy by reference -- XXX stopgap! */
	g = object;
    }
    return g;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
