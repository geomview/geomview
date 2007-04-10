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
 * GeomtoNoff.c
 * author: Brian T. Luense
 * date: August 18, 1994
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "geomclass.h"
#include "geom.h"
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
#include "GeomtoNoff.h"

/*The following defines a Geomview function called GeomtoNoff which
 * takes a Geom and a TransformN, applies the TransformN to the Geom,
 * and then converts the Geom to an nOFF (without destroying the
 * original Geom).  It keeps the same colors and appearances as the
 * original with the exception that Lists are returned without
 * color.
 */

static void *toNoffDefault(int sel, Geom * g, va_list * args)
{
  TransformN *t = va_arg(*args, TransformN *);
  PolyList *off;
  NPolyList *noff;
  if (g == NULL)
    return (void *) NULL;
  if (t && t->idim != GeomDimension(g) + 1)
    return (void *) NULL;
  off = (PolyList *) AnyToPL(g, TM_IDENTITY);
  noff = (NPolyList *) GeomtoNoff((Geom *) off, t);
  return (void *) noff;
}

static void *toNoffSkel(int sel, Geom * g, va_list * args)
{
  NPolyList *noff;
  int i, j, k, npoly, *numvertsperpoly, *indexlist;
  float *vertlist;
  Skel *s = (Skel *) g;
  TransformN *t = va_arg(*args, TransformN *);
  HPointN *hptn;
  ColorA *colorlist;

  if (g == NULL)
    return (void *) NULL;
  if (t && t->idim != GeomDimension(g) + 1)
    return (void *) NULL;
  for (i = 0, npoly = 0; i < s->nlines; i++) {
    if (((s->l) + i)->nv == 1)
      npoly++;
    else
      npoly += ((s->l) + i)->nv - 1;
  }
  numvertsperpoly = (int *) malloc((sizeof *numvertsperpoly) * npoly);
  for (i = 0, k = 0; i < s->nlines; i++)
    if (((s->l) + i)->nv == 1)
      numvertsperpoly[k++] = 1;
    else
      for (j = 0; j < ((s->l) + i)->nv - 1; j++)
	numvertsperpoly[k++] = 2;
  vertlist = (float *) malloc((sizeof *vertlist) * (s->pdim) * (s->nvert));
  hptn = HPtNCreate(s->pdim, NULL);
  hptn->v = (float *) malloc((sizeof *(hptn->v)) * s->pdim);
  for (i = 0; i < s->nvert; i++) {
    memcpy(hptn->v, (s->p + i * (s->pdim)), (sizeof *(hptn->v)) * s->pdim);
    hptn = HPtNTransform(t, hptn, hptn);
    memcpy(vertlist + (i * (s->pdim)), hptn->v,
	   (sizeof *vertlist) * (s->pdim));
  }
  indexlist = (int *) malloc((sizeof *indexlist) * 2 * npoly);
  for (i = 0, k = 0; i < s->nlines; i++)
    if (((s->l) + i)->nv == 1)
      indexlist[k++] = (s->vi)[((s->l) + i)->v0];
    else
      for (j = 0; j < ((s->l) + i)->nv - 1; j++) {
	indexlist[k++] = (s->vi)[((s->l) + i)->v0 + j];
	indexlist[k++] = (s->vi)[((s->l) + i)->v0 + j + 1];
      }
  if (s->nc == 0)
    colorlist = NULL;
  else {
    colorlist = (ColorA *) malloc((sizeof *colorlist) * npoly);
    for (i = 0, k = 0; i < s->nlines; i++)
      if (((s->l) + i)->nv == 1)
	colorlist[k++] = (s->c)[((s->l) + i)->c0];
      else
	for (j = 0; j < ((s->l) + i)->nv - 1; j++)
	  colorlist[k++] = (s->c)[((s->l) + i)->c0];
  }
  noff = (NPolyList *) GeomCCreate(NULL, NPolyListMethods(),
				   CR_NPOLY, npoly,
				   CR_NVERT, numvertsperpoly,
				   CR_VERT, indexlist,
				   CR_DIM, s->pdim - 1,
				   CR_POINT4, vertlist,
				   CR_POLYCOLOR, colorlist,
				   CR_APPEAR, g->ap, CR_END);
  free(numvertsperpoly);
  free(vertlist);
  free(indexlist);
  return (void *) noff;
}

