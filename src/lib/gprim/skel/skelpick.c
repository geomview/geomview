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

#define	VSUB3(a, b, dst)  { (dst).x = (a).x - (b).x; (dst).y = (a).y - (b).y; (dst).z = (a).z - (b).z; }

#define	VDOT2(v1, v2)	( (v1).x*(v2).x + (v1).y*(v2).y )

Skel *SkelPick(Skel *s, Pick *p, Appearance *ap, Transform T, TransformN *TN)
{
  return NULL;
}

#ifdef notdef

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
