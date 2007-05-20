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
#include "nodedata.h"

/*
 * List editing, deletion and creation.
 */
Geom *ListReplace(List *list, Geom *geom)
{
    Geom *g;

    if(list == NULL) {
	return NULL;
    }
    g = list->car;
    list->car = geom;

    return g;
}

Geom *ListRemove(Geom *list, Geom *g)
{
    List *l;
    List **prev;

    if (list == NULL) {
	return NULL;
    }

    if (list->Class != ListClass) {
	OOGLError(1,
		  "ListRemove: %x is a %s not a List!", list, GeomName(list));
	return NULL;
    }
    for (prev = (List **)(void *)&list; (l = *prev) != NULL; prev = &l->cdr) {
	if (l->car == g) {
	    *prev = l->cdr;
	    l->cdr = NULL;
	    GeomDelete( (Geom *)l );
	    break;
	}
    }

    return list;
}

/*
 * Delete a List.
 * Don't use a for-loop over all the list elements;
 * someone else might have pointers to them.
 */
void
ListDelete(List *l)
{
    if (l->cdr) GeomDelete((Geom *)l->cdr);
    if (l->car) GeomDelete(l->car);
    if (l->carhandle) HandlePDelete(&l->carhandle);
}

List *ListCopy(List *list)
{
    List *l, *nl;
    Geom *newcar;
    List *newlist;
    List **tailp = &newlist;

    for (l = list; l != NULL; l = l->cdr) {
	newcar = GeomCopy(l->car);
	if (newcar == NULL)
	    continue;
	nl = OOGLNewE(List, "ListCopy: List");
	GGeomInit(nl, list->Class, list->magic, NULL);
	*tailp = nl;
	tailp = &nl->cdr;
	nl->car = newcar;
	nl->carhandle = NULL;
    }
    *tailp = NULL;
    return newlist;
}


int ListGet(List *l, int attr, void *attrp)
{
    switch(attr) {
    case CR_GEOM: *(Geom **)attrp = l->car; break;
    case CR_GEOMHANDLE: *(Handle **)attrp = l->carhandle; break;
    case CR_CDR: *(Geom **)attrp = (Geom *)l->cdr; break;
    default: return -1;
    }
    return 1;
}

Geom *ListAppend(Geom *lg, Geom *g)
{
    List *new = OOGLNewE(List, "ListAppend: List");
    List *l = (List*)lg;

    if (l && l->Class->Delete != (GeomDeleteFunc *)ListDelete) {
	OOGLError(0, "ListAppend: attempt to append to a %s, not a List",
		  GeomName((Geom *)l));
	return NULL;
    }

    new->car = g;
    new->cdr = NULL;
    if (l) {
	while (l->cdr) l = l->cdr;
	l->cdr = new;
	GGeomInit(new, lg->Class, lg->magic, NULL);
    } else {
	l = new;
	GGeomInit(new, ListClass, LISTMAGIC, NULL);
    }
    new->carhandle = NULL;

    return lg ? lg : (Geom *)new;
}

List *ListCreate (List *exist, GeomClass *Classp, va_list *a_list )
{
    List *list, *l;
    int attr, copy = 1;
    Handle *h;
    Geom *g;
    bool tree_changed = false;

    if (exist == NULL) {
	list = OOGLNewE( List, "ListCreate: new List" );
	GGeomInit(list, Classp, LISTMAGIC, NULL);
	list->cdr = NULL;
	list->carhandle = NULL;
	list->car = NULL;
    } else {
	if (exist->Class != Classp) {
	    OOGLError(0, "ListCreate: existing_value %x (magic %x) not a List",
		exist, exist->magic);
	    return NULL;
	}
	list = exist;
    }

    while ((attr = va_arg (*a_list, int))) switch (attr) {
	case CR_HANDLE_GEOM:
	    /*
	     * Like GeomReplace, but takes a Handle too.
	     */
	    h = va_arg(*a_list, Handle *);
	    g = va_arg(*a_list, Geom *);
	    if (g == NULL && h != NULL)
		g = (Geom *)HandleObject(h);
	    if (copy) {
		if (h) RefIncr((Ref *)h);
		RefIncr((Ref *)g);
	    }
	    GeomDelete(list->car);
	    HandlePDelete(&list->carhandle);
	    list->carhandle = h;
	    list->car = g;
	    if (h) {
		HandleRegister(&list->carhandle,
			       (Ref *)list, &list->car, HandleUpdRef);
		HandleSetObject(list->carhandle, (Ref *)g);
	    }
	    tree_changed = true;
	    break;
	case CR_GEOM:	/* == CR_CAR */
	    if (list->car != NULL || list->carhandle != NULL) {
		l = OOGLNewE(List, "ListCreate: List");
		GGeomInit(l, Classp, LISTMAGIC, NULL);
		l->car = list->car;
		l->carhandle = list->carhandle;
		RefIncr((Ref *)list->carhandle);
		l->cdr = list->cdr;
		list->cdr = l;
	    }
	    list->car = va_arg (*a_list, Geom *);
	    if (copy) {
		RefIncr((Ref *)list->car);
	    }
	    tree_changed = true;
	    break;
	case CR_GEOMHANDLE:
	    if (list->car != NULL || list->carhandle != NULL) {
		l = OOGLNewE(List, "ListCreate: List");
		l->car = list->car;
		l->carhandle = list->carhandle;
		l->cdr = list->cdr;
		list->cdr = l;
		list->car = NULL;
	    }
	    h = va_arg(*a_list, Handle *);
	    if (copy) RefIncr((Ref *)h);
	    HandlePDelete(&list->carhandle);
	    list->carhandle = h;
	    HandleRegister(&list->carhandle,
			   (Ref *)list, &list->car, HandleUpdRef);
	    tree_changed = true;
	    break;
	case CR_CDR:
	    l = va_arg (*a_list, List *);
	    if (l && l->Class != Classp) {
		OOGLError(0, "ListCreate: CDR %x (magic %x) not a List",
			l, l->magic);
		goto fail;
	    }
	    if (list->cdr) {
		GeomDelete((Geom *)list->cdr);
	    }
	    if (copy) {
		RefIncr((Ref *)l);
	    }
	    list->cdr = l;
	    tree_changed = true;
	    break;
	default:
	    if (GeomDecorate (list, &copy, attr, a_list)) {
		OOGLError (0, "ListCreate: Undefined attribute: %d", attr);

	      fail:
		if (exist == NULL) {
		    GeomDelete ((Geom *)list);
		}
		return NULL;
	    }
    }

    if (tree_changed) {
	GeomNodeDataPrune((Geom *)list);
    }

    return list;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
