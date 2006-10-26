/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
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

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Geometry Technologies, Inc.";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "geomclass.h"
#include "geom.h"
#include "transform3.h"
#include "skelP.h"
#include "ndmeshP.h"
#include "hpointn.h"
#include "hpoint3.h"
#include "pointlist.h"
#include "quadP.h"
#include "meshP.h"
#include "polylistP.h"
#include "bezierP.h"
#include "vectP.h"
#include "listP.h"
#include "discgrp.h"
#include "instP.h"
#include "transformn.h"
#include "npolylistP.h"
#include "plutil.h"
#include "dim.h"

#include "BasicFns.h"
#include "ColorMapDefs.h"
#include "ColorPoint.h"
#include "LoadMap.h"
#include "WithColor.h"
#include "crayola.h"

extern IOBFILE *infile;

static int CRAYINIT = 1;
static int PTLISTINIT = 1;

extern TransformN *TmNCreateProjection(TransformN *UnivCam, int *axes);

static void *projectCamWCDefault(int sel, Geom * g, va_list * args)
{
  TransformN *ObjUniv, *UnivCam, *ProjMat;
  int *axes, i, npts, dim;
  float *ptr;
  HPointN *hptn1, *hptn2;
  HPoint3 *pts;
  ColorA *col;
  colormap *map;
  char *objname, *camname;

  ObjUniv = va_arg(*args, TransformN *);
  UnivCam = va_arg(*args, TransformN *);
  axes = va_arg(*args, int *);
  objname = va_arg(*args, char *);
  camname = va_arg(*args, char *);

  ProjMat = TmNCreateProjection(UnivCam, axes);

  if (PTLISTINIT) {
    pointlist_init();
    PTLISTINIT = 0;
  }
  if (CRAYINIT) {
    crayolaInit();
    CRAYINIT = 0;
  }
  npts = (long) GeomCall(GeomMethodSel("PointList_length"), g);
  if (npts == 0)
    return (void *) NULL;
  pts = OOGLNewNE(HPoint3, npts, "Points");
  pts =
      GeomCall(GeomMethodSel("PointList_get"), g, TM_IDENTITY,
	       POINTLIST_SELF);
  dim = GeomDimension(g);
  hptn1 = HPtNCreate(dim + 1, NULL);
  hptn2 = HPtNCreate(4, NULL);
  printf("(echo(ND-color %s)\\n)\n", camname);
  fflush(stdout);
  map = NDcolorFLoad(infile, "stdin");
  fprintf(stderr, "dimdirs = %d withrespectto = %s\n", map->numdirs,
	  *(map->coordsys));
  for (i = 0; i < npts; i++) {
    hptn1 = extract(pts + i, hptn1, dim);
    if (map->numdirs != 0) {
      col = colorpoint(hptn1, map, objname);
      fprintf(stderr, "Found color\n");
      craySetColorAtV(g, col, i, NULL, NULL);
      fprintf(stderr, "Set color\n");
    }
    hptn1 = HPtNTransform(ObjUniv, hptn1, hptn1);
    hptn2 = HPtNTransform(ProjMat, hptn1, hptn2);
    HPtNToHPt3(hptn2, NULL, pts + i);
  }
  GeomCall(GeomMethodSel("PointList_set"), g, POINTLIST_SELF, pts);
  HPtNDelete(hptn1);
  HPtNDelete(hptn2);
  TmNDelete(ProjMat);
  g->pdim = 4;
  g->geomflags = (g->geomflags) & (~(VERT_4D));
  g->geomflags |= VERT_C;
  return (void *) g;
}

