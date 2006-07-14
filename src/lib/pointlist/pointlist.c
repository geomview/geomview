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
#include "pointlistP.h"

static char methods[][POINTLIST_MAXNAME] =
{
  POINTLIST_METHNAMES
};

void *pointlist_default(int sel, Geom *geom, va_list *args);

void pointlist_init() {
  int i;

  for (i = 0; i < POINTLIST_MAXMETH; i++) 
	    GeomNewMethod(methods[i], pointlist_default);

  /* This should be the only place where the methods are mentioned by 
   * name. */
  ptlBezier_init();
  ptlInst_init();
  ptlList_init();
  ptlMesh_init();
  ptlPolylist_init();
  ptlQuad_init();
  ptlVect_init();

}

void pointlist_initspec(SpecFunc *specfunc, int n_func, GeomClass *class) {
  int i;

  for (i = 0; i < n_func; i++) 
    GeomSpecifyMethod(GeomMethodSel(specfunc[i].name), class, 
		     specfunc[i].func);
}

void *pointlist_default(int sel, Geom *geom, va_list *args) {
  return NULL;
}

HPoint3 *PointList_get(Geom *geom, Transform T, int CoordSystem)
{
  return (HPoint3 *)GeomCall(GeomMethodSel("PointList_get"), geom,
			     T, CoordSystem);
}

HPoint3 *PointList_fillin(Geom *geom, Transform T, int CoordSystem,
			  HPoint3 *plist) {
  return (HPoint3 *)GeomCall(GeomMethodSel("PointList_fillin"), geom,
			     T, CoordSystem, plist);
}

void PointList_set(Geom *geom, int CoordSystem, HPoint3 *plist)
{
  GeomCall(GeomMethodSel("PointList_set"), geom, CoordSystem, plist);
}

int PointList_length(Geom *geom) {
  return (int)(long)GeomCall(GeomMethodSel("PointList_length"), geom);
}
 

