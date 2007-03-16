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
  Geom *quadrant;
  Point3 *spherepoints;
  Point3 *spherenormals;
  Point3 *spheretexcoord;
  Point3 center;
  int npts = sphere->nphi * sphere->ntheta;
  float theta, phi;
  int i, j, ptno;
  
  spherepoints = OOGLNewNE(Point3, npts, "sphere mesh points");
  spherenormals = OOGLNewNE(Point3, npts, "sphere mesh normals");
  spheretexcoord = OOGLNewNE(Point3, npts, "sphere texture coords");

  HPt3ToPt3(&sphere->center, &center);

  /* Generate the part of the sphere in the positive quadrant, after
   * translating into the center-of-mass co-ordinate system.
   *
   * The +z pole is at \theta = +\pi/2. The minimal resolution will
   * yield an Octahedron.
   */
  for (ptno = j = 0; j < sphere->ntheta; j++) {
    theta = (float)j * (float)M_PI_2/(float)(sphere->ntheta-1);
    for (i = 0; i < sphere->nphi; i++) {
      phi = (float)i * (float)M_PI_2/(float)(sphere->nphi-1);
      spherenormals[ptno].x = cos(phi) * cos(theta);
      spherenormals[ptno].y = sin(phi) * cos(theta);
      spherenormals[ptno].z = sin(theta);
      Pt3Comb(1.0, &center, sphere->radius, spherenormals + ptno, 
	      spherepoints + ptno);
      spheretexcoord[ptno].x = 0.25 + phi / 2.0 / M_PI;
      spheretexcoord[ptno].y = 0.5 + theta / M_PI ;
      spheretexcoord[ptno].z = 0.0;
      ++ptno;
    }
  }

  quadrant = GeomCCreate(NULL, MeshMethods(),
			 CR_NOCOPY,
			 CR_NU, sphere->ntheta,
			 CR_NV, sphere->nphi,
			 CR_POINT, spherepoints,
			 CR_NORMAL, spherenormals,
			 CR_U, spheretexcoord,
			 CR_END);

  if (quadrant == NULL) {
    OOGLError(1, "SphereReDice: can't create Mesh");
  }

  HandleSetObject(sphere->geomhandle, (Ref *)quadrant);
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
