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
#include "bezierP.h"
#include "pointlistP.h"

void *bezier_PointList_get(int sel, Geom *geom, va_list *args);
void *bezier_PointList_fillin(int sel, Geom *geom, va_list *args);
void *bezier_PointList_set(int sel, Geom *geom, va_list *args);
void *bezier_PointList_length(int sel, Geom *geom, va_list *args);

#define MAX_METHODS 4

static SpecFunc methods[] = {
  {"PointList_get", bezier_PointList_get},
  {"PointList_fillin", bezier_PointList_fillin},
  {"PointList_set", bezier_PointList_set},
  {"PointList_length", bezier_PointList_length}
  };

static char msg[] = "ptlBezier.c";

void
ptlBezier_init() {
  pointlist_initspec(methods, MAX_METHODS, GeomClassLookup("bezier"));
}

void *bezier_PointList_get(int sel, Geom *geom, va_list *args) {
  HPoint3 *pt;
  TransformPtr t;
  int n_points;

  t = va_arg(*args, TransformPtr);

  n_points = (int)(long)GeomCall(GeomMethodSel("PointList_length"), geom);
  pt = OOGLNewNE(HPoint3, n_points, msg);

  return (GeomCall(GeomMethodSel("PointList_fillin"), geom, t, 0, pt));

}

void *bezier_PointList_fillin(int sel, Geom *geom, va_list *args) {
  int i = 0;
  TransformPtr t;
  HPoint3 *pt;
  Bezier *b = (Bezier *)geom;

  t = va_arg(*args, TransformPtr);
  (void)va_arg(*args, int);      
  pt = va_arg(*args, HPoint3 *);

  if (b->CtrlPnts != NULL) {
    if (b->dimn == 3) {
      for (i = 0; i < (b->degree_u + 1) * (b->degree_v + 1); i++)
	HPt3From(&pt[i], b->CtrlPnts[i*3], b->CtrlPnts[i*3 + 1], 
		 b->CtrlPnts[i*3 + 2], 1.0);
    } else {
      if (b->dimn == 4) {
	for (i = 0; i < (b->degree_u + 1) * (b->degree_v + 1); i++)
	  HPt3From(&pt[i], b->CtrlPnts[i*4], b->CtrlPnts[i*4 + 1],
		   b->CtrlPnts[i*4 + 2], b->CtrlPnts[i*4 + 3]);
      } else {
	OOGLError(1, "Bezier patch of unfamiliar dimensions.");
	OOGLFree(pt);
	return NULL;
      }
    }
  }

#ifdef DONT_DO_THIS
  if (b->flag & BEZ_ST) {
    HPt3From(&pt[i], b->STCords[0], b->STCords[1], b->STCords[2], 
	     b->STCords[3]);
    HPt3From(&pt[i+1], b->STCords[4], b->STCords[5], b->STCords[6],
	     b->STCords[7]);
  }
#endif /*DONT_DO_THIS*/

  HPt3TransformN(t, pt, pt, i+1);

  return pt;
}


void *bezier_PointList_set(int sel, Geom *geom, va_list *args) {
  int i;
  HPoint3 *pt;
  Bezier *b = (Bezier *)geom;

  (void)va_arg(*args, int);
  pt = va_arg(*args, HPoint3 *);
  i = 0;
  if (b->CtrlPnts != NULL) {
    if (b->dimn == 3) {
      for (i = 0; i < (b->degree_u + 1) * (b->degree_v + 1); i++) {
	b->CtrlPnts[i*3] = pt[i].x;
	b->CtrlPnts[i*3 + 1] = pt[i].y;
	b->CtrlPnts[i*3 + 2] = pt[i].z;
      }
    } else {
      if (b->dimn == 4) {
	for (i = 0; i < (b->degree_u + 1) * (b->degree_v + 1); i++) {
	  b->CtrlPnts[i*4] = pt[i].x;
	  b->CtrlPnts[i*4 + 1] = pt[i].y;
	  b->CtrlPnts[i*4 + 2] = pt[i].z;
	  b->CtrlPnts[i*4 + 3] = pt[i].w;
	}
      } else {
	OOGLError(1, "Bezier patch of unfamiliar dimension.");
	return NULL;
      }
    }
  }

#ifdef DONT_DO_THIS
  if (b->flag & BEZ_ST) {
    b->STCords[0] = pt[i].x;
    b->STCords[1] = pt[i].y;
    b->STCords[2] = pt[i].z;
    b->STCords[3] = pt[i++].w;
    b->STCords[4] = pt[i].x;
    b->STCords[5] = pt[i].y;
    b->STCords[6] = pt[i].z;
    b->STCords[7] = pt[i].w;
  }
#endif /*DONT_DO_THIS*/

  GeomDelete((Geom *)b->mesh);
  b->mesh = NULL;

  return geom;
}

void *bezier_PointList_length(int sel, Geom *geom, va_list *args) {
  Bezier *b = (Bezier *)geom;
  return ((void *)(long)((b->degree_u + 1) * (b->degree_v + 1)
#ifdef DONT_DO_THIS
	+ (b->flag & BEZ_ST ? 2 : 0)
#endif 
	));
}