static void *projectCamWCMesh(int sel, Geom * g, va_list * args)
{
  TransformN *ObjUniv, *UnivCam, *ProjMat;
  int *axes, i, npts, dim;
  float *ptr;
  HPointN *hptn1, *hptn2;
  HPoint3 *pts;
  ColorA *col;
  colormap *map;
  char *objname, *camname;

  ObjUniv = va_arg(*args, TransformN *);
  UnivCam = va_arg(*args, TransformN *);
  axes = va_arg(*args, int *);
  objname = va_arg(*args, char *);
  camname = va_arg(*args, char *);

  ProjMat = TmNCreateProjection(UnivCam, axes);

  if (PTLISTINIT) {
    pointlist_init();
    PTLISTINIT = 0;
  }
  npts = (long) GeomCall(GeomMethodSel("PointList_length"), g);
  if (npts == 0)
    return (void *) NULL;
  pts = OOGLNewNE(HPoint3, npts, "Points");
  pts =
      GeomCall(GeomMethodSel("PointList_get"), g, TM_IDENTITY,
	       POINTLIST_SELF);
  dim = GeomDimension(g);
  hptn1 = HPtNCreate(dim + 1, NULL);
  hptn2 = HPtNCreate(4, NULL);
  printf("(echo(ND-color %s)\\n)\n", camname);
  fflush(stdout);
  map = NDcolorFLoad(infile, "stdin");
  if (map->numdirs != 0) {
    free(((Mesh *) g)->c);
    ((Mesh *) g)->c = (ColorA *) malloc((sizeof *col) * npts);
    ((Mesh *) g)->flag |= MESH_C;
  }
  for (i = 0; i < npts; i++) {
    hptn1 = extract(pts + i, hptn1, dim);
    if (map->numdirs != 0) {
      col = colorpoint(hptn1, map, objname);
      memcpy(((Mesh *) g)->c + i, col, sizeof *col);
    }
    hptn1 = HPtNTransform(ObjUniv, hptn1, hptn1);
    hptn2 = HPtNTransform(ProjMat, hptn1, hptn2);
    HPtNToHPt3(hptn2, NULL, pts + i);
  }
  GeomCall(GeomMethodSel("PointList_set"), g, POINTLIST_SELF, pts);
  HPtNDelete(hptn1);
  HPtNDelete(hptn2);
  TmNDelete(ProjMat);
  g->pdim = 4;
  g->geomflags = (g->geomflags) & (~VERT_4D);
  ((Mesh *) g)->flag = (((Mesh *) g)->flag) & (~MESH_4D);
  return (void *) g;
}

static void *projectCamWCQuad(int sel, Geom * g, va_list * args)
{
  TransformN *ObjUniv, *UnivCam, *ProjMat;
  int *axes, i, npts, dim;
  float *ptr;
  HPointN *hptn1, *hptn2;
  ColorA *col;
  HPoint3 *pts;
  colormap *map;
  char *objname, *camname;

  ObjUniv = va_arg(*args, TransformN *);
  UnivCam = va_arg(*args, TransformN *);
  axes = va_arg(*args, int *);
  objname = va_arg(*args, char *);
  camname = va_arg(*args, char *);

  ProjMat = TmNCreateProjection(UnivCam, axes);

  if (PTLISTINIT) {
    pointlist_init();
    PTLISTINIT = 0;
  }
  npts = (long) GeomCall(GeomMethodSel("PointList_length"), g);
  if (npts == 0)
    return (void *) NULL;
  pts = OOGLNewNE(HPoint3, npts, "Points");
  pts =
      GeomCall(GeomMethodSel("PointList_get"), g, TM_IDENTITY,
	       POINTLIST_SELF);
  dim = GeomDimension(g);
  hptn1 = HPtNCreate(dim + 1, NULL);
  hptn2 = HPtNCreate(4, NULL);
  printf("(echo(ND-color %s)\\n)\n", camname);
  fflush(stdout);
  map = NDcolorFLoad(infile, "stdin");
  if (map->numdirs != 0) {
    free(((Quad *) g)->c);
    ((Quad *) g)->c = (QuadC *) malloc((sizeof *(((Quad *) g)->c)) *
				       (((Quad *) g)->maxquad));
    ((Quad *) g)->flag |= QUAD_C;
  }
  for (i = 0; i < npts; i++) {
    hptn1 = extract(pts + i, hptn1, dim);
    if (map->numdirs != 0) {
      col = colorpoint(hptn1, map, objname);
      memcpy(((((Quad *) g)->c + i / 4) + i % 4), col, sizeof *col);
      free(col);
    }
    hptn1 = HPtNTransform(ObjUniv, hptn1, hptn1);
    hptn2 = HPtNTransform(ProjMat, hptn1, hptn2);
    HPtNToHPt3(hptn2, NULL, pts + i);
  }
  GeomCall(GeomMethodSel("PointList_set"), g, POINTLIST_SELF, pts);
  HPtNDelete(hptn1);
  HPtNDelete(hptn2);
  TmNDelete(ProjMat);
  g->pdim = 4;
  g->geomflags = (g->geomflags) & (~VERT_4D);
  return (void *) g;
}

