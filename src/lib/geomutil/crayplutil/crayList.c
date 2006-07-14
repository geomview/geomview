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
#include "listP.h"
#include "crayolaP.h"

void *cray_list_HasColor(int sel, Geom *geom, va_list *args);
void *cray_list_HasVColor(int sel, Geom *geom, va_list *args);
void *cray_list_HasFColor(int sel, Geom *geom, va_list *args);

void *cray_list_CanUseVColor(int sel, Geom *geom, va_list *args);
void *cray_list_CanUseFColor(int sel, Geom *geom, va_list *args);

void *cray_list_UseVColor(int sel, Geom *geom, va_list *args);
void *cray_list_UseFColor(int sel, Geom *geom, va_list *args);

void *cray_list_EliminateColor(int sel, Geom *geom, va_list *args);

void *cray_list_SetColorAll(int sel, Geom *geom, va_list *args);
void *cray_list_SetColorAt(int sel, Geom *geom, va_list *args);
void *cray_list_SetColorAtV(int sel, Geom *geom, va_list *args);
void *cray_list_SetColorAtF(int sel, Geom *geom, va_list *args);

void *cray_list_GetColorAt(int sel, Geom *geom, va_list *args);
void *cray_list_GetColorAtV(int sel, Geom *geom, va_list *args);
void *cray_list_GetColorAtF(int sel, Geom *geom, va_list *args);

#define MAX_METHODS 14
static craySpecFunc methods[] = {
  {"crayHasColor", cray_list_HasColor},
  {"crayHasVColor", cray_list_HasVColor},
  {"crayHasFColor", cray_list_HasFColor},
  
  {"crayCanUseVColor", cray_list_CanUseVColor},
  {"crayCanUseFColor", cray_list_CanUseFColor},
  
  {"crayUseVColor", cray_list_UseVColor},
  {"crayUseFColor", cray_list_UseFColor},
  
  {"crayEliminateColor", cray_list_EliminateColor},
  
  {"craySetColorAll", cray_list_SetColorAll},
  {"craySetColorAt", cray_list_SetColorAt},
  {"craySetColorAtV", cray_list_SetColorAtV},
  {"craySetColorAtF", cray_list_SetColorAtF},
  
  {"crayGetColorAt", cray_list_GetColorAt},
  {"crayGetColorAtV", cray_list_GetColorAtV},
  {"crayGetColorAtF", cray_list_GetColorAtF}
  };

int
cray_list_init() {
  crayInitSpec(methods, MAX_METHODS, GeomClassLookup("list"));
  return 0;
}

static Geom *ListElement(Geom *list, int elem) {
  int i;
  List *l = (List *)list;
  
  for (i = 0; i < elem && l != NULL; i++) l = l->cdr;

  if (l == NULL) {
    OOGLError(1, "Unable to retrieve list element %d\n", elem);
    return NULL;
  }

  return l->car;
}

/* 
 * Performs the named function (with its arguements included) on each 
 * element of the list found in the variable geom, then returns the
 * result of all of them |'d together
 */
#define LIST_EACH(list, func) \
{ \
  long val; \
  for (val = 0; (list = (Geom*)((List *)(list))->cdr);) \
    val |= (long)func; \
  return (void *)val; \
}

void *cray_list_HasColor(int sel, Geom *geom, va_list *args) {
  int *gpath = va_arg(*args, int *);
  if (gpath != NULL) 
    return (void *)(long)(crayHasColor(ListElement(geom, *gpath), gpath + 1));
  LIST_EACH(geom, crayHasColor(((List *)geom)->car, NULL));
}

void *cray_list_HasVColor(int sel, Geom *geom, va_list *args) {
  int *gpath = va_arg(*args, int *);
  if (gpath != NULL) 
    return (void *)(long)(crayHasVColor(ListElement(geom, *gpath), gpath + 1));
  LIST_EACH(geom, crayHasVColor(((List *)geom)->car, NULL));
}

void *cray_list_HasFColor(int sel, Geom *geom, va_list *args) {
  int *gpath = va_arg(*args, int *);
  if (gpath != NULL) 
    return (void *)(long)(crayHasFColor(ListElement(geom, *gpath), gpath + 1));
  LIST_EACH(geom, crayHasFColor(((List *)geom)->car, NULL));
}

void *cray_list_CanUseVColor(int sel, Geom *geom, va_list *args) {
  int *gpath = va_arg(*args, int *);
  if (gpath != NULL) 
    return (void *)(long)(crayCanUseVColor(ListElement(geom, *gpath), gpath + 1));
  LIST_EACH(geom, crayCanUseVColor(((List *)geom)->car, NULL));
}

void *cray_list_CanUseFColor(int sel, Geom *geom, va_list *args) {
  int *gpath = va_arg(*args, int *);
  if (gpath != NULL) 
    return (void *)(long)(crayCanUseFColor(ListElement(geom, *gpath), gpath + 1));
  LIST_EACH(geom, crayCanUseFColor(((List *)geom)->car, NULL));
}

