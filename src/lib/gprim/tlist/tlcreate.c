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

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";

/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "tlistP.h"
/*
 * Tlist creation, editing and deletion
 */

void
TlistDelete( register Tlist *tlist )
{
    if(tlist) {
	if(tlist->tlist != NULL) GeomDelete(tlist->tlist);
	if(tlist->tlisthandle != NULL) HandlePDelete(&tlist->tlisthandle);
	if(tlist->elements != NULL) OOGLFree(tlist->elements);
    }
}

Tlist *
TlistCopy( Tlist *t) 
{
  register Tlist *nt;
  int i;

  nt = OOGLNewE(Tlist, "TlistCopy: Tlist");
  GGeomInit(nt, t->Class, t->magic, NULL);
  nt->nelements = t->nelements;
  nt->elements = OOGLNewNE(Transform, nt->nelements, "TList transforms");
  for (i = 0; i < nt->nelements; i++)
    TmCopy(t->elements[i], nt->elements[i]);
  nt->tlist = GeomCopy(t->tlist);
  nt->tlisthandle = NULL;
  return(nt);

}

Geom *
TlistReplace( register Tlist *tlist, Geom *geom )
{
    register Geom *old;

    if(tlist == NULL)
	return NULL;

    old = tlist->tlist;
    tlist->tlist = geom;
    return old;
}

int
TlistGet( register Tlist *tlist, int attr, register void *attrp )
{
    switch(attr) {
	case CR_GEOM: *(Geom **)attrp = (Geom *)tlist->tlist; break;
	default: return -1;
    }
    return 1;
}
	
Tlist *
TlistCreate(Tlist *exist, GeomClass *Classp, va_list a_list)
{
    register Tlist *tlist;
    Geom *g;
    Handle *h;
    int copy = 1;
    int copyel = 0;
    int attr;
    register int i;
    Transform *elements = (Transform *) NULL;

    if (exist == NULL) {
	tlist = OOGLNewE(Tlist, "TlistCreate tlist");
	tlist->tlisthandle = NULL;
	tlist->tlist = NULL;
	tlist->nelements = 0;
	tlist->elements = NULL;
	GGeomInit (tlist, Classp, TLISTMAGIC, NULL);
    } else {
	/* XXX Check that exist is a tlist. */
	tlist = exist;
    }

    while ((attr = va_arg (a_list, int))) switch(attr) {
	case CR_GEOM:
	case CR_TLIST:
	    h = NULL;
	    goto settlist;
	case CR_HANDLE_GEOM:
	    h = va_arg(a_list, Handle *);
	  settlist:
	    g = va_arg(a_list, Geom *);
	    if(copy) RefIncr((Ref *)g);
	    if(tlist->tlist) GeomDelete(tlist->tlist);
	    tlist->tlist = g;
	  sethandle:
	    if(tlist->tlisthandle) HandleDelete(tlist->tlisthandle);
	    if(copy && h) RefIncr((Ref *)h);
	    tlist->tlisthandle = h;
	    break;
	case CR_TLISTHANDLE:
	    h = va_arg(a_list, Handle *);
	    goto sethandle;
	case CR_ELEM:
	    elements = va_arg (a_list, Transform *);
	    copyel = copy;
	    break;
	case CR_NELEM:
	    tlist->nelements = va_arg (a_list, int);
	    if (tlist->elements != NULL) {
		OOGLFree (tlist->elements);
		tlist->elements = NULL;
	    }
	    break;
	default:
	    if(GeomDecorate(tlist, &copy, attr, ALISTADDR a_list)) {
		OOGLError (0, "TlistCreate: undefined option: %d", attr);
		if (exist == NULL) GeomDelete ((Geom *) tlist);
		return NULL;
	    }
    }

    if ((tlist->elements == NULL) && (tlist->nelements > 0)) {
	if (elements != NULL && !copyel)
	    tlist->elements = elements;
	else {
	    tlist->elements = OOGLNewNE(Transform, tlist->nelements,
				"TlistCreate: matrices");
	    if (elements == NULL) 
		for (i=0; i<tlist->nelements; i++)
		    TmIdentity (tlist->elements[i] );
	    else
		memcpy (tlist->elements, elements,
				tlist->nelements * sizeof (Transform));
	}    
    }

    return tlist;
}
