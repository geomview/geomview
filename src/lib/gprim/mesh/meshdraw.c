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
  Appearance *ap = &_mgc->astk->ap;
  Material *mat = &_mgc->astk->mat;
  int normal_need;

  m.p  = (HPoint3 *)alloca(npts*sizeof(HPoint3));
  m.n  = NULL;
  m.nq = NULL;
  m.c  = (ColorA *)alloca(npts*sizeof(ColorA));
  m.bsptree = NULL;

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
    m.flag &= ~MESH_ALPHA;
    m.flag |= alpha ? MESH_C|MESH_ALPHA : MESH_C;
  }

  /* The drawing routines might need either polygon or vertex normals,
   * so if either is missing and either might be needed, we force it
   * to be computed.
   */
  m.flag &= ~(MESH_N|MESH_NQ);
  normal_need = (ap->flag & APF_NORMALDRAW) ? MESH_N|MESH_NQ : 0;
  if (ap->flag & APF_FACEDRAW) {
    if (ap->shading == APF_FLAT) {
      normal_need |= MESH_NQ;
    }
    if (ap->shading == APF_SMOOTH) {
      normal_need |= MESH_N;
    }
    if (ap->flag & APF_TRANSP) {
      if ((mat->override & MTF_ALPHA) && (mat->valid & MTF_ALPHA)) {
	if (mat->diffuse.a != 1.0) {
	  m.flag |= MESH_ALPHA;
	} else {
	  m.flag &= ~MESH_ALPHA;
	}
      }
    }
  }
  MeshComputeNormals(&m, normal_need);

  /* Generate a BSP-tree if the object or parts of it might be
   * translucent.
   */
  if ((ap->flag & APF_FACEDRAW) &&
      (ap->flag & APF_TRANSP) &&
      (m.flag & MESH_ALPHA)) {
    BSPTreeCreate((Geom *)(void *)&m);
    BSPTreeAddObject(m.bsptree, (Geom *)(void *)&m);
    BSPTreeFinalize(m.bsptree);
  }

  if(_mgc->astk->useshader) {
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
  mgmeshst(m.flag, m.nu, m.nv, m.p, m.n, m.nq,
	   colored ? m.c : mesh->c, mesh->u, m.flag);

  if (m.bsptree) {
    mgbsptree(m.bsptree);
    BSPTreeFree((Geom *)(void *)&m);
  }

  if (m.n)
    OOGLFree(m.n);
  if (m.nq)
    OOGLFree(m.nq);
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

  if ((mesh->flag & (MESH_N|MESH_NQ)) != (MESH_N|MESH_NQ)) {
    Appearance *ap = mggetappearance();
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

  if (mesh->bsptree == NULL) {
    /* This means we are a top-level drawing node (will never happen ...) */
    BSPTreeCreate((Geom *)mesh);
  }
  if (mesh->bsptree->tree == NULL) {
    /* This means we are an inferior drawing node and may add our
     * polygons to the tree, do that.
     */
    BSPTreeAddObject(mesh->bsptree, (Geom *)mesh);
  }

  if (_mgc->space & TM_CONFORMAL_BALL) {
    cmodel_clear(_mgc->space);
    cm_draw_mesh(mesh);
    return mesh;
  } else if(_mgc->astk->useshader) {
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
    mgmeshst(mesh->flag | MESH_C, mesh->nu, mesh->nv, mesh->p,
	     mesh->n, mesh->nq, c, mesh->u, mesh->flag);
  } else {
    mgmeshst(mesh->flag, mesh->nu, mesh->nv, mesh->p,
	     mesh->n, mesh->nq, mesh->c, mesh->u, mesh->flag);
  }

  /* If we have a private BSP-tree, then draw it now. Software shading
   * & transparency will not work, to be fixed.
   */
  if (mesh->bsptree->geom == (Geom *)mesh) {
    if (mesh->bsptree->tree == NULL) {
      BSPTreeFinalize(mesh->bsptree);
    }
    mgbsptree(mesh->bsptree);
  }

  return mesh;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
