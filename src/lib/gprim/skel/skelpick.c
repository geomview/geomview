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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "skelP.h"
#include "pickP.h"

static Skel *NSkelPick(Skel *s, Pick *p, Appearance *ap,
		       Transform T, TransformN *TN, int *axes)
{
  Point3 plist[2];
  int i, j, ok[2];
  bool found;
  unsigned int apflag = 0;
  HPointN ptN[1];
  
  if (!TN)
    return NULL;

  /* we have full-featured ND-points here. */
  ptN->flags = 0;
  ptN->dim = s->pdim;

  /* Make sure that the edges do not count as visible - otherwise 
   * they could really mess things up */
  if (ap != NULL) {
    apflag = ap->flag;
    ap->flag &= ~APF_EDGEDRAW;
  }
    
  found = false;

  for (i = 0; i < s->nlines && !found; i++) {
    int nv = abs(s->l[i].nv);
    int *idx = s->vi + s->l[i].v0;
    
    ptN->v = s->p + idx[0]*s->pdim;
    ok[0] = (0 < HPtNNTransPt3(TN, axes, ptN, &plist[0]));
    if (nv == 1) {
      if (ok[0] && PickFace(1, &plist[0], p, ap)) {
	found = true;
	p->vi = idx[0];
	break;
      }
      continue;
    }
    for (j = 0; j < nv - 1;) {
      ptN->v = s->p + idx[++j]*s->pdim;
      ok[1] = (0 < HPtNNTransPt3(TN, axes, ptN, &plist[1]));
      if ((ok[0]||ok[1]) && PickFace(2, plist, p, ap)) {
	found = true;
	p->vi = p->vi ? idx[j] : idx[j-1];
	p->ei[0] = idx[j-1];
	p->ei[1] = idx[j];
	break;
      }
      ok[0] = ok[1];
      plist[0] = plist[1];
    }
    if (s->l[i].nv < 0) {
      ptN->v = s->p + idx[0]*s->pdim;
      ok[1] = (0 < HPtNNTransPt3(TN, axes, ptN, &plist[1]));
      if ((ok[0]||ok[1]) && PickFace(2, plist, p, ap)) {
	found = true;
	p->vi = p->vi ? idx[0] : idx[j];
	p->ei[0] = idx[j];
	p->ei[1] = idx[0];
	break;
      }
    }
    if (found) {
      break;
    }
  }

  if (ap != NULL) ap->flag = apflag;

  if (!found) {
    return NULL;
  }

  if (p->found & PW_VERT) {
    ptN->v = s->p + p->vi*s->pdim;
    HPtNTransformComponents(TN, ptN, axes, &p->v);
  } else
    p->vi = -1;

  if (p->found & PW_EDGE) {
    ptN->v = s->p + p->ei[0]*s->pdim;
    HPtNTransformComponents(TN, ptN, axes, &p->e[0]);
    ptN->v = s->p + p->ei[1]*s->pdim;
    HPtNTransformComponents(TN, ptN, axes, &p->e[1]);
  } else
    p->ei[0] = p->ei[1] = -1;

  if (p->found & PW_FACE) {
    p->fi = i;
    if (p->f) {
      OOGLFree(p->f);
    }
    p->f = OOGLNewNE(HPoint3, p->fn, "Skel pick");
    for (i = 0; i < p->fn; i++) {
      int vi = s->vi[s->l[p->fi].v0+i];
      ptN->v = s->p + vi*s->pdim;
      HPtNTransformComponents(TN, ptN, axes, &p->f[i]);
    }
  }

  if (TN) {
    p->TprimN = TmNCopy(TN, p->TprimN);
    memcpy(p->axes, axes, sizeof(p->axes));
  } else
    TmCopy(T, p->Tprim);

  return s;
}

