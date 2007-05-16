/* Copyright (C) 2006 Claus-Justus Heine
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

#include "ndmeshP.h"
#include "pickP.h"
#include "appearance.h"

/* from meshP.h, works only has long as mdim is fixed at 2 */
#define MESHINDEX(u, v, mesh) \
  (((v)%(mesh)->mdim[1])*(mesh)->mdim[0] + ((u)%(mesh)->mdim[0]))

#define MESHPOINT(u, v, mesh, plist) ((plist)[MESHINDEX(u, v, mesh)])

NDMesh *
NDMeshPick(NDMesh *mesh, Pick *pick, Appearance *ap,
	   Transform T, TransformN *TN, int *axes)
{
  Point3 plist[4];
  int nu, nv, maxnu, maxnv;
  int foundu, foundv;
  unsigned int apflag = 0;
  HPt3Coord xa, xb, xc, xd;

  if (!TN)
    return NULL; /* no 3d pick for ND object. */

  if (mesh->meshd > 2)
    return NULL;

  foundu = foundv = -1;

  maxnu = mesh->mdim[0];
  maxnv = mesh->mdim[1];

  /* Make sure that vects do not as visible - otherwise they
   * will wreak havoc with the edge picking stuff. */
  if (ap != NULL) {
    apflag = ap->flag;
    ap->flag &= ~APF_VECTDRAW;
  }

  for (nv = 0; nv < ((mesh->geomflags & MESH_VWRAP) ? maxnv : maxnv-1); nv++) {
    for (nu =0; nu < ((mesh->geomflags & MESH_UWRAP) ? maxnu : maxnu-1); nu++) {
      /* Demand that at least 1 of the vertices be in front of the viewer.
       * Note: we must ensure that all four are evaluated!
       */
      xa = HPtNNTransPt3(TN, axes, MESHPOINT(nu, nv, mesh, mesh->p),
			 &plist[0]);
      xb = HPtNNTransPt3(TN, axes, MESHPOINT(nu+1, nv, mesh, mesh->p),
			 &plist[1]);
      xc = HPtNNTransPt3(TN, axes, MESHPOINT(nu+1, nv+1, mesh, mesh->p),
			 &plist[2]);
      xd = HPtNNTransPt3(TN, axes, MESHPOINT(nu, nv+1, mesh, mesh->p),
			 &plist[3]);
      if((0 < xa) || (0 < xb) || (0 < xc) || (0 < xd)) {
	if (PickFace(4, plist, pick, ap)) {
	  foundu = nu;
	  foundv = nv;
	}
      }
    }
  }

  if (ap != NULL) ap->flag = apflag;

  if (foundu == -1) return NULL;

  if (pick->found & PW_VERT) {
    pick->vi = MESHINDEX(foundu + ((pick->vi == 1 || pick->vi == 2) ? 1 : 0), 
			 foundv + pick->vi/2, mesh);
    HPtNTransformComponents(TN, mesh->p[pick->vi], axes, &pick->v);
  }
  if (pick->found & PW_EDGE) {
    pick->ei[0] = 
      MESHINDEX(foundu + ((pick->ei[0] == 1 || pick->ei[0] == 2) ? 1 : 0), 
		foundv + pick->ei[0]/2, mesh);
    pick->ei[1] = 
      MESHINDEX(foundu + ((pick->ei[1] == 1 || pick->ei[1] == 2) ? 1 : 0), 
		foundv + pick->ei[1]/2, mesh);
    HPtNTransformComponents(TN, mesh->p[pick->ei[0]], axes, &pick->e[0]);
    HPtNTransformComponents(TN, mesh->p[pick->ei[1]], axes, &pick->e[1]);
  }
  if (pick->found & PW_FACE) {
    if(pick->f) OOGLFree(pick->f);
    pick->f = OOGLNewNE(HPoint3, 4, "Mesh pick");
    pick->fi = MESHINDEX(foundu, foundv, mesh);
    HPtNTransformComponents(TN, MESHPOINT(foundu, foundv, mesh, mesh->p),
			    axes, &pick->f[0]);
    HPtNTransformComponents(TN, MESHPOINT(foundu+1, foundv, mesh, mesh->p),
			    axes, &pick->f[1]);
    HPtNTransformComponents(TN, MESHPOINT(foundu+1, foundv+1, mesh, mesh->p),
			    axes, &pick->f[2]);
    HPtNTransformComponents(TN, MESHPOINT(foundu, foundv+1, mesh, mesh->p),
			    axes, &pick->f[3]);
  }

  if (TN) {
    pick->TprimN = TmNCopy(TN, pick->TprimN);
    memcpy(pick->axes, axes, sizeof(pick->axes));
  } else
    TmCopy(T, pick->Tprim);

  return mesh;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
