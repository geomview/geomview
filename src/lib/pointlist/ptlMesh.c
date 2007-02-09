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
#include "meshP.h"
#include "pointlistP.h"


void *mesh_PointList_get(int sel, Geom *geom, va_list *args);
void *mesh_PointList_fillin(int sel, Geom *geom, va_list *args);
void *mesh_PointList_set(int sel, Geom *geom, va_list *args);
void *mesh_PointList_length(int sel, Geom *geom, va_list *args);

#define MAX_METHODS 4

static SpecFunc methods[] = {
  {"PointList_get", mesh_PointList_get},
  {"PointList_fillin", mesh_PointList_fillin},
  {"PointList_set", mesh_PointList_set},
  {"PointList_length", mesh_PointList_length}
};

static char msg[] = "ptlMesh.c";

void
ptlMesh_init() {
  pointlist_initspec(methods, MAX_METHODS, GeomClassLookup("mesh"));
}

void *mesh_PointList_get(int sel, Geom *geom, va_list *args) {
  HPoint3 *pt;
  Mesh *m = (Mesh *)geom;
  TransformPtr t;
  pt = OOGLNewNE(HPoint3, m->nu * m->nv, msg);
  t = va_arg(*args, TransformPtr);
  return GeomCall(GeomMethodSel("PointList_fillin"), geom, t, 0, pt);
}

void *mesh_PointList_fillin(int sel, Geom *geom, va_list *args) {
  HPoint3 *pt;
  TransformPtr t;
  Mesh *m = (Mesh *)geom;

  t = va_arg(*args, TransformPtr);
  (void)va_arg(*args, int);
  pt = va_arg(*args, HPoint3 *);

  memcpy(pt, m->p, m->nu * m->nv * sizeof(HPoint3));
  HPt3TransformN(t, pt, pt, m->nu * m->nv);

  return pt;
}

void *mesh_PointList_set(int sel, Geom *geom, va_list *args) {
  Mesh *m = (Mesh *)geom;
  HPoint3 *plist;

  /* This will make the mesh no longer a z-mesh (in general, this
   * is desirable although we may regret it later */
  m->geomflags &= ~MESH_Z;

  (void)va_arg(*args, int);
  plist = va_arg(*args, HPoint3 *);
  memcpy(m->p, plist, m->nu * m->nv * sizeof(HPoint3));
  return geom;
}

void *mesh_PointList_length(int sel, Geom *geom, va_list *args) {
  Mesh *m = (Mesh *)geom;
  return((void *)(long)(m->nu * m->nv));
}
