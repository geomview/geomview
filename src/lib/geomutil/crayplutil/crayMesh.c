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
#include "crayolaP.h"

static char msg[] = "crayMesh.c";

void *cray_mesh_HasVColor(int sel, Geom *geom, va_list *args);
void *cray_mesh_UseVColor(int sel, Geom *geom, va_list *args);
void *cray_mesh_EliminateColor(int sel, Geom *geom, va_list *args);
void *cray_mesh_SetColorAll(int sel, Geom *geom, va_list *args);
void *cray_mesh_SetColorAt(int sel, Geom *geom, va_list *args);
void *cray_mesh_SetColorAtF(int sel, Geom *geom, va_list *args);
void *cray_mesh_SetColorAtV(int sel, Geom *geom, va_list *args);
void *cray_mesh_GetColorAt(int sel, Geom *geom, va_list *args);
void *cray_mesh_GetColorAtV(int sel, Geom *geom, va_list *args);

#define MAX_METHODS 11

static craySpecFunc methods[] = {
  {"crayHasColor", cray_mesh_HasVColor},
  {"crayHasVColor", cray_mesh_HasVColor},
  {"crayCanUseVColor", crayTrue},
  {"crayUseVColor", cray_mesh_UseVColor},
  {"crayEliminateColor", cray_mesh_EliminateColor},
  {"craySetColorAll", cray_mesh_SetColorAll},
  {"craySetColorAt", cray_mesh_SetColorAt},
  {"craySetColorAtF", cray_mesh_SetColorAtF},
  {"craySetColorAtV", cray_mesh_SetColorAtV},
  {"crayGetColorAt", cray_mesh_GetColorAt},
  {"crayGetColorAtV", cray_mesh_GetColorAtV}
  };

int
cray_mesh_init() {
  crayInitSpec(methods, MAX_METHODS, GeomClassLookup("mesh"));
  return 0;
}

void *cray_mesh_HasVColor(int sel, Geom *geom, va_list *args) {
  Mesh *m = (Mesh *)geom;
  return (void *)(long)(m->geomflags & MESH_C);
}

void *cray_mesh_UseVColor(int sel, Geom *geom, va_list *args) {
  int i;
  Mesh *m = (Mesh *)geom;
  ColorA *def;

  if (crayHasVColor(geom, NULL)) return 0;

  def = va_arg(*args, ColorA *);

  m->c = OOGLNewNE(ColorA, m->nu * m->nv, msg);
  for (i = 0; i < m->nu * m->nv; i++) {
    m->c[i].r = def->r;
    m->c[i].g = def->g;
    m->c[i].b = def->b;
    m->c[i].a = def->a;
  }
  m->geomflags |= MESH_C;
  return (void *)geom;
}

void *cray_mesh_EliminateColor(int sel, Geom *geom, va_list *args) {
  Mesh *m = (Mesh *)geom;
  
  if (!crayHasColor(geom, NULL)) return 0;
  
  OOGLFree(m->c);
  m->c = NULL;
  m->geomflags ^= MESH_C;
  return (void *)geom;
}

void *cray_mesh_SetColorAll(int sel, Geom *geom, va_list *args) {
  int i;
  Mesh *m = (Mesh *)geom;
  ColorA *color;
 
  if (!crayHasVColor(geom, NULL)) return NULL;
  color = va_arg(*args, ColorA *);
  for (i = 0; i < m->nu * m->nv; i++) m->c[i] = *color;
  return (void *)geom;
}

void *cray_mesh_SetColorAt(int sel, Geom *geom, va_list *args) {
  ColorA *color;
  int vindex, findex, *eindex;
  color = va_arg(*args, ColorA *);
  vindex = va_arg(*args, int);
  findex = va_arg(*args, int);
  eindex = va_arg(*args, int *);
  if (vindex != -1)
    return (void *)(long)craySetColorAtV(geom, color, vindex, NULL, NULL);
  if (eindex[0] != eindex[1]) {
    craySetColorAtV(geom, color, eindex[0], NULL, NULL);
    return (void *)(long)craySetColorAtV(geom, color, eindex[1], NULL, NULL);
  }
  return (void *)(long)craySetColorAtF(geom, color, findex, NULL);
}

void *cray_mesh_SetColorAtV(int sel, Geom *geom, va_list *args) {
  Mesh *m = (Mesh *)geom;
  ColorA *color;
  int index;

  color = va_arg(*args, ColorA *);
  index = va_arg(*args, int);
  if (!crayHasVColor(geom, NULL) || index == -1) return 0;
  m->c[index].r = color->r;
  m->c[index].g = color->g;
  m->c[index].b = color->b;
  m->c[index].a = color->a;
  return (void *)color;
}

void *cray_mesh_SetColorAtF(int sel, Geom *geom, va_list *args) {
  Mesh *m = (Mesh *)geom;
  ColorA *color;
  int index, u, v;

  color = va_arg(*args, ColorA *);
  index = va_arg(*args, int);
  if (!crayHasColor(geom, NULL) || index == -1) return 0;

  u = index % m->nu;
  v = index / m->nu;

  m->c[MESHINDEX(u, v, m)] = *color;
  m->c[MESHINDEX(u+1, v, m)] = *color;
  m->c[MESHINDEX(u+1, v+1, m)] = *color;
  m->c[MESHINDEX(u, v+1, m)] = *color;
  return (void *)m;
}

void *cray_mesh_GetColorAt(int sel, Geom *geom, va_list *args) {
  ColorA *color;
  int i;
  color = va_arg(*args, ColorA *);
  i = va_arg(*args, int);
  return (void *)(long)crayGetColorAtV(geom, color, i, NULL, NULL);
}
     
void *cray_mesh_GetColorAtV(int sel, Geom *geom, va_list *args) {
 ColorA *color;
 int i;
 Mesh *m = (Mesh *)geom;
 if (!crayHasVColor(geom, NULL)) return 0;
 color = va_arg(*args, ColorA *);
 i = va_arg(*args, int);
 color->r = m->c[i].r;
 color->g = m->c[i].g;
 color->b = m->c[i].b;
 color->a = m->c[i].a;
 return (void *)color;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
 