Skel *SkelPick(Skel *s, Pick *p, Appearance *ap,
	       Transform T, TransformN *TN, int *axes)
{
  Point3 plist[2];
  int i, j, ok[2];
  bool found;
  int v4d;
  unsigned int apflag = 0;

  if (s->pdim > 4) {
    return NSkelPick(s, p, ap, T, TN, axes);
  }

  /* Make sure that the edges do not count as visible - otherwise 
   * they could really mess things up */
  if (ap != NULL) {
    apflag = ap->flag;
    ap->flag &= ~APF_EDGEDRAW;
  }
    
  found = false;

  v4d = (s->geomflags & VERT_4D) != 0;

  for (i = 0; i < s->nlines && !found; i++) {
    int nv = abs(s->l[i].nv);
    int *idx = &s->vi[s->l[i].v0];
    
    if (TN) {
      ok[0] = (0 < HPt3NTransPt3(TN, axes,
				 (HPoint3 *)(s->p + idx[0]*s->pdim), v4d,
				 &plist[0]));
    } else {
      ok[0] = (0 < HPt3TransPt3(T, (HPoint3 *)(s->p + idx[0]*s->pdim),
				&plist[0]));
    }
    
    if (nv == 1) {
      if (ok[0] && PickFace(1, &plist[0], p, ap)) {
	found = true;
	p->vi = idx[0];
	break;
      }
      continue;
    }
    for (j = 0; j < nv - 1;) {
      if (TN) {
	ok[1] = (0 < HPt3NTransPt3(TN, axes,
				   (HPoint3 *)(s->p + idx[++j]*s->pdim), v4d,
				   &plist[1]));
      } else {
	ok[1] = (0 < HPt3TransPt3(T,
				  (HPoint3 *)(s->p + idx[++j]*s->pdim),
				  &plist[1]));
      }
      
      if ((ok[0]||ok[1]) && PickFace(2, plist, p, ap)) {
	found = true;
	p->vi = p->vi ? idx[j] : idx[j-1];
	p->ei[0] = idx[j-1];
	p->ei[1] = idx[j];
	break;
      }
      ok[0] = ok[1];
      plist[0] = plist[1];
    }
    if (s->l[i].nv < 0) {
      if (TN)
	ok[1] = (0 < HPt3NTransPt3(TN, axes,
				   (HPoint3 *)(s->p + idx[0]*s->pdim), v4d,
				   &plist[1]));
      else
	ok[1] = (0 < HPt3TransPt3(T, (HPoint3 *)(s->p + idx[0]*s->pdim),
				  &plist[1]));
      if ((ok[0]||ok[1]) && PickFace(2, plist, p, ap)) {
	found = true;
	p->vi = p->vi ? idx[0] : idx[j];
	p->ei[0] = idx[j];
	p->ei[1] = idx[0];
	break;
      }
    }
    if (found) {
      break;
    }
  }

  if (ap != NULL) {
    ap->flag = apflag;
  }

  if (!found) {
    return NULL;
  }

  if (p->found & PW_VERT) {
    if (TN) {
      HPt3NTransHPt3(TN, axes, (HPoint3 *)(s->p + p->vi*s->pdim), v4d, &p->v);
    } else {
      HPt3Transform(T, (HPoint3 *)(s->p + p->vi*s->pdim), &p->v);
    }
  } else {
    p->vi = -1;
  }

  if (p->found & PW_EDGE) {
    if (TN) {
      HPt3NTransHPt3(TN, axes,
		     (HPoint3 *)(s->p + p->ei[0]*s->pdim), v4d,
		     &p->e[0]);
      HPt3NTransHPt3(TN, axes,
		     (HPoint3 *)(s->p + p->ei[1]*s->pdim), v4d,
		     &p->e[1]);
    } else {
      HPt3Transform(T, (HPoint3 *)(s->p + p->ei[0]*s->pdim), &p->e[0]);
      HPt3Transform(T, (HPoint3 *)(s->p + p->ei[1]*s->pdim), &p->e[1]);
    }
  } else {
    p->ei[0] = p->ei[1] = -1;
  }

  if (p->found & PW_FACE) {
    p->fi = i;
    if (p->f) {
      OOGLFree(p->f);
    }
    p->f = OOGLNewNE(HPoint3, p->fn, "Skel pick");
    if (TN) {
      for (i = 0; i < p->fn; i++) {
	int vi = s->vi[s->l[p->fi].v0+i];
	HPt3NTransHPt3(TN, axes, (HPoint3 *)(s->p + vi*s->pdim), v4d, &p->f[i]);
      }
    } else {
      for (i = 0; i < p->fn; i++) {
	int vi = s->vi[s->l[p->fi].v0+i];
	HPt3Transform(T, (HPoint3 *)(s->p + vi*s->pdim), &p->f[i]);
      }
    }
  } else {
    p->fi = -1; 
  }

  if (TN) {
    p->TprimN = TmNCopy(TN, p->TprimN);
    memcpy(p->axes, axes, sizeof(p->axes));
  } else
    TmCopy(T, p->Tprim);

  return s;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
