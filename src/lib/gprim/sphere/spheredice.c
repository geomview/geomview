/* Copyright (C) 2007 Claus-Justus Heine
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

#include "sphereP.h"

Sphere *SphereDice(Sphere *sphere, int nu, int nv)
{
  if (nu < 2) nu = SPHERE_DEFAULT_MESH_SIZE;
  if (nv < 2) nv = SPHERE_DEFAULT_MESH_SIZE;
  if (nu != sphere->ntheta || nv != sphere->nphi) {
    sphere->geomflags |= SPHERE_REMESH;
    sphere->ntheta = nu;
    sphere->nphi = nv;
  }
  return sphere;
}

void SphereReDice(Sphere *sphere)
{
  Geom *facet;
  Point3 *spherepoints;
  Point3 *spherenormals;
  TxST *spheretexcoord = NULL;
  int npts;
  float theta, phi, thetafrac, phifrac, x, y, z, r;
  float phiscale, thetascale, thetastart;
  int i, j, ptno, nphi, ntheta;
  
  nphi       = sphere->nphi;
  phiscale   = 0.25;
  ntheta     = sphere->ntheta;
  thetascale = 0.5;
  thetastart = 0;

  switch (sphere->geomflags & SPHERE_TXMASK) {
  case SPHERE_TXSTEREOGRAPHIC:
    ntheta = 2*sphere->ntheta; thetastart = -0.5; thetascale = 1.0;
    break;
  case SPHERE_TXSINUSOIDAL:
    nphi = 4*sphere->nphi; phiscale = 1.0;
    break;
  }

  npts = nphi * ntheta;
  spherepoints = OOGLNewNE(Point3, npts, "sphere mesh points");
  spherenormals = OOGLNewNE(Point3, npts, "sphere mesh normals");
  if ((sphere->geomflags & SPHERE_TXMASK) != SPHERE_TXNONE) {
    spheretexcoord = OOGLNewNE(TxST, npts, "sphere texture coords");
  }

  /* Generate the part of the sphere in the positive quadrant, after
   * translating into the center-of-mass co-ordinate system.
   *
   * The +z pole is at \theta = +\pi/2. The minimal resolution will
   * yield an Octahedron.
   */
  for (ptno = j = 0; j < ntheta; j++) {
    thetafrac = thetascale * (float)j / (float)(ntheta-1);
    theta = (thetastart + thetafrac) * M_PI;
    r = cos(theta);
    z = sin(theta);
    for (i = 0; i < nphi; i++) {
      phifrac = phiscale * (float)i / (float)(nphi-1);
      phi = 2.0 * phifrac * M_PI;
      spherenormals[ptno].x = x = cos(phi) * r;
      spherenormals[ptno].y = y = sin(phi) * r;
      spherenormals[ptno].z = z;
      Pt3Copy(spherenormals + ptno, spherepoints + ptno);
      Pt3Mul(sphere->radius, spherepoints + ptno, spherepoints + ptno);
      switch (sphere->geomflags & SPHERE_TXMASK) {
      case SPHERE_TXNONE:
	break;
      case SPHERE_TXSINUSOIDAL:
	spheretexcoord[ptno].s = 0.5 + r * (phifrac - 0.5);
	spheretexcoord[ptno].t = 0.5 + thetafrac;
	break;
      case SPHERE_TXCYLINDRICAL:
	spheretexcoord[ptno].s = phifrac;
	spheretexcoord[ptno].t = 0.5 + thetafrac;
	break;
      case SPHERE_TXRECTANGULAR:
	spheretexcoord[ptno].s = phifrac;
	spheretexcoord[ptno].t = 0.5 * (z + 1.0);
	break;
      case SPHERE_TXSTEREOGRAPHIC:
	spheretexcoord[ptno].s = 0.5 + x / (1.0 + (z < -0.9999 ? -0.9999 : z));
	spheretexcoord[ptno].t = 0.5 + y / (1.0 + (z < -0.9999 ? -0.9999 : z));
	break;
      case SPHERE_ONEFACE:
	spheretexcoord[ptno].s = 0.5 * (x + 1.0);
	spheretexcoord[ptno].t = 0.5 * (z + 1.0);
	break;
      }
      ++ptno;
    }
  }

  facet = GeomCCreate(NULL, MeshMethods(),
		      CR_NOCOPY,
		      CR_NV, ntheta,
		      CR_NU, nphi,
		      CR_POINT, spherepoints,
		      CR_NORMAL, spherenormals,
		      spheretexcoord ? CR_U : CR_END, spheretexcoord,
		      CR_END);
  
  if (facet == NULL) {
    OOGLError(1, "SphereReDice: can't create Mesh");
  }

  sphere->geom = facet;
  HandleSetObject(sphere->geomhandle, (Ref *)facet);

  sphere->geomflags &= ~SPHERE_REMESH;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
