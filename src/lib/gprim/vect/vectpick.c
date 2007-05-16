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

#include <stdlib.h> /* for abs() */
#include "vectP.h"
#include "pickP.h"
#include "appearance.h"

Vect *VectPick(Vect *v, Pick *p, Appearance *ap,
	       Transform T, TransformN *TN, int *axes)
{
  Point3 plist[2];
  int i, j, k, ok[2];
  int found, v4d;
  unsigned int apflag = 0;

  /* Make sure that the edges do not count as visible - otherwise 
   * they could really mess things up */
  if (ap != NULL) {
    apflag = ap->flag;
    ap->flag &= ~APF_EDGEDRAW;
  }
    
  found = 0;

  v4d = (v->geomflags & VERT_4D) != 0;

  for (i = k = 0; i < v->nvec; i++) {
    int nv = abs(v->vnvert[i]);
    
    if (TN)
      ok[0] = (0 < HPt3NTransPt3(TN, axes, &v->p[k], v4d, &plist[0]));
    else
      ok[0] = (0 < HPt3TransPt3(T, &v->p[k], &plist[0]));
    if(nv == 1) {
	if(ok[0] && PickFace(1, &plist[0], p, ap)) {
	    found = 1;
	    p->vi = k;
	}
	k++;
	continue;
    }
    for (j = 0; j < nv - 1;) {
      if (TN)
	ok[1] = (0 < HPt3NTransPt3(TN, axes, &v->p[k + (++j)], v4d, &plist[1]));
      else
	ok[1] = (0 < HPt3TransPt3(T, &v->p[k + (++j)], &plist[1]));
      if ((ok[0]||ok[1]) && PickFace(2, plist, p, ap)) {
	found = 1;
	p->vi = p->vi ? k + j : k + j - 1;
	p->ei[0] = k + j - 1;
	p->ei[1] = k + j;
      }
      ok[0] = ok[1];
      plist[0] = plist[1];
    }
    if (v->vnvert[i] < 0) {
      if (TN)
	ok[1] = (0 < HPt3NTransPt3(TN, axes, &v->p[k], v4d, &plist[1]));
      else
	ok[1] = (0 < HPt3TransPt3(T, &v->p[k], &plist[1]));
      if ((ok[0]||ok[1]) && PickFace(2, plist, p, ap)) {
	found = 1;
	p->vi = p->vi ? k : k + j;
	p->ei[0] = k + j;
	p->ei[1] = k;
      }
    }
    k += nv;
  }

  if (ap != NULL) ap->flag = apflag;

  if (!found) return NULL;

  if (p->found & PW_VERT) {
    if (TN)
      HPt3NTransHPt3(TN, axes, &v->p[p->vi], v4d, &p->v);
    else
      HPt3Transform(T, &v->p[p->vi], &p->v);
  } else
    p->vi = -1;

  if (p->found & PW_EDGE) {
    if (TN) {
      HPt3NTransHPt3(TN, axes, &v->p[p->ei[0]], v4d, &p->e[0]);
      HPt3NTransHPt3(TN, axes, &v->p[p->ei[1]], v4d, &p->e[1]);
    } else {
      HPt3Transform(T, &v->p[p->ei[0]], &p->e[0]);
      HPt3Transform(T, &v->p[p->ei[1]], &p->e[1]);
    }
  } else
    p->ei[0] = p->ei[1] = -1;

  /* It really doesn't make sense to claim that we found a face hit...*/
  p->found &= ~PW_FACE;
  p->fi = -1;

  if (TN) {
    p->TprimN = TmNCopy(TN, p->TprimN);
    memcpy(p->axes, axes, sizeof(p->axes));
  } else
  TmCopy(T, p->Tprim);

  return v;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
