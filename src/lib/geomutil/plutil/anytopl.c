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

/*
 * anytopl.c
 * author:  Celeste Fowler
 * date:  June 12, 1992
 * Rewritten, Stuart Levy, July, 1996.
 */
#include <stdlib.h>
#include <stdio.h>
#include "transformn.h"
#include "polylistP.h"
#include "npolylistP.h"
#include "bezierP.h"
#include "quadP.h"
#include "meshP.h"
#include "ndmeshP.h"
#include "vectP.h"
#include "listP.h"
#include "discgrpP.h"
#include "instP.h"
#include "skelP.h"
#include "hpoint3.h"
#include "point3.h"
#include "plutil.h"

#ifndef alloca
#include <alloca.h>
#endif

#define PL_C		0x1
#define	PL_N		0x2
#define	PL_H		0x4
#define PL_ND		0x8


#define PL_HASPC	PL_HASPCOL
#define	PL_HASVC	PL_HASVCOL

typedef struct _face {
  int nv;		/* Number of vertices on this face */
  int v0;		/* Index of first vertex in vtable */
  ColorA fcolor;	/* Face color if specified */
} Face;

typedef struct _vert {
  HPoint3 p;
  ColorA c;
  Point3 n;
  float *ndp;		/* Pointer to raw data for N-D vertex */
  int hdim;		/* Dimension of this point */
  int has;
} Vert;

struct _PLData {
  int maxdim;
  int some, all;	/* PL_HASVN, PL_HASVC, PL_HASPN, PL_HASPC */
  vvec faces;		/* Table {Face} of all faces */
  vvec verts;		/* Table {Vert} of vertices */
  vvec vtable;		/* Table of vertex-indices for all faces */
			/* Top of stacks of... */
  TransformN *Tn;	/*   N-D to 3-D transform */
  Transform T;		/*   3-D transform */
  Appearance *ap;	/*   appearance */
};

#define PLnextvert(PL)  VVCOUNT((PL)->verts)

Geom *PLDataToGeom(PLData *PL, int want, int unwant);
static void PLDataDelete(PLData *PL);
static PLData *PLDataCreate();
static void initmethods();
static void PLaddface(PLData *PL, int nv, int verts[], ColorA *c);
static void PLaddvect(PLData *PL, int nv, int verts[], ColorA *c);
static int PLaddverts(PLData *PL, int nv, HPoint3 *v, ColorA *c, Point3 *n);
static int PLaddNDverts(PLData *PL, int nv, int hdim, float *v, ColorA *c);
static void PLaddseg(PLData *PL, int v0, int v1, ColorA *c);

static int toPLsel;

static ColorA white = {1,1,1,1};
static ColorA black = {0,0,0,1};
static Point3 noNormal = {0,0,0};

/*
 * AnyToPL converter.
 * Converts any geomview object to a polylist and returns a pointer
 * to the polylist.  NB to future developers:
 * There are a lot of assumptions about the internals ofny
 * the various data types.  If anything changes, things will have to
 * be adjusted accordingly.
 */

Geom *AnyToPL(Geom *g, Transform T)
{
  PLData *pd = AnyGeomToPLData(g, T, NULL, NULL, NULL);
  Geom *ng = PLDataToGeom(pd, 0, PL_ND);
  PLDataDelete(pd);
  return ng;
}

static HPoint3 *hpoints(PLData *PL)
{
  HPointN hp;
  int i;
  HPoint3 *pts = OOGLNewNE(HPoint3, VVCOUNT(PL->verts), "PL verts");
  HPoint3 *p = pts;
  Vert *v = VVEC(PL->verts, Vert);
  static int xyzw[] = {0, 1, 2, -1};

  for(i = 0; i < VVCOUNT(PL->verts); i++, p++, v++) {
    if(v->hdim == 0) {
	*p = v->p;
    } else {
	if(PL->Tn) {
	    hp.dim = v->hdim;
	    hp.v = v->ndp;
	    HPtNTransformComponents(PL->Tn, &hp, xyzw, p);
	} else {
	    /* Project to 3-space: just take first 3 and final components. */
	    memcpy(p, v->ndp, 3*sizeof(float));
	    p->w = v->ndp[v->hdim-1];
	}
    }
  }
  return pts;
}

