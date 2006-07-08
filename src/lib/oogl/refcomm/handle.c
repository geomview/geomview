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

#include <string.h>
#include "handleP.h"


Handle *AllHandles = NULL;

void
HandleUpdRef(Handle **hp, Ref *parent, Ref **objp)
{
    register Handle *h;

    if((h = *hp) != NULL && objp != NULL && h->object != *objp) {
	if(h->ops->Delete) {
	    RefIncr((Ref *)h->object);
	    if(*objp) (*h->ops->Delete)(*objp);
	}
	*objp = h->object;
    }
}
	    
void HandleUnregister(Handle **hp)
{
    register Handle *h;
    register HRef *rp;
    register int i;

    if(hp == NULL || (h = *hp) == NULL)
	return;
    for(i = h->nrefs, rp = &h->refs[i]; --i >= 0; ) {
	rp--;
	if(rp->hp == hp)
	    *rp = h->refs[--h->nrefs];
    }
}

void HandleUnregisterJust(Handle **hp, Ref *obj, void *info, void (*update) P((Handle **, Ref *, void *)))
{
    register Handle *h;
    register HRef *rp;
    register int i;

    if(hp == NULL || (h = *hp) == NULL)
	return;

    for(i = h->nrefs, rp = &h->refs[i]; --i >= 0; ) {
	rp--;
	if(rp->hp == hp &&
			(obj == NULL || rp->parentobj == obj) &&
			(info == NULL || rp->info == info) &&
			(update == NULL || rp->update == update)) {
	    *rp = h->refs[--h->nrefs];
	}
    }
}

/*
 * Remove all callbacks on any handle with the given properties.
 */
void HandleUnregisterAll(Ref *obj, void *info, void (*update) P((Handle **, Ref *, void *)))
{
    register Handle *h;
    register HRef *rp;
    register int i;

    for(h = AllHandles; h != NULL; h = h->next) {
	for(i = h->nrefs, rp = &h->refs[i]; --i >= 0; ) {
	    rp--;
	    if((obj == NULL || rp->parentobj == obj) &&
			(info == NULL || rp->info == info) &&
			(update == NULL || rp->update == update)) {
		*rp = h->refs[--h->nrefs];
	    }
	}
    }
}

static
void handleupdate(Handle *h, register HRef *rp)
{
    if(rp->update && h == *rp->hp)
	(*rp->update)(rp->hp, rp->parentobj, rp->info);
    else { /* BUG */
	OOGLError(1, "handleupdate mismatch: h %x %s, rp->hp %x, *rp->hp %x, rp->parentobj %x, rp->update %x",
		h, h->name, rp->hp, *rp->hp, rp->parentobj, rp->update);
	if(*rp->hp)
	    OOGLError(1, "... *rp->hp->name %s", (*rp->hp)->name);
    }
}

int HandleRegister(Handle **hp, Ref *parentobj, void *info, void (*update) P((Handle **, Ref *, void *)))
{
    register Handle *h;
    register HRef *rp;
    register int i;

    if(hp == NULL || (h = *hp) == NULL)
	return 0;
    for(i = h->nrefs, rp = h->refs; --i > 0; rp++) {
	if(rp->hp == hp && rp->parentobj == parentobj && rp->info == info)
	    goto doit;
    }

    if(h->nrefs >= h->maxrefs) {
	h->refs = h->maxrefs == 0 ?
		OOGLNewNE(HRef, (h->maxrefs = 3), "HandleRegister") :
		OOGLRenewNE(HRef, h->refs, (h->maxrefs += h->maxrefs+1),
				"HandleRegister");
    }
    rp = &h->refs[h->nrefs++];

    rp->hp = hp;
  doit:
    rp->parentobj = parentobj;
    rp->info = info;
    rp->update = update;
    /* handleupdate(h, rp);	Do we need this?  I hope not. - slevy */
    return 1;
}

char *
HandleName(Handle *h)
{
   return h ? h->name : NULL;
}

Handle *
HandleByName(char *name, HandleOps *ops)
{
    register Handle *h;

    for(h = AllHandles; h != NULL; h = h->next) {
	if(h->ops == ops && strcmp(h->name, name) == 0)
	    return h;
    }
    return NULL;
}

