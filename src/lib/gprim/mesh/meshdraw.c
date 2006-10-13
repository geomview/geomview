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

#include "meshP.h"
#include "mgP.h"
#include "hpointn.h"
#include "cmodel.h"
#include <stdlib.h>
#ifndef alloca
#include <alloca.h>
#endif

static int
draw_projected_mesh(mgmapfunc NDmap, void *NDinfo, Mesh *mesh)
{
  Mesh m = *mesh;
  HPointN *h = HPtNCreate(5, NULL);
  HPoint3 *op, *np;
  int i, colored = 0;
  int npts = m.nu * m.nv;
  m.p = (HPoint3 *)alloca(npts*sizeof(HPoint3));
  m.n = NULL;
  m.c = (ColorA *)alloca(npts*sizeof(ColorA));
  if (mesh->geomflags & VERT_4D) {
    for(i = 0, op = mesh->p, np = m.p; i < npts; i++, op++, np++) {
      /* Set the point's first four components from our 4-D mesh vertex */
      *(HPoint3 *)h->v = *op;
      colored = (*NDmap)(NDinfo, h, np, &m.c[i]);
    }
  } else {
    for(i = 0, op = mesh->p, np = m.p; i < npts; i++, op++, np++) {
      /* Set the point's first four components from our 4-D mesh vertex */
      HPt3Dehomogenize(op, op);
      *(HPoint3 *)h->v = *op;
      colored = (*NDmap)(NDinfo, h, np, &m.c[i]);
    }
  }
  m.flag &= ~MESH_4D;
  if(colored) m.flag |= MESH_C;
  MeshComputeNormals(&m);
  if(_mgc->astk->useshader) {
    ColorA *c = colored ? m.c :
      (_mgc->astk->mat.override & MTF_DIFFUSE) ? NULL : mesh->c;
    if(c) {
      (*_mgc->astk->shader)(npts, mesh->p, mesh->n, c, m.c);
    } else {
      for(i = 0; i < npts; i++) {
	(*_mgc->astk->shader)(1, mesh->p + i, mesh->n + i,
			      (ColorA *)&_mgc->astk->mat.diffuse, m.c + i);
      }
    }
    colored = 1;
  }
  mgmeshst(m.flag, m.nu, m.nv, m.p, m.n, colored ? m.c : mesh->c, mesh->u);
  OOGLFree(m.n);
  HPtNDelete(h);
  return 0;
}

Mesh *
MeshDraw(Mesh *mesh)
{
  /* We pass mesh->flag verbatim to mgmesh() -- MESH_[UV]WRAP == MM_[UV]WRAP */

  if(!(mesh->flag & MESH_N)) {
    Appearance *ap;

    ap = mggetappearance();
    if(ap->shading != APF_CONSTANT || ap->flag & APF_NORMALDRAW)
      MeshComputeNormals(mesh);
  }

  if(_mgc->NDinfo) {
    draw_projected_mesh(_mgc->NDmap, _mgc->NDinfo, mesh);
  } else if (_mgc->space & TM_CONFORMAL_BALL) {
    cmodel_clear(_mgc->space);
    cm_draw_mesh(mesh);
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
    mgmeshst(mesh->flag | MESH_C, mesh->nu, mesh->nv, mesh->p, mesh->n, c, mesh->u);
  } else {
    mgmeshst(mesh->flag, mesh->nu, mesh->nv, mesh->p, mesh->n, mesh->c, mesh->u);
  }
  return mesh;
}