static float *ndpoints(PLData *PL)
{
  HPointN hp;
  int i;
  int hdim = PL->maxdim ? PL->maxdim : 4;
  float *pts = OOGLNewNE(float, hdim*VVCOUNT(PL->verts), "PL ndverts");
  float *p = pts;
  Vert *v = VVEC(PL->verts, Vert);
  VARARRAY(comp, int, hdim);

  for(i = 0; i < hdim-1; i++) {
    comp[i] = i;
  }
  comp[hdim-1] = -1;

  for(i = 0; i < VVCOUNT(PL->verts); i++, v++, p += hdim) {
    if(PL->Tn) {
	if(v->hdim) {
	    hp.dim = v->hdim;
	    hp.v = v->ndp;
	} else {
	    hp.dim = 4;
	    hp.v = &v->p.x;
	}
	HPtNTransformComponents(PL->Tn, &hp, comp, (HPoint3 *)p);
    } else {
	if(v->hdim == hdim) {
	    memcpy(p, v->ndp, hdim*sizeof(float));
	} else if(v->hdim == 0) {
	    memcpy(p, &v->p, 3*sizeof(float));
	    memset(p+3, 0, (hdim-4)*sizeof(float));
	    p[hdim-1] = v->p.w;
	} else if(v->hdim < hdim) {
	    memcpy(p, v->ndp, (v->hdim-1)*sizeof(float));
	    memset(p+v->hdim, 0, (hdim - v->hdim - 1)*sizeof(float));
	    p[hdim-1] = v->ndp[v->hdim-1];
	} else {
	    memcpy(p, v->ndp, (hdim-1)*sizeof(float));
	    p[hdim-1] = v->ndp[v->hdim-1];
	}
    }
  }
  return pts;
}


/* Unpack PLData */
Geom *PLDataToGeom(PLData *PL, int want, int unwant)
{
  HPoint3 *hpts = NULL;
  float *nhpts = NULL;
  ColorA *c = NULL, *pc = NULL;
  Point3 *n = NULL;
  int *nvert;
  int i;
  Face *fp;
  Vert *vp;
  Geom *pl, *npl;

  if(PL == NULL || VVCOUNT(PL->faces) == 0 || VVCOUNT(PL->verts) == 0)
	return NULL;
  nvert = (int *)alloca(VVCOUNT(PL->faces) * sizeof(int));
  fp = VVEC(PL->faces, Face);
  for(i = 0; i < VVCOUNT(PL->faces); i++, fp++)
    nvert[i] = fp->nv;

  if((PL->some & PL_HASVC) && !(unwant & PL_C)) {
    c = (ColorA *)alloca(VVCOUNT(PL->verts)*sizeof(ColorA));
    for(i = 0, vp = VVEC(PL->verts, Vert); i < VVCOUNT(PL->verts); i++, vp++)
	c[i] = vp->c;
  }
  if((PL->some & PL_HASPC) && !(unwant & PL_C)) {
    pc = (ColorA *)alloca(VVCOUNT(PL->faces)*sizeof(ColorA));
    for(i = 0, fp = VVEC(PL->faces, Face); i < VVCOUNT(PL->faces); i++, fp++)
	pc[i] = fp->fcolor;
  }
  if((PL->some & PL_HASVN) && !(unwant & PL_N)) {
    n = (Point3 *)alloca(VVCOUNT(PL->verts)*sizeof(Point3));
    for(i = 0, vp = VVEC(PL->verts, Vert); i < VVCOUNT(PL->verts); i++, vp++)
	n[i] = vp->n;
  }

  /* Do we need to remap points? */
  if(PL->maxdim > 4 && !(unwant & PL_ND)) {
    nhpts = ndpoints(PL);
    npl = GeomCCreate(NULL, NPolyListMethods(),
		CR_DIM, PL->maxdim-1,
		CR_NPOLY, VVCOUNT(PL->faces),
		CR_NVERT, nvert,
		CR_VERT, VVEC(PL->vtable, int),
		CR_POINT4, nhpts,
		CR_COLOR, c,
		CR_POLYCOLOR, pc,
		CR_END);
    return npl;
  } else {
    hpts = hpoints(PL);
    pl = GeomCCreate(NULL, PolyListMethods(),
		CR_NPOLY, VVCOUNT(PL->faces),
		CR_NVERT, nvert,
		CR_VERT, VVEC(PL->vtable, int),
		CR_POINT4, hpts,
		CR_NORMAL, n,
		CR_COLOR, c,
		CR_POLYCOLOR, pc,
		CR_END);
    if(want & PL_N)
	PolyListComputeNormals((PolyList *)pl, PL_HASPN|PL_HASPFL);
    return pl;
  }
}

