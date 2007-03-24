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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef alloca
#include <alloca.h>
#endif
#include <stdlib.h>
#include "meshP.h"
#include "mgP.h"
#include "hpointn.h"
#include "cmodel.h"
#include "bsptreeP.h"

static int
draw_projected_mesh(mgNDctx *NDctx, Mesh *mesh)
{
  Mesh m = *mesh;
  HPointN *h;
  HPoint3 *op, *np;
  int i, colored = 0, alpha = 0;
  int npts = m.nu * m.nv;
  mgNDmapfunc mapHPtN = NDctx->mapHPtN;
  const Appearance *ap = &_mgc->astk->ap;
  const Material *mat = &_mgc->astk->mat;
  int normal_need;

  m.p  = (HPoint3 *)alloca(npts*sizeof(HPoint3));
  m.n  = NULL;
  m.nq = NULL;
  m.c  = (ColorA *)alloca(npts*sizeof(ColorA));
  m.ap = NULL;
  RefInit((Ref *)(void *)&m, mesh->magic);
  DblListInit(&m.pernode);

  h = HPtNCreate(5, NULL);
  if (ap->flag & APF_KEEPCOLOR) {
    colored = 0;
  } else {
    HPoint3 dummyv;
    ColorA dummyc;
    /* Dummy transform to determine whether we have ND colors or not */
    colored = mapHPtN(NDctx, h, &dummyv, &dummyc);
  }

  m.geomflags &= ~VERT_4D;
  for(i = 0, op = mesh->p, np = m.p; i < npts; i++, op++, np++) {
    if (mesh->geomflags & VERT_4D) {
      /* Set the point's first four components from our 4-D mesh vertex */
      Pt4ToHPtN(op, h);
    } else {
      /* Set the point's first THREE components from our 4-D mesh vertex */
      HPt3ToHPtN(op, NULL, h);
    }
    if (colored) {
      mapHPtN(NDctx, h, np, &m.c[i]);
      if (m.c[i].a < 1.0) {
	alpha = 1;
      }
    } else {
      mapHPtN(NDctx, h, np, NULL);
    }
  }

  if (colored) {
    if (alpha) {
      m.geomflags |= COLOR_ALPHA;
    } else {
      m.geomflags &= ~COLOR_ALPHA;
    }
    m.geomflags |= MESH_C;
  }

  /* The drawing routines might need either polygon or vertex normals,
   * so if either is missing and either might be needed, we force it
   * to be computed.
   */
  m.geomflags &= ~(MESH_N|MESH_NQ);
  normal_need = (ap->flag & APF_NORMALDRAW) ? MESH_N|MESH_NQ : 0;
  if (ap->flag & APF_FACEDRAW) {
    if (ap->shading == APF_FLAT) {
      normal_need |= MESH_NQ;
    }
    if (ap->shading == APF_SMOOTH) {
      normal_need |= MESH_N;
    }
    if (GeomHasAlpha((Geom *)(void *)&m, ap)) {
      /* could re-use per quad normals here */
    }
  }
  MeshComputeNormals(&m, normal_need);

  if(_mgc->astk->flags & MGASTK_SHADER) {
    ColorA *c = colored ? m.c : (mat->override & MTF_DIFFUSE) ? NULL : mesh->c;
    if(c) {
      (*_mgc->astk->shader)(npts, m.p, m.n ? m.n : m.nq, c, m.c);
    } else {
      for(i = 0; i < npts; i++) {
	(*_mgc->astk->shader)(1, m.p + i, m.n + i,
			      (ColorA *)&_mgc->astk->mat.diffuse, m.c + i);
      }
    }
    colored = 1;
  }
  mgmeshst(MESH_MGWRAP(m.geomflags), m.nu, m.nv, m.p, m.n, m.nq,
	   colored ? m.c : mesh->c, mesh->u, m.geomflags);

  /* Generate a BSP-tree if the object or parts of it might be
   * translucent.
   */
  if (NDctx->bsptree && (m.geomflags & GEOM_ALPHA)) {
    GeomNodeDataMove((Geom *)mesh, (Geom *)&m);
    GeomBSPTree((Geom *)&m, NDctx->bsptree, BSPTREE_ADDGEOM);
    GeomNodeDataMove((Geom *)&m, (Geom *)mesh);
  }

  if (m.n) {
    OOGLFree(m.n);
  }
  if (m.nq) {
    OOGLFree(m.nq);
  }
  HPtNDelete(h);

  return 0;
}

Mesh *
MeshDraw(Mesh *mesh)
{
  mgNDctx *NDctx = NULL;

  /* We pass mesh->flag verbatim to mgmesh() -- MESH_[UV]WRAP == MM_[UV]WRAP */

  mgctxget(MG_NDCTX, &NDctx);

  if(NDctx) {
    draw_projected_mesh(NDctx, mesh);
    return mesh;
  }

  if ((mesh->geomflags & (MESH_N|MESH_NQ)) != (MESH_N|MESH_NQ)) {
    const Appearance *ap = &_mgc->astk->ap;
    int need = 0;
      
    if (ap->flag & APF_NORMALDRAW) {
      need = MESH_N|MESH_NQ;
    } else if (ap->flag & APF_FACEDRAW) {
      if (ap->shading == APF_FLAT) {
	need |= MESH_NQ;
      }
      if (ap->shading == APF_SMOOTH) {
	need |= MESH_N;
      }
    }
    MeshComputeNormals(mesh, need);
  }

  if (_mgc->space & TM_CONFORMAL_BALL) {
    cmodel_clear(_mgc->space);
    cm_draw_mesh(mesh);
    return mesh;
  } else if(_mgc->astk->flags & MGASTK_SHADER) {
    int i, npts = mesh->nu * mesh->nv;
    ColorA *c = (ColorA *)alloca(npts * sizeof(ColorA));
    if(mesh->c && !(_mgc->astk->mat.override & MTF_DIFFUSE)) {
      (*_mgc->astk->shader)(npts, mesh->p, mesh->n, mesh->c, c);
    } else {
      for(i = 0; i < npts; i++) {
	(*_mgc->astk->shader)(1, mesh->p + i, mesh->n + i,
			      (ColorA *)&_mgc->astk->mat.diffuse, c + i);
      }
    }
    mgmeshst(MESH_MGWRAP(mesh->geomflags), mesh->nu, mesh->nv, mesh->p,
	     mesh->n, mesh->nq, c, mesh->u, mesh->geomflags | MESH_C);
  } else {
    mgmeshst(MESH_MGWRAP(mesh->geomflags), mesh->nu, mesh->nv, mesh->p,
	     mesh->n, mesh->nq, mesh->c, mesh->u, mesh->geomflags);
  }

  return mesh;
}

Mesh *MeshBSPTree(Mesh *mesh, BSPTree *tree, int action)
{
  if (!never_translucent((Geom *)mesh) && action == BSPTREE_ADDGEOM) {
    BSPTreeAddObject(tree, (Geom *)mesh);
  }

  return mesh;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