static Handle *
handlecreate(char *name, HandleOps *ops)
{
    register Handle *h;

    h = OOGLNewE(Handle, "new Handle");
    RefInit((Ref *)h, HANDLEMAGIC);
    h->ops = ops;
    h->name = strdup(name);
    h->object = NULL;
    h->maxrefs = 0;
    h->nrefs = 0;
    h->refs = NULL;
    h->whence = NULL;
    h->permanent = 1;
    h->next = AllHandles;
    AllHandles = h;
    return h;
}

/*
 * Assign a new object to a Handle.
 */
int
HandleSetObject(register Handle *h, Ref *object)
{
    register int i;
    Ref *oldobj;

    if(h == NULL)
	return 0;

    if(h->object == object)
	return 1;

    oldobj = h->object;
    h->object = object;
    if(h->ops->Delete != NULL) {
	if(oldobj) (*h->ops->Delete)(oldobj);
	RefIncr(object);
    }

    if(object != NULL && object->handle == NULL)
	object->handle = h;

    for(i = h->nrefs; --i >= 0; )
	handleupdate(h, &h->refs[i]);

    return 1;
}

/*
 * Create a Handle, don't reassign its value.
 */
Handle *
HandleCreate(char *name, HandleOps *ops)
{
    register Handle *h;

    h = HandleByName(name, ops);
    if(h == NULL) h = handlecreate(name, ops);
    return h;
}

/*
 * Create a Handle (or reassign its value if it already exists)
 */
Handle *
HandleAssign(char *name, HandleOps *ops, Ref *object)
{
    register Handle *h;

    h = HandleByName(name, ops);
    if(h == NULL)
	h = handlecreate(name, ops);
    HandleSetObject(h, object);
    return h;
}

/*
 * Return the current object held by a Handle.
 */
Ref *
HandleObject(Handle *h)
{ return h->object; }

void HandlePDelete(Handle **hp)
{
    if(hp && *hp) {
	HandleUnregister(hp);
	HandleDelete(*hp);
    }
}

void HandleDelete(Handle *h)
{
  register Handle **hp;

  if(h == NULL)
    return;
  if(h->magic != HANDLEMAGIC) {
    OOGLWarn("Internal warning: HandleDelete of non-Handle %x (%x != %x)",
	h, h->magic, HANDLEMAGIC);
    return;
  }
  /*
   * Don't delete a Handle if it still has active references.
   * Also don't delete Handles marked "permanent"; those loaded with '<'
   * aren't permanent, but others are.
   */
  if(h->nrefs > 0 || h->permanent) {
    if(h->ref_count > 0) RefDecr((Ref *)h);
    return;
  }
#ifdef HANDLE_REF_COUNTS_OK
  if(RefDecr((Ref*)h) > 0)
    return;
#else
  /* Avoid RefDecr() error messages.
   * We're not correctly maintaining Handle ref counts in the rest of the code.
   * Until we do, avoid spurious RefDecr() error messages.
   * Note that we never actually free a Handle under this ifdef case,
   * so there's no danger of reusing freed memory.  -SL 92.08.31
   */
  if(h->ref_count == 0 || RefDecr((Ref *)h) > 0)
    return;
#endif

  if(h->object && h->object->handle == h && h->ops->Delete && h->object->ref_count > 0) {
    h->object->handle = NULL;
    (*h->ops->Delete)(h->object);
  }
  for(hp = &AllHandles; *hp != NULL; hp = &(*hp)->next) {
    if(*hp == h) {
	*hp = h->next;
	break;
    }
  }

  /* Don't let Pool code think we have something cached in this handle! */
  if(h->whence && h->whence->seekable) {
    h->whence->flags &= ~PF_ANY;
    PoolClose(h->whence);
    h->whence = 0;
  }
#ifdef HANDLE_REF_COUNTS_OK
  h->object = NULL;
  if(h->refs)
    OOGLFree(h->refs);
  if(h->name)
    free(h->name);
  h->magic &= ~0x80000000;
  OOGLFree(h);
#else
  /* Suspect that some Handles are being used after deletion.
   * Invalidate the object pointer to cause a likely crash,
   * but leave the Handle in memory as a debugging aid.  -slevy 92.07.27
   */
  h->magic &= ~0x80000000;
  h->object = (Ref *)((long)h->object | 1);
#endif
}
