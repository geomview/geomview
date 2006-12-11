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

#include <math.h>
#include "ooglutil.h"
#include "npolylistP.h"
#include "point3.h"
#include "hpoint3.h"
#include "polyint.h"
#include "pickP.h"
#include "appearance.h"

NPolyList *
NPolyListPick(NPolyList *pl, Pick *p, Appearance *ap,
	      Transform T, TransformN *TN, int *axes)
{
  int i;
  vvec plist;
  int fi, vi, ok, found = -1;
  Poly *poly;
  unsigned int apflag = 0;
  HPointN ptN[1];

  if (!TN)
    return NULL; /* no 3d pick for ND object. */

  /* we have full-featured ND-points here. */
  ptN->flags = 0;
  ptN->dim = pl->pdim;

  /* Make sure that vects do not count as visible - otherwise they
   * will wreak havoc with the edge picking stuff. */
  if (ap != NULL) {
    apflag = ap->flag;
    ap->flag &= ~APF_VECTDRAW;
  }
  
  VVINIT(plist, Point3, 0);

  found = -1;
  for (fi = 0, poly = pl->p; fi < pl->n_polys; ++fi, ++poly) {
    int *idx = pl->vi + pl->pv[fi];
    
    vvneeds(&plist, poly->n_vertices);
    ok = 0;

    for (vi= 0 ; vi < poly->n_vertices; vi++) {
      ptN->v = pl->v + idx[vi]*pl->pdim;
      if (0 < HPtNNTransPt3(TN, axes, ptN, VVINDEX(plist, Point3, vi)))
	ok = 1;
    }
    if (ok && PickFace(poly->n_vertices, VVINDEX(plist, Point3, 0), p, ap)) 
      found = fi;
  }

  vvfree(&plist);

  if (ap != NULL) ap->flag = apflag;

  if (found == -1) return NULL;

  if (p->found & PW_VERT) {
    p->vi = pl->vi[pl->pv[found]+p->vi];
    ptN->v = pl->v + p->vi*pl->pdim;
    HPtNTransformComponents(TN, ptN, axes, &p->v);
  }
  if (p->found & PW_EDGE) {
    p->ei[0] = pl->vi[pl->pv[found]+p->ei[0]];
    p->ei[1] = pl->vi[pl->pv[found]+p->ei[1]];
    ptN->v = pl->v + p->ei[0]*pl->pdim;
    HPtNTransformComponents(TN, ptN, axes, &p->e[0]);
    ptN->v = pl->v + p->ei[1]*pl->pdim;
    HPtNTransformComponents(TN, ptN, axes, &p->e[1]);
  }
  if (p->found & PW_FACE) {
    if(p->f) OOGLFree(p->f);
    p->f = OOGLNewNE(HPoint3, p->fn, "PolyList pick");
    for (i = 0; i < p->fn; i++) {
      ptN->v = pl->v + pl->vi[pl->pv[found]+i]*pl->pdim;
      HPtNTransformComponents(TN, ptN, axes, &p->f[i]);
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
