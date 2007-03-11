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

#include <string.h>
#include "handleP.h"

static DBLLIST(AllHandles);
static DBLLIST(FreeRefs);
static DBLLIST(FreeHandles);

#define HANDLE_DEBUG 0

void handle_dump(void);

static HandleOps NullOps = {
  "null",
};

void HandlesSetObjSaved(bool obj_saved)
{
  HandleOps *ops;
  Handle *h;

  DblListIterateNoDelete(&AllHandles, HandleOps, node, ops) {
    DblListIterateNoDelete(&ops->handles, Handle, opsnode, h) {
      h->obj_saved = obj_saved;
    }
  }
}

/* iterate over Ref->handles */
Handle *HandleRefIterate(Ref *r, Handle *pos)
{
  if (pos == NULL) {
    return DblListEmpty(&r->handles)
      ? NULL
      : REFGET(Handle, DblListContainer(r->handles.next, Handle, objnode));
  } else {
    return pos->objnode.next == &r->handles
      ? NULL
      : REFGET(Handle, DblListContainer(pos->objnode.next, Handle, objnode));
  }
}

/* iterate over Pool->handles */
Handle *HandlePoolIterate(Pool *pool, Handle *pos)
{
  if (pos == NULL) {
    return DblListEmpty(&pool->handles)
      ? NULL
      : REFGET(Handle, DblListContainer(pool->handles.next, Handle, poolnode));
  } else {
    return pos->objnode.next == &pool->handles
      ? NULL
      : REFGET(Handle, DblListContainer(pos->poolnode.next, Handle, poolnode));
  }
}

void
HandleUpdRef(Handle **hp, Ref *parent, Ref **objp)
{
  Handle *h;

  if((h = *hp) != NULL && objp != NULL && h->object != *objp) {
    if (*objp) {
      if (h->ops->delete) {
	(*h->ops->delete)(*objp);
      } else {
	REFPUT(*objp);
      }
    }
    *objp = REFGET(Ref, h->object);
  }
}
	    
void HandleUnregister(Handle **hp)
{
  Handle *h;
  HRef *r, *rn;

  if (hp == NULL || (h = *hp) == NULL) {
    return;
  }

  DblListIterate(&h->refs, HRef, node, r, rn) {
    if(r->hp == hp) {
      DblListDelete(&r->node);
      DblListAdd(&FreeRefs, &r->node);
      REFPUT(h);
    }
  }
}

void HandleUnregisterJust(Handle **hp, Ref *obj, void *info,
			  void (*update) P((Handle **, Ref *, void *)))
{
  Handle *h;
  HRef *rp, *rnext;

  if(hp == NULL || (h = *hp) == NULL) {
    return;
  }

  DblListIterate(&h->refs, HRef, node, rp, rnext) {
    if(rp->hp == hp &&
       (obj == NULL || rp->parentobj == obj) &&
       (info == NULL || rp->info == info) &&
       (update == NULL || rp->update == update)) {
      DblListDelete(&rp->node);
      DblListAdd(&FreeRefs, &rp->node);
      REFPUT(h);
    }
  }
}

/*
 * Remove all callbacks on any handle with the given properties.
 */
void HandleUnregisterAll(Ref *obj,
			 void *info,
			 void (*update) P((Handle **, Ref *, void *)))
{
  HandleOps *ops;
  Handle *h;
  HRef *r, *rn;

  DblListIterateNoDelete(&AllHandles, HandleOps, node, ops) {
    DblListIterateNoDelete(&ops->handles, Handle, opsnode, h) {
      DblListIterate(&h->refs, HRef, node, r, rn) {
	if((obj == NULL || r->parentobj == obj) &&
 	   (info == NULL || r->info == info) &&
	   (update == NULL || r->update == update)) {
	  DblListDelete(&r->node);
	  DblListAdd(&FreeRefs, &r->node);
	  REFPUT(h);
	}
      }
    }
  }
}