static void *projectCamWCNDMesh(int sel, Geom * g, va_list * args)
{
  int i, *axes, numpts = 1;
  TransformN *ObjUniv, *UnivCam, *ProjMat;
  float *ptr;
  colormap *map;
  ColorA *col;
  char *camname, *objname;

  ObjUniv = va_arg(*args, TransformN *);
  UnivCam = va_arg(*args, TransformN *);
  axes = va_arg(*args, int *);
  objname = va_arg(*args, char *);
  camname = va_arg(*args, char *);

  ProjMat = TmNCreateProjection(UnivCam, axes);

  for (i = 0; i < ((NDMesh *) g)->meshd; i++)
    numpts *= (((NDMesh *) g)->mdim)[i];
  printf("(echo(ND-color %s)\\n)\n", camname);
  fflush(stdout);
  map = NDcolorFLoad(infile, "stdin");
  if (map->numdirs != 0) {
    free(((NDMesh *) g)->c);
    ((NDMesh *) g)->c = (ColorA *) malloc((sizeof *col) * numpts);
    ((NDMesh *) g)->flag |= MESH_C;
  }
  for (i = 0; i < numpts; i++) {
    if (map->numdirs != 0) {
      col = colorpoint((((NDMesh *) g)->p)[i], map, objname);
      memcpy(((NDMesh *) g)->c + i, col, sizeof *col);
      free(col);
    }
    (((NDMesh *) g)->p)[i] = HPtNTransform
	(ObjUniv, (((NDMesh *) g)->p)[i], (((NDMesh *) g)->p)[i]);
    (((NDMesh *) g)->p)[i] = HPtNTransform
	(ProjMat, (((NDMesh *) g)->p)[i], (((NDMesh *) g)->p)[i]);
  }
  ((NDMesh *) g)->pdim = 3;
  TmNDelete(ProjMat);
  return (void *) g;
}

static void *projectCamWCSkel(int sel, Geom * g, va_list * args)
{
  int i, j, k, l, numcolors = 0, *axes, *newvi, newnvi = 0;
  TransformN *ObjUniv, *UnivCam, *ProjMat;
  float *ptr;
  HPointN *hptn1, *hptn2, tmp;
  ColorA *col;
  colormap *map;
  char *objname, *camname;
  Skline *newlines;

  ObjUniv = va_arg(*args, TransformN *);
  UnivCam = va_arg(*args, TransformN *);
  axes = va_arg(*args, int *);
  objname = va_arg(*args, char *);
  camname = va_arg(*args, char *);

  ProjMat = TmNCreateProjection(UnivCam, axes);

  hptn1 = HPtNCreate(((Skel *) g)->pdim, NULL);
  hptn2 = HPtNCreate(4, NULL);
  printf("(echo(ND-color %s)\\n)\n", camname);
  fflush(stdout);
  map = NDcolorFLoad(infile, "stdin");
  if (map->numdirs != 0) {
    free(((Skel *) g)->c);
    for (i = 0; i < ((Skel *) g)->nlines; i++)
      if ((((Skel *) g)->l + i)->nv == 1) {
	numcolors++;
	newnvi++;
      } else {
	numcolors += (((Skel *) g)->l + i)->nv - 1;
	newnvi += 2 * ((((Skel *) g)->l + i)->nv - 1);
      }
    ((Skel *) g)->c = (ColorA *) malloc((sizeof *col) * numcolors);
    newlines = (Skline *) malloc((sizeof *newlines) * numcolors);
    newvi = (int *) malloc((sizeof *newvi) * newnvi);
    for (i = 0, k = 0, l = 0; i < ((Skel *) g)->nlines; i++) {
      if ((((Skel *) g)->l + i)->nv == 1) {
	(newlines + k)->nv = 1;
	(newlines + k)->v0 = l;
	newvi[l] = (((Skel *) g)->vi)
	    [(((Skel *) g)->l + i)->v0];
	(newlines + k)->nc = 1;
	(newlines + k)->c0 = k;
	hptn1->v = ((Skel *) g)->p + newvi[l]
	    * ((Skel *) g)->pdim;
	col = colorpoint(hptn1, map, objname);
	memcpy(((Skel *) g)->c + k, col, sizeof *col);
	free(col);
	k++;
	l++;
      } else
	for (j = 0; j < (((Skel *) g)->l + i)->nv - 1; j++) {
	  (newlines + k)->nv = 2;
	  (newlines + k)->v0 = l;
	  newvi[l] = (((Skel *) g)->vi)
	      [(((Skel *) g)->l + i)->v0 + j];
	  newvi[l + 1] = (((Skel *) g)->vi)
	      [(((Skel *) g)->l + i)->v0 + j + 1];
	  (newlines + k)->nc = 1;
	  (newlines + k)->c0 = k;
	  hptn1->v = ((Skel *) g)->p + newvi[l]
	      * ((Skel *) g)->pdim;
	  col = colorpoint(hptn1, map, objname);
	  memcpy(((Skel *) g)->c + k, col, sizeof *col);
	  free(col);
	  k++;
	  l += 2;
	}
    }
    free(((Skel *) g)->l);
    ((Skel *) g)->l = newlines;
    free(((Skel *) g)->vi);
    ((Skel *) g)->nvi = newnvi;
    ((Skel *) g)->vi = newvi;
    ((Skel *) g)->nlines = ((Skel *) g)->nc = numcolors;
  }
  tmp.dim = ((Skel *) g)->pdim;
  tmp.flags = 0;
  for (i = 0; i < ((Skel *) g)->nvert; i++) {
    tmp.v = ((Skel *) g)->p + i * (((Skel *) g)->pdim);
    HPtNCopy(&tmp, hptn1);
    hptn1 = HPtNTransform(ObjUniv, hptn1, hptn1);
    hptn2 = HPtNTransform(ProjMat, hptn1, hptn2);
    for (j = 0; j < 4; j++)
      *(((Skel *) g)->p + i * 4 + j) = hptn2->v[j];
  }
  ((Skel *) g)->pdim = 4;
  HPtNDelete(hptn1);
  HPtNDelete(hptn2);
  TmNDelete(ProjMat);
  return (void *) g;
}

