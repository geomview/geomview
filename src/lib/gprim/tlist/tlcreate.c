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

#include "tlistP.h"
/*
 * Tlist creation, editing and deletion
 */

DEF_FREELIST(Tlist);

void TlistFreeListPrune(void)
{
  FreeListNode *old;
  Tlist *oldtl;
  size_t size = 0;
  
  while (TlistFreeList) {
    old = TlistFreeList;
    TlistFreeList = old->next;
    oldtl = (Tlist *)old;
    if (oldtl->elements != NULL && oldtl->nallocated != 0) {
      OOGLFree(oldtl->elements);
    }
    size += sizeof(Transform) * oldtl->nallocated;
    OOGLFree(old);
    size += sizeof(Tlist);
  }
  OOGLWarn("Freed %ld bytes.\n", size);
}

void TlistDelete(Tlist *tlist)
{
    if(tlist) {
	if (tlist->tlist != NULL) GeomDelete(tlist->tlist);
	if (tlist->tlisthandle != NULL) HandlePDelete(&tlist->tlisthandle);
	if (tlist->elements != NULL && tlist->freelisthead == NULL) {
	  OOGLFree(tlist->elements);
	  tlist->elements = NULL;
	  tlist->nallocated = 0;
	}
    }
}

Tlist *TlistCopy(Tlist *t) 
{
  Tlist *nt;
  int i;

  FREELIST_NEW(Tlist, nt);
  GGeomInit(nt, t->Class, t->magic, NULL);
  nt->freelisthead = &TlistFreeList;
  nt->nelements = t->nelements;
  if (nt->nallocated < nt->nelements) {
    nt->elements =
      OOGLRenewNE(Transform, nt->elements, nt->nelements, "Tlist transforms");
    nt->nallocated = nt->nelements;
  }
  for (i = 0; i < nt->nelements; i++) {
    TmCopy(t->elements[i], nt->elements[i]);
  }
  nt->tlist = GeomCopy(t->tlist);
  nt->tlisthandle = NULL;
  return(nt);

}

Geom *
TlistReplace( Tlist *tlist, Geom *geom )
{
    Geom *old;

    if(tlist == NULL)
	return NULL;

    old = tlist->tlist;
    tlist->tlist = geom;
    return old;
}

int
TlistGet( Tlist *tlist, int attr, void *attrp )
{
    switch(attr) {
	case CR_GEOM: *(Geom **)attrp = (Geom *)tlist->tlist; break;
	default: return -1;
    }
    return 1;
}
	
Tlist *
TlistCreate(Tlist *exist, GeomClass *Classp, va_list *a_list)
{
    Tlist *tlist;
    Geom *g;
    Handle *h;
    int  copy   = true;
    bool copyel = false;
    bool doinit = false;
    int attr;
    int i;
    Transform *elements = NULL;

    if (exist == NULL) {
	FREELIST_NEW(Tlist, tlist);
	GGeomInit (tlist, Classp, TLISTMAGIC, NULL);
	tlist->freelisthead = &TlistFreeList;
	tlist->tlisthandle = NULL;
	tlist->tlist = NULL;
	tlist->nelements = 0;
    } else {
	/* XXX Check that exist is a tlist. */
	tlist = exist;
    }

    while ((attr = va_arg (*a_list, int))) switch(attr) {
	case CR_GEOM:
	case CR_TLIST:
	    h = NULL;
	    goto settlist;
	case CR_HANDLE_GEOM:
	    h = va_arg(*a_list, Handle *);
	  settlist:
	    g = va_arg(*a_list, Geom *);
	    if(copy) {
		RefIncr((Ref *)g);
	    }
	    if(tlist->tlist) {
		GeomDelete(tlist->tlist);
	    }
	    tlist->tlist = g;
	  sethandle:
	    if(tlist->tlisthandle) {
		HandleDelete(tlist->tlisthandle);
	    }
	    if(copy && h) {
		RefIncr((Ref *)h);
	    }
	    tlist->tlisthandle = h;
	    break;
	case CR_TLISTHANDLE:
	    h = va_arg(*a_list, Handle *);
	    goto sethandle;
	case CR_ELEM:
	    elements = va_arg (*a_list, Transform *);
	    copyel = copy;
	    if (!copyel) {
		OOGLFree(tlist->elements);
		tlist->elements   = NULL;
		tlist->nallocated = 0;
	    }
	    if (elements == NULL) {
	      doinit = true;
	    }
	    break;
	case CR_NELEM:
	    tlist->nelements = va_arg (*a_list, int);
	    doinit = true;
	    break;
	default:
	    if(GeomDecorate(tlist, &copy, attr, a_list)) {
		OOGLError (0, "TlistCreate: undefined option: %d", attr);
		if (exist == NULL) {
		    GeomDelete ((Geom *) tlist);
		}
		return NULL;
	    }
    }
    
    if (elements != NULL && !copyel) {
      if (tlist->elements) {
	OOGLFree(tlist->elements);
      }
      tlist->elements   = elements;
      tlist->nallocated = tlist->nelements;
    } else {
      if (tlist->nallocated < tlist->nelements) {
	tlist->elements =
	  OOGLRenewNE(Transform, tlist->elements, tlist->nelements,
		      "TlistCreate: matrices");
	tlist->nallocated = tlist->nelements;
      }
      if (elements == NULL) {
	if (doinit) {
	  for (i = 0; i < tlist->nelements; i++) {
	    TmIdentity (tlist->elements[i] );
	  }
	}
      } else {
	memcpy (tlist->elements, elements,
		tlist->nelements * sizeof (Transform));
      }
    }

    return tlist;
}
