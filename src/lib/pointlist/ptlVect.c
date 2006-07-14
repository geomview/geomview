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

#include "ooglutil.h"
#include "geom.h"
#include "vectP.h"
#include "pointlistP.h"

void *vect_PointList_get(int sel, Geom *geom, va_list *args);
void *vect_PointList_fillin(int sel, Geom *geom, va_list *args);
void *vect_PointList_set(int sel, Geom *geom, va_list *args);
void *vect_PointList_length(int sel, Geom *geom, va_list *args);

#define MAX_METHODS 4

static SpecFunc methods[] = {
  {"PointList_get", vect_PointList_get},
  {"PointList_fillin", vect_PointList_fillin},
  {"PointList_set", vect_PointList_set},
  {"PointList_length", vect_PointList_length}
};

static char msg[] = "ptlVect.c";

void
ptlVect_init() {
  pointlist_initspec(methods, MAX_METHODS, GeomClassLookup("vect"));
}

void *vect_PointList_get(int sel, Geom *geom, va_list *args) {
  Vect *v = (Vect *)geom;
  HPoint3 *plist;
  TransformPtr t;

  plist = OOGLNewNE(HPoint3, v->nvert, msg);
  t = va_arg(*args, TransformPtr);
  return GeomCall(GeomMethodSel("PointList_fillin"), geom, t, 0, plist);
}

void *vect_PointList_fillin(int sel, Geom *geom, va_list *args) {
  Vect *v = (Vect *)geom;
  TransformPtr t;
  HPoint3 *plist;

  t = va_arg(*args, TransformPtr);
  (void)va_arg(*args, int);
  plist = va_arg(*args, HPoint3 *);
  memcpy(plist, v->p, v->nvert * sizeof(HPoint3));
  HPt3TransformN(t, plist, plist, v->nvert);
  return((void *)plist);
}

void *vect_PointList_set(int sel, Geom *geom, va_list *args) {
  Vect *v = (Vect *)geom;
  HPoint3 *plist;
  
  (void)va_arg(*args, int);
  plist = va_arg(*args, HPoint3 *);
  memcpy(v->p, plist, v->nvert * sizeof(HPoint3));
  return((void *)v);
}


void *vect_PointList_length(int sel, Geom *geom, va_list *args) {
  Vect *v = (Vect *)geom;
  
  return((void *)(long)v->nvert);
}