static void *toNoffNDMesh(int sel, Geom * g, va_list * args)
{
  NPolyList *noff;
  int i, j, k, npoly, *numvertsperpoly, *indexlist;
  float *vertlist;
  NDMesh *m = (NDMesh *) g;
  TransformN *t = va_arg(*args, TransformN *);
  if (g == NULL)
    return (void *) NULL;
  if (t && t->idim != GeomDimension(g) + 1)
    return (void *) NULL;
  if (m->meshd != 2)
    return (void *) NULL;
  if (((m->mdim)[0] == 1) && ((m->mdim)[1] == 1)) {
    npoly = 1;
    numvertsperpoly = (int *) malloc(sizeof *numvertsperpoly);
    *numvertsperpoly = 1;
    vertlist = (float *) malloc((sizeof *vertlist) * (m->pdim + 1));
    *(m->p) = HPtNTransform(t, *(m->p), *(m->p));
    memcpy(vertlist, (*(m->p))->v, (sizeof *vertlist) * (m->pdim + 1));
    indexlist = (int *) malloc(sizeof *indexlist);
    *indexlist = 1;
    noff = (NPolyList *) GeomCCreate(NULL, NPolyListMethods(),
				     CR_FLAG, m->geomflags,
				     CR_NPOLY, npoly,
				     CR_NVERT, numvertsperpoly,
				     CR_VERT, indexlist,
				     CR_DIM, m->pdim,
				     CR_POINT4, vertlist,
				     CR_COLOR, m->c,
				     CR_APPEAR, g->ap, CR_END);
    free(numvertsperpoly);
    free(vertlist);
    free(indexlist);
    return (void *) noff;
  }
  if ((m->mdim)[0] == 1) {
    npoly = (m->mdim)[1] - 1;
    numvertsperpoly = (int *) malloc((sizeof *numvertsperpoly) * npoly);
    for (i = 0; i < npoly; i++)
      numvertsperpoly[i] = 2;
    vertlist =
	(float *) malloc((sizeof *vertlist) * (m->pdim + 1) *
			 (m->mdim)[1]);
    for (i = 0; i < (m->mdim)[1]; i++) {
      (m->p)[i] = HPtNTransform(t, (m->p)[i], (m->p)[i]);
      memcpy(vertlist + (i * (m->pdim + 1)), ((m->p)[i])->v,
	     (sizeof *vertlist) * (m->pdim + 1));
    }
    indexlist = (int *) malloc((sizeof *indexlist) * 2 * npoly);
    for (i = 0; i < 2 * npoly; i += 2) {
      indexlist[i] = i / 2;
      indexlist[i + 1] = i / 2 + 1;
    }
    noff = (NPolyList *) GeomCCreate(NULL, NPolyListMethods(),
				     CR_FLAG, m->geomflags,
				     CR_NPOLY, npoly,
				     CR_NVERT, numvertsperpoly,
				     CR_VERT, indexlist,
				     CR_DIM, m->pdim,
				     CR_POINT4, vertlist,
				     CR_COLOR, m->c,
				     CR_APPEAR, g->ap, CR_END);
    free(numvertsperpoly);
    free(vertlist);
    free(indexlist);
    return (void *) noff;
  }
  if ((m->mdim)[1] == 1) {
    npoly = (m->mdim)[0] - 1;
    numvertsperpoly = (int *) malloc((sizeof *numvertsperpoly) * npoly);
    for (i = 0; i < npoly; i++)
      numvertsperpoly[i] = 2;
    vertlist =
	(float *) malloc((sizeof *vertlist) * (m->pdim + 1) *
			 (m->mdim)[0]);
    for (i = 0; i < (m->mdim)[0]; i++) {
      (m->p)[i] = HPtNTransform(t, (m->p)[i], (m->p)[i]);
      memcpy(vertlist + (i * (m->pdim + 1)), ((m->p)[i])->v,
	     (sizeof *vertlist) * (m->pdim + 1));
    }
    indexlist = (int *) malloc((sizeof *indexlist) * 2 * npoly);
    for (i = 0; i < 2 * npoly; i += 2) {
      indexlist[i] = i / 2;
      indexlist[i + 1] = i / 2 + 1;
    }
    noff = (NPolyList *) GeomCCreate(NULL, NPolyListMethods(),
				     CR_FLAG, m->geomflags,
				     CR_NPOLY, npoly,
				     CR_NVERT, numvertsperpoly,
				     CR_VERT, indexlist,
				     CR_DIM, m->pdim,
				     CR_POINT4, vertlist,
				     CR_COLOR, m->c,
				     CR_APPEAR, g->ap, CR_END);
    free(numvertsperpoly);
    free(vertlist);
    free(indexlist);
    return (void *) noff;
  }
  npoly = ((m->mdim)[0] - 1) * ((m->mdim)[1] - 1);
  numvertsperpoly = (int *) malloc((sizeof *numvertsperpoly) * npoly);
  for (i = 0; i < npoly; i++)
    numvertsperpoly[i] = 4;
  vertlist =
      (float *) malloc((sizeof *vertlist) * (m->pdim + 1) *
		       ((m->mdim)[0]) * ((m->mdim)[1]));
  for (i = 0; i < ((m->mdim)[0]) * ((m->mdim)[1]); i++) {
    (m->p)[i] = HPtNTransform(t, (m->p)[i], (m->p)[i]);
    memcpy(vertlist + (i * (m->pdim + 1)), ((m->p)[i])->v,
	   (sizeof *vertlist) * (m->pdim + 1));
  }
  indexlist = (int *) malloc((sizeof *indexlist) * 4 * npoly);
  for (i = 0, k = 0; i < (m->mdim)[1] - 1; i++)
    for (j = 0; j < (m->mdim)[0] - 1; j++) {
      indexlist[k++] = i * (m->mdim)[0] + j;
      indexlist[k++] = i * (m->mdim)[0] + j + 1;
      indexlist[k++] = (i + 1) * (m->mdim)[0] + j + 1;
      indexlist[k++] = (i + 1) * (m->mdim)[0] + j;
    }
  noff = (NPolyList *) GeomCCreate(NULL, NPolyListMethods(),
				   CR_FLAG, m->geomflags,
				   CR_NPOLY, npoly,
				   CR_NVERT, numvertsperpoly,
				   CR_VERT, indexlist,
				   CR_DIM, m->pdim,
				   CR_POINT4, vertlist,
				   CR_COLOR, m->c,
				   CR_APPEAR, g->ap, CR_END);
  free(numvertsperpoly);
  free(vertlist);
  free(indexlist);
  return (void *) noff;
}

