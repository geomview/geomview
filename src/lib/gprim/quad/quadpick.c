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

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips,
 * Celeste Fowler */

#include <math.h>
#include "quadP.h"
#include "hpoint3.h"
#include "point3.h"
#include "pickP.h"
#include "appearance.h"


Quad *
QuadPick(Quad *q, Pick *pick, Appearance *ap, Transform T)
{
  Point3 plist[4], *p, got, ep;
  int i, v, e, maxpt;
  float closest;
  int found;
  unsigned int apflag;
  HPt3Coord xa, xb, xc, xd;

  found = -1;

  /* Make sure that vectors do not appear visible to the pick code - 
   * otherwise they will screw up edge picking */
  if (ap != NULL) {
    apflag = ap->flag;
    ap->flag &= ~APF_VECTDRAW;
  }

  for (i = 0; i < q->maxquad; i++) { 
    xa = HPt3TransPt3(T, &q->p[i][0], &plist[0]);
    xb = HPt3TransPt3(T, &q->p[i][1], &plist[1]);
    xc = HPt3TransPt3(T, &q->p[i][2], &plist[2]);
    xd = HPt3TransPt3(T, &q->p[i][3], &plist[3]);
    /* Note -- we need to be sure that all four are evaluated!! */
    if((xa > 0) || (xb > 0) || (xc > 0) || (xd > 0)) {
	if(PickFace(4, plist, pick, ap))
	    found = i;
    }
  }

  if (ap != NULL) ap->flag = apflag;
  
  if (found == -1) return NULL;

  if (pick->found & PW_VERT) {
    HPt3Transform(T, &q->p[found][pick->vi], &pick->v);
    pick->vi = found*4 + pick->vi;
  }
  if (pick->found & PW_EDGE) {
    HPt3Transform(T, &q->p[found][pick->ei[0]], &pick->e[0]);
    HPt3Transform(T, &q->p[found][pick->ei[1]], &pick->e[1]);
    pick->ei[0] = found*4 + pick->ei[0];
    pick->ei[1] = found*4 + pick->ei[1];
  }
  if (pick->found & PW_FACE) {
    if(pick->f) OOGLFree(pick->f);
    pick->f = OOGLNewNE(HPoint3, 4, "Quad pick");
    HPt3TransformN(T, q->p[found], pick->f, 4);
    pick->fi = found;
  }

  TmCopy(T, pick->Tprim);

  return q;
}
