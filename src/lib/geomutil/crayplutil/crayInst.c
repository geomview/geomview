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
#include "instP.h"
#include "crayolaP.h"

void *cray_inst_HasColor(int sel, Geom *geom, va_list *args);
void *cray_inst_HasVColor(int sel, Geom *geom, va_list *args);
void *cray_inst_HasFColor(int sel, Geom *geom, va_list *args);

void *cray_inst_CanUseVColor(int sel, Geom *geom, va_list *args);
void *cray_inst_CanUseFColor(int sel, Geom *geom, va_list *args);

void *cray_inst_UseVColor(int sel, Geom *geom, va_list *args);
void *cray_inst_UseFColor(int sel, Geom *geom, va_list *args);

void *cray_inst_EliminateColor(int sel, Geom *geom, va_list *args);

void *cray_inst_SetColorAll(int sel, Geom *geom, va_list *args);
void *cray_inst_SetColorAt(int sel, Geom *geom, va_list *args);
void *cray_inst_SetColorAtV(int sel, Geom *geom, va_list *args);
void *cray_inst_SetColorAtF(int sel, Geom *geom, va_list *args);

void *cray_inst_GetColorAt(int sel, Geom *geom, va_list *args);
void *cray_inst_GetColorAtV(int sel, Geom *geom, va_list *args);
void *cray_inst_GetColorAtF(int sel, Geom *geom, va_list *args);

#define MAX_METHODS 14

static craySpecFunc methods[] = {
  {"crayHasColor", cray_inst_HasColor},
  {"crayHasVColor", cray_inst_HasVColor},
  {"crayHasFColor", cray_inst_HasFColor},
  
  {"crayCanUseVColor", cray_inst_CanUseVColor},
  {"crayCanUseFColor", cray_inst_CanUseFColor},
  
  {"crayUseVColor", cray_inst_UseVColor},
  {"crayUseFColor", cray_inst_UseFColor},
  
  {"crayEliminateColor", cray_inst_EliminateColor},
  
  {"craySetColorAll", cray_inst_SetColorAll},
  {"craySetColorAt", cray_inst_SetColorAt},
  {"craySetColorAtV", cray_inst_SetColorAtV},
  {"craySetColorAtF", cray_inst_SetColorAtF},
  
  {"crayGetColorAt", cray_inst_GetColorAt},
  {"crayGetColorAtV", cray_inst_GetColorAtV},
  {"crayGetColorAtF", cray_inst_GetColorAtF}
  };

int
cray_inst_init() {
  crayInitSpec(methods, MAX_METHODS, GeomClassLookup("inst"));
  return 0;
}

void *cray_inst_HasColor(int sel, Geom *geom, va_list *args) {
  int *gpath = va_arg(*args, int *);
  return (void *)(long)(crayHasColor(((Inst *)geom)->geom, 
				     gpath == NULL ? NULL : gpath + 1));
}

void *cray_inst_HasVColor(int sel, Geom *geom, va_list *args) {
  int *gpath = va_arg(*args, int *);
  return (void *)(long)(crayHasVColor(((Inst *)geom)->geom, 
				      gpath == NULL ? NULL : gpath + 1));
}

void *cray_inst_HasFColor(int sel, Geom *geom, va_list *args) {
  int *gpath = va_arg(*args, int *);
  return (void *)(long)(crayHasFColor(((Inst *)geom)->geom, 
				      gpath == NULL ? NULL : gpath + 1));
}

void *cray_inst_CanUseVColor(int sel, Geom *geom, va_list *args) {
  int *gpath = va_arg(*args, int *);
  return (void *)(long)(crayCanUseVColor(((Inst *)geom)->geom, 
					 gpath == NULL ? NULL : gpath + 1));
}

void *cray_inst_CanUseFColor(int sel, Geom *geom, va_list *args) {
  int *gpath = va_arg(*args, int *);
  return (void *)(long)(crayCanUseFColor(((Inst *)geom)->geom,
					 gpath == NULL ? NULL : gpath + 1));
}
  

