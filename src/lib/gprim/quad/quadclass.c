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

#include "quadP.h"

static GeomClass *aQuadMethods = NULL;

/* these are known only to the library, not the user */
extern Quad *QuadCopy( Quad * );
extern Quad *QuadDelete( Quad * );
extern Geom *QuadCreate ( va_list);

int QuadPresent = 1;

static char quadName[] = "quad";

char *
QuadName()
{
	return quadName;
}

GeomClass *
QuadMethods()
{
    if( !aQuadMethods ) {
	aQuadMethods = GeomClassCreate(quadName);

	aQuadMethods->name = QuadName;
        aQuadMethods->methods = (GeomMethodsFunc *)QuadMethods;
	aQuadMethods->copy = (GeomCopyFunc *)QuadCopy;
	aQuadMethods->create = (GeomCreateFunc *)QuadCreate;
        aQuadMethods->fload = (GeomFLoadFunc *)QuadFLoad;
	aQuadMethods->fsave = (GeomFSaveFunc *)QuadFSave;
	aQuadMethods->Delete = (GeomDeleteFunc *)QuadDelete;
        aQuadMethods->bound = (GeomBoundFunc *)QuadBound;
	aQuadMethods->boundsphere = 
	  (GeomBoundSphereFunc *)QuadSphere;
        aQuadMethods->evert = (GeomEvertFunc *)QuadEvert;
        aQuadMethods->draw = (GeomDrawFunc *)QuadDraw;
        aQuadMethods->bsptree = (GeomBSPTreeFunc *)QuadBSPTree;
	aQuadMethods->pick = (GeomPickFunc *) QuadPick;
	aQuadMethods->transform = (GeomTransformFunc *) QuadTransform;
	aQuadMethods->transformto = (GeomTransformToFunc *) QuadTransformTo;
    }
    return aQuadMethods;
}
