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

#include "ndmeshP.h"
#include "mesh.h"	/* plain old 3-D mesh */
#include "mgP.h"

#include "meshP.h"
#include "mg.h"
#include "hpointn.h"
#include "bsptreeP.h"
#include <stdlib.h>

static void
draw_projected_ndmesh(mgNDctx *NDctx, NDMesh *mesh)
{
  Mesh m;
  HPointN **op;
  HPoint3 *np;
  int i, colored = 0, alpha = 0;
  int npts = mesh->mdim[0] * mesh->mdim[1];
  mgNDmapfunc mapHPtN = NDctx->mapHPtN;
  Appearance *ap = &_mgc->astk->ap;
  Material *mat = &_mgc->astk->mat;
  int normal_need;

  memset(&m, 0, sizeof(m));
  GGeomInit((Geom *)(void *)&m, MeshMethods(), MESHMAGIC, NULL);
  m.p = OOGLNewNE(HPoint3, npts, "projected points");
  m.n = NULL;
  m.c = OOGLNewNE(ColorA, npts, "ND colors");
  m.nu = mesh->mdim[0];
  m.nv = mesh->mdim[1];
  m.geomflags = mesh->geomflags & ~MESH_4D;

  if (ap->flag & APF_KEEPCOLOR) {
    colored = 0;
  } else {
    HPoint3 dummyv;
    ColorA dummyc;
    /* Dummy transform to determine whether we have ND colors or not */
    colored = mapHPtN(NDctx, *mesh->p, &dummyv, &dummyc);
  }

  for(i = 0, op = mesh->p, np = m.p; i < npts; i++, op++, np++) {
    /* Set the point's first four components from our N-D mesh vertex */
    if (colored) {
      mapHPtN(NDctx, *op, np, &m.c[i]);
      if (m.c[i].a < 1.0) {
	alpha = 1;
      }
    } else {
      mapHPtN(NDctx, *op, np, NULL);
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
  normal_need = 0;    
  m.geomflags &= ~(MESH_N|MESH_NQ);
  if (ap->flag & APF_NORMALDRAW) {
    normal_need = MESH_N|MESH_NQ;
  } else if (ap->flag & APF_FACEDRAW) {
    switch (ap->shading) {
    case APF_FLAT:
    case APF_VCFLAT: normal_need |= MESH_NQ; break;
    case APF_SMOOTH: normal_need |= MESH_N; break;
    default: break;
    }
    if (GeomHasAlpha(MeshGeom(&m), ap)) {
      /* could re-use per quad normals here */
    }
  }
  if (normal_need) {
    MeshComputeNormals(&m, normal_need);
  }

  if(_mgc->astk->flags & MGASTK_SHADER) {
    ColorA *c = colored ? m.c : (mat->override & MTF_DIFFUSE) ? NULL : mesh->c;
    if(c) {
      (*_mgc->astk->shader)(npts, m.p, m.n ? m.n : m.nq, c, mesh->c);
    } else {
      for(i = 0; i < npts; i++) {
	(*_mgc->astk->shader)(1, m.p + i, m.n + i,
			      (ColorA *)&_mgc->astk->mat.diffuse, m.c + i);
      }
    }
    colored = 1;
  }

  mgmesh(MESH_MGWRAP(m.geomflags),
	 m.nu, m.nv, m.p, m.n, m.nq, colored ? m.c : mesh->c,
	 m.geomflags);

  /* Generate a BSP-tree if the object or parts of it might be
   * translucent.
   */
  if (NDctx->bsptree && (m.geomflags & GEOM_ALPHA)) {
    GeomNodeDataMove(NDMeshGeom(mesh), MeshGeom(&m));
    GeomBSPTree(MeshGeom(&m), NDctx->bsptree, BSPTREE_ADDGEOM);
    GeomNodeDataMove(MeshGeom(&m), NDMeshGeom(mesh));
  }

  if (m.n) {
    OOGLFree(m.n);
  }
  if (m.nq) {
    OOGLFree(m.nq);
  }
  OOGLFree(m.p);
  OOGLFree(m.c);
}

NDMesh *
NDMeshDraw(NDMesh *mesh)
{
  mgNDctx *NDctx = NULL;

  mgctxget(MG_NDCTX, &NDctx);

  if(NDctx) {
    draw_projected_ndmesh(NDctx, mesh);
  }

  return mesh;
}

/* A dummy, just to make GeomBSPTree() not bail out. */
NDMesh *NDMeshBSPTree(NDMesh *mesh, BSPTree *tree, int action)
{
  (void)tree;
  (void)action;
  return mesh;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