static void *projectCamWCList(int sel, Geom * g, va_list * args)
{
  List *l;
  TransformN *ObjUniv, *UnivCam;
  int *axes;
  char *objname, *camname;
  ObjUniv = va_arg(*args, TransformN *);
  UnivCam = va_arg(*args, TransformN *);
  axes = va_arg(*args, int *);
  objname = va_arg(*args, char *);
  camname = va_arg(*args, char *);
  for (l = (List *) g; l != NULL; l = l->cdr)
    GeomProjCamWC(l->car, ObjUniv, UnivCam, axes, objname, camname);
  return (void *) g;
}

static void *projectCamWCNPolyList(int sel, Geom * g, va_list * args)
{
  int i, j, *axes;
  TransformN *ObjUniv, *UnivCam, *ProjMat;
  float *ptr;
  HPointN *hptn1, *hptn2, tmp;
  ColorA *col;
  colormap *map;
  char *objname, *camname;

  ObjUniv = va_arg(*args, TransformN *);
  UnivCam = va_arg(*args, TransformN *);
  axes = va_arg(*args, int *);
  objname = va_arg(*args, char *);
  camname = va_arg(*args, char *);

  ProjMat = TmNCreateProjection(UnivCam, axes);

  hptn1 = HPtNCreate(((NPolyList *) g)->pdim, NULL);
  hptn2 = HPtNCreate(4, NULL);
  printf("(echo(ND-color %s)\\n)\n", camname);
  fflush(stdout);
  map = NDcolorFLoad(infile, "stdin");
  if (map->numdirs != 0) {
    free(((NPolyList *) g)->vcol);
    ((NPolyList *) g)->vcol = (ColorA *) malloc
	((sizeof *col) * ((NPolyList *) g)->n_verts);
    ((NPolyList *) g)->flags |= PL_HASVCOL;
    ((NPolyList *) g)->flags &= ~PL_HASPCOL;
  }
  tmp.dim = ((Skel *) g)->pdim;
  tmp.flags = 0;
  for (i = 0; i < ((NPolyList *) g)->n_verts; i++) {
    tmp.v = ((NPolyList *) g)->v + i * (((NPolyList *) g)->pdim);
    HPtNCopy(&tmp, hptn1);
    if (map->numdirs != 0) {
      col = colorpoint(hptn1, map, objname);
      memcpy(((NPolyList *) g)->vcol + i, col, sizeof *col);
      free(col);
    }
    hptn1 = HPtNTransform(ObjUniv, hptn1, hptn1);
    hptn2 = HPtNTransform(ProjMat, hptn1, hptn2);
    for (j = 0; j < 4; j++)
      *(((NPolyList *) g)->v + i * 4 + j) = hptn2->v[j];
  }
  ((NPolyList *) g)->pdim = 4;
  HPtNDelete(hptn1);
  HPtNDelete(hptn2);
  TmNDelete(ProjMat);
  return (void *) g;
}

static int WColorSel = 0;

Geom *GeomProjCamWC(Geom * g, TransformN * ObjUniv, TransformN * UnivCam,
		    int *axes, char *objectname, char *camname)
{
  if (WColorSel == 0) {
    WColorSel = GeomNewMethod("projCam", projectCamWCDefault);
    GeomSpecifyMethod(WColorSel, SkelMethods(), projectCamWCSkel);
    GeomSpecifyMethod(WColorSel, NDMeshMethods(), projectCamWCNDMesh);
    GeomSpecifyMethod(WColorSel, ListMethods(), projectCamWCList);
    GeomSpecifyMethod(WColorSel, NPolyListMethods(),
		      projectCamWCNPolyList);
    GeomSpecifyMethod(WColorSel, MeshMethods(), projectCamWCMesh);
    GeomSpecifyMethod(WColorSel, QuadMethods(), projectCamWCQuad);
  }
  return (Geom *) GeomCall(WColorSel, g, ObjUniv, UnivCam, axes,
			   objectname, camname);
}