static PLData *
PLDataCreate()
{
  PLData *PL = OOGLNewE(PLData, "PLData");

  if(toPLsel == 0)
    initmethods();
  PL->maxdim = 0;
  PL->some = 0;
  PL->all = PL_HASVC|PL_HASPC|PL_HASVN;
  VVINIT(PL->faces, Face, 1000);  vvzero(&PL->faces);
  VVINIT(PL->verts, Vert, 1000);  vvzero(&PL->faces);
  VVINIT(PL->vtable, int, 4000);
  PL->Tn = NULL;
  TmIdentity(PL->T);
  PL->ap = ApCreate(AP_DO, APF_FACEDRAW|APF_VECTDRAW,
		    AP_LINEWIDTH, 1,
		    AP_NORMSCALE, 1.0,
		    AP_SHADING, APF_FLAT,
		    AP_END);
  return PL;
}

static void
PLDataDelete(PLData *PL)
{
  if(PL) {
    vvfree(&PL->faces);
    vvfree(&PL->verts);
    vvfree(&PL->vtable);
    if(PL->Tn) TmNDelete(PL->Tn);
    if(PL->ap) ApDelete(PL->ap);
  }
}

static void *beziertoPL(int sel, Bezier *bez, va_list *args) {
  PLData *PL = va_arg(*args, PLData *);
  if(PL->ap && (PL->ap->flag & APF_DICE))
    BezierDice( bez, PL->ap->dice[0], PL->ap->dice[1] );
  if(bez->mesh == NULL || bez->geomflags & BEZ_REMESH)
    BezierReDice(bez);
  return GeomCall(sel, (Geom *)bez->mesh, PL);
}

static void *discgrptoPL(int sel, DiscGrp *dg, va_list *args) {
  Geom *geom = NULL;
  PLData *PL = va_arg(*args, PLData *);
  int i;

  if(dg->big_list == NULL || dg->big_list->el_list == NULL)
    return NULL;

  geom = dg->geom ? dg->geom : dg->ddgeom ?  dg->ddgeom : DiscGrpDirDom(dg);

  for (i=0; i<dg->big_list->num_el; ++i) {
    AnyGeomToPLData(geom, dg->big_list->el_list[i].tform, NULL, NULL, PL);
    if(dg->camgeom)
	AnyGeomToPLData(dg->camgeom,
			  dg->big_list->el_list[i].tform, NULL, NULL, PL);
  }
  return PL;
}

static void *insttoPL(int sel, Inst *inst, va_list *args) {
  GeomIter *it;
  Transform T;
  PLData *PL = va_arg(*args, PLData *);

  it = GeomIterate((Geom *)inst, DEEP);
  while(NextTransform(it, T)) {
    AnyGeomToPLData(inst->geom, T, NULL, NULL, PL);
  }
  return PL;
}

static void *listtoPL(int sel, List *list, va_list *args) {
  List *l;
  PLData *PL = va_arg(*args, PLData *);

  for(l = list; l != NULL; l = l->cdr) {
    GeomCall(sel, l->car, PL);
  }
  return PL;
}

