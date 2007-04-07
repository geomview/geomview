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

#include "geom.h"
#include "quadP.h"
#include "crayolaP.h"

static char msg[] = "crayQuad.c";

void *cray_quad_HasVColor(int sel, Geom *geom, va_list *args);

void *cray_quad_UseVColor(int sel, Geom *geom, va_list *args);

void *cray_quad_EliminateColor(int sel, Geom *geom, va_list *args);

void *cray_quad_SetColorAll(int sel, Geom *geom, va_list *args);
void *cray_quad_SetColorAt(int sel, Geom *geom, va_list *args);
void *cray_quad_SetColorAtV(int sel, Geom *geom, va_list *args);
void *cray_quad_SetColorAtF(int sel, Geom *geom, va_list *args);

void *cray_quad_GetColorAt(int sel, Geom *geom, va_list *args);
void *cray_quad_GetColorAtV(int sel, Geom *geom, va_list *args);
void *cray_quad_GetColorAtF(int sel, Geom *geom, va_list *args);

#define MAX_METHODS 12

static craySpecFunc methods[] = {
  {"crayHasColor", cray_quad_HasVColor},
  {"crayHasVColor", cray_quad_HasVColor},
  
  {"crayCanUseVColor", crayTrue},
  
  {"crayUseVColor", cray_quad_UseVColor},
  
  {"crayEliminateColor", cray_quad_EliminateColor},
  
  {"craySetColorAll", cray_quad_SetColorAll},
  {"craySetColorAt", cray_quad_SetColorAt},
  {"craySetColorAtV", cray_quad_SetColorAtV},
  {"craySetColorAtF", cray_quad_SetColorAtF},
  
  {"crayGetColorAt", cray_quad_GetColorAt},
  {"crayGetColorAtV", cray_quad_GetColorAtV},
  {"crayGetColorAtF", cray_quad_GetColorAtF}
  };

int
cray_quad_init() {
  crayInitSpec(methods, MAX_METHODS, GeomClassLookup("quad"));
  return 0;
}

void *cray_quad_HasVColor(int sel, Geom *geom, va_list *args) {
  Quad *q = (Quad *)geom;
  return (void *)(long)(q->geomflags & QUAD_C);
}

void *cray_quad_UseVColor(int sel, Geom *geom, va_list *args) {
  Quad *q = (Quad *)geom;
  ColorA *color;
  int i;

  if (crayHasVColor(geom, NULL)) return 0;
  color = va_arg(*args, ColorA *);
  q->c = OOGLNewNE(QuadC, q->maxquad, msg);
  for (i = 0; i < q->maxquad; i++) 
    q->c[i][0] = q->c[i][1] = q->c[i][2] = q->c[i][3] = *color;
  q->geomflags |= QUAD_C;
  return (void *)geom;
}

void *cray_quad_EliminateColor(int sel, Geom *geom, va_list *args) {
  Quad *q = (Quad *)geom;

  if (!crayHasVColor(geom, NULL)) return 0;
  OOGLFree(q->c);
  q->c = NULL;
  q->geomflags &= ~QUAD_C;
  return (void *)geom;
}

void *cray_quad_SetColorAll(int sel, Geom *geom, va_list *args) {
  int i;
  Quad *q = (Quad *)geom;
  ColorA *color = va_arg(*args, ColorA *);
  if (!crayHasColor(geom, NULL)) return NULL;
  for (i = 0; i < q->maxquad; i++) {
    q->c[i][0] = *color;
    q->c[i][1] = *color;
    q->c[i][2] = *color;
    q->c[i][3] = *color;
  }
  return (void *)geom;
}

void *cray_quad_SetColorAt(int sel, Geom *geom, va_list *args) {
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

void *cray_quad_SetColorAtV(int sel, Geom *geom, va_list *args) {
  Quad *q = (Quad *)geom;
  ColorA *color;
  int index;
  color = va_arg(*args, ColorA *);
  index = va_arg(*args, int);
  if (index == -1 || !crayHasVColor(geom, NULL)) return 0;
  q->c[0][index] = *color;
  return (void *)geom;
}


/*
 * Note - this is somewhat strangely written. Quad colors may be set
 * (and gotten) by vertex or by face, although they are always stored 
 * by vertex.
 */
void *cray_quad_SetColorAtF(int sel, Geom *geom, va_list *args) {
  Quad *q = (Quad *)geom;
  ColorA *color;
  int index;
  color = va_arg(*args, ColorA *);
  index = va_arg(*args, int);
  if (index == -1 || !crayHasVColor(geom, NULL)) return 0;
  q->c[index][0] = q->c[index][1] = q->c[index][2] = 
    q->c[index][3] = *color;
  return (void *)geom;
}


void *cray_quad_GetColorAt(int sel, Geom *geom, va_list *args) {
  ColorA *color;
  int vindex, findex;

  color = va_arg(*args, ColorA *);
  vindex = va_arg(*args, int);
  findex = va_arg(*args, int);
  if (!crayHasVColor(geom, NULL)) return 0;
  if (vindex != -1) 
    return (void *)(long)crayGetColorAtV(geom, color, vindex, NULL, NULL);
  return (void *)(long)crayGetColorAtF(geom, color, findex, NULL);
}

void *cray_quad_GetColorAtV(int sel, Geom *geom, va_list *args) {
  Quad *q = (Quad *)geom;
  ColorA *color;
  int index;

  color = va_arg(*args, ColorA *);
  index = va_arg(*args, int);
  if (!crayHasVColor(geom, NULL) || index == -1) return 0;
  *color = q->c[0][index];
  return (void *)geom;
}

void *cray_quad_GetColorAtF(int sel, Geom *geom, va_list *args) {
  Quad *q = (Quad *)geom;
  ColorA *color;
  int index;

  color = va_arg(*args, ColorA *);
  index = va_arg(*args, int);
  if (!crayHasVColor(geom, NULL) || index == -1) return 0;
  *color = q->c[index][0];
  return (void *)geom;
}



/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
 