void *cray_inst_UseVColor(int sel, Geom *geom, va_list *args) {
  ColorA *c = va_arg(*args, ColorA *);
  int *gpath = va_arg(*args, int *);
  return (void *)(long)(crayUseVColor(((Inst *)geom)->geom, c, 
				      gpath == NULL ? NULL : gpath + 1));
}

void *cray_inst_UseFColor(int sel, Geom *geom, va_list *args) {
  ColorA *c = va_arg(*args, ColorA *);
  int *gpath = va_arg(*args, int *);
  return (void *)(long)(crayUseFColor(((Inst *)geom)->geom, c, 
				      gpath == NULL ? NULL : gpath + 1));
}

void *cray_inst_EliminateColor(int sel, Geom *geom, va_list *args) {
  int *gpath = va_arg(*args, int *);
  return (void *)(long)(crayEliminateColor(((Inst *)geom)->geom, 
					   gpath == NULL ? NULL : gpath + 1));
}

void *cray_inst_SetColorAll(int sel, Geom *geom, va_list *args) {
  ColorA *c = va_arg(*args, ColorA *);
  int *gpath = va_arg(*args, int *);
  return (void *)(long)(craySetColorAll(((Inst *)geom)->geom, c, 
					gpath == NULL ? NULL : gpath + 1));
}
     
void *cray_inst_SetColorAt(int sel, Geom *geom, va_list *args) {
  ColorA *c = va_arg(*args, ColorA *);
  int vindex = va_arg(*args, int), findex = va_arg(*args, int), 
  *edge = va_arg(*args, int *), *gpath = va_arg(*args, int *);
  HPoint3 *pt = va_arg(*args, HPoint3 *);
  return (void *)(long)(craySetColorAt(((Inst *)geom)->geom, c, vindex, 
				       findex, edge, 
				       gpath == NULL ? NULL : gpath + 1, pt));
}

void *cray_inst_SetColorAtV(int sel, Geom *geom, va_list *args) {
  ColorA *c = va_arg(*args, ColorA *);
  int index = va_arg(*args, int), *gpath = va_arg(*args, int *);
  HPoint3 *pt = va_arg(*args, HPoint3 *);
  return (void *)(long)(craySetColorAtV(((Inst *)geom)->geom, c, index, 
					gpath == NULL ? NULL : gpath + 1,
					pt));

}

void *cray_inst_SetColorAtF(int sel, Geom *geom, va_list *args) {
  ColorA *c = va_arg(*args, ColorA *);
  int index = va_arg(*args, int), *gpath = va_arg(*args, int *);
  return (void *)(long)(craySetColorAtF(((Inst *)geom)->geom, c, index, 
					gpath == NULL ? NULL : gpath + 1));

}

void *cray_inst_GetColorAt(int sel, Geom *geom, va_list *args) {
  Geom *newgeom = ((Inst *)geom)->geom;
  ColorA *c = va_arg(*args, ColorA *);
  int vindex = va_arg(*args, int), findex = va_arg(*args, int), 
  *edge = va_arg(*args, int *), *gpath = va_arg(*args, int *);
  HPoint3 *pt = va_arg(*args, HPoint3 *);
  return (void *)(long)(crayGetColorAt(newgeom, c, vindex, findex, edge, 
				 gpath == NULL ? NULL : gpath + 1, pt));
}

void *cray_inst_GetColorAtV(int sel, Geom *geom, va_list *args) {
  ColorA *c = va_arg(*args, ColorA *);
  int index = va_arg(*args, int), *gpath = va_arg(*args, int *);
  HPoint3 *pt = va_arg(*args, HPoint3 *);
  return (void *)(long)(crayGetColorAtV(((Inst *)geom)->geom, c, index, 
					gpath == NULL ? NULL : gpath + 1, pt));
}

void *cray_inst_GetColorAtF(int sel, Geom *geom, va_list *args) {
  ColorA *c = va_arg(*args, ColorA *);
  int index = va_arg(*args, int), *gpath = va_arg(*args, int *);
  return (void *)(long)(crayGetColorAtF(((Inst *)geom)->geom, c, index, 
					gpath == NULL ? NULL : gpath + 1));
}
