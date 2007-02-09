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
#include "point3.h"
#include "hpoint3.h"
#include "geom.h"
#include "bezierP.h"
#include "crayolaP.h"

#ifndef FUDGE
#define FUDGE .1
#endif

void *cray_bezier_HasVColor(int sel, Geom *geom, va_list *args);
void *cray_bezier_UseVColor(int sel, Geom *geom, va_list *args);
void *cray_bezier_EliminateColor(int sel, Geom *geom, va_list *args);
void *cray_bezier_SetColorAll(int sel, Geom *geom, va_list *args);
void *cray_bezier_SetColorAt(int sel, Geom *geom, va_list *args);
void *cray_bezier_SetColorAtF(int sel, Geom *geom, va_list *args);
void *cray_bezier_GetColorAt(int sel, Geom *geom, va_list *args);

#define MAX_METHODS 10

static craySpecFunc methods[] = {
  {"crayHasColor", cray_bezier_HasVColor},
  {"crayHasVColor", cray_bezier_HasVColor},
  {"crayCanUseVColor", crayTrue},
  {"crayUseVColor", cray_bezier_UseVColor},
  {"crayEliminateColor", cray_bezier_EliminateColor},
  {"craySetColorAll", cray_bezier_SetColorAll},
  {"craySetColorAt", cray_bezier_SetColorAt},
  {"craySetColorAtF", cray_bezier_SetColorAtF},
  {"crayGetColorAt", cray_bezier_GetColorAt},
  {"crayGetColorAtV", cray_bezier_GetColorAt}
  };

int
cray_bezier_init() {
  crayInitSpec(methods, MAX_METHODS, GeomClassLookup("bezier"));
  return 0;
}

void *cray_bezier_HasVColor(int sel, Geom *geom, va_list *args) {
  Bezier *b = (Bezier *)geom;
  return (void *)(long)(b->geomflags & BEZ_C);
}

void *cray_bezier_UseVColor(int sel, Geom *geom, va_list *args) {
  int i;
  Bezier *b = (Bezier *)geom;
  ColorA *def;

  if (crayHasColor(geom, NULL)) return 0;

  def = va_arg(*args, ColorA *);
  for (i = 0; i < 4; i++) b->c[i] = *def; 

  b->geomflags |= BEZ_C;

  return (void *)geom;
}

void *cray_bezier_EliminateColor(int sel, Geom *geom, va_list *args) {
  Bezier *b = (Bezier *)geom;

  if (!crayHasColor(geom, NULL)) return NULL;
  b->geomflags ^= BEZ_C;
  return (void *)geom;
}

void *cray_bezier_SetColorAll(int sel, Geom *geom, va_list *args) {
  int i;
  Bezier *b = (Bezier *)geom;
  ColorA *color;

  if (!crayHasColor(geom, NULL)) return NULL;
  color = va_arg(*args, ColorA *);
  for (i = 0; i < 4; i++) b->c[i] = *color;
  return (void *)geom;
}

#define prtpt3(pt) fprintf(stderr, "%.3f %.3f %.3f\n", \
			   ((Point3 *)(pt))->x, ((Point3 *)(pt))->y, \
			   ((Point3 *)(pt))->z)
#define prtpt4(pt) fprintf(stderr, "%.3f %.3f %.3f %.3f\n", \
			   ((HPoint3 *)(pt))->x, ((HPoint3 *)(pt))->y, \
			   ((HPoint3 *)(pt))->z, ((HPoint3 *)(pt))->w)

static int WhichCorner(Bezier *b, int vindex, HPoint3 *pt) {
  float close, newclose;
  const Point3 *pt3;
  HPoint3 *pt4;
  int index;

  pt3 = (Point3 *)pt;
  pt4 = pt;

  if (b->dimn != 3 && b->dimn != 4) {
    OOGLError(1, "Bezier patch of unrecognized dimension %d.\n", b->dimn);
    return -1;
  }
  /* Hits on a corner of the mesh are vertex hits.  Other hits are face 
   * hits and color the entire thing */
  close = FUDGE;
  index = -1;
  newclose = b->dimn == 3 ? 
    Pt3Distance(pt3, (const Point3 *)(&b->CtrlPnts[0])) :
      HPt3Distance(pt4, (HPoint3 *)(&b->CtrlPnts[0]));
  if (newclose < close) {
    index = 0;
    close = newclose;
  }
  newclose = b->dimn == 3 ?
    Pt3Distance(pt3, (Point3 *)(&b->CtrlPnts[b->degree_u * 3])) :
      HPt3Distance(pt4, (HPoint3 *)(&b->CtrlPnts[b->degree_u * 4]));
  if (newclose < close) {
    index = 1;
    close = newclose;
  }
  newclose = b->dimn == 3 ?
    Pt3Distance(pt3, (const Point3 *)(&b->CtrlPnts[((b->degree_v + 1) *
						    b->degree_u) * 3])) 
      : HPt3Distance(pt4, (HPoint3 *)(&b->CtrlPnts[((b->degree_v + 1) *
						    b->degree_u) * 4]));
  
  if (newclose < close) {
    index = 2;
    close = newclose;
  }
  newclose = b->dimn == 3 ? 
    Pt3Distance(pt3, (const Point3 *)(&b->CtrlPnts[((b->degree_v + 1) *
						    (b->degree_u + 1) - 1)
						   * 3]))
      : HPt3Distance(pt4, (HPoint3 *)(&b->CtrlPnts[((b->degree_v + 1) *
						    (b->degree_u + 1) - 1) 
						   * 4]));
  if (newclose < close) {
    index = 3;
    newclose = close;
  }

  return index;
}

void *cray_bezier_SetColorAt(int sel, Geom *geom, va_list *args) {
  Bezier *b = (Bezier *)geom;
  ColorA *color;
  int vindex, index;
  HPoint3 *pt;

  color = va_arg(*args, ColorA *);
  vindex = va_arg(*args, int);
  (void)va_arg(*args, int);		/* findex */
  (void)va_arg(*args, int *);		/* edge */
  (void)va_arg(*args, int *);		/* gpath */
  pt = va_arg(*args, HPoint3 *);

  index = WhichCorner(b, vindex, pt);
  if (index < 0) return (void *)(long)craySetColorAtF(geom, color, 0, NULL);

  b->c[index] = *color;
  return (void *)geom;
}

void *cray_bezier_SetColorAtF(int sel, Geom *geom, va_list *args) {
  int i;
  Bezier *b = (Bezier *)geom;
  ColorA *color;
  if (!crayHasColor(geom, NULL)) return NULL;
  color = va_arg(*args, ColorA *);
  for (i = 0; i < 4; i++) b->c[i] = *color;
  return (void *)geom;
}

void *cray_bezier_GetColorAt(int sel, Geom *geom, va_list *args) {
  Bezier *b = (Bezier *)geom;
  ColorA *color;
  int vindex, index;
  HPoint3 *pt;

  if (!crayHasColor(geom, NULL)) return NULL;
  color = va_arg(*args, ColorA *);
  vindex = va_arg(*args, int);
  (void)va_arg(*args, int);                    /* findex */
  (void)va_arg(*args, int *);                  /* edge */
  (void)va_arg(*args, int *);                  /* gpath */
  pt = va_arg(*args, HPoint3 *);

  index = WhichCorner(b, vindex, pt);
  if (index < 0) index = 0;

  *color = b->c[index];
  return (void *)geom;
}
