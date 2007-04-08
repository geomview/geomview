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

#include <stdio.h>
#include "geom.h"
#include "polylistP.h"
#include "crayolaP.h"

void *cray_polylist_HasColor(int sel, Geom *geom, va_list *args);
void *cray_polylist_HasVColor(int sel, Geom *geom, va_list *args);
void *cray_polylist_HasFColor(int sel, Geom *geom, va_list *args);

void *cray_polylist_UseVColor(int sel, Geom *geom, va_list *args);
void *cray_polylist_UseFColor(int sel, Geom *geom, va_list *args);

void *cray_polylist_EliminateColor(int sel, Geom *geom, va_list *args);

void *cray_polylist_SetColorAll(int sel, Geom *geom, va_list *args);
void *cray_polylist_SetColorAt(int sel, Geom *geom, va_list *args);
void *cray_polylist_SetColorAtV(int sel, Geom *geom, va_list *args);
void *cray_polylist_SetColorAtF(int sel, Geom *geom, va_list *args);

void *cray_polylist_GetColorAt(int sel, Geom *geom, va_list *args);
void *cray_polylist_GetColorAtV(int sel, Geom *geom, va_list *args);
void *cray_polylist_GetColorAtF(int sel, Geom *geom, va_list *args);

#define MAX_METHODS 15

static craySpecFunc methods[] = {
  {"crayHasColor", cray_polylist_HasColor},
  {"crayHasVColor", cray_polylist_HasVColor},
  {"crayHasFColor", cray_polylist_HasFColor},

  {"crayCanUseVColor", crayTrue},
  {"crayCanUseFColor", crayTrue},

  {"crayUseVColor", cray_polylist_UseVColor},
  {"crayUseFColor", cray_polylist_UseFColor},

  {"crayEliminateColor", cray_polylist_EliminateColor},

  {"craySetColorAll", cray_polylist_SetColorAll},
  {"craySetColorAt", cray_polylist_SetColorAt},
  {"craySetColorAtV", cray_polylist_SetColorAtV},
  {"craySetColorAtF", cray_polylist_SetColorAtF},

  {"crayGetColorAt", cray_polylist_GetColorAt},
  {"crayGetColorAtV", cray_polylist_GetColorAtV},
  {"crayGetColorAtF", cray_polylist_GetColorAtF}
  };

int
cray_polylist_init() {
  crayInitSpec(methods, MAX_METHODS, GeomClassLookup("polylist"));
  return 0;
}

void *cray_polylist_HasColor(int sel, Geom *geom, va_list *args) {
  return (void *)(long)(crayHasVColor(geom, NULL) || crayHasFColor(geom, NULL));
}

void *cray_polylist_HasVColor(int sel, Geom *geom, va_list *args) {
  PolyList *p = (PolyList *)geom;
  return (void *)(long)(p->geomflags & PL_HASVCOL);
}

void *cray_polylist_HasFColor(int sel, Geom *geom, va_list *args) {
  PolyList *p = (PolyList *)geom;
  return (void *)(long)(p->geomflags & PL_HASPCOL);
}

void *cray_polylist_UseVColor(int sel, Geom *geom, va_list *args) {
  PolyList *p = (PolyList *)geom;
  ColorA *def;
  int i, j;

  def = va_arg(*args, ColorA *);
  for (i = 0; i < p->n_verts; i++) p->vl[i].vcol = *def;

  if (p->geomflags & PL_HASPCOL) {
    for (i = 0; i < p->n_polys; i++) 
      for (j = 0; j < p->p[i].n_vertices; j++) 
	p->p[i].v[j]->vcol = p->p[i].pcol;
    p->geomflags ^= PL_HASPCOL;
  }

  p->geomflags |= PL_HASVCOL;
  return (void *)p;
}

void *cray_polylist_UseFColor(int sel, Geom *geom, va_list *args) {
  PolyList *p = (PolyList *)geom;
  ColorA *def;
  int i;

  def = va_arg(*args, ColorA *);
  for (i = 0; i < p->n_polys; i++) p->p[i].pcol = *def;
  if (p->geomflags & PL_HASVCOL) {
    for (i = 0; i < p->n_polys; i++) 
      if (p->p[i].n_vertices) p->p[i].pcol = p->p[i].v[0]->vcol;
    p->geomflags ^= PL_HASVCOL;
  }

  p->geomflags |= PL_HASPCOL;
  return (void *)p;
}