static void putmesh(PLData *PL, int base, int nu, int nv, int uwrap, int vwrap) {
  int v0 = 1, prev0v = 0;
  int u0 = 1, prev0u = 0;
  int u, v, prevu, prevv;
  if(vwrap) {
    v0 = 0, prev0v = nv-1;
  }
  if(uwrap) {
    u0 = 0, prev0u = nu-1;
  }

#define MESHV(u,v)  (base + nu*(v) + u)

  if(PL->ap->flag & (APF_FACEDRAW|APF_NORMALDRAW)) {
    for(prevv = prev0v, v = v0; v < nv; prevv = v, v++) {
	for(prevu = prev0u, u = u0; u < nu; prevu = u, u++) {
	    int face[4];
	    face[0] = MESHV(prevu,prevv);
	    face[1] = MESHV(u,prevv);
	    face[2] = MESHV(u,v);
	    face[3] = MESHV(prevu,v);
	    PLaddface(PL, 4, face, NULL);
	}
    }
  }

  if(nu == 1 || nv == 1 ||
    (PL->ap->flag & (APF_EDGEDRAW|APF_FACEDRAW|APF_NORMALDRAW)) == APF_EDGEDRAW) {
    for(v = 0; v < nv; v++)
	for(prevu = prev0u, u = u0; u < nu; prevu = u, u++)
	    PLaddseg(PL, MESHV(prevu, v), MESHV(u, v), NULL);
    for(u = 0; u < nu; u++)
	for(prevv = prev0v, v = v0; v < nv; prevv = v, v++)
	    PLaddseg(PL, MESHV(u, prevv), MESHV(u, v), NULL);
  }

#undef MESHV
}


static void *meshtoPL(int sel, Mesh *m, va_list *args) {
  int base;
  PLData *PL = va_arg(*args, PLData *);

  base = PLaddverts(PL, m->nu*m->nv, m->p, m->c, m->n);
  putmesh(PL, base, m->nu, m->nv,
	  m->geomflags & MESH_UWRAP, m->geomflags & MESH_VWRAP);
  return PL;
}

static void *ndmeshtoPL(int sel, NDMesh *ndm, va_list *args) {
  int nu = ndm->mdim[0];
  int nv = ndm->meshd > 1 ? ndm->mdim[1] : 1;
  int u, v, base = -1;
  PLData *PL = va_arg(*args, PLData *);
  HPointN **pp = ndm->p;
  ColorA *c = ndm->c;


  base = PLnextvert(PL);
  pp = ndm->p;
  for(v = 0; v < nv; v++) {
    for(u = 0; u < nu; u++, pp++) {
	PLaddNDverts(PL, 1, (*pp)->dim, (*pp)->v, c);
	if(c) c++;
    }
  }
  putmesh(PL, base, nu, nv, 0, 0);
  return PL;
}


static void *npolylisttoPL(int sel, NPolyList *npl, va_list *args)
{
  PLData *PL = va_arg(*args, PLData *);
  int base;
  Poly *p;
  int i, vi;
#define VMAX 100
  int face[VMAX];

  base = PLaddNDverts(PL, npl->n_verts, npl->pdim, npl->v, npl->vcol);
  vvneeds(&PL->vtable, VVCOUNT(PL->vtable) + npl->nvi);

  for(i = 0, p = npl->p; i < npl->n_polys; i++, p++) {
    int *vip = face;
    if(p->n_vertices > VMAX)
	vip = OOGLNewNE(int, p->n_vertices, "npolylist face");

    for(vi = 0; vi < p->n_vertices; vi++)
      vip[vi] = base + npl->vi[vi + npl->pv[i]];

    PLaddface(PL, p->n_vertices, vip,
			npl->geomflags & PL_HASPCOL ? &p->pcol : NULL);
    if(p->n_vertices > VMAX)
	OOGLFree(vip);
  }
  return PL;
}

static void *polylisttoPL(int sel, PolyList *pl, va_list *args) {
  PLData *PL = va_arg(*args, PLData *);
  Poly *p;
  Vertex *v = pl->vl;
  int base, i, vi;
  int face[VMAX];

  base = PLnextvert(PL);
  for(i = 0; i < pl->n_verts; i++, v++) {
    PLaddverts(PL, 1, &v->pt,
		pl->geomflags & PL_HASVCOL ? &v->vcol : NULL,
		pl->geomflags & PL_HASVN ? &v->vn : NULL);
  }

  p = pl->p;
  for(i = 0; i < pl->n_polys; i++, p++) {
    int *vip = face;
    if(p->n_vertices > VMAX)
	vip = OOGLNewNE(int, p->n_vertices, "polylist face");
    for(vi = 0; vi < p->n_vertices; vi++)
	vip[vi] = base + (p->v[vi] - pl->vl);
    PLaddface(PL, p->n_vertices, vip,
	      pl->geomflags & PL_HASPCOL ? &p->pcol : NULL);
    if(p->n_vertices > VMAX)
	OOGLFree(vip);
  }
  return PL;
}