static void *toNoffNPolyList(int sel, Geom * g, va_list * args)
{
  TransformN *t = va_arg(*args, TransformN *);
  NPolyList *noff;
  HPointN hptnsrc, hptndst;
  HPtNCoord *oldcoords = ((NPolyList *)g)->v;
  int i;
  if (g == NULL)
    return (void *) NULL;
  if (t && t->idim != GeomDimension(g) + 1)
    return (void *) NULL;
  hptnsrc.dim = hptndst.dim = g->pdim;
  noff = (NPolyList *)GeomCopy(g);
  for (i = 0; i < noff->n_verts; i++) {
    hptnsrc.v = oldcoords + i * noff->pdim;
    hptndst.v = noff->v + i * noff->pdim;
    HPtNTransform(t, &hptnsrc, &hptndst);
    HPtNDehomogenize(&hptndst, &hptndst);
  }

  return (void *)noff;
}

/* Convert a list of Noff objects to a single Noff object */
static NPolyList *_toNoffList(List *lh)
{
  List *l;
  NPolyList *noff, *carpl;
  int numvertindex, npoly, *numvertsperpoly, *indexlist, nvert;
  int *nvpptr, *ilptr, i, j, numvertsofar = 0, dim;
  float *vertlist, *vlptr, zero = 0.0, *sourceptr;

  for (l = lh, npoly = 0, nvert = 0, numvertindex = 0; l != NULL;
       l = l->cdr) {
    carpl = (NPolyList *)l->car;
    npoly += carpl->n_polys;
    nvert += carpl->n_verts;
    numvertindex += carpl->nvi;
  }
  dim = GeomDimension((Geom *)lh);
  ilptr = indexlist = (int *) malloc((sizeof *indexlist) * numvertindex);
  vlptr = vertlist =
    (float *)malloc(sizeof(float)*nvert*(dim+1));
  nvpptr = numvertsperpoly = (int *)malloc(sizeof(int)*npoly);
  for (l = lh; l != NULL; l = l->cdr) {
    carpl = (NPolyList *)l->car;
    for (i = 0; i < carpl->n_polys; i++) {
      *(nvpptr++) = (carpl->p + i)->n_vertices;
    }
    sourceptr = carpl->v;
    for (i = 0; i < carpl->n_verts; i++) {
      memcpy(vlptr, sourceptr, sizeof(float) * carpl->pdim);
      for (j = 0; j < dim - carpl->pdim+1; j++) {
	*vlptr++ = 0.0;
      }
      vlptr += carpl->pdim;
      sourceptr += carpl->pdim;
    }
    for (i = 0; i < carpl->nvi; i++) {
      *(ilptr++) = *(carpl->vi + i) + numvertsofar;
    }
    numvertsofar += carpl->n_verts;
  }
  noff = (NPolyList *)GeomCCreate(NULL, NPolyListMethods(),
				  CR_NPOLY, npoly,
				  CR_NVERT, numvertsperpoly,
				  CR_VERT, indexlist,
				  CR_DIM, GeomDimension((Geom *)lh),
				  CR_POINT4, vertlist,
				  CR_APPEAR, lh->ap, CR_END);
  free(indexlist);
  free(vertlist);
  free(numvertsperpoly);
  return noff;
}

