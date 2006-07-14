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
#include "tlistP.h"


Inst *
InstDice( Inst *inst, int nu, int nv )
{
    GeomDice(inst->geom, nu, nv);
    return inst;
}

Inst *
InstEvert( Inst *inst )
{
    GeomEvert( inst->geom );
    return inst;
}

void
InstHandleScan( Inst *inst, int (*func)(), void *arg )
{
    if(inst == NULL)
	return;

    if(inst->axishandle)
	(*func)(&inst->axishandle, inst, arg);
    if(inst->tlisthandle)
	(*func)(&inst->tlisthandle, inst, arg);
    if(inst->geomhandle)
	(*func)(&inst->geomhandle, inst, arg);

    if(inst->geom)
	GeomHandleScan(inst->geom, func, arg);
    if(inst->tlist)
	GeomHandleScan(inst->tlist, func, arg);
}


Inst *
InstPosition( inst, T )
    Inst *inst;
    Transform T;
{
    GeomIter *it;

    it = GeomIterate((Geom *)inst, DEEP);
    if(NextTransform(it, T) > 0) {
	if(NextTransform(it, T) == 0)
	    return inst;
	DestroyIter(it);
	return NULL;	/* Error -- InstPosition on a multi-element inst */
    }
    return NULL;	/* Bizarre but possible -- no transforms at all */
}

/*
 * Force a single transform equal to T.
 * We discard any tlist/tlisthandle and just assign to axis.
 */
Inst *
InstTransformTo( inst, T )
    Inst *inst;
    Transform T;
{
    if(inst->tlist) {
	GeomDelete(inst->tlist);
	inst->tlist = NULL;
    }
    if(inst->tlisthandle) {
	HandlePDelete(&inst->tlisthandle);
	inst->tlisthandle = NULL;
    }
    TmCopy( T, inst->axis );
    return inst;
}

/*
 * Postmultiply inst by transform T.  Tricky:
 * - If this is a simple inst, just apply transform to axis.
 * - Otherwise, if leading node of tlist is an unshared, single-element TList,
 *	multiply our transform into its matrix.
 * - Failing that, insert a single-element TList as top of hierarchy.
 */
      
Inst *
InstTransform( inst, T )
    Inst *inst;
    Transform T;
{
    register Tlist *tl;

    if(inst->tlist == NULL && inst->tlisthandle == NULL) {
	TmConcat( inst->axis, T, inst->axis );
    } else {
	tl = (Tlist *)inst->tlist;
	if(tl != NULL && tl->Class == TlistClass && tl->nelements == 1
			&& tl->ref_count == 1) {
	    TmConcat( tl->elements[0], T, tl->elements[0] );
	} else {
	    inst->tlist = GeomCCreate(NULL, TlistMethods(),
			CR_NELEM, 1, CR_ELEM, T, 
			CR_HANDLE_GEOM, inst->tlisthandle, tl, NULL);
	    inst->tlisthandle = NULL;
	}
    }
    return inst;
}