static
void handleupdate(Handle *h, HRef *rp)
{
  if(rp->update && h == *rp->hp) {
    (*rp->update)(rp->hp, rp->parentobj, rp->info);
  } else { /* BUG */
    OOGLError(1, "handleupdate mismatch: h %x %s, rp->hp %x, *rp->hp %x, rp->parentobj %x, rp->update %x",
	      h, h->name, rp->hp, *rp->hp, rp->parentobj, rp->update);
    if(*rp->hp) {
      OOGLError(1, "... *rp->hp->name %s", (*rp->hp)->name);
    }
  }
}

int HandleRegister(Handle **hp,
		   Ref *parentobj,
		   void *info,
		   void (*update) P((Handle **, Ref *, void *)))
{
  Handle *h;
  HRef *rp;

  if (hp == NULL || (h = *hp) == NULL) {
    return false;
  }
  
  DblListIterateNoDelete(&h->refs, HRef, node, rp) {
    if (rp->hp == hp && rp->parentobj == parentobj && rp->info == info) {
      goto update_out;
    }
  }

  if (DblListEmpty(&FreeRefs)) {
    rp = OOGLNewE(HRef, "HandleRegister");
  } else {
    rp = DblListContainer(FreeRefs.next, HRef, node);
    DblListDelete(&rp->node);
  }

  REFINCR(h);

  rp->hp = hp;
  rp->parentobj = parentobj;
  rp->info = info;
  
  DblListAdd(&h->refs, &rp->node);

 update_out:
  rp->update = update;
  handleupdate(h, rp);
  return true;
}

char *
HandleName(Handle *h)
{
  return h ? h->name : NULL;
}

HandleOps *HandleOpsByName(char *name)
{
  HandleOps *ops;

  DblListIterateNoDelete(&AllHandles, HandleOps, node, ops) {
    if (strcmp(name, ops->prefix)) {
      return ops;
    }
  }
  return NULL;
}

Handle *
HandleByName(char *name, HandleOps *ops)
{
  Handle *h;

  if (ops == NULL) {
    DblListIterateNoDelete(&AllHandles, HandleOps, node, ops) {
      DblListIterateNoDelete(&ops->handles, Handle, opsnode, h) {
	if (strcmp(h->name, name) == 0) {
	  return REFGET(Handle, h);
	}
      }
    }
  } else {
    if (ops->handles.next == NULL) {
      DblListInit(&ops->handles);
      DblListAdd(&AllHandles, &ops->node);
    }
    DblListIterateNoDelete(&ops->handles, Handle, opsnode, h) {
      if (strcmp(h->name, name) == 0) {
	return REFGET(Handle, h);
      }
    }
  }
  return NULL;
}

static Handle *
handlecreate(char *name, HandleOps *ops)
{
  Handle *h;

#if HANDLE_DEBUG  
  OOGLWarn("Creating handle with name \"%s\"", name);
#endif

  if (DblListEmpty(&FreeHandles)) {
    h = OOGLNewE(Handle, "new Handle");
  } else {
    h = DblListContainer(FreeHandles.next, Handle, opsnode);
    DblListDelete(&h->opsnode);
  }
  RefInit((Ref *)h, HANDLEMAGIC);
  h->ops = ops = ops ? ops : &NullOps;
  h->name = strdup(name);
  h->object = NULL;
  h->whence = NULL;
  h->permanent = false;

  DblListInit(&h->refs);

  /* The following two are nodes, not list-heads, but this way we can
   * safely call DblListDelete(), even if there is no associated list.
   */
  DblListInit(&h->objnode);
  DblListInit(&h->poolnode);

  if (ops->handles.next == NULL) {
    DblListInit(&ops->handles);
    DblListAdd(&AllHandles, &ops->node);
  }
  DblListAddTail(&ops->handles, &h->opsnode);

  /*  handle_dump();*/
  
  return h;
}

/*
 * Assign a new object to a Handle.
 */
