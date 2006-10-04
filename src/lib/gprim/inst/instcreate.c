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
 * Inst creation, editing, retrieval and deletion.
 */

#include "instP.h"
#include "transobj.h"

void
InstDelete( inst )
    Inst *inst;
{
    if( inst ) {
	if(inst->geom) GeomDelete(inst->geom);
	if(inst->geomhandle) HandlePDelete(&inst->geomhandle);
	if(inst->tlist) GeomDelete(inst->tlist);
	if(inst->tlisthandle) HandlePDelete(&inst->tlisthandle);
	if(inst->axishandle) HandlePDelete(&inst->axishandle);
    }
}

Inst *
InstCopy( Inst *inst ) 
{
  Inst *ni;

  ni = OOGLNewE(Inst, "InstCopy: Inst");
  GGeomInit(ni, inst->Class, inst->magic, NULL);
  TmCopy(inst->axis, ni->axis);
  ni->geom = GeomCopy(inst->geom);
  ni->geomhandle = NULL;
  ni->tlist = GeomCopy(inst->tlist);
  ni->tlisthandle = NULL;
  ni->axishandle = NULL;
  ni->instflag = inst->instflag;
  ni->location = inst->location;
  ni->origin = inst->origin;
  ni->originpt = inst->originpt;
  return(ni);

}

Geom *
InstReplace( Inst *inst, Geom *geom )
{
    Geom *old;

    if(inst == NULL)
	return NULL;

    old = inst->geom;
    inst->geom = geom;
    return old;
}

int
InstGet( Inst *inst, int attr, void *attrp )
{
    switch(attr) {
    case CR_GEOM: *(Geom **)attrp = inst->geom; break;
    case CR_GEOMHANDLE: *(Handle **)attrp = inst->geomhandle; break;
    case CR_TLIST: *(Geom **)attrp = inst->tlist; break;
    case CR_TLISTHANDLE: *(Geom **)attrp = (Geom *)inst->tlisthandle; break;
    case CR_AXISHANDLE: *(Handle **)attrp = inst->axishandle; break;
    case CR_AXIS:
	TmCopy(inst->axis, (float (*)[4])attrp);
	return (inst->tlist == NULL && inst->tlisthandle == NULL) ? 1 : 0;
    case CR_LOCATION: *(int *)attrp = inst->location; break;
    default:
	return -1;
    }
    return 1;
}

Inst *
InstCreate ( Inst *exist, GeomClass *classp, va_list *a_list )
{
    Inst *inst;
    int attr;
    int copy = 1;
    Transform *t;
    Geom *g;
    Handle *h;

    if (exist == NULL) {
	inst = OOGLNewE(Inst, "InstCreate inst");
	GGeomInit (inst, classp, INSTMAGIC, NULL);
	TmIdentity(inst->axis);
	inst->instflag = 0;
	inst->geomhandle = NULL;
	inst->geom = NULL;
	inst->tlisthandle = NULL;
	inst->tlist = NULL;
	inst->axishandle = NULL;
	inst->location = L_NONE;
	inst->origin = L_NONE;
    } else {
	/* Check that exist is an inst. */
	inst = exist;
    }

    while ((attr = va_arg (*a_list, int))) {
	switch(attr) {
	case CR_FLAG:
	    inst->instflag = va_arg(*a_list, int);
	    break;

	case CR_GEOMHANDLE:
	    h = va_arg(*a_list, Handle *);
	    if(copy) RefIncr((Ref *)h);
	    if(inst->geomhandle)
		HandlePDelete(&inst->geomhandle);
	    inst->geomhandle = h;
	    HandleRegister(&inst->geomhandle, (Ref *)inst, &inst->geom, HandleUpdRef);
	    break;

	case CR_HANDLE_GEOM:
	    h = va_arg(*a_list, Handle *);
	    if(copy) RefIncr((Ref*)h);
	    if(inst->geomhandle)
		HandlePDelete(&inst->geomhandle);
	    inst->geomhandle = h;
	    if(h) HandleRegister(&inst->geomhandle,
			(Ref *)inst, &inst->geom, HandleUpdRef);
	    /* Fall into CR_GEOM case */

	case CR_GEOM:
	    g = va_arg(*a_list, Geom *);
	    if(copy) RefIncr((Ref *)g);
	    if(inst->geom)
		GeomDelete(inst->geom);
	    inst->geom = g;
	    break;

	case CR_AXIS:
	    t = va_arg(*a_list, Transform *);
	    InstTransformTo(inst, (*t), NULL);
	    break;

	case CR_AXISHANDLE:
	    h = va_arg(*a_list, Handle *);
	    if(copy) RefIncr((Ref *)h);
	    if(inst->axishandle)
		HandlePDelete(&inst->axishandle);
	    inst->axishandle = h;
	    HandleRegister(&inst->axishandle, (Ref *)inst, inst->axis, TransUpdate);
	    break;

	case CR_TLIST:
	    g = va_arg (*a_list, Geom *);
	    if(copy) RefIncr((Ref *)g);
	    if(inst->tlist)
		GeomDelete(inst->tlist);
	    inst->tlist = g;
	    break;

	case CR_TLISTHANDLE:
	    h = va_arg(*a_list, Handle *);
	    if(copy) RefIncr((Ref *)h);
	    if(inst->tlisthandle != NULL)
		HandlePDelete(&inst->tlisthandle);
	    inst->tlisthandle = h;
	    HandleRegister(&inst->tlisthandle, (Ref *)inst, &inst->tlist, HandleUpdRef);
	    break;

	case CR_LOCATION:
	    inst->location = va_arg(*a_list, int);
	    break;

	default:
	    if(GeomDecorate(inst, &copy, attr, a_list)) {
		OOGLError (0, "InstCreate: Undefined option: %d", attr);
		if(exist == NULL) GeomDelete ((Geom *)inst);
		return NULL;
	    }
	}
    }

    return inst;
}
