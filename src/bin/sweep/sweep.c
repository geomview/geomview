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

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "ooglutil.h"
#include "hpoint3.h"
#include "point3.h"
#include "vectP.h"
#include "transform.h"
#include "transform3.h"
#include "sweep.h"

static char msg[] = "sweep.c";

Geom *TranslationSweep(float length, Point3 *p, Geom *g) {
  int h, i, j, k;
  Vect *v;
  Geom *new;
  int nverts;
  int npoly;
  HPoint3 hp3, *point4;
  int *nvert, *vert;

  if (g == NULL) return NULL;
  if (strcmp(GeomName(g), "vect")) {
    OOGLError(1, "TranslationSweep called with non-vector object.");
    return NULL;
  }

  length /= Pt3Length(p);
  hp3.x = p->x * length;
  hp3.y = p->y * length;
  hp3.z = p->z * length;
  hp3.w = 1;
  v = (Vect *) g;
  nverts = abs(v->nvert);

  point4 = OOGLNewNE(HPoint3, nverts * 2, msg);
  for (i = 0; i < nverts; i++) {
    HPt3Copy(&v->p[i], &point4[i]);
    HPt3Copy(&v->p[i], &point4[i + nverts]);
    HPt3Add(&point4[i + nverts], &hp3, &point4[i + nverts]);
  }

  nvert = OOGLNewNE(int, nverts, msg);
  vert = OOGLNewNE(int, nverts * 4, msg);
  for (i = 0; i < nverts; i++) nvert[i] = 4;
  for (h = i = k = 0; i < v->nvec; i++) {
    for (j = 0; j < abs(v->vnvert[i]);) {
      vert[h++] = k + j;
      vert[h++] = k + nverts + j++;
      vert[h++] = k + nverts + j;
      vert[h++] = k + j;
    }
    if (j) h -= 4;
    if (v->vnvert[i] < 0) {
      vert[h++] = k;
      vert[h++] = k + nverts;
      vert[h++] = k + nverts + j - 1;
      vert[h++] = k + j - 1;
    } 
    k += j;
  }

  npoly = h / 4;

  new = GeomCreate("polylist",
		   CR_NPOLY, npoly,
		   CR_POINT4, point4,
		   CR_NVERT, nvert,
		   CR_VERT, vert,
		   CR_END);

  OOGLFree(point4);
  OOGLFree(nvert);
  OOGLFree(vert);

  return(new);

}

Geom *RotationSweep(float angle, Point3 *end, Point3 *axis, 
		    int divisions, Geom *g) {
  int h, i, j, k, m;
  Vect *v;
  Geom *new;
  int v_nvert;
  int npoly;
  HPoint3 *point4;
  int *nvert, *vert;
  Transform T, TInv, R;
  int fullcircle = 0;

  if (g == NULL) return NULL;
  if (strcmp(GeomName(g), "vect")) {
    OOGLError(1, "RotationSweep called with non-vector object.");
    return NULL;
  }

  /* Hack to deal with things that should go all the way around */
  if (fabs(angle - 2.0 * M_PI) < .01) fullcircle = 1;

  v = (Vect *) g;

  v_nvert = abs(v->nvert);

  TmTranslate(T, end->x, end->y, end->z);
  TmInvert(T, TInv);
  TmRotate(R, angle / (float)divisions, axis); 

  point4 = OOGLNewNE(HPoint3, v_nvert * (divisions + 1), msg);
  for (i = 0; i < v_nvert; i++) {
    HPt3Copy(&v->p[i], &point4[i * (divisions+1)]);
    for (j = 1; j < divisions+1; j++) {
      HPt3Copy(&point4[i*(divisions+1) + j - 1], 
	       &point4[i*(divisions+1) + j]);
      HPt3Transform(TInv, &point4[i*(divisions+1) + j], 
		    &point4[i*(divisions+1) + j]);
      HPt3Transform(R, &point4[i*(divisions+1) + j], 
		    &point4[i*(divisions+1) + j]);
      HPt3Transform(T, &point4[i*(divisions+1) + j],
		    &point4[i*(divisions+1) + j]);
    }
  }

  nvert = OOGLNewNE(int, v_nvert * (divisions+1), msg);
  vert = OOGLNewNE(int, v_nvert * (divisions+1) * 4, msg);
  for (i = 0; i < v_nvert * (divisions+1); i++) nvert[i] = 4;
  for (h = i = k = 0; i < v->nvec; i++) {
    for (j = 0; j < abs(v->vnvert[i]); j++) {
      for (m = 1; m < divisions + 1; m++) {
	vert[h++] = ((k + j)*(divisions+1)) + m - 1;
	vert[h++] = ((k + j)*(divisions+1)) + m;
	vert[h++] = ((k + j + 1)*(divisions+1)) + m;
	vert[h++] = ((k + j + 1)*(divisions+1)) + m - 1;
      }
      if (fullcircle) {
	vert[h-2] = (k + j + 1) * (divisions + 1);
	vert[h-3] = (k + j) * (divisions + 1);
      }
    }
    if (j) h -= 4 * divisions;
    if (v->vnvert[i] < 0) { 
      for (m = 1; m < divisions + 1; m++) {
	vert[h++] = (k) * (divisions + 1) + m - 1;
	vert[h++] = (k) * (divisions + 1) + m;
	vert[h++] = (k + j - 1) * (divisions + 1) + m;
	vert[h++] = (k + j - 1) * (divisions + 1) + m - 1;
      }
      if (fullcircle) {
	vert[h-2] = (k + j - 1) * (divisions + 1);
	vert[h-3] = k * (divisions + 1); 
      }
    }
    k += j;
  }
  
  npoly = h / 4;

  new = GeomCreate("polylist",
		   CR_NPOLY, npoly,
		   CR_POINT4, point4,
		   CR_NVERT, nvert,
		   CR_VERT, vert,
		   CR_END);

  OOGLFree(point4);
  OOGLFree(nvert);
  OOGLFree(vert);

  return(new);

}