void *cray_list_UseVColor(int sel, Geom *geom, va_list *args) {
  ColorA *c = va_arg(*args, ColorA *);
  int *gpath = va_arg(*args, int *);
  if (gpath != NULL) 
    return (void *)(long)(crayUseVColor(ListElement(geom, *gpath), c, gpath + 1));
  LIST_EACH(geom, crayUseVColor(((List *)geom)->car, c, NULL));
}

void *cray_list_UseFColor(int sel, Geom *geom, va_list *args) {
  ColorA *c = va_arg(*args, ColorA *);
  int *gpath = va_arg(*args, int *);
  if (gpath != NULL) 
    return (void *)(long)(crayUseFColor(ListElement(geom, *gpath), c, gpath + 1));
  LIST_EACH(geom, crayUseFColor(((List *)geom)->car, c, NULL));
}

void *cray_list_EliminateColor(int sel, Geom *geom, va_list *args) {
  int *gpath = va_arg(*args, int *);
  if (gpath != NULL) 
    return (void *)(long)(crayEliminateColor(ListElement(geom, *gpath), gpath + 1));
  LIST_EACH(geom, crayEliminateColor(((List *)geom)->car, NULL));
}

void *cray_list_SetColorAll(int sel, Geom *geom, va_list *args) {
  ColorA *c = va_arg(*args, ColorA *);
  int *gpath = va_arg(*args, int *);

  if (gpath != NULL) 
    return (void *)(long)craySetColorAll(ListElement(geom, *gpath), c, gpath + 1);
  LIST_EACH(geom, craySetColorAll(((List *)geom)->car, c, NULL));
}

void *cray_list_SetColorAt(int sel, Geom *geom, va_list *args) {
  ColorA *c = va_arg(*args, ColorA *);
  int vindex = va_arg(*args, int), findex = va_arg(*args, int),
  *edge = va_arg(*args, int *), *gpath = va_arg(*args, int *);
  HPoint3 *pt = va_arg(*args, HPoint3 *);
  if (gpath != NULL) 
    return (void *)(long)(craySetColorAt(ListElement(geom, *gpath), c, vindex,
				   findex, edge, gpath + 1, pt));
  LIST_EACH(geom, craySetColorAt(((List *)geom)->car, c, vindex, findex,
				 edge, NULL, pt));
}

void *cray_list_SetColorAtV(int sel, Geom *geom, va_list *args) {
  ColorA *c = va_arg(*args, ColorA *);
  int index = va_arg(*args, int), *gpath = va_arg(*args, int *);
  HPoint3 *pt = va_arg(*args, HPoint3 *);
  if (gpath != NULL) 
    return (void *)(long)(craySetColorAtV(ListElement(geom, *gpath), c, index, 
				    gpath + 1, pt));
  LIST_EACH(geom, craySetColorAtV(((List *)geom)->car, c, index, NULL, pt));
}

void *cray_list_SetColorAtF(int sel, Geom *geom, va_list *args) {
  ColorA *c = va_arg(*args, ColorA *);
  int index = va_arg(*args, int), *gpath = va_arg(*args, int *);
  if (gpath != NULL) 
    return (void *)(long)(craySetColorAtF(ListElement(geom, *gpath), c, index, 
				    gpath + 1));
  LIST_EACH(geom, craySetColorAtF(((List *)geom)->car, c, index, NULL));
}

void *cray_list_GetColorAt(int sel, Geom *geom, va_list *args) {
  ColorA *c = va_arg(*args, ColorA *);
  int vindex = va_arg(*args, int), findex = va_arg(*args, int),
  *edge = va_arg(*args, int *), *gpath = va_arg(*args, int *);
  HPoint3 *pt = va_arg(*args, HPoint3 *);
  if (gpath != NULL) 
    return (void *)(long)(crayGetColorAt(ListElement(geom, *gpath), c, vindex, 
				   findex, edge, gpath + 1, pt));
  LIST_EACH(geom, crayGetColorAt(((List *)geom)->car, c, vindex, 
				 findex, edge, NULL, pt));
}

void *cray_list_GetColorAtV(int sel, Geom *geom, va_list *args) {
  ColorA *c = va_arg(*args, ColorA *);
  int index = va_arg(*args, int), *gpath = va_arg(*args, int *);
  HPoint3 *pt = va_arg(*args, HPoint3 *);
  if (gpath != NULL)
      return (void *)(long)(crayGetColorAtV(ListElement(geom, *gpath), c, index, 
				      gpath + 1, pt));
  LIST_EACH(geom, crayGetColorAtV(((List *)geom)->car, c, index, NULL, pt));
}

void *cray_list_GetColorAtF(int sel, Geom *geom, va_list *args) {
  ColorA *c = va_arg(*args, ColorA *);
  int index = va_arg(*args, int), *gpath = va_arg(*args, int *);
  if (gpath != NULL) 
    return (void *)(long)(crayGetColorAtF(ListElement(geom, *gpath), c, index, 
				    gpath + 1));
  LIST_EACH(geom, crayGetColorAtF(((List *)geom)->car, c, index, NULL));
}
