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

#include <stdio.h>
#include <string.h>
#include "geom.h"
#include "geomclass.h"
#include "transform.h"
#include "sphereP.h"

Sphere *SphereFLoad(file, fname) 
     FILE *file;
     char *fname;
{
  HPoint3 center;
  float radius;
  Geom *sphere;
  char *token;
  int space;

  if (file == NULL) return NULL;

  space = TM_EUCLIDEAN;

  token = GeomToken(file);
  switch(*token) {
  case 'E': space = TM_EUCLIDEAN; token++; break;
  case 'H': space = TM_HYPERBOLIC; token++; break;
  case 'S':
    if(token[1] == 'S') {
	space = TM_SPHERICAL;
	token++;
    }
    break;
  }
  if(strcmp(token, "SPHERE") != 0)
    return NULL;

  if ( fgetnf(file, 1, &radius, 0) != 1 || fgetnf(file, 3, &center.x, 0) != 3 ) {
    OOGLSyntax(file, "%s: SPHERE: expected radius, then x y z", fname);
    return NULL;
  }

  center.w = 1.0;

  sphere = GeomCreate("sphere", CR_RADIUS, (double)radius, 
		      CR_CENTER, &center, CR_SPACE, space, CR_END);

  return (Sphere *)sphere;
}
