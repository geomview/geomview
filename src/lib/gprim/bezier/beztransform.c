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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips,
   Celeste Fowler */

#include "hpoint3.h"
#include "point3.h"
#include "bezierP.h"

Bezier *
BezierTransform(Bezier *b, Transform T, TransformN *dummy)
{
  int i;
  Point3 *p;
  HPoint3 *hp;
  int limit = (b->degree_u + 1)*(b->degree_v + 1);

  (void)dummy;
 
  if (b->CtrlPnts != NULL) {
    if (b->dimn == 3)
      for (i = 0, p = (Point3 *)b->CtrlPnts; i < limit; i++, p++)
	Pt3Transform(T, p, p);
    else if (b->dimn == 4)
      for (i = 0, hp = (HPoint3 *)b->CtrlPnts; i < limit; i++, hp++)
	HPt3Transform(T, hp, hp);
    else {
      OOGLError(1, "Bezier patch of unfamiliar dimensions.");
      return NULL;
    }
  }

  GeomDelete((Geom *)b->mesh);
  b->mesh = NULL;

  return(b);
}

