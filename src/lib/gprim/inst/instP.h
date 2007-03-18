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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef INSTPDEFS
#define INSTPDEFS

/*
 * Private definitions for Inst's.
 */

#include "inst.h"
#include "bboxP.h"	/* includes geomclass.h */

#define INSTMAGIC	GeomMagic('i', 3)

#define INSTFIELDS						\
  GEOMFIELDS;							\
  Geom		*geom;						\
  Handle	*geomhandle;					\
  Transform 	axis;		/* this ought to go... */	\
  Handle	*axishandle;					\
  TransformN    *NDaxis;	/* maybe not. */		\
  Handle	*NDaxishandle;					\
  Geom		*tlist;						\
  Handle	*tlisthandle;					\
  Geom		*txtlist;					\
  Handle	*txtlisthandle;					\
  int		location;	/* L_LOCAL, etc. */		\
  int		origin;		/* L_NONE, L_LOCAL, etc. */	\
  Point3	originpt	/* Only if origin != L_NONE */

struct Inst {
  INSTFIELDS;
};

extern GeomClass *InstClass;		/* Public read-only; valid only
					 * after first call to InstMethods()
					 */

extern Inst *InstCreate( Inst *exist, GeomClass *Class, va_list *attrs );
extern void  InstDelete( Inst * );
extern Inst *InstCopy( Inst * );
extern Inst *InstDice( Inst *, int nu, int nv );

#endif /*INSTPDEFS*/

/*
 * Local Variables ***
 * c-basic-offset: 2 ***
 * End: ***
 */
