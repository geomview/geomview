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
#ifndef POINTLISTP_H
#define POINTLISTP_H

#include "pointlist.h"

#define POINTLIST_MAXNAME 128
#define POINTLIST_MAXMETH 4
#define POINTLIST_METHNAMES "PointList_get", "PointList_fillin", \
                            "PointList_set", "PointList_length"

typedef struct {
  char name[POINTLIST_MAXNAME];
  GeomExtFunc *func;
} SpecFunc;

void pointlist_initspec(SpecFunc *specfunc, int n_func, GeomClass *class);

void ptlBezier_init();
void ptlInst_init();
void ptlList_init();
void ptlMesh_init();
void ptlPolylist_init();
void ptlQuad_init();
void ptlVect_init();

#endif
