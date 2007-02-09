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

#include "bezierP.h"

static GeomClass *aBezierMethods = NULL;
static GeomClass *aBezierListMethods = NULL;

int BezierPresent = 1;
int BezierListPresent = 1;

static char nameBezier[] = "bezier";
static char nameBezierList[] = "bezierlist";

char *
BezierName()
{
	return nameBezier;
}

char *
BezierListName()
{
	return nameBezierList;
}


GeomClass *
BezierMethods()
{
    if( !aBezierMethods ) {
	aBezierMethods = GeomClassCreate(nameBezier);

	aBezierMethods->name = BezierName;
        aBezierMethods->methods = (GeomMethodsFunc *)BezierMethods;
        aBezierMethods->create = (GeomCreateFunc *)BezierCreate;
        aBezierMethods->evert = (GeomEvertFunc *)BezierEvert;
	aBezierMethods->Delete = (GeomDeleteFunc *)BezierDelete;
        aBezierMethods->bound = (GeomBoundFunc *)BezierBound;
	aBezierMethods->boundsphere = 
	  (GeomBoundSphereFunc *)BezierBoundSphere;
        aBezierMethods->draw = (GeomDrawFunc *)BezierDraw;
        aBezierMethods->bsptree = (GeomBSPTreeFunc *)BezierBSPTree;
	aBezierMethods->copy = (GeomCopyFunc *)BezierCopy;
        aBezierMethods->pick = (GeomPickFunc *)BezierPick;
	aBezierMethods->dice = (GeomDiceFunc *)BezierDice;
	aBezierMethods->transform = 
	  (GeomTransformFunc *)BezierTransform;
	aBezierMethods->transformto =
	  (GeomTransformToFunc *)BezierTransform;
    }
    return aBezierMethods;
}

GeomClass *
BezierListMethods()
{
    if( !aBezierListMethods ) {
	(void) ListMethods();
	aBezierListMethods = GeomSubClassCreate("list", "bezierlist");

	aBezierListMethods->name = BezierListName;
        aBezierListMethods->methods = (GeomMethodsFunc *)BezierListMethods;
        aBezierListMethods->fload = (GeomFLoadFunc *)BezierListFLoad;
	aBezierListMethods->fsave = (GeomFSaveFunc *)BezierListFSave;
	aBezierListMethods->import = NULL;
	aBezierListMethods->export = NULL;
    }
    return aBezierListMethods;
}
