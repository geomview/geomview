/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2007 Claus-Justus Heine
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

#include <stdio.h>
#include "transform.h"
#include "geom.h"
#include "geomclass.h"
#include "sphereP.h"

static const char *texmap[] = {
  "SINUSOIDAL", "CYLINDRICAL", "RECTANGULAR", "STEREOGRAPHIC", "ONEFACE"
};

Sphere *SphereFSave(Sphere *sphere, FILE *f, char *fname)
{
  int texmeth = 0;

  (void)fname;

  if (sphere == NULL) {
    return NULL;
  }
  if ((texmeth = SPHERE_TXMETH(sphere->geomflags)) != 0) {
    fprintf(f, "ST");
  }
  switch(sphere->space) {
  case TM_HYPERBOLIC:
    fprintf(f, "%c", 'H');
    break;
  case TM_SPHERICAL:
    fprintf(f, "%c", 'S');
    break;
  }
  fprintf(f, "SPHERE");
  if (texmeth != 0) {
    fprintf(f, " %s\n", texmap[texmeth-1]);
  } else {
    fprintf(f, "\n");
  }
  fprintf(f, "%g %g %g %g\n", sphere->radius, sphere->center.x, 
	  sphere->center.y, sphere->center.z);
  return (ferror(f) ? NULL : sphere);
}
