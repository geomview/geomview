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

/*
 * iterate.c -- iteration over OOGL hierarchies
 */

#include <geomclass.h>
#include "listP.h"
#include "tlistP.h"
#include "instP.h"
#include "sphereP.h"
#include "discgrpP.h"

#define	UNKNOWN	0
#define	LIST	1
#define	TLIST	2
#define	INST	3
#define SPHERE	4
#define DISCGRP 5
#define OTHER	6

#define	VALID	0x13ac2480	/* Magic number */

struct istate {			/* Iteration state structure: per nest lvl */
	struct istate *parent;
	Geom	*g;
	int	kind;
	int	seq;		/* Seq number within Tlist */
	Transform Ti;
};

struct GeomIter {
	struct istate *stack;
	int flags;
};

static union itpool { union itpool *next; struct GeomIter iter; } *itpool;
static struct istate *ispool;

#define	NewIter(it)							\
    if (itpool) {							\
	it = &itpool->iter;						\
	itpool = itpool->next;						\
    } else {								\
	it = OOGLNewE(GeomIter, "GeomIter");				\
    }
#define	NewIstate(is)							\
    if (ispool) {							\
	is = ispool;							\
	ispool = ispool->parent;					\
    } else {								\
	is = OOGLNewE(struct istate, "GeomIter state");			\
    }

#define	FreeIter(it)							\
    ((union itpool *)(it))->next = itpool, itpool = (union itpool *)(it)
#define	FreeIstate(is) (is)->parent = ispool, ispool = (is)

GeomIter *_GeomIterate(Geom *g, int flags)
{
    GeomIter *it;
    struct istate *is;

    NewIter(it);
    it->flags = (flags & 0xf) | VALID;
    NewIstate(is);
    it->stack = is;
    is->kind = UNKNOWN;
    is->g = g;
    is->parent = NULL;
    return it;
}

/*
 * Iterate over an OOGL hierarchy, producing one Transform per call.
 * Returns 1 normally, 0 when no more Transforms remain.
 * Destroys the GeomIter upon returning 0.
 *
 * To make this work as a coroutine we effectively unroll the recursive
 * hierarchy traversal, so please excuse the goto's.
 *
 * For speed, we act as a friend to the List, TList, Inst, and Sphere
 * classes.   XXX
 *
 * This function has to be called through the inline function
 * NextTransform(); we assume here that iter != NULL and it->stack !=
 * NULL.
 */