void *cray_polylist_EliminateColor(int sel, Geom *geom, va_list *args) {
  PolyList *p = (PolyList *)geom;
  if (!crayHasColor(geom, NULL)) return 0;
  p->geomflags &= ~PL_HASVCOL;
  p->geomflags &= ~PL_HASPCOL;
  return (void *)p;
}

void *cray_polylist_SetColorAll(int sel, Geom *geom, va_list *args) {
  int i;
  PolyList *p = (PolyList *)geom;
  ColorA *color = va_arg(*args, ColorA *);

  if (!crayHasVColor(geom, NULL)) return NULL;

  for (i = 0; i < p->n_verts; i++) p->vl[i].vcol = *color;
  for (i = 0; i < p->n_polys; i++) p->p[i].pcol = *color;
  return geom;
}

void *cray_polylist_SetColorAt(int sel, Geom *geom, va_list *args) {
  ColorA *color;
  int vindex, findex, *eindex;
  color = va_arg(*args, ColorA *);
  vindex = va_arg(*args, int);
  findex = va_arg(*args, int);
  eindex = va_arg(*args, int *);
  if (crayHasVColor(geom, NULL)) {
    if (vindex != -1)
      return (void *)(long)craySetColorAtV(geom, color, vindex, NULL, NULL);
    if (eindex[0] != eindex[1]) {
      craySetColorAtV(geom, color, eindex[0], NULL, NULL);
      return  (void *)(long)craySetColorAtV(geom, color, eindex[1], NULL, NULL);
    }
  }
  return (void *)(long)craySetColorAtF(geom, color, findex, NULL);
}

void *cray_polylist_SetColorAtV(int sel, Geom *geom, va_list *args) {
  PolyList *p = (PolyList *)geom;
  ColorA *color;
  int index;

  color = va_arg(*args, ColorA *);
  index = va_arg(*args, int);
  if (!crayHasVColor(geom, NULL) || index == -1) return (void *)0;
  p->vl[index].vcol = *color;
  return (void *)geom;
}

void *cray_polylist_SetColorAtF(int sel, Geom *geom, va_list *args) {
  int i;
  PolyList *p = (PolyList *)geom;
  ColorA *color;
  int index;

  color = va_arg(*args, ColorA *);
  index = va_arg(*args, int);
  if (index == -1) return (void *)0;
  if (crayHasFColor(geom, NULL)) p->p[index].pcol = *color;
  else if (crayHasVColor(geom, NULL)) 
    for (i = 0; i < p->p[index].n_vertices; i++)
      p->p[index].v[i]->vcol = *color;
  return (void *)geom;
}

void *cray_polylist_GetColorAt(int sel, Geom *geom, va_list *args) {
  ColorA *color;
  int vindex, findex;

  color = va_arg(*args, ColorA *);
  vindex = va_arg(*args, int);
  findex = va_arg(*args, int);
  if (crayHasVColor(geom, NULL))
    return (void *)(long)crayGetColorAtV(geom, color, vindex, NULL, NULL);
  if (crayHasFColor(geom, NULL))
    return (void *)(long)crayGetColorAtF(geom, color, findex, NULL);
  return NULL;
}

void *cray_polylist_GetColorAtV(int sel, Geom *geom, va_list *args) {
  PolyList *p = (PolyList *)geom;
  ColorA *color;
  int index;
  
  color = va_arg(*args, ColorA *);
  index = va_arg(*args, int);
  if (!crayHasVColor(geom, NULL) || index == -1) return (void *)0;
  *color = p->vl[index].vcol;
  return (void *)geom;
}

void *cray_polylist_GetColorAtF(int sel, Geom *geom, va_list *args) {
  PolyList *p = (PolyList *)geom;
  ColorA *color;
  int index;
  
  color = va_arg(*args, ColorA *);
  index = va_arg(*args, int);
  if (!crayHasFColor(geom, NULL) || index == -1) return (void *)0;
  *color = p->p[index].pcol;
  return (void *)geom;
}
     
/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
 
