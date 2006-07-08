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

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "bboxP.h"

static GeomClass *aBBoxMethods = NULL;

int BBoxPresent = 1;

static char bboxName[] = "bbox";

extern BBox *BBoxCopy ( BBox * );
extern Geom *BBoxCreate ( va_list *);
extern BBox *BBoxDelete ( BBox * );

extern BBox *BBoxFSave( BBox *, FILE *, char * );
extern BBox *BBoxFLoad( FILE *, char * );

char *
BBoxName()
{
	return bboxName;
}

GeomClass      *
BBoxMethods()
{
	if (!aBBoxMethods)
	{
		aBBoxMethods = GeomClassCreate(bboxName);
		aBBoxMethods->name = (GeomNameFunc *) BBoxName;
		aBBoxMethods->bound = (GeomBoundFunc *) BBoxBound;
		aBBoxMethods->copy = (GeomCopyFunc *) BBoxCopy;
		aBBoxMethods->create = (GeomCreateFunc *) BBoxCreate;
		aBBoxMethods->draw = (GeomDrawFunc *) BBoxDraw;
		aBBoxMethods->methods = (GeomMethodsFunc *) BBoxMethods;
		aBBoxMethods->name = (GeomNameFunc *) BBoxName;
		aBBoxMethods->transform = (GeomTransformFunc *) BBoxTransform;
		aBBoxMethods->fsave = (GeomFSaveFunc *) BBoxFSave;
		aBBoxMethods->fload = (GeomFLoadFunc *) BBoxFLoad;
		/*
		 * Note we need no delete method -- GeomDelete does all.
		 */
	}
	return aBBoxMethods;
}
