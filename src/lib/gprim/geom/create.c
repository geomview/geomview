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

#include "geomclass.h"

Geom *
GeomCreate(char *type, ...)
{
  va_list a_list;
  Geom *newgeom = (Geom *) NULL;
  GeomClass *Class = (GeomClass *) NULL;

  va_start (a_list, type);
  Class = GeomClassLookup(type);
  /* now use the create method to get a new geom ... */
  if (Class == NULL) {
    GeomError(0/*Unknown Class XXX*/, "GeomCreate: unknown object class %s", type);
    va_end (a_list);
    return NULL;
  }

  if (Class->create)
    newgeom = (Geom *) (*Class->create)(NULL, Class, &a_list);
  /* need error check here */

  va_end (a_list);
  return newgeom;
}

int
GeomSet(Geom *g, ...)
{
  int ok = -1;
  va_list a_list;

  va_start (a_list, g);

  if (g && g->Class && g->Class->create)
    if((*g->Class->create)(g, g->Class, &a_list))
      ok = 1;

  va_end (a_list);
  return ok;
}


int
GeomGet(Geom *g, int attr, void *attrp)
{
  if(g == NULL)
    return -1;

  switch(attr) {
  case CR_APPEAR:
    *(Appearance **)attrp = g->ap;
    break;
#if 0
  case CR_HANDLE:
    *(Handle **)attrp = g->handle;
    break;
#endif
  default:
    if(g->Class->get)
      return (*g->Class->get)(g, attr, attrp);
  }
  return 0;
}

    
Geom *
GeomCCreate(Geom *g, GeomClass *c, ...)
{
  va_list a_list;
  Geom *newgeom = g;
  GeomClass *Class = c;

  va_start (a_list, c);
  if(Class == NULL && newgeom != NULL)
    Class = newgeom->Class;

  if (Class && Class->create)
    newgeom = (Geom *) (*Class->create)(newgeom, Class, &a_list);
  /* need error check here */

  va_end (a_list);
  return newgeom;
}

/*
 * Initialize common data for Geom objects
 */
void
GGeomInit(Geom *g, GeomClass *Class, int magic, Appearance *ap)
{
  RefInit((Ref *)g, magic);
  g->Class = Class;
  g->ap = ap;
  if(ap != NULL) { /* If it's a real Appearance, bump its ref count */
    RefIncr((Ref *)ap);
  }
  g->aphandle = NULL;
  g->geomflags = 0;
  g->pdim = 4;
  g->freelisthead = NULL; /* has to be overridden by derived class */
  g->bsptree = NULL;
  g->ppath = NULL;
  g->ppathlen = 0;
  DblListInit(&g->pernode);
}

/*
 * Handle one exceptional item from a GeomCreate() arg list.
 * We know how to set common Geom fields.
 */
/* copyp: Flag: "copy" parameters passed by reference? */
/* feature: Attribute -- value already va_arg'ed by caller */
int
GeomDecorate(Geom *g, int *copyp, int feature, va_list *ap)
{
  Appearance *nap;
  int val;

  if(feature == 0 || g == NULL)
    return 1;

  switch(feature) {
  case CR_4D:		/* this is a token, value pair so it can be
			   set conditionally */
    val = va_arg(*ap, int);
    g->geomflags &= ~VERT_4D;
    g->geomflags |= val ? VERT_4D : 0;
    break;
  case CR_APPEAR:		/* Assign or remove Appearance.  */
    nap = va_arg(*ap, Appearance *);
    if(nap && *copyp) RefIncr((Ref *)nap);
    if(g->ap) ApDelete(g->ap);
    g->ap = nap;
    break;
  case CR_COPY:
    *copyp = 1;
    break;
  case CR_NOCOPY:
    *copyp = 0;
    break;
  default:
    return 1; /* Unknown attribute */
  }
  return 0;
}

Appearance *
GeomAppearance(Geom *g)
{
  return g ? g->ap : NULL;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