static void *quadtoPL(int sel, Quad *q, va_list *args) {
  PLData *PL = va_arg(*args, PLData *);
  int base, v, i;

  base = PLaddverts(PL, q->maxquad*4, *q->p, *q->c, *q->n);
  for(i = 0, v = base; i < q->maxquad; i++) {
    int face[4];
    face[0] = v++;
    face[1] = v++;
    face[2] = v++;
    face[3] = v++;
    PLaddface(PL, 4, face, NULL);
  }
  return PL;
}

static void *skeltoPL(int sel, Skel *s, va_list *args) {
  PLData *PL = va_arg(*args, PLData *);
  int base, i;
  int vert, edge[2];
  Skline *l = s->l;

  if(PL->ap && (PL->ap->flag & APF_VECTDRAW) == 0)
    return NULL;
  base = s->pdim == 4
	? PLaddverts(PL, s->nvert, (HPoint3 *)s->p, NULL, NULL)
	: PLaddNDverts(PL, s->nvert, s->pdim, s->p, NULL);

  vvneeds(&PL->vtable, VVCOUNT(PL->vtable) + 2*s->nvi);

  for(i = 0; i < s->nlines; i++, l++) {
    int v = l->v0;
    int nv = l->nv;
    int c = l->c0;
    int nc = l->nc;
    if(nv == 1) {
	vert = s->vi[v] + base;
	PLaddvect(PL, 1, &vert, nc>0 ? &s->c[c] : NULL);
    } else {
	while(--nv > 0) {
	    edge[0] = s->vi[v] + base;
	    edge[1] = s->vi[++v] + base;
	    PLaddvect(PL, 2, edge, nc>0 ? &s->c[c] : NULL);
	    nc--;
	    c++;
	}
    }
  }
  return PL;
}

static void *vecttoPL(int sel, Vect *v, va_list *args) {
  PLData *PL = va_arg(*args, PLData *);
  int base, i, vno;
  short *vip, *cip;
  ColorA *cp, *lastc;

  if(PL->ap && (PL->ap->flag & APF_VECTDRAW) == 0)
    return NULL;
  base = PLaddverts(PL, v->nvert, v->p, NULL, NULL);
  vvneeds(&PL->vtable, VVCOUNT(PL->vtable) + 2*v->nvert);
  vno = base;
  vip = v->vnvert;
  cip = v->vncolor;
  cp = v->c;
  lastc = (v->ncolor>0) ? cp : NULL;
  for(i = 0, vno = base; i < v->nvec; i++, vip++, cip++) {
    int nv = *vip;
    int nc = *cip;
    if(nv < 0)
	nv = -nv;
    if(nv == 1) {
	PLaddvect(PL, 1, &vno, nc>0 ? cp : lastc);
    } else {
	while(--nv > 0) {
	    PLaddseg(PL, vno, vno+1, nc>0 ? cp : lastc);
	    vno++;
	    if(nc > 1) {
		nc--;
		cp++;
	    }
	}
	if(*vip < 0)
	    PLaddseg(PL, vno, vno + *vip + 1, nc>0 ? cp : lastc);
    }
    if(nc > 0) {
	lastc = cp + nc - 1;
	cp += nc;
    }
    vno++;
  }
  return PL;
}

static int
PLaddverts(PLData *PL, int nv, HPoint3 *v, ColorA *c, Point3 *n)
{
  Vert *vp;
  Material *mat;
  int i;
  int base = PLnextvert(PL);
  int has = 0;
  ColorA *cdef = &white;

  if(nv <= 0)
    return base;

  /* Apply appearance */
  if(PL->ap && (mat = PL->ap->mat) != NULL) {
    if(mat->override & MTF_DIFFUSE)
	c = NULL;
    if(mat->valid & MTF_DIFFUSE) {
	cdef = &mat->diffuse;
	has = PL_HASVC;
    }
  }
  if(c) has = PL_HASVC;
  if(n) has |= PL_HASVN;
  PL->some |= has;
  PL->all &= ~has;
  vvneeds(&PL->verts, VVCOUNT(PL->verts) + nv);
  vp = &VVEC(PL->verts, Vert)[ VVCOUNT(PL->verts) ];
  for(i = 0; i < nv; i++, vp++) {
    vp->has = has;
    HPt3Transform(PL->T, v++, &vp->p);
    vp->c = c ? *c++ : *cdef;
    vp->n = (n != NULL) ? *n++ : noNormal;
    vp->hdim = 0;
    vp->ndp = NULL;
  }
  VVCOUNT(PL->verts) += nv;
  return base;
}


