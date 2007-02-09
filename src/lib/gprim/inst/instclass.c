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

#include "instP.h"
#include "pick.h"

GeomClass *InstClass = NULL;

int InstPresent = 1;

static char instName[] = "inst";


char *
InstName()
{
	return instName;
}

extern Geom *InstPick( Geom *, Pick *, Appearance *, Transform );
extern GeomScanFunc InstHandleScan;

GeomClass *
InstMethods()
{
    if( !InstClass ) {
	InstClass = GeomClassCreate(instName);
	
	InstClass->name = InstName;
        InstClass->methods = (GeomMethodsFunc *)InstMethods;
        InstClass->create = (GeomCreateFunc *)InstCreate;
	InstClass->Delete = (GeomDeleteFunc *)InstDelete;
	InstClass->copy = (GeomCopyFunc *)InstCopy;
        InstClass->bound = (GeomBoundFunc *)InstBound;
	InstClass->boundsphere = 
	  (GeomBoundSphereFunc *)InstBoundSphere;
        InstClass->evert = (GeomEvertFunc *)InstEvert;
	InstClass->dice = (GeomDiceFunc *)InstDice;
	InstClass->position = (GeomPositionFunc *)InstPosition;
	InstClass->transformto = (GeomTransformToFunc *)InstTransformTo;
        InstClass->transform = (GeomTransformFunc *)InstTransform;
	InstClass->pick = (GeomPickFunc *)InstPick;
        InstClass->draw = (GeomDrawFunc *)InstDraw;
	InstClass->bsptree = (GeomBSPTreeFunc *)InstBSPTree;
	InstClass->replace = (GeomReplaceFunc *)InstReplace;
	InstClass->get = (GeomGetFunc *)InstGet;
	InstClass->scan = (GeomScanFunc *)InstHandleScan;

	InstClass->import = InstImport;
	InstClass->export = (GeomExportFunc *)InstExport;
    }

    return InstClass;
}