static void *toNoffList(int sel, Geom * g, va_list * args)
{
  TransformN *t = va_arg(*args, TransformN *), *smaller;
  List *l;
  Geom *nl = NULL;
  NPolyList *noff;
  int numvertindex, npoly, *numvertsperpoly, *indexlist, nvert,
      *nvpptr, *ilptr, i, j, numvertsofar = 0, idim, odim;
  float *vertlist, *vlptr, zero = 0.0, *sourceptr;
  void *result;

  if (g == NULL)
    return (void *) NULL;

  odim = t->odim;
  idim = GeomDimension(g) + 1;
  if (t->idim != idim)
    return (void *) NULL;

  for (l = (List *)g; l != NULL; l = l->cdr) {
    nl = ListAppend(nl, GeomtoNoff(l->car, t));
  }

  result = (void *)_toNoffList((List *)nl);

  GeomDelete(nl);

  return result;
}

static void *toNoffInst(int sel, Geom *g, va_list *args)
{
  TransformN *t = va_arg(*args, TransformN *);
  Inst *inst = (Inst *)g;
  
  /* if we have a single ND-transform, or a single 3d-transform, then
   * we simply concat with t (from the left). We "origin" and
   * "location" != L_NONE/L_LOCAL cannot be suported here.
   */
  if (inst->location > L_LOCAL || inst->origin != L_NONE) {
    return NULL;
  }

  if (inst->NDaxis) {
    if (t) {
      t = TmNConcat(inst->NDaxis, t, NULL);
    } else {
      t = REFGET(TransformN, inst->NDaxis);
    }
    GeomGet(g, CR_GEOM, &g);
    g = GeomtoNoff(g, t);
    TmNDelete(t);
    return (void *)g;
  } else {
    /* Not very efficient, but so what: convert into a list of Noff
     * objects, then convert that list to a single Noff object.
     */
    GeomIter *it;
    Transform T;
    NPolyList *noff;
    TransformN *tmp = NULL;
    Geom *l = NULL;

    it = GeomIterate((Geom *)inst, DEEP);
    while (NextTransform(it, T)) {
      tmp = TmNCopy(t, tmp);
      TmNApplyT3TN(T, NULL, tmp);
      l = ListAppend(l, GeomtoNoff(inst->geom, tmp));
    }
    TmNDelete(tmp);
    noff = _toNoffList((List *)l);
    GeomDelete(l);
    return (void *)noff;
  }
  
}

