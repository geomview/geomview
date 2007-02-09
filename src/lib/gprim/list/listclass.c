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

#include "listP.h"

GeomClass *ListClass = NULL;

int ListPresent = 1;

static char listName[] = "list";

char *ListName(void)
{
    return listName;
}

extern void ListHandleScan( List *, int (*func)(), void *arg );

GeomClass *ListMethods(void)
{
    if( !ListClass ) {
        ListClass = GeomClassCreate(listName);
	
	ListClass->name = ListName;
        ListClass->methods = (GeomMethodsFunc *)ListMethods;
        ListClass->create = (GeomCreateFunc *)ListCreate;
	ListClass->get = (GeomGetFunc *)ListGet;
        ListClass->Delete = (GeomDeleteFunc *)ListDelete;
        ListClass->bound = (GeomBoundFunc *)ListBound;
	ListClass->boundsphere = 
	    (GeomBoundSphereFunc *)ListBoundSphere;
        ListClass->evert = (GeomEvertFunc *)ListEvert;
	ListClass->dice = (GeomDiceFunc *)ListDice;
        ListClass->transform = (GeomTransformFunc *)ListTransform;
	ListClass->transformto = (GeomTransformToFunc *)ListTransform;
	ListClass->pick = (GeomPickFunc *)ListPick;
	ListClass->copy = (GeomCopyFunc *)ListCopy;
        ListClass->draw = (GeomDrawFunc *)ListDraw;
        ListClass->bsptree = (GeomBSPTreeFunc *)ListBSPTree;
	ListClass->replace = (GeomReplaceFunc *)ListReplace;
	ListClass->scan = (GeomScanFunc *)ListHandleScan;

	ListClass->import = (GeomImportFunc *)ListImport;
	ListClass->export = (GeomExportFunc *)ListExport;
    }

    return ListClass;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
