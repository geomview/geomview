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

#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
#include "config.h"
#endif

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Geometry Technologies, Inc.";


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips,
 * Celeste Fowler */

#include "meshP.h"
#include "pickP.h"
#include "appearance.h"

Mesh *
MeshPick(Mesh *mesh, Pick *pick, Appearance *ap, Transform T)
{
  Point3 plist[4];
  int i, nu, nv, maxnu, maxnv, maxpt;
  int foundu, foundv;
  unsigned int apflag;

  HPt3Coord xa, xb, xc, xd;

  foundu = foundv = -1;

  maxnu = mesh->nu;
  maxnv = mesh->nv;
  maxpt = maxnu * maxnv;

  /* Make sure that vects do not register as visible - otherwise they
   * will wreak havoc with the edge picking stuff. */
  if (ap != NULL) {
    apflag = ap->flag;
    ap->flag &= ~APF_VECTDRAW;
  }

  for (nv = 0; nv < ((mesh->flag & MESH_VWRAP) ? maxnv : maxnv - 1); nv++) {
    for (nu = 0; nu < ((mesh->flag & MESH_UWRAP) ? maxnu : maxnu - 1); nu++) {
	/* Demand that at least 1 of the vertices be in front of the viewer.
	 * Note: we must ensure that all four are evaluated!
	 */
      xa = HPt3TransPt3(T, &MESHPOINT(nu, nv, mesh, mesh->p), &plist[0]);
      xb = HPt3TransPt3(T, &MESHPOINT(nu+1, nv, mesh, mesh->p), &plist[1]);
      xc = HPt3TransPt3(T, &MESHPOINT(nu+1, nv+1, mesh, mesh->p), &plist[2]);
      xd = HPt3TransPt3(T, &MESHPOINT(nu, nv+1, mesh, mesh->p), &plist[3]);
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
    HPt3Transform(T, &mesh->p[pick->vi], &pick->v);
  }
  if (pick->found & PW_EDGE) {
    pick->ei[0] = 
      MESHINDEX(foundu + ((pick->ei[0] == 1 || pick->ei[0] == 2) ? 1 : 0), 
		foundv + pick->ei[0]/2, mesh);
    pick->ei[1] = 
      MESHINDEX(foundu + ((pick->ei[1] == 1 || pick->ei[1] == 2) ? 1 : 0), 
		foundv + pick->ei[1]/2, mesh);
    HPt3Transform(T, &mesh->p[pick->ei[0]], &pick->e[0]);
    HPt3Transform(T, &mesh->p[pick->ei[1]], &pick->e[1]);
  }
  if (pick->found & PW_FACE) {
    if(pick->f) OOGLFree(pick->f);
    pick->f = OOGLNewNE(HPoint3, 4, "Mesh pick");
    pick->fi = MESHINDEX(foundu, foundv, mesh);
    HPt3Transform(T, &MESHPOINT(foundu, foundv, mesh, mesh->p), 
		  &pick->f[0]);
    HPt3Transform(T, &MESHPOINT(foundu+1, foundv, mesh, mesh->p), 
		  &pick->f[1]);
    HPt3Transform(T, &MESHPOINT(foundu+1, foundv+1, mesh, mesh->p), 
		  &pick->f[2]);
    HPt3Transform(T, &MESHPOINT(foundu, foundv+1, mesh, mesh->p), 
		  &pick->f[3]);
  }

  TmCopy(T, pick->Tprim);

  return mesh;
}