static void *toNoffPolyList(int sel, Geom * g, va_list * args)
{
  TransformN *t = va_arg(*args, TransformN *);
  NPolyList *noff;
  PolyList *l = (PolyList *) g;
  HPointN *hptn;
  int npoly, *numvertsperpoly, *indexlist, ldim, i, j, k, indexlen;
  float *vertlist;
  ColorA *vertcolors, *facecolors;

  if (l == NULL)
    return (void *) NULL;
  if (t && t->idim != GeomDimension(g) + 1)
    return (void *) NULL;
  ldim = GeomDimension(g);
  npoly = l->n_polys;
  numvertsperpoly = (int *) malloc((sizeof *numvertsperpoly) * npoly);
  for (i = 0, indexlen = 0; i < npoly; i++)
    indexlen += numvertsperpoly[i] = ((l->p) + i)->n_vertices;
  hptn = HPtNCreate(ldim + 1, NULL);
  vertlist =
      (float *) malloc((sizeof *vertlist) * (l->n_verts) * (ldim + 1));
  for (i = 0; i < l->n_verts; i++) {
    extract(&(((l->vl) + i)->pt), hptn, ldim);
    hptn = HPtNTransform(t, hptn, hptn);
    memcpy(vertlist + (i * (ldim + 1)), hptn->v,
	   (sizeof *vertlist) * (ldim + 1));
  }
  indexlist = (int *) malloc((sizeof *indexlist) * indexlen);
  for (i = 0, k = 0; i < npoly; i++)
    for (j = 0; j < numvertsperpoly[i]; j++)
      indexlist[k++] =
	  (((long) (((l->p) + i)->v)[j]) -
	   ((long) (l->vl))) / (sizeof *(l->vl));
  if ((l->geomflags) & (PL_HASVCOL)) {
    vertcolors = (ColorA *) malloc((sizeof *vertcolors) * (l->n_verts));
    for (i = 0; i < l->n_verts; i++)
      memcpy(vertcolors + i, &(((l->vl) + i)->vcol), (sizeof *vertcolors));
  } else
    vertcolors = NULL;
  if ((l->geomflags) & (PL_HASPCOL)) {
    facecolors = (ColorA *) malloc((sizeof *facecolors) * (l->n_polys));
    for (i = 0; i < l->n_polys; i++)
      memcpy(facecolors + i, &(((l->p) + i)->pcol), (sizeof *facecolors));
  } else
    facecolors = NULL;
  noff = (NPolyList *) GeomCCreate(NULL, NPolyListMethods(),
				   CR_FLAG, l->geomflags,
				   CR_NPOLY, npoly,
				   CR_NVERT, numvertsperpoly,
				   CR_VERT, indexlist,
				   CR_DIM, ldim,
				   CR_POINT4, vertlist,
				   CR_COLOR, vertcolors,
				   CR_POLYCOLOR, facecolors,
				   CR_APPEAR, l->ap, CR_END);
  free(numvertsperpoly);
  free(vertlist);
  free(indexlist);
  free(hptn);
  free(vertcolors);
  free(facecolors);
  return (void *) noff;
}
static int ConvertSel = 0;

Geom *GeomtoNoff(Geom * g, TransformN * t)
{
  if (ConvertSel == 0) {
    ConvertSel = GeomNewMethod("anyntonoff", toNoffDefault);
    GeomSpecifyMethod(ConvertSel, NDMeshMethods(), toNoffNDMesh);
    GeomSpecifyMethod(ConvertSel, SkelMethods(), toNoffSkel);
    GeomSpecifyMethod(ConvertSel, NPolyListMethods(), toNoffNPolyList);
    GeomSpecifyMethod(ConvertSel, PolyListMethods(), toNoffPolyList);
    GeomSpecifyMethod(ConvertSel, ListMethods(), toNoffList);
    GeomSpecifyMethod(ConvertSel, InstMethods(), toNoffInst);
  }
  return (Geom *) GeomCall(ConvertSel, g, t);
}