int
HandleSetObject(Handle *h, Ref *object)
{
  HRef *ref;

  if(h == NULL) {
    return false;
  }

  if(h->object == object) {
    return true;
  }

  DblListDelete(&h->objnode);

  if (h->object) {
    if (h->ops->delete != NULL) {
      (*h->ops->delete)(h->object);
    } else {
      REFPUT(h->object);
    }
  }
  h->object = REFGET(Ref, object);
  if (object) {
    DblListAddTail(&object->handles, &h->objnode);
  }

  DblListIterateNoDelete(&h->refs, HRef, node, ref) {
    handleupdate(h, ref);
  }

  return true;
}

/*
 * Create a Handle, don't reassign its value.
 */
Handle *
HandleCreate(char *name, HandleOps *ops)
{
  Handle *h;

  h = HandleByName(name, ops);
  if (h == NULL) {
    h = handlecreate(name, ops);
  }
  return h;
}

/*
 * Global handle: increase the ref-count twice.
 */
Handle *
HandleCreateGlobal(char *name, HandleOps *ops)
{
  Handle *h;
  
  h = HandleAssign(name, ops, NULL);
  if (!h->permanent) {
    h->permanent = true;
    return REFGET(Handle, h);
  } else {
    return h;
  }
}

/*
 * Create a Handle (or reassign its value if it already exists)
 */
Handle *
HandleAssign(char *name, HandleOps *ops, Ref *object)
{
  Handle *h;

  h = HandleCreate(name, ops);
  HandleSetObject(h, object);
  return h;
}

/*
 * Return the current object held by a Handle.
 */
Ref *
HandleObject(Handle *h)
{
  return h->object;
}

Pool *
HandlePool(Handle *h)
{
  return h->whence;
}

void HandlePDelete(Handle **hp)
{
  if(hp && *hp) {
    HandleUnregister(hp);
    HandleDelete(*hp);
    *hp = NULL;
  }
}

/* Simple debugging aid: dump the list of currently defined handles
 * and their ref-counts, and the ref. counts of the attached object
 * (if any).
 */
void handle_dump(void)
{
  HandleOps *ops;
  Handle *h;

  OOGLWarn("Active handles:");

  DblListIterateNoDelete(&AllHandles, HandleOps, node, ops) {
    DblListIterateNoDelete(&ops->handles, Handle, opsnode, h) {
      OOGLWarn("  %s@%s (h: #%d, o: #%d )",
	       h->name,
	       ops->prefix,
	       RefCount((Ref *)h),
	       h->object ? RefCount((Ref *)h->object) : -1);
    }
  }
}

/* In principle, it is not necessary to distinguish between permanent
 * and non-permanent handles IF the ref-counts are kept up to date.
 */
void HandleDelete(Handle *h)
{
  if (h == NULL) {
    return;
  }

  if (h->magic != HANDLEMAGIC) {
    OOGLWarn("Internal warning: HandleDelete of non-Handle %x (%x != %x)",
	     h, h->magic, HANDLEMAGIC);
    return;
  }

  if (REFPUT(h) > 0) {
    return;
  }
  
#if HANDLE_DEBUG
  OOGLWarn("Deleting handle with name \"%s\"", h->name);
  if (!DblListEmpty(&h->refs)) {
    OOGLWarn("Deleting handle with active back references.");
    abort();
  }
#endif

  /* remove ourselves from various lists */
  DblListDelete(&h->objnode);
  DblListDelete(&h->opsnode);
  DblListDelete(&h->poolnode);

  if (h->object) {
    /* undo RefIncr() from HandleSetObject() */
    if (h->ops->delete) {
      (*h->ops->delete)(h->object);
    } else {
      REFPUT(h->object);
    }
  }

   /* Don't let Pool code think we have something cached in this handle! */
  if (h->whence && h->whence->seekable) {
    h->whence->flags &= ~PF_ANY;
    PoolClose(h->whence);
  }

  if(h->name) {
    free(h->name);
  }

  DblListAdd(&FreeHandles, &h->opsnode);

  /*  handle_dump();*/
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