int _NextTransform(GeomIter *it, Transform T)
{
    struct istate *is;
    Geom *g;

    if ((is = it->stack) == NULL) {
	goto gone;
    }

    g = is->g;

  again:
    switch(is->kind) {
    case UNKNOWN:				/* Determine type */
  discover:
	if(g->Class == TlistClass) {
	    if(g->magic != TLISTMAGIC) {
		GeomError(0,"NextTransform: Surprise Tlist %x", g);
		goto pop;
	    }
	    is->kind = TLIST;
	    is->seq = 0;
	    goto isTLIST;
	}
	if(g->Class == DiscGrpClass) {
	    if(g->magic != DISCGRPMAGIC) {
		GeomError(0,"NextTransform: Surprise DiscGrp %x", g);
		goto pop;
	    }
	    is->kind = DISCGRP;
	    is->seq = 0;
	    goto isDISCGRP;
	}
	if(g->Class == ListClass) {
	    if(g->magic != LISTMAGIC) {
		GeomError(0,"NextTransform: Surprise List %x", g);
		goto pop;
	    }
	    is->kind = LIST;
	    goto isLIST;
	}
	if(g->Class == InstClass || g->Class == SphereClass) {
	    Inst *inst = (Inst *)g;

	    if(inst->magic != INSTMAGIC && inst->magic != SPHEREMAGIC) {
		GeomError(0,"NextTransform: Surprise Inst %x", g);
		goto pop;
	    }
	    if(inst->tlist == NULL) {
		/*
		 * Just use this transform.  We're a leaf, so return it now.
		 * Also pop stack for next iteration.
		 */
		if((is = is->parent))
		    TmConcat(inst->axis, is->Ti, T);
		else
		    TmCopy(inst->axis, T);
		FreeIstate(it->stack);
		it->stack = is;
		return 1;
	    } else {
		/*
		 * Inst has a child.
		 * The axis field is *ignored* in this case; just
		 * traverse the child.
		 */
		is->g = g = inst->tlist;
		/*is->kind = UNKNOWN;*/
		goto discover;
	    }
	}
	/* Other objects aren't relevant for NextTransform(), so discard. */
	goto pop;

    case TLIST:
      isTLIST:
      {
	Tlist *TL = (Tlist *)g;
	TransformPtr Tp;	/* ought to be ``Transform *Tp'' */

	if(is->seq >= TL->nelements)
	    goto pop;

	Tp = TL->elements[is->seq++];

	if(TL->tlist != NULL && !(it->flags & SHALLOW)) {
	    if(is->parent)
		TmConcat(Tp, is->parent->Ti, is->Ti);
	    else
		TmCopy(Tp, is->Ti);
	    g = TL->tlist;
	    goto push;
	}

	/* We're a leaf -- return a matrix now */
	if(is->parent)
	    TmConcat(Tp, is->parent->Ti, T);
	else
	    TmCopy(Tp, T);
	return 1;
      }

    case DISCGRP:
      isDISCGRP:
      {
	DiscGrp *dg = (DiscGrp *)g;
	TransformPtr Tp;	/* ought to be ``Transform *Tp'' */

	if(is->seq >= dg->big_list->num_el)
	    goto pop;

	Tp = dg->big_list->el_list[is->seq++].tform;

	/* if it's another discrete group ... */
	if(dg->geom != NULL &&
	    dg->geom->magic == DISCGRPMAGIC && !(it->flags & SHALLOW)) {
	    if(is->parent)
		TmConcat(Tp, is->parent->Ti, is->Ti);
	    else
		TmCopy(Tp, is->Ti);
	    g = dg->geom;
	    goto push;
	}

	/* We're a leaf -- return a matrix now */
	if(is->parent)
	    TmConcat(Tp, is->parent->Ti, T);
	else
	    TmCopy(Tp, T);
	return 1;
      }

    case LIST:
     isLIST:
      {
	/* Might need some intra-List state too if List format
	 * gets fancier -- if we have one List object which contains a
	 * bundle of Geom's.
	 */
	List *L = (List *)g;

	if(L == NULL)
		goto pop;
	while(L->car == NULL) {
	    L = L->cdr;
	    if(L == NULL)
		goto pop;
	}
	g = L->car;
	is->g = (Geom *)L->cdr;
	if(is->parent)
	    TmCopy(is->parent->Ti, is->Ti);
	else
	    TmIdentity(is->Ti);
	goto push;
      }

    default:
	GeomError(1,"NextTransform: called with invalid GeomIter (%x)", it);
	return 0;
    }

  push:			/* Push g as a new element */
			/* is->Ti *must* be set correctly before getting here! */
    NewIstate(is);
    is->g = g;
    is->parent = it->stack;
    it->stack = is;
    goto discover;	/* Recurse */

  pop:
    is = is->parent;
    FreeIstate(it->stack);
    it->stack = is;
    if(is != NULL) {
	g = is->g;
	goto again;	/* Recurse -- type already determined */
    }
    /* stack empty -- finished */

 gone:
    it->flags = 0;
    it->stack = NULL;
    FreeIter(it);
    return 0;
}

void
DestroyIter(it)
    GeomIter *it;
{
    if((it->flags & 0xfffffff0) == VALID) {
	it->flags = 0;
	it->stack = NULL;
	FreeIter(it);
    } else {
	GeomError(1,"DestroyIter -- already destroyed %x", it);
    }
}

/*
 * Local Variables: ***
 * c-basic-offset: 4 ***
 * End: ***
 */
