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
  int i, j, k, which, ok[2];
  int found;
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
    
  found = 0;

  for (i = 0; i < s->nlines; i++) {
    int nv = abs(s->l[i].nv);
    int *idx = s->vi + s->l[i].v0;
    
    which = 0;
    ptN->v = s->p + idx[0]*s->pdim;
    ok[0] = (0 < HPtNNTransPt3(TN, axes, ptN, &plist[0]));
    if (nv == 1) {
      if (ok[0] && PickFace(1, &plist[0], p, ap)) {
	found = 1;
	p->vi = idx[0];
      }
      k++;
      continue;
    }
    for (j = 0; j < nv - 1;) {
      ptN->v = s->p + idx[++j]*s->pdim;
      ok[1] = (0 < HPtNNTransPt3(TN, axes, ptN, &plist[1]));
      if ((ok[0]||ok[1]) && PickFace(2, plist, p, ap)) {
	found = 1;
	p->vi = p->vi ? idx[j] : idx[j-1];
	p->ei[0] = idx[j-1];
	p->ei[1] = idx[j];
      }
      ok[0] = ok[1];
      plist[0] = plist[1];
    }
    if (s->l[i].nv < 0) {
      ptN->v = s->p + idx[0]*s->pdim;
      ok[1] = (0 < HPtNNTransPt3(TN, axes, ptN, &plist[1]));
      if ((ok[0]||ok[1]) && PickFace(2, plist, p, ap)) {
	found = 1;
	p->vi = p->vi ? idx[0] : idx[j];
	p->ei[0] = idx[j];
	p->ei[1] = idx[0];
      }
    }
  }

  if (ap != NULL) ap->flag = apflag;

  if (!found) return NULL;

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

  /* It really doesn't make sense to claim that we found a face hit...*/
  p->found &= ~PW_FACE;
  p->fi = -1;

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
  int i, j, k, which, ok[2];
  int found, v4d;
  unsigned int apflag = 0;

  if (s->pdim > 4)
    return NSkelPick(s, p, ap, T, TN, axes);

  /* Make sure that the edges do not count as visible - otherwise 
   * they could really mess things up */
  if (ap != NULL) {
    apflag = ap->flag;
    ap->flag &= ~APF_EDGEDRAW;
  }
    
  found = 0;

  v4d = (s->geomflags & VERT_4D) != 0;

  for (i = 0; i < s->nlines; i++) {
    int nv = abs(s->l[i].nv);
    int *idx = &s->vi[s->l[i].v0];
    
    which = 0;
    if (TN)
      ok[0] = (0 < HPt3NTransPt3(TN, axes,
				 (HPoint3 *)(s->p + idx[0]*s->pdim), v4d,
				 &plist[0]));
    else
      ok[0] = (0 < HPt3TransPt3(T, (HPoint3 *)(s->p + idx[0]*s->pdim),
				&plist[0]));
    if (nv == 1) {
      if (ok[0] && PickFace(1, &plist[0], p, ap)) {
	found = 1;
	p->vi = idx[0];
      }
      k++;
      continue;
    }
    for (j = 0; j < nv - 1;) {
      if (TN)
	ok[1] = (0 < HPt3NTransPt3(TN, axes,
				   (HPoint3 *)(s->p + idx[++j]*s->pdim), v4d,
				   &plist[1]));
      else
	ok[1] = (0 < HPt3TransPt3(T,
				  (HPoint3 *)(s->p + idx[++j]*s->pdim),
				  &plist[1]));
      if ((ok[0]||ok[1]) && PickFace(2, plist, p, ap)) {
	found = 1;
	p->vi = p->vi ? idx[j] : idx[j-1];
	p->ei[0] = idx[j-1];
	p->ei[1] = idx[j];
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
	found = 1;
	p->vi = p->vi ? idx[0] : idx[j];
	p->ei[0] = idx[j];
	p->ei[1] = idx[0];
      }
    }
  }

  if (ap != NULL) ap->flag = apflag;

  if (!found) return NULL;

  if (p->found & PW_VERT) {
    if (TN)
      HPt3NTransHPt3(TN, axes, (HPoint3 *)(s->p + p->vi*s->pdim), v4d, &p->v);
    else
      HPt3Transform(T, (HPoint3 *)(s->p + p->vi*s->pdim), &p->v);
  } else
    p->vi = -1;

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

  return s;
}

#ifdef notdef

#define	VSUB3(a, b, dst)  { (dst).x = (a).x - (b).x; (dst).y = (a).y - (b).y; (dst).z = (a).z - (b).z; }

#define	VDOT2(v1, v2)	( (v1).x*(v2).x + (v1).y*(v2).y )

Vect *
VectPick(Vect *v, Pick *p, Appearance *ap, Transform T, TransformN *TN)
{
  HPoint3 *hp, *hp0, *hp1;
  Point3 pt0, pt1, pv;
  int picked = -1;
  float w0, w1;
  int e, k, found;
  
  found = 0;
  for(e = 0, hp = v->p;  e < v->nvec;  e++) {
    hp0 = hp;
    w0 = HPt3TransPt3(T, hp++, &pt0);
    switch (k = v->vnvert[e]) {
    case 0:
      /* invalid !! */
      break;
    case 1:
    case -1:
      if(pt0.z < p->got.z && fabs(pt0.x) < p->thresh
	 && fabs(pt0.y) < p->thresh && w0 > 0) {
	picked = e;
	p->got = pt0;
	if (p->want & PW_VERT) {
	  found |= PW_VERT;
	  p->v = pt0;
	}
      }
      break;
    default:
      if (k < 0) {
	k = 1-k;
	w1 = w0;
	pt1 = pt0;
	hp1 = hp0;
	w0 = HPt3TransPt3(T, hp0=(&hp[k-3]), &pt0);
      } else {
	hp1 = hp;
	w1 = HPt3TransPt3(T, hp++, &pt1);
      }
      do {
	Point3 near;
	float t;		/* Line parameter "t" */
	Point3 lv;		/* pt1 - pt0 = vector "l" */

	/* pt0 - {0,0} = vector "p" */
	VSUB3(pt1, pt0, lv);
	t = VDOT2(lv, lv);		/* l.l */
	if(t) {
	  t = -VDOT2(pt0, lv) / t;	/* t = p.l / l.l */
	  if(t < 0) t = 0;
	  else if(t > 1) t = 1;
	}
	near.x = pt0.x + lv.x*t;
	near.y = pt0.y + lv.y*t;
	near.z = pt0.z + lv.z*t;
	if(fabs(near.x) <= p->thresh && fabs(near.y) <= p->thresh &&
	   near.z < p->got.z && (w0 > 0 || w1 > 0)) {
	  picked = e;
	  p->got = near;
	  if (p->want & PW_EDGE) {
	    found |= PW_EDGE;
	    p->e[0] = pt0;
	    p->e[1] = pt1;
	  }
	  if (p->want & PW_VERT) {
	    if (t < p->thresh) {
	      found |= PW_VERT;
	      p->v = pt0;
	    } else if (t > 1-p->thresh) {
	      found |= PW_VERT;
	      p->v = pt1;
	    }
	  }
	}
	pt0 = pt1;
	w0 = w1;
	hp0 = hp1;
	hp1 = hp;
	w1 = HPt3TransPt3(T, hp++, &pt1);
      } while(--k > 1);
      hp--;
      break;
    }
  }
  if (found) {
    p->found = found;
    TmCopy(T, p->Tprim);
  }
  if(picked >= 0) {
    p->gprim = (Geom *)v;
    return v;
  }
  return NULL;
}
#endif


/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
