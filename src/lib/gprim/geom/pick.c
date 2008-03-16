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

#include "geomclass.h"
#include "pickP.h"
#include "bbox.h"

#define PICK_WANTED(pick, ap) \
  (pick->want & (!(pick->want & PW_VISIBLE)			\
		   || ap == NULL || (ap->flag & APF_FACEDRAW)	\
		 ? PW_VERT | PW_EDGE | PW_FACE :		\
		   (ap->flag & (APF_EDGEDRAW|APF_VECTDRAW))	\
		   ? PW_VERT|PW_EDGE : 0))

int PickFace(int n_verts, Point3 *verts, Pick *pick, Appearance *ap)
{
  Point3 got, ep;
  int v, e;

  if (PolyNearPosZInt(n_verts, verts, pick->thresh, &got, &v, &e, &ep,
		      PICK_WANTED(pick, ap), pick->got.z))
      return PickFillIn(pick, n_verts, &got, v, e, ap);
  else return 0;

}

int PickFillIn(Pick *pick, int n_verts, Point3 *got, int vertex, int edge, Appearance *ap)
{
  int found = 0;

  (void)ap;

  pick->got = *got;

  vvcopy(&pick->gcur, &pick->gpath);

  found = 0;
  if (vertex != -1) {
    found |= PW_VERT;
    pick->vi = vertex;
  }
  if (edge != -1) {
    found |= PW_EDGE;
    pick->ei[0] = edge;
    pick->ei[1] = (edge + 1) % n_verts;
  }
  if (pick->want & PW_FACE) {
    found |= PW_FACE;
    pick->fn = n_verts;
  }
  if (found) {
    pick->found = found;
    if (pick->f != NULL) OOGLFree(pick->f);
    pick->f = NULL;
  }

  return found;
}

Geom *
GeomMousePick(Geom *g, Pick *p, Appearance *ap,
	      Transform Tg, TransformN *TgN, int *axes,
	      double x, double y)
{
    Pick *pick = NULL;
    Transform Txy, T;
    TransformN *TN = NULL;

    if (!p)
	pick = p = PickSet(NULL, PA_END);
    p->x0 = x;
    p->y0 = y;
    if (TgN) {
	HPointN *tmp = HPtNCreate(TgN->odim, NULL);

	tmp->v[axes[0]] = -x;
	tmp->v[axes[1]] = -y;
	
	TN = TmNTranslateOrigin(NULL, tmp);
	HPtNDelete(tmp);	
	TmNConcat(TgN, TN, TN);
    } else {
	TmTranslate(Txy, -x, -y, 0.);
	TmConcat(Tg, Txy, T);
    }
    g = GeomPick(g, p, ap, T, TN, axes);
    if (g && !pick) {
	/* Only bother if the caller will get to see these */
	if (TN) {
	    p->TmirpN = TmNInvert(p->TprimN, p->TmirpN);
	    p->TwN    = TmNInvert(TN, p->TwN);
	} else {
	    TmInvert(p->Tprim, p->Tmirp);
	    TmInvert(T, p->Tw);
	}
    }
    if (pick)
	PickDelete(pick);
    if (TN)
	TmNDelete(TN);
    return g;
}

/*
 * Simple generic Pick routine based on bounding-box intersection.
 * We succeed if the picked point lies within the screen projection of the
 * object, i.e. if the picked point lies within the smallest screen square
 * surrounding the object.  The depth is that of the midpoint of the
 * bounding box: average of min and max depth.
 */
static Geom *
GenericPick(Geom *g, Pick *p, Appearance *ap,
	    Transform T, TransformN *TN, int axes[4])
{
    Geom *bbox;
    HPoint3 min, max;

    (void)ap;

    if (TN) {
	TransformN *proj;
	int i;
	
	proj = TmNCreate(TN->idim, 4, NULL);
	for (i = 0; i < TN->idim; i++) {
	    proj->a[i*4+0] = TN->a[i*TN->odim+axes[3]];
	    proj->a[i*4+1] = TN->a[i*TN->odim+axes[0]];
	    proj->a[i*4+2] = TN->a[i*TN->odim+axes[1]];
	    proj->a[i*4+3] = TN->a[i*TN->odim+axes[2]];
	}
	bbox = GeomBound(g, NULL, proj);
	TmNDelete(proj);
    } else {
	bbox = GeomBound(g, T, NULL);
    }
    BBoxMinMax((BBox*)bbox, &min, &max);
    if (min.x <= 0 && max.x >= 0 && min.y <= 0 && max.y >= 0
	&& .5*(min.z + max.z) <= p->got.z) {
	p->got.x = p->got.y = 0;
	p->got.z = .5 * (min.z + max.z);
	p->gprim = g;
	if (TN) {
	    p->TprimN = TmNCopy(TN, p->TprimN);
	    memcpy(p->axes, axes, sizeof(p->axes));
	} else {
	    TmCopy(T, p->Tprim);
	}
	return g;
    }
    return NULL;
}

Geom *
GeomPick(Geom *g, Pick *p, Appearance *ap,
	 Transform T, TransformN *TN, int *axes)
{
   Appearance *nap = ap;
   Geom *result;

   if(g == NULL)
	return NULL;

   if(g->Class->pick == NULL) {
	/* OOGLError(1, "Note: using GenericPick for class %s",
				(*g->Class->name)()); */
	g->Class->pick = (GeomPickFunc *)GenericPick;
   }
   if(g->ap && (p->want & PW_VISIBLE))
	nap = ApMerge( g->ap, ap, 0 );
   result = (*g->Class->pick)(g, p, nap, T, TN, axes);
   if(ap != nap)
	ApDelete(nap);
   return result;
}