static int
PLaddNDverts(PLData *PL, int nv, int hdim, float *v, ColorA *c)
{
  Vert *vp;
  Material *mat;
  int i;
  int base = PLnextvert(PL);
  int has = 0;
  ColorA *cdef = &white;

  if(nv <= 0)
	return base;
  if(PL->maxdim < hdim)
	PL->maxdim = hdim;
  /* Apply appearance */
  if((mat = PL->ap->mat) != NULL) {
    if(mat->override & MTF_DIFFUSE)
	c = NULL;
    if(mat->valid & MTF_DIFFUSE) {
	cdef = (ColorA *)&mat->diffuse;
	has = PL_HASVC;
    }
  }
  if(c) has = PL_HASVC;
  PL->some |= has;
  PL->all &= ~has;
  vvneeds(&PL->verts, VVCOUNT(PL->verts) + nv);
  vp = &VVEC(PL->verts, Vert)[ VVCOUNT(PL->verts) ];
  for(i = 0; i < nv; i++, vp++) {
    vp->has = has;
    /* If we could have N-D transforms embedded in an OOGL tree,
     * this part might change to alloc'ing a new N-D point, applying the
     * current N-D transform to this point, and storing vp->ndp as pointing
     * to the new copy.
     */
    vp->hdim = hdim;
    vp->ndp = v;
    v += hdim;
    vp->c = *(c ? c++ : cdef);
  }
  VVCOUNT(PL->verts) += nv;
  return base;
}

static void
PLaddface(PLData *PL, int nv, int verts[], ColorA *c)
{
  Material *mat;
  Face *fp;
  int i;

  if(nv <= 0)
    return;

  if(PL->ap->flag & APF_FACEDRAW) {
    vvneeds(&PL->vtable, VVCOUNT(PL->vtable) + nv);
    memcpy( &VVEC(PL->vtable, int)[VVCOUNT(PL->vtable)], verts, nv*sizeof(int));

    fp = VVAPPEND(PL->faces, Face);
    fp->v0 = VVCOUNT(PL->vtable);
    fp->nv = nv;
    VVCOUNT(PL->vtable) += nv;

    /* Apply appearance */
    if((mat = PL->ap->mat) != NULL) {
      if(mat->override & MTF_DIFFUSE)
	  c = NULL;
      if(mat->valid & MTF_DIFFUSE)
	  c = (ColorA *)&mat->diffuse;
    }
    if(c) {
	fp->fcolor = *c;
	PL->some |= PL_HASPC;
    } else {
	fp->fcolor = white;
	PL->all &= ~PL_HASPC;
    }
  }
  if(PL->ap->flag & APF_EDGEDRAW && nv>1) {
    PLaddseg(PL, verts[nv-1], verts[0], NULL);
    for(i = 1; i < nv; i++)
	PLaddseg(PL, verts[i-1], verts[i], NULL);
  }
#ifdef notyet
  if(PL->ap->flag & APF_NORMALDRAW) {
    /* Well, maybe not now. */
  }
#endif
}

static void
PLaddseg(PLData *PL, int v0, int v1, ColorA *c)
{
  Face *fp = VVAPPEND(PL->faces, Face);
  Material *mat = PL->ap->mat;
  if(mat && (mat->valid & MTF_EDGECOLOR)) {
    if(c == NULL || mat->override & MTF_EDGECOLOR) {
      fp->fcolor.r = mat->edgecolor.r;
      fp->fcolor.g = mat->edgecolor.g;
      fp->fcolor.b = mat->edgecolor.b;
      fp->fcolor.a = mat->diffuse.a;
      PL->some |= PL_HASPC;
    } else if(c) {
	fp->fcolor = *c;
	PL->some |= PL_HASPC;
    }
  } else if(c) {
    fp->fcolor = *c;
    PL->some |= PL_HASPC;
  } else {
    PL->all &= ~PL_HASPC;
  }
  fp->v0 = VVCOUNT(PL->vtable);
  fp->nv = 2;
  *VVAPPEND(PL->vtable, int) = v0;
  *VVAPPEND(PL->vtable, int) = v1;
}

