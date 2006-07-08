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

#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
#include "config.h"
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
#include "mgP.h"
#include "hpointn.h"
#include <stdlib.h>
#ifndef alloca
#include <alloca.h>
#endif

static void
draw_projected_ndmesh(mgmapfunc NDmap, void *NDinfo, NDMesh *mesh)
{
    Mesh m;
    HPointN **op;
    HPoint3 *np;
    int i, colored = 0;
    int npts = mesh->mdim[0] * mesh->mdim[1];

    memset(&m, 0, sizeof(m));
    m.p = (HPoint3 *)alloca(npts*sizeof(HPoint3));
    m.n = NULL;
    m.c = (ColorA *)alloca(npts*sizeof(ColorA));
    m.nu = mesh->mdim[0];
    m.nv = mesh->mdim[1];
    m.flag = mesh->flag & ~MESH_4D;
    for(i = 0, op = mesh->p, np = m.p; i < npts; i++, op++, np++) {
	/* Set the point's first four components from our N-D mesh vertex */
	colored = (*NDmap)(NDinfo, *op, np, &m.c[i]);
    }
    if(colored) m.flag |= MESH_C;
    MeshComputeNormals(&m);
    mgmesh(m.flag, m.nu, m.nv, m.p, m.n, colored ? m.c : mesh->c);
    OOGLFree(m.n);
}

NDMesh *
NDMeshDraw(register NDMesh *mesh)
{
  Transform T;
  float focallen;

  if(_mgc->NDinfo) {
    mgpushtransform();
    CamGet(_mgc->cam, CAM_FOCUS, &focallen);
    TmTranslate(T, 0., 0., -focallen);
    TmConcat(T, _mgc->C2W, T);
    mgsettransform(T);
    draw_projected_ndmesh(_mgc->NDmap, _mgc->NDinfo, mesh);
    mgpoptransform();
  }
  return mesh;
}