void
PickDelete(Pick *p)
{	/* Note we don't GeomDelete(p->gprim); it wasn't RefIncr'd */
  if(p) {
    if (p->f)
	OOGLFree(p->f);	/* free the face list, if any */
    if (p->TprimN)
	TmNDelete(p->TprimN);
    if (p->TmirpN)
	TmNDelete(p->TmirpN);
    if (p->TwN)
	TmNDelete(p->TwN);
    if (p->TselfN)
	TmNDelete(p->TselfN);
    vvfree(&p->gcur);
    vvfree(&p->gpath);
    OOGLFree(p);
  }
}

Pick *
PickSet(Pick *p, int attr, ...)
{
    va_list al;
    int a;

    if(p == NULL) {
	/*
	 * Create new Pick structure
	 */
	p = OOGLNewE(Pick, "new Pick");
	p->got.x = 0;  p->got.y = 0;  p->got.z = 1;
	p->thresh = 0.02;
	p->want = 0;
	p->found = 0;
	VVINIT(p->gcur, int, 1);
	VVINIT(p->gpath, int, 1);
	p->gprim = NULL;
	p->TprimN = NULL;
	p->TmirpN = NULL;
	p->TwN = NULL;
	p->TselfN = NULL;
	HPt3From(&p->v, 0.0, 0.0, 0.0, 1.0);
	p->vi = -1;
	HPt3From(&p->e[0], 0.0, 0.0, 0.0, 1.0);
	HPt3From(&p->e[1], 0.0, 0.0, 0.0, 1.0);
	p->ei[0] = -1;
	p->ei[1] = -1;
	p->f = NULL;
	p->fn = 0;
	p->fi = -1;
	TmIdentity(p->Tw2n);
	TmIdentity(p->Tc2n);
	TmIdentity(p->Ts2n);
	TmIdentity(p->Tprim);
	TmIdentity(p->Tmirp);
	TmIdentity(p->Tw);
	TmIdentity(p->Tself);
    }
    va_start(al, attr);
    for(a = attr; a != PA_END; a = va_arg(al, int)) {
	switch(a) {
	case PA_WANT:	p->want = va_arg(al, int); break;
	case PA_THRESH:	p->thresh = va_arg(al, double); break;
	case PA_POINT:	p->got = *va_arg(al, Point3 *); break;
	case PA_DEPTH:	p->got.z = va_arg(al, double); break;
	case PA_GPRIM:	p->gprim = va_arg(al, Geom *); break;
	case PA_TPRIM:  TmCopy(*va_arg(al, Transform *), p->Tprim); break;
	case PA_TPRIMN:
	  p->TprimN = TmNCopy(*va_arg(al, TransformN **), p->TprimN); break;
	case PA_VERT:	p->v = *va_arg(al, HPoint3 *); break;
	case PA_EDGE: { HPoint3 *e = va_arg(al, HPoint3 *);
			p->e[0] = e[0];
			p->e[1] = e[1];
		      }
	  break;
	case PA_FACE:	p->f = va_arg(al, HPoint3 *); break;
	case PA_FACEN:	p->fn = va_arg(al, int); break;

	case PA_TW2N:   TmCopy(*va_arg(al, Transform *), p->Tw2n); break;
	case PA_TC2N:   TmCopy(*va_arg(al, Transform *), p->Tc2n); break;
	case PA_TS2N:   TmCopy(*va_arg(al, Transform *), p->Ts2n); break;

	default:
	    OOGLError(1, "PickSet: unknown attribute %d", a);
	    va_end(al);
	    return p;
	}
    }
    va_end(al);
    return p;
}

int
PickGet(Pick *p, int attr, void *attrp)
{
    if(p == NULL)
	return -1;
    switch(attr) {
    case PA_WANT:   *(int *)attrp = p->want; return 1;
    case PA_THRESH: *(float *)attrp = p->thresh; return 1;
    case PA_POINT:  *(Point3 *)attrp = p->got; break;
    case PA_DEPTH:  *(float *)attrp = p->got.z; break;
    case PA_GPRIM:  *(Geom **)attrp = p->gprim; break;
    case PA_TPRIM:  TmCopy(p->Tprim, *(Transform *)attrp); break;
    case PA_TPRIMN:
      *((TransformN **)attrp) = TmNCopy(p->TprimN, *((TransformN **)attrp));
      break;
    case PA_TWORLD: TmCopy(p->Tw, *(Transform *)attrp); break;
    case PA_VERT: *(HPoint3 *)attrp = p->v; break;
    case PA_EDGE:
      ((HPoint3 *)attrp)[0] = p->e[0];
      ((HPoint3 *)attrp)[1] = p->e[1];
      break;
    case PA_FACE: *(HPoint3 **)attrp = p->f; break;
    case PA_FACEN: *(int *)attrp = p->fn; break;
    default:
	return -1;
    }
    return p->found;
}

/*
 * Local Variables: ***
 * c-basic-offset: 4 ***
 * End: ***
 */
