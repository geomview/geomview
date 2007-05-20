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
#include "color.h"
#include "crayolaP.h"

ColorA crayDefColor = {CRAY_DEFR, CRAY_DEFG, CRAY_DEFB, CRAY_DEFA};

static char methods[][CRAYOLA_MAXNAME] = 
{
  CRAYOLA_METHNAMES
};
int sel[CRAYOLA_MAXMETH];

void crayolaInit(void)
{
  int i;

  for (i = 0; i < CRAYOLA_MAXMETH; i++) 
    sel[i] = GeomNewMethod(methods[i], crayFalse);

  cray_bezier_init();
  cray_inst_init();
  cray_list_init();
  cray_mesh_init();
  cray_polylist_init();
  cray_npolylist_init();
  cray_quad_init();
  cray_vect_init();
  cray_skel_init();
}

void crayInitSpec(craySpecFunc *specfunc, int n_func, GeomClass *class) {
  int i;
  for (i = 0; i < n_func; i++) 
    GeomSpecifyMethod(GeomMethodSel(specfunc[i].name), class,
				    specfunc[i].func);
}

int crayHasColor(Geom *geom, int *gpath) {
  return (int)(long)GeomCall(sel[0], geom, gpath);
}

int crayHasVColor(Geom *geom, int *gpath) {
  return (int)(long)GeomCall(sel[1], geom, gpath);
}

int crayHasFColor(Geom *geom, int *gpath) {
  return (int)(long)GeomCall(sel[2], geom, gpath);
}

int crayCanUseVColor(Geom *geom, int *gpath) {
  return (int)(long)GeomCall(sel[3], geom, gpath);
}

int crayCanUseFColor(Geom *geom, int *gpath) {
  return (int)(long)GeomCall(sel[4], geom, gpath);
}

int crayUseVColor(Geom *geom, ColorA *color, int *gpath) {
  return (int)(long)GeomCall(sel[5], geom, color, gpath);
}

int crayUseFColor(Geom *geom, ColorA *color, int *gpath) {
  return (int)(long)GeomCall(sel[6], geom, color, gpath);
}

int crayEliminateColor(Geom *geom, int *gpath) {
  return (int)(long)GeomCall(sel[7], geom, gpath);
}

int craySetColorAll(Geom *geom, ColorA *color, int *gpath) {
  return (int)(long)GeomCall(sel[8], geom, color, gpath);
}

int craySetColorAt(Geom *geom, ColorA *color, int vindex, int findex, 
		   int *edge, int *gpath, HPoint3 *pt) {
  return (int)(long)GeomCall(sel[9], geom, color, vindex, findex, edge, gpath, 
		       pt);
}

int craySetColorAtV(Geom *geom, ColorA *color, int index, int *gpath,
		    HPoint3 *pt) {
  return (int)(long)GeomCall(sel[10], geom, color, index, gpath, pt);
}

int craySetColorAtF(Geom *geom, ColorA *color, int index, int *gpath) {
  return (int)(long)GeomCall(sel[11], geom, color, index, gpath);
}

int crayGetColorAt(Geom *geom, ColorA *color, int vindex, int findex,
		   int *edge, int *gpath, HPoint3 *pt) {
  return (int)(long)GeomCall(sel[12], geom, color, vindex, findex, edge, gpath, pt);
}

int crayGetColorAtV(Geom *geom, ColorA *color, int index, int *gpath,
		    HPoint3 *pt) {
  return (int)(long)GeomCall(sel[13], geom, color, index, gpath, pt);
}

int crayGetColorAtF(Geom *geom, ColorA *color, int index, int *gpath) {
  return (int)(long)GeomCall(sel[14], geom, color, index, gpath);
}

void *crayTrue(int sel, Geom *geom, va_list *args) {
  return (void *)1;
}

void *crayFalse(int sel, Geom *geom, va_list *args) {
  return (void *)0;
}
/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
