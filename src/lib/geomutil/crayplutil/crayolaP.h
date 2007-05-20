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
#ifndef CRAYOLAP_H
#define CRAYOLAP_H

#include "crayola.h"

#define CRAYOLA_MAXNAME 128

/* 
 * WARNING:  The order of the names of the routines is hardcoded
 * into the functions themselves.
 */
#define CRAYOLA_MAXMETH 15
#define CRAYOLA_METHNAMES "crayHasColor", "crayHasVColor", "crayHasFColor", \
"crayCanUseVColor", "crayCanUseFColor", \
"crayUseVColor", "crayUseFColor", "crayEliminateColor", \
"craySetColorAll", \
"craySetColorAt", "craySetColorAtV", "craySetColorAtF", \
"crayGetColorAt", "crayGetColorAtV", "crayGetColorAtF"

typedef struct {
  char name[CRAYOLA_MAXNAME];
  GeomExtFunc *func;
} craySpecFunc;

void crayInitSpec(craySpecFunc *specfunc, int n_func, GeomClass *class);

void *crayTrue(int sel, Geom *geom, va_list *args);
void *crayFalse(int sel, Geom *geom, va_list *args);

int cray_bezier_init();
int cray_inst_init();
int cray_list_init();
int cray_mesh_init();
int cray_polylist_init();
int cray_npolylist_init();
int cray_quad_init();
int cray_vect_init();
int cray_skel_init();

#endif
