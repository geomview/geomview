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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips,
   Celeste Fowler */

#include "listP.h"

void
ListHandleScan( List *list, int (*func)(), void *arg )
{
    while(list) {
	if(list->carhandle)
	    (*func)(&list->carhandle, list, arg);
	GeomHandleScan( list->car, func, arg );
	list = list->cdr;
    }
}

List *
ListDice( List *list, int nu, int nv )
{
    List *l;
    for(l = list; l != NULL; l = l->cdr)
	GeomDice(l->car, nu, nv);
    return list;
}

List *
ListEvert( List *list )
{
    List *l;

    for(l = list; l != NULL; l = l->cdr)
	GeomEvert( l->car );

    return list;
}

BBox *ListBound(List *list, Transform  T, TransformN *TN)
{
    Geom *carbox = NULL;
    BBox *bbox = NULL;
    List *l;

    for(l = list; l != NULL; l = l->cdr) {
	    carbox = GeomBound( l->car, T, TN );
	if(carbox == NULL)
		continue;
	if(bbox) {
	    BBoxUnion3(bbox, (BBox *)carbox, bbox);
	    GeomDelete( carbox );
	} else {
	    bbox = (BBox *)carbox;
	}
    }
    return bbox;
}

List *ListTransform(List *list, Transform T, TransformN *TN)
{
    List *l;

    for(l = list; l != NULL; l = l->cdr)
	    GeomTransform( l->car, T, TN );

    return list;
}

