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
#include <stdlib.h> /* for abs() */

#include "geom.h"
#include "vectP.h"
#include "crayolaP.h"

static char msg[] = "crayVect.c";

void *cray_vect_HasColor(int sel, Geom *geom, va_list *args);
void *cray_vect_HasVColor(int sel, Geom *geom, va_list *args);
void *cray_vect_HasFColor(int sel, Geom *geom, va_list *args);

void *cray_vect_UseVColor(int sel, Geom *geom, va_list *args);
void *cray_vect_UseFColor(int sel, Geom *geom, va_list *args);

void *cray_vect_EliminateColor(int sel, Geom *geom, va_list *args);

void *cray_vect_SetColorAt(int sel, Geom *geom, va_list *args);
void *cray_vect_SetColorAtV(int sel, Geom *geom, va_list *args);

void *cray_vect_GetColorAt(int sel, Geom *geom, va_list *args);
void *cray_vect_GetColorAtV(int sel, Geom *geom, va_list *args);

#define MAX_METHODS 12

static craySpecFunc methods[] = {
  {"crayHasColor", cray_vect_HasColor},
  {"crayHasVColor", cray_vect_HasVColor},
  {"crayHasFColor", cray_vect_HasFColor},
  
  {"crayCanUseVColor", crayTrue},
  {"crayCanUseFColor", crayTrue},
  
  {"crayUseVColor", cray_vect_UseVColor},
  {"crayUseFColor", cray_vect_UseFColor},
  
  {"crayEliminateColor", cray_vect_EliminateColor},
  
  {"craySetColorAt", cray_vect_SetColorAt},
  {"craySetColorAtV", cray_vect_SetColorAtV},
  
  {"crayGetColorAt", cray_vect_GetColorAt},
  {"crayGetColorAtV", cray_vect_GetColorAtV}
  };

int
cray_vect_init() {
  crayInitSpec(methods, MAX_METHODS, GeomClassLookup("vect"));
  return 0;
}

void *cray_vect_HasColor(int sel, Geom *geom, va_list *args) {
  Vect *v = (Vect *)geom;
  return (void *)(long)(v->ncolor);
}

void *cray_vect_HasFColor(int sel, Geom *geom, va_list *args) {
  Vect *v = (Vect *)geom;
  return (void *)(long)((v->ncolor == v->nvec) ? 1 : 0);
}

void *cray_vect_HasVColor(int sel, Geom *geom, va_list *args) {
  Vect *v = (Vect *)geom;
  return (void *)(long)((v->ncolor == v->nvert) ? 1 : 0);
}

void *cray_vect_UseVColor(int sel, Geom *geom, va_list *args) {
  Vect *v = (Vect *)geom;
  int h, i, j, k;
  ColorA *color, *def;

  def = va_arg(*args, ColorA *);

  /* h = current point
   * i = current polyline
   * j = current point in polyline
   * k = current color index (in cold list of colors)
   */
  color = OOGLNewNE(ColorA, v->nvert, msg);
  for (h = i = k = 0; i < v->nvec; i++) {
    if (v->vncolor[i]) def = &v->c[k];
    for (j = 0; j < abs(v->vnvert[i]); j++) {
      color[h++] = *def;
      if (v->vncolor[i] > 1) def++;
    }
    k += v->vncolor[i];
    v->vncolor[i] = abs(v->vnvert[i]);
  }

  if (v->c != NULL) OOGLFree(v->c);
  v->c = color;
  v->ncolor = v->nvert;
  return (void *)geom;
}

