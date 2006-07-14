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
#include "instP.h"
#include "pointlistP.h"

void *inst_PointList_get(int sel, Geom *geom, va_list *args);
void *inst_PointList_fillin(int sel, Geom *geom, va_list *args);
void *inst_PointList_set(int sel, Geom *geom, va_list *args);
void *inst_PointList_length(int sel, Geom *geom, va_list *args);

#define MAX_METHODS 4

static SpecFunc methods[] = {
  {"PointList_get", inst_PointList_get},
  {"PointList_fillin", inst_PointList_fillin},
  {"PointList_set", inst_PointList_set},
  {"PointList_length", inst_PointList_length}
};

static char msg[] = "ptlInst.c";

void
ptlInst_init() {
  pointlist_initspec(methods, MAX_METHODS, GeomClassLookup("inst"));
}

void *inst_PointList_get(int sel, Geom *geom, va_list *args) {
  HPoint3 *plist;
  int n_points;
  TransformPtr t;
  int coordsys;

  n_points = (int)(long)GeomCall(GeomMethodSel("PointList_length"), geom);
  plist = OOGLNewNE(HPoint3, n_points, msg);

  t = va_arg(*args, TransformPtr);
  coordsys = va_arg(*args, int);

  return GeomCall(GeomMethodSel("PointList_fillin"), geom, t, 
		  coordsys, plist);

}

void *inst_PointList_fillin(int sel, Geom *geom, va_list *args) {
  Inst *inst = (Inst *)geom;
  HPoint3 *plist;
  int coordsys;
  GeomIter *it;
  Transform Tnew;
  int i, n_points;
  TransformPtr t;

  t = va_arg(*args, TransformPtr);
  coordsys = va_arg(*args, int);
  plist = va_arg(*args, HPoint3 *);

  n_points = (int)(long)GeomCall(GeomMethodSel("PointList_length"), inst->geom);

  it = GeomIterate(geom, DEEP);
  for (i = 0; NextTransform(it, Tnew); i += n_points) 
    if (coordsys == POINTLIST_SELF) {
      TmConcat(Tnew, t, Tnew);
      GeomCall(GeomMethodSel("PointList_fillin"), inst->geom, &Tnew[0][0],
	       coordsys, &plist[i]);
    } else if (coordsys == POINTLIST_PRIMITIVE) 
      GeomCall(GeomMethodSel("PointList_fillin"), inst->geom, t, coordsys,
	       &plist[i]);
    else {
      OOGLError(1, "Unrecognized coordinate system in inst_PointList_fillin");
      return NULL;
    }

  return plist;

}


void *inst_PointList_set(int sel, Geom *geom, va_list *args) {
  Inst *inst = (Inst *)geom;
  HPoint3 *plist;
  Transform T, TInv;
  int coordsys;
  GeomIter *it;

  coordsys = va_arg(*args, int);
  plist = va_arg(*args, HPoint3 *);

  /* If the point list has more than one copy of the points 
   * of the geom in it, just use the first set */
  it = GeomIterate(geom, DEEP);
  if (NextTransform(it, T) && coordsys == POINTLIST_SELF) {
    Tm3Invert(T, TInv);
    HPt3TransformN( TInv, plist, plist, 
	       (int)(long)GeomCall(GeomMethodSel("PointList_length"), inst->geom) );
  }
  GeomCall(GeomMethodSel("PointList_set"), inst->geom, coordsys, plist);
  return NULL;

}


void *inst_PointList_length(int sel, Geom *geom, va_list *args) {
  Inst *inst = (Inst *)geom;
  int i, n_points;
  Transform T;
  GeomIter *it;

  n_points = 
    (int)(long)GeomCall(GeomMethodSel("PointList_length"), inst->geom);
  it = GeomIterate(geom, DEEP);
  for (i = 0; NextTransform(it, T); i += n_points);
  return (void *)(long)i;

}
