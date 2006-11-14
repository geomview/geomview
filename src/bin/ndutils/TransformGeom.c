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

/*
 * TransformGeom.c
 * author: Brian T. Luense
 * date: August 18, 1994
 */

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
#include "TransformGeom.h"

Geom *GeomProjCam(Geom *, TransformN *, TransformN *, int *);
static int PTLINIT = 1;

TransformN *TmNCreateProjection(TransformN *UnivCam, int *axes)
{
  TransformN *ProjMat;
  float *ptr;
  int i;

  ProjMat = TmNCreate(UnivCam->idim, 4, NULL);
  ptr = ProjMat->a;
  for (i = 0; i < (UnivCam->idim); i++) {
    *ptr++ = *(UnivCam->a + i * (UnivCam->idim) + axes[3]);
    *ptr++ = *(UnivCam->a + i * (UnivCam->idim) + axes[0]);
    *ptr++ = *(UnivCam->a + i * (UnivCam->idim) + axes[1]);
    *ptr++ = *(UnivCam->a + i * (UnivCam->idim) + axes[2]);
  }
  return ProjMat;
}


static void *projectCamDefault(int sel, Geom * g, va_list * args)
{
  TransformN *ObjUniv, *UnivCam, *ProjMat;
  int *axes, i, npts, dim;
  HPointN *hptn1, *hptn2;
  HPoint3 *pts;

  ObjUniv = va_arg(*args, TransformN *);
  UnivCam = va_arg(*args, TransformN *);
  axes = va_arg(*args, int *);

  ProjMat = TmNCreateProjection(UnivCam, axes);
  
  if (PTLINIT) {
    pointlist_init();
    PTLINIT = 0;
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
  for (i = 0; i < npts; i++) {
    hptn1 = extract(pts + i, hptn1, dim);
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
  return (void *) g;
}

static void *projectCamMesh(int sel, Geom * g, va_list * args)
{
  TransformN *ObjUniv, *UnivCam, *ProjMat;
  int *axes, i, npts, dim;
  HPointN *hptn1, *hptn2;
  HPoint3 *pts;

  ObjUniv = va_arg(*args, TransformN *);
  UnivCam = va_arg(*args, TransformN *);
  axes = va_arg(*args, int *);

  ProjMat = TmNCreateProjection(UnivCam, axes);

  if (PTLINIT) {
    pointlist_init();
    PTLINIT = 0;
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
  for (i = 0; i < npts; i++) {
    hptn1 = extract(pts + i, hptn1, dim);
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

static void *projectCamQuad(int sel, Geom * g, va_list * args)
{
  TransformN *ObjUniv, *UnivCam, *ProjMat;
  int *axes, i, npts, dim;
  HPointN *hptn1, *hptn2;
  HPoint3 *pts;

  ObjUniv = va_arg(*args, TransformN *);
  UnivCam = va_arg(*args, TransformN *);
  axes = va_arg(*args, int *);

  ProjMat = TmNCreateProjection(UnivCam, axes);

  if (PTLINIT) {
    pointlist_init();
    PTLINIT = 0;
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
  for (i = 0; i < npts; i++) {
    hptn1 = extract(pts + i, hptn1, dim);
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

static void *projectCamNDMesh(int sel, Geom * g, va_list * args)
{
  int i, *axes, numpts = 1;
  TransformN *ObjUniv, *UnivCam, *ProjMat;

  ObjUniv = va_arg(*args, TransformN *);
  UnivCam = va_arg(*args, TransformN *);
  axes = va_arg(*args, int *);

  ProjMat = TmNCreateProjection(UnivCam, axes);

  for (i = 0; i < ((NDMesh *) g)->meshd; i++)
    numpts *= (((NDMesh *) g)->mdim)[i];
  for (i = 0; i < numpts; i++) {
    (((NDMesh *) g)->p)[i] = HPtNTransform
	(ObjUniv, (((NDMesh *) g)->p)[i], (((NDMesh *) g)->p)[i]);
    (((NDMesh *) g)->p)[i] = HPtNTransform
	(ProjMat, (((NDMesh *) g)->p)[i], (((NDMesh *) g)->p)[i]);
  }
  ((NDMesh *) g)->pdim = 4;
  TmNDelete(ProjMat);
  return (void *) g;
}

static void *projectCamSkel(int sel, Geom * g, va_list * args)
{
  int i, j, *axes;
  TransformN *ObjUniv, *UnivCam, *ProjMat;
  HPointN *hptn1, *hptn2, tmp;

  ObjUniv = va_arg(*args, TransformN *);
  UnivCam = va_arg(*args, TransformN *);
  axes = va_arg(*args, int *);

  ProjMat = TmNCreateProjection(UnivCam, axes);

  hptn1 = HPtNCreate(((Skel *) g)->pdim, NULL);
  hptn2 = HPtNCreate(4, NULL);
  tmp.dim = ((Skel *) g)->pdim;
  tmp.flags = 0;
  for (i = 0; i < ((Skel *) g)->nvert; i++) {
    tmp.v = ((Skel *) g)->p + i * (((Skel *) g)->pdim);
    HPtNCopy(&tmp, hptn1);
    hptn1 = HPtNTransform(ObjUniv, hptn1, hptn1);
    hptn2 = HPtNTransform(ProjMat, hptn1, hptn2);
    for (j = 0; j < 3; j++)
      *(((Skel *) g)->p + i * 4 + j) = hptn2->v[j+1];
    *(((Skel *) g)->p + i * 4 + 3) = hptn2->v[0];
  }
  ((Skel *) g)->pdim = 4;
  HPtNDelete(hptn1);
  HPtNDelete(hptn2);
  TmNDelete(ProjMat);
  return (void *) g;
}

static void *projectCamList(int sel, Geom * g, va_list * args)
{
  List *l;
  TransformN *ObjUniv, *UnivCam;
  int *axes;
  ObjUniv = va_arg(*args, TransformN *);
  UnivCam = va_arg(*args, TransformN *);
  axes = va_arg(*args, int *);
  for (l = (List *) g; l != NULL; l = l->cdr)
    GeomProjCam(l->car, ObjUniv, UnivCam, axes);
  return (void *) g;
}

static void *projectCamNPolyList(int sel, Geom * g, va_list * args)
{
  int i, j, *axes;
  TransformN *ObjUniv, *UnivCam, *ProjMat;
  HPointN *hptn1, *hptn2, tmp;

  ObjUniv = va_arg(*args, TransformN *);
  UnivCam = va_arg(*args, TransformN *);
  axes = va_arg(*args, int *);

  ProjMat = TmNCreateProjection(UnivCam, axes);

  hptn1 = HPtNCreate(((NPolyList *) g)->pdim, NULL);
  hptn2 = HPtNCreate(4, NULL);
  tmp.dim = ((NPolyList *) g)->pdim;
  tmp.flags = 0;
  for (i = 0; i < ((NPolyList *) g)->n_verts; i++) {
    tmp.v = ((NPolyList *) g)->v + i * (((NPolyList *) g)->pdim);
    HPtNCopy(&tmp, hptn1);
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

static int SnapSel = 0;

Geom *GeomProjCam(Geom * g, TransformN * ObjUniv, TransformN * UnivCam,
		  int *axes)
{
  if (SnapSel == 0) {
    SnapSel = GeomNewMethod("projCam", projectCamDefault);
    GeomSpecifyMethod(SnapSel, SkelMethods(), projectCamSkel);
    GeomSpecifyMethod(SnapSel, NDMeshMethods(), projectCamNDMesh);
    GeomSpecifyMethod(SnapSel, ListMethods(), projectCamList);
    GeomSpecifyMethod(SnapSel, NPolyListMethods(), projectCamNPolyList);
    GeomSpecifyMethod(SnapSel, MeshMethods(), projectCamMesh);
    GeomSpecifyMethod(SnapSel, QuadMethods(), projectCamQuad);
  }
  return (Geom *) GeomCall(SnapSel, g, ObjUniv, UnivCam, axes);
}
