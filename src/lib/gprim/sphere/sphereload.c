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

#include <stdio.h>
#include <string.h>
#include "geom.h"
#include "geomclass.h"
#include "transform.h"
#include "sphereP.h"

static const char *texmap[] = {
  "SINUSOIDAL", "CYLINDRICAL", "RECTANGULAR", "STEREOGRAPHIC", "ONEFACE", NULL
};

Sphere *SphereFLoad(IOBFILE *file, char *fname)
{
  HPoint3 center;
  float radius;
  Geom *sphere;
  char *token;
  int space, i, c;
  unsigned txmapmeth = SPHERE_TXCYLINDRICAL;
  bool tex = false;

  if (file == NULL) return NULL;

  space = TM_EUCLIDEAN;

  token = GeomToken(file);
  if (token[0] == 'S' && token[1] == 'T') {
    tex = true;
    token += 2;
  }
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
  if(strcmp(token, "SPHERE") != 0) {
    return NULL;
  }

  GeomAcceptToken();
  if (tex) {
    c = iobfnextc(file, 0);
    for (i = 0; texmap[i] != NULL; i++) {
      if (c == (int)texmap[i][0]) {
	break;
      }
    }
    if (texmap[i] != NULL) {
      token = GeomToken(file);
      for (i = 0; texmap[i] != NULL; i++) {
	if (strcmp(texmap[i], token) == 0) {
	  break;
	}
      }
      if (texmap[i] == NULL) {
	OOGLSyntax(file, "%s: SPHERE: expected texture mapping method", fname);
	return NULL;
      }
      txmapmeth = GEOMFLAG((i+1) << 1);
    }
  }

  if (iobfgetnf(file, 1, &radius, 0) != 1 ||
      iobfgetnf(file, 3, &center.x, 0) != 3) {
    OOGLSyntax(file, "%s: SPHERE: expected radius, then x y z", fname);
    return NULL;
  }

  center.w = 1.0;

  sphere = GeomCreate("sphere", CR_RADIUS, (double)radius, 
		      CR_CENTER, &center, CR_SPACE, space,
		      tex ? CR_SPHERETX : CR_END, txmapmeth,
		      CR_END);

  return (Sphere *)sphere;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