void *cray_vect_UseFColor(int sel, Geom *geom, va_list *args)
{
  Vect *v = (Vect *)geom;
  int i, k;
  ColorA *color, *def;

  def = va_arg(*args, ColorA *);

  color = OOGLNewNE(ColorA, v->nvec, msg);

  /* 
   * i = current polyline
   * j = current vertex of current polyline
   * k = current color
   * h = current vertex of vect
   */
  for (i = k = 0; i < v->nvec; i++) {
    switch(v->vncolor[i]) {
    case 1:
      def = &v->c[k++];
    case 0:
      color[i] = *def;
      break;
    default:
      if (v->vncolor[i] != abs(v->vnvert[i])) {
	OOGLError(1, "Illegal # of colors / # of vertices combination.");
	return (void *)0;
      }
      def = &v->c[k + 1];
      color[i] = *def;
      k += abs(v->vnvert[i]);
      break;
    }
    v->vncolor[i] = 1;
  }

  if (v->c != NULL) OOGLFree(v->c);
  v->c = color;
  v->ncolor = v->nvec;

  return (void *)geom;
}


void *cray_vect_EliminateColor(int sel, Geom *geom, va_list *args)
{
  int i;
  Vect *v = (Vect *)geom;
  if (!crayHasColor(geom, NULL)) return 0;
  if (v->ncolor) OOGLFree(v->c);
  v->c = NULL;
  v->ncolor = 0;
  for (i = 0; i < v->nvec; i++) v->vncolor[i] = 0;
  return (void *)geom;
}

void *cray_vect_SetColorAt(int sel, Geom *geom, va_list *args) {
  ColorA *color;
  int vindex, *eindex;
  HPoint3 *pt;

  color = va_arg(*args, ColorA *);
  vindex = va_arg(*args, int);
  /*findex = */(void)va_arg(*args, int);
  eindex = va_arg(*args, int *);
  pt = va_arg(*args, HPoint3 *);
  if (vindex != -1) craySetColorAtV(geom, color, vindex, NULL, pt);
  else {
    craySetColorAtV(geom, color, eindex[0], NULL, pt);
    craySetColorAtV(geom, color, eindex[1], NULL, pt);
  }
  return (void *)geom;
}

void *cray_vect_SetColorAtV(int sel, Geom *geom, va_list *args) {
  Vect *v = (Vect *)geom;
  ColorA *color;
  int index;
  int i, j, k;

  color = va_arg(*args, ColorA *);
  index = va_arg(*args, int);
  if (index == -1) return NULL;
  for (i = j = k = 0; i < v->nvec;
       i++, j+= abs(v->vnvert[i]), k += v->vncolor[i])
    if (j + abs(v->vnvert[i]) > index) break;
  switch(v->vncolor[i]) {
  case 0:
    return NULL;
  case 1:
    v->c[k] = *color;
    break;
  default:
    v->c[k + (index - j)] = *color;
    break;
  }

  return (void *)geom;
}

void *cray_vect_GetColorAt(int sel, Geom *geom, va_list *args)
{
  ColorA *color;
  int vindex, *eindex;

  color = va_arg(*args, ColorA *);
  vindex = va_arg(*args, int);
  /*findex = */(void)va_arg(*args, int);
  eindex = va_arg(*args, int *);
  if (vindex != -1) 
    return (void *)(long)crayGetColorAtV(geom, color, vindex, NULL, NULL);
  else return (void *)(long)crayGetColorAtV(geom, color, eindex[0], NULL, NULL);
}

void *cray_vect_GetColorAtV(int sel, Geom *geom, va_list *args) {
  Vect *v = (Vect *)geom;
  ColorA *color;
  int index;
  int i, j, k;

  color = va_arg(*args, ColorA *);
  index = va_arg(*args, int);
  if (index == -1 || !v->ncolor) return NULL;
  for (i = j = k = 0; i < v->nvec;
       i++, j+= abs(v->vnvert[i]), k += v->vncolor[i])
    if (j + abs(v->vnvert[i]) > index) break;
  switch(v->vncolor[i]) {
  case 1:
    k++;
  case 0:
    *color = v->c[k];
    break;
  default:
    *color = v->c[k + (index - j)];
    break;
  }
    
  return (void *)geom;
}
