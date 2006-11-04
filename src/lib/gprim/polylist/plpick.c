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
 * Celeste Fowler */

#include <math.h>
#include "ooglutil.h"
#include "polylistP.h"
#include "point3.h"
#include "hpoint3.h"
#include "polyint.h"
#include "pickP.h"
#include "appearance.h"

PolyList *
PolyListPick(PolyList *pl, Pick *p, Appearance *ap,
	     Transform T, TransformN *TN, int *axes)
{
  int i;
  vvec plist;
  int fi, vi, ok, found = -1, v4d;
  Poly *poly;
  unsigned int apflag = 0;

  /* Make sure that vects do not count as visible - otherwise they
   * will wreak havoc with the edge picking stuff. */
  if (ap != NULL) {
    apflag = ap->flag;
    ap->flag &= ~APF_VECTDRAW;
  }
  
  v4d = (pl->geomflags & VERT_4D) != 0;

  VVINIT(plist, Point3, 0);

  found = -1;
  for (fi=0, poly=pl->p; fi<pl->n_polys; ++fi, ++poly) {
    vvneeds(&plist, poly->n_vertices);
    ok = 0;
    if (TN) {
      for (vi=0; vi<poly->n_vertices; vi++) {
	if (0 < HPt3NTransPt3(TN, axes, &poly->v[vi]->pt, v4d,
			      VVINDEX(plist, Point3, vi)))
	  ok = 1;
      }
    } else {
      for (vi=0; vi<poly->n_vertices; vi++) {
	if (0 < HPt3TransPt3(T, &poly->v[vi]->pt, VVINDEX(plist, Point3, vi)))
	  ok = 1;
      }
    }
    if (ok && PickFace(poly->n_vertices, VVINDEX(plist, Point3, 0), p, ap)) 
      found = fi;
  }

  vvfree(&plist);

  if (ap != NULL) ap->flag = apflag;

  if (found == -1) return NULL;

  if (p->found & PW_VERT) {
    p->vi = pl->p[found].v[p->vi] - pl->vl;
    if (TN)
      HPt3NTransHPt3(TN, axes, &pl->vl[p->vi].pt, v4d, &p->v);
    else
      HPt3Transform(T, &pl->vl[p->vi].pt, &p->v);
  }
  if (p->found & PW_EDGE) {
    p->ei[0] = pl->p[found].v[p->ei[0]] - pl->vl;
    p->ei[1] = pl->p[found].v[p->ei[1]] - pl->vl;
    if (TN) {
      HPt3NTransHPt3(TN, axes, &pl->vl[p->ei[0]].pt, v4d, &p->e[0]);
      HPt3NTransHPt3(TN, axes, &pl->vl[p->ei[1]].pt, v4d, &p->e[1]);
    } else {
      HPt3Transform(T, &pl->vl[p->ei[0]].pt, &p->e[0]);
      HPt3Transform(T, &pl->vl[p->ei[1]].pt, &p->e[1]);
    }
  }
  if (p->found & PW_FACE) {
    if(p->f) OOGLFree(p->f);
    p->f = OOGLNewNE(HPoint3, p->fn, "PolyList pick");
    if (TN) {
      for (i = 0; i < p->fn; i++)
	HPt3NTransHPt3(TN, axes, &pl->p[found].v[i]->pt, v4d, &p->f[i]);
    } else {
      for (i = 0; i < p->fn; i++)
	HPt3Transform(T, &pl->p[found].v[i]->pt, &p->f[i]);
    }
    p->fi = found;
  }

  if (TN) {
    p->TprimN = TmNCopy(TN, p->TprimN);
    memcpy(p->axes, axes, sizeof(p->axes));
  } else
    TmCopy(T, p->Tprim);

  return pl;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