static void
PLaddvect(PLData *PL, int nv, int verts[], ColorA *c)
{
  Material *mat;
  ColorA edgec;
  Face *fp;

  if(nv <= 0)
    return;

  if(!(PL->ap->flag & APF_VECTDRAW))
    return;

  vvneeds(&PL->vtable, VVCOUNT(PL->vtable) + nv);
  memcpy( &VVEC(PL->vtable, int)[VVCOUNT(PL->vtable)], verts, nv*sizeof(int));

  fp = VVAPPEND(PL->faces, Face);
  fp->v0 = VVCOUNT(PL->vtable);
  fp->nv = nv;
  VVCOUNT(PL->vtable) += nv;

  /* Apply appearance */
  if((mat = PL->ap->mat) != NULL) {
    if(mat->override & MTF_EDGECOLOR)
	c = NULL;
    if((mat->valid & MTF_EDGECOLOR) && c == NULL) {
      edgec.r = mat->edgecolor.r;
      edgec.g = mat->edgecolor.g;
      edgec.b = mat->edgecolor.b;
      edgec.a = mat->diffuse.a;
      c = &edgec;
    }
  }
  if(c) {
    fp->fcolor = *c;
    PL->some |= PL_HASPC;
  } else {
    fp->fcolor = black;
    PL->all &= ~PL_HASPC;
  }
}

static void initmethods() {
  toPLsel = GeomNewMethod( "toPLData", NULL );
  GeomSpecifyMethod( toPLsel, BezierMethods(), (GeomExtFunc*)beziertoPL );
  GeomSpecifyMethod( toPLsel, DiscGrpMethods(), (GeomExtFunc*)discgrptoPL );
  GeomSpecifyMethod( toPLsel, InstMethods(), (GeomExtFunc*)insttoPL );
  GeomSpecifyMethod( toPLsel, ListMethods(), (GeomExtFunc*)listtoPL );
  GeomSpecifyMethod( toPLsel, MeshMethods(), (GeomExtFunc*)meshtoPL );
  GeomSpecifyMethod( toPLsel, NDMeshMethods(), (GeomExtFunc*)ndmeshtoPL );
  GeomSpecifyMethod( toPLsel, NPolyListMethods(), (GeomExtFunc*)npolylisttoPL );
  GeomSpecifyMethod( toPLsel, PolyListMethods(), (GeomExtFunc*)polylisttoPL );
  GeomSpecifyMethod( toPLsel, QuadMethods(), (GeomExtFunc*)quadtoPL );
  GeomSpecifyMethod( toPLsel, SkelMethods(), (GeomExtFunc*)skeltoPL );
  GeomSpecifyMethod( toPLsel, VectMethods(), (GeomExtFunc*)vecttoPL );
}


PLData *AnyGeomToPLData(Geom *g, Transform T, TransformN *Tn, Appearance *pap,
			PLData *PL)
{
  Transform Told;
  TransformN *Tnold = NULL;
  Appearance *apold = NULL;

  if(g == NULL)
	return PL;

  if(PL == NULL)
    PL = PLDataCreate();

  if(pap || g->ap) {
    apold = PL->ap;
    if(pap && g->ap) {
	PL->ap = ApMerge(pap, PL->ap, 0);
	ApMerge(g->ap, PL->ap, APF_INPLACE);
    } else {
	PL->ap = ApMerge(pap ? pap : g->ap, PL->ap, 0);
    }
  }

  if(T) {
    TmCopy(PL->T, Told);
    TmConcat(T, Told, PL->T);
  }
  if(Tn) {
    Tnold = PL->Tn;
    PL->Tn = TmNConcat(Tn, PL->Tn, NULL);
  }
  GeomCall(toPLsel, g, PL);

  if(apold) {
    ApDelete(PL->ap);
    PL->ap = apold;
  }
  if(T)
    TmCopy(Told, PL->T);
  if(Tn) {
    TmNDelete(PL->Tn);
    PL->Tn = Tnold;
  }
  return PL;
}
