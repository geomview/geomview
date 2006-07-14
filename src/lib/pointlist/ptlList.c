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
#include "listP.h"
#include "pointlistP.h"


void *list_PointList_get(int sel, Geom *geom, va_list *args);
void *list_PointList_fillin(int sel, Geom *geom, va_list *args);
void *list_PointList_set(int sel, Geom *geom, va_list *args);
void *list_PointList_length(int sel, Geom *geom, va_list *args);

#define MAX_METHODS 4

static SpecFunc methods[] = {
  {"PointList_get", list_PointList_get},
  {"PointList_fillin", list_PointList_fillin},
  {"PointList_set", list_PointList_set},
  {"PointList_length", list_PointList_length}
  };

static char msg[] = "ptlList.c";

void
ptlList_init() {
  pointlist_initspec(methods, MAX_METHODS, GeomClassLookup("list"));
}

void *list_PointList_get(int sel, Geom *geom, va_list *args) {
  HPoint3 *plist;
  TransformPtr t;
  
  plist = OOGLNewNE(HPoint3, 
		    (int)(long)GeomCall(GeomMethodSel("PointList_length"), geom), 
		     msg);
  t = va_arg(*args, TransformPtr);

  return(GeomCall(GeomMethodSel("PointList_fillin"), geom, t, 0, plist));

}

void *list_PointList_fillin(int sel, Geom *geom, va_list *args) {
  TransformPtr t;
  int n_points1;
  HPoint3 *plist;
  List *l = (List *)geom;

  t = va_arg(*args, TransformPtr);
  (void)va_arg(*args, int);
  plist = va_arg(*args, HPoint3 *);

  n_points1 = (int)(long)GeomCall(GeomMethodSel("PointList_length"), l->car);

  GeomCall(GeomMethodSel("PointList_fillin"), l->car, t, 0, plist);
  GeomCall(GeomMethodSel("PointList_fillin"), (Geom *)l->cdr, t, 0, 
	   &plist[n_points1]);
  return plist;
}


void *list_PointList_set(int sel, Geom *geom, va_list *args) {
  HPoint3 *pt1;
  int n_points1, coordsys;
  List *l = (List *)geom;

  coordsys = va_arg(*args, int);
  pt1 = va_arg(*args, HPoint3 *);
  n_points1 = (int)(long)GeomCall(GeomMethodSel("PointList_length"), l->car);

  GeomCall(GeomMethodSel("PointList_set"), l->car, coordsys, pt1);
  GeomCall(GeomMethodSel("PointList_set"), (Geom *)l->cdr, 
	   coordsys, &pt1[n_points1]);

  return geom;
}

void *list_PointList_length(int sel, Geom *geom, va_list *args) {
  List *l = (List *)geom;
  return  (void *)(long)
    ((int)(long)GeomCall(GeomMethodSel("PointList_length"), l->car) + 
     (int)(long)GeomCall(GeomMethodSel("PointList_length"), (Geom *)l->cdr));
}

