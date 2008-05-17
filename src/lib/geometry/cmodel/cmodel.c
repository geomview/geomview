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

/* conformal model graphics for geomview */

#include <stdlib.h>
#include "cmodelP.h"
#include "mgP.h"

static int cm_show_subdivision = 0;
static int cm_maxrefine = 6;
/*static double cm_cosmaxbend = .95;*/
static double cm_cosmaxbend = .98;

static int alldone = TRUE;
static int curv;

struct vertex *edge_start(struct edge *e, int ori);
void split_triangle(struct triangle *t);
void split_edge(struct edge *e, splitfunc split);
void refine_once(splitfunc split);
void refine();
struct edge *new_edge_p(struct vertex *v1, struct vertex *v2);
void make_new_quad(Transform T, HPoint3 *p, ColorA *c);
void make_new_triangle(HPoint3 *a, HPoint3 *b, HPoint3 *c, ColorA *col,
		       Transform T, Poly *p, int allvisible);

void set_cm_refine(double cm_cmb, int cm_mr, int cm_ss)
{
  /* These tests allow us to call this routine in a way that sets
     only some of the values; parameters with out-of-range values
     are not set. */

  if (cm_cmb >= -1 && cm_cmb <=1)
    cm_cosmaxbend = cm_cmb;
  if (cm_mr >=0)
    cm_maxrefine = cm_mr;
  if (cm_mr >=0)
    cm_show_subdivision = cm_ss;
  return;
}

void cmodel_clear(int space)
{
  static int initialized = FALSE; 
   
  if (initialized) {
    clear_all_vertexs();
    clear_all_edges();
    clear_all_triangles();
  }
  else {
    initialize_vertexs();
    initialize_edges();
    initialize_triangles();
    initialized = TRUE;
  }
  if (space & TM_SPHERICAL)
    curv = 1;
  else if (space & TM_HYPERBOLIC)
    curv = -1;
  else if (space & TM_EUCLIDEAN)
    curv = 0;
  /* else error */
}

void cm_read_quad(Quad *q)
{
  int i = q->maxquad;
  QuadP *qp = q->p;
  QuadC *qc = q->c;
  Transform T;

  mggettransform(T);
   
  if (q->geomflags & QUAD_C) {
    while (i-- > 0)
      make_new_quad(T, (HPoint3 *)qp++, (ColorA *)qc++);
  } 
  else {
    while (i-- > 0)
      make_new_quad(T, (HPoint3 *)qp++, NULL);
  }
  return;
}

void make_new_quad(Transform T, HPoint3 *p, ColorA *c)
{
  HPoint3 tp, polar;
  int i;
  struct vertex *v[4];
  struct edge *e1, *e2, *e3, *e4, *e5;
  int apflags = _mgc->astk->ap.flag;

  if ((apflags & (APF_EDGEDRAW | APF_FACEDRAW | APF_NORMALDRAW)) == 0)
    return;

  /* make 4 new vertices */
  tp.w = 1.;
  if (c) { /* vertex colors defined */
    for (i = 0; i < 4; i++) {
      projective_to_conformal(curv, p++, T, (Point3 *)(void *)&tp);
      v[i] = simple_new_vertex(&tp, c++);
    }
  }
  else {  /* no vertex colors so get color from appearance stack */
    c = (ColorA*)&_mgc->astk->ap.mat->diffuse;
    for (i = 0; i < 4; i++) {
      projective_to_conformal(curv, p++, T, (Point3 *)(void *)&tp);
      v[i] = simple_new_vertex(&tp, c);
    }
  }
  triangle_polar_point(curv, (Point3 *)(void *)&v[0]->V.pt, (Point3 *)(void *)&v[1]->V.pt,
		       (Point3 *)(void *)&v[2]->V.pt, &polar);
  for (i=0; i<4; i++)
    v[i]->polar = polar;

  e1 = new_edge_p(v[0], v[1]); 
  e2 = new_edge_p(v[1], v[2]); 
  e4 = new_edge_p(v[2], v[3]); 
  e5 = new_edge_p(v[3], v[0]);

  if (apflags & (APF_FACEDRAW|APF_NORMALDRAW)) {
    /* make two triangles and five edges */
    new_triangle(e1, e2,
		 e3 = new_edge_p(v[2], v[0]),
		 TRUE, TRUE, TRUE, NULL);
    new_triangle(e3, e4, e5,
		 FALSE, TRUE, TRUE, NULL);
  }

  /* set the four original edges visible if required */
  if (apflags & APF_EDGEDRAW) {
    e1->visible = TRUE;
    e2->visible = TRUE; 
    e4->visible = TRUE; 
    e5->visible = TRUE; 
  }
  return;
}

void cm_draw_mesh(Mesh *m)
{
  HPoint3 *pt, *newpt, *ppt;
  Point3 *n, *newn, *pn;
  ColorA *c = NULL, *newc = NULL, *pc = NULL;
  mgshadefunc shader = _mgc->astk->shader;
  int i, npt;
  Transform T;

  mggettransform(T);
   
  mgpushtransform();
  mgidentity();

  npt = m->nu * m->nv;
  pt = m->p;
  n = m->n;
  newpt = ppt = OOGLNewNE(HPoint3, npt, "CModel mesh points");
  newn = pn = OOGLNewNE(Point3, npt, "CModel mesh normals");
  if (_mgc->astk->flags & MGASTK_SHADER) {
    newc = pc = OOGLNewNE(ColorA, npt, "CModel mesh color");
    c = m->c ? m->c : (ColorA *)&_mgc->astk->mat.diffuse;
  }
  for (i = 0; i < npt; ++i) {
    projective_vector_to_conformal(curv, pt, n, T, (Point3 *)ppt, pn);
    ppt->w = 1.;
    if (newc) {
      (*shader)(1, ppt, pn, c, pc);
      pc++;
      if (m->c) c++;
    }
    ++pt; ++n; ++ppt; ++pn;
  }
  mgmesh(MESH_MGWRAP(m->geomflags),
	 m->nu, m->nv, newpt, newn, NULL, newc ? newc : m->c, m->geomflags);

  OOGLFree(newpt);
  OOGLFree(newn);
  if (newc) {
    OOGLFree(newc);
  }

  mgpoptransform();
}

void cm_read_vect(Vect *v)
{
  int i, nv, nc;
  HPoint3 pt, *p = v->p;
  ColorA *c = v->c, *col = (ColorA *)(void *)&_mgc->astk->mat.edgecolor;
  struct vertex *v0, *v1, *v2;
  struct edge *e;
  Transform T;

  mggettransform(T);

  pt.w = 1.;
  for (i = 0; i < v->nvec; i++) {
    nv = abs(v->vnvert[i]);
    nc = v->vncolor[i];

    /* get position, color and make a new vertex */
    projective_to_conformal(curv, p++, T, (Point3 *)(void *)&pt);
    if (nc > 0) {nc--; col = c++;}
    v0 = v1 = simple_new_vertex(&pt, col);

    if (nv == 1) { /* point, not polyline */
      v0->visible = TRUE;
      continue;
    }

    do { /* copy polyline */

         /* get position, color and make a new vertex */
      projective_to_conformal(curv, p++, T, (Point3 *)(void *)&pt);
      if (nc > 0) {nc--; col = c++;}
      v2 = simple_new_vertex(&pt, col);

      e = new_edge_p(v1, v2);
      e->visible = e->hascolor = TRUE;
      v1 = v2;
    } while(--nv > 1);

    if (v->vnvert[i] < 0) { /* if polyline is closed */
      e = new_edge_p(v2, v0);
      e->visible = e->hascolor = TRUE;
    }
  }
}

void cm_read_polylist(PolyList *polylist)
{
  int i, j, nv, n, vertcolors, facecolors;
  HPoint3 center;
  ColorA *col;
  Transform T;
  Poly *p;

  mggettransform(T);

  p = polylist->p;
  n = polylist->n_polys;
  vertcolors = (polylist->geomflags & (PL_HASVCOL|PL_HASPCOL)) == PL_HASVCOL;
  facecolors = (polylist->geomflags & PL_HASPCOL);
  col = (ColorA*)&_mgc->astk->mat.diffuse;
  for (i = 0; i < n; i++) {
    if (facecolors) col = &p->pcol;
    nv = p->n_vertices;
    if (nv == 3) {
      make_new_triangle(&p->v[0]->pt,
			&p->v[1]->pt,
			&p->v[2]->pt,
			col, T, p, TRUE);
    }
    else {
      center.x = center.y = center.z = center.w = 0;
      for (j = 0; j < nv; j++)
	HPt3Add(&center, &p->v[j]->pt, &center);
      for (j = 1; j < nv; j++)
	make_new_triangle(&p->v[j-1]->pt, &p->v[j]->pt, &center,
			  vertcolors ? &p->v[j]->vcol : col, T, p, FALSE);
      make_new_triangle(&p->v[nv-1]->pt, &p->v[0]->pt, &center,
			vertcolors ? &p->v[0]->vcol : col, T, p, FALSE);
    }
    ++p;
  }
}

void make_new_triangle(HPoint3 *a, HPoint3 *b, HPoint3 *c, ColorA *col,
		       Transform T, Poly *p, int allvisible)
{
  struct vertex v, *v1, *v2, *v3;
  Point3 ap, bp, cp;
  struct edge *e1, *e2, *e3;
  int apflags = _mgc->astk->ap.flag;

  projective_to_conformal(curv, a, T, &ap);
  projective_to_conformal(curv, b, T, &bp);
  projective_to_conformal(curv, c, T, &cp);

  /*fprintf(stderr,"Making tri %.1f %.1f %.1f, %.1f %.1f %.1f, %.1f %.1f %.1f\n",ap.x,ap.y,ap.z,bp.x,bp.y,bp.z,cp.x,cp.y,cp.z);*/
  /* convert to the conformal model */
  /* make a model vertex */
  triangle_polar_point(curv,&ap, &bp, &cp, &v.polar);
  v.V.vcol = *col;

  v1 = new_vertex(&ap, &v, NULL);
  v2 = new_vertex(&bp, &v, NULL);
  v3 = new_vertex(&cp, &v, NULL);
  e1 = new_edge_p(v1, v2);
  e2 = new_edge_p(v2, v3); 
  e3 = new_edge_p(v3, v1);

  if (apflags & (APF_FACEDRAW|APF_NORMALDRAW))
    new_triangle(e1, e2, e3, TRUE, TRUE, TRUE, p);

  if (apflags & APF_EDGEDRAW) {
    e1->visible = TRUE;
    if (allvisible) { 
      e2->visible = TRUE; 
      e3->visible = TRUE; 
    }
  }	

  return;
}

struct edge *new_edge_p(struct vertex *v1, struct vertex *v2)
{
  HPoint3 polar;
  edge_polar_point(curv,(Point3 *)(void *)&v1->V.pt,(Point3 *)(void *)&v2->V.pt, &polar);
  return new_edge(v1,v2,&polar);
}

void set_normal(HPoint3 *point, HPoint3 *polar, Point3 *normal)
{
  Point3 t;
   
  if (polar == NULL) return;
  Pt3Mul(polar->w, (Point3 *)point, &t);
  Pt3Sub((Point3 *)polar, &t, normal);
  /*fprintf(stderr,"sn gives %.2f %.2f %.2f (%.2f) for polar %.2f %.2f %.2f %.2f\n",normal->x,normal->y,normal->z,sqrt(Pt3Dot(normal,normal)),polar->x,polar->y,polar->z,polar->w);*/
  Pt3Unit(normal);
}

void cmodel_draw(int plflags)
{
  struct triangle *tp;
  struct edge *ep;
  struct vertex *vp;
  ColorA col[2];
  HPoint3 pts[2];
  Vertex *Vertp = NULL, *verts = NULL;
  Poly *Polyp = NULL, *polys = NULL;
  int npolys, keepflags, nverts, useshader, shading;
  mgshadefunc shader;

  refine();

  /* set the transform to the identity before displaying anything
     since Poincare model data is already transformed */
  mgpushtransform();
  mgidentity();

  if ((npolys = triangle_count()) != 0) {
    polys = Polyp = OOGLNewNE(Poly, npolys, "CModel Polys");
  }

  if ((nverts = vertex_count()) != 0) {
    verts = Vertp = OOGLNewNE(Vertex, nverts, "CModel Vertices");
  }

  shading = _mgc->astk->ap.shading;
  useshader = _mgc->astk->flags & MGASTK_SHADER;
  shader = _mgc->astk->shader;

  vp = first_vertex();
  while (vp != NULL) {
    Vertp->pt = vp->V.pt;
    /* visible vertices must be displayed
       invisible vertices are part of faces so we
       must compute normals */

    if (vp->visible) {
      mgpolyline(1, &Vertp->pt, 1, &vp->V.vcol, 0);
    } else if (IS_SMOOTH(shading)) {
      set_normal(&vp->V.pt, &vp->polar, &Vertp->vn);
      if (useshader)
	(*shader)(1, &Vertp->pt, &Vertp->vn, &vp->V.vcol, &Vertp->vcol);
      else
	Vertp->vcol = vp->V.vcol;
    }

    vp->vxp = Vertp++;
    vp = vp->next;
  }

  ep = first_edge();
  while (ep != NULL) { /* draw visible edges */
    if (ep->visible) {
      pts[0] = ep->v1->V.pt;
      pts[1] = ep->v2->V.pt;
      if (ep->hascolor) {
	col[0] = ep->v1->V.vcol;
	col[1] = ep->v2->V.vcol;
	mgpolyline(2, pts, 2, col, 0);
      } else {
	col[0].r =  _mgc->astk->ap.mat->edgecolor.r;
	col[0].g =  _mgc->astk->ap.mat->edgecolor.g;
	col[0].b =  _mgc->astk->ap.mat->edgecolor.b;
      }
      col[0].a = 1;
      mgpolyline(2, pts, 1, &col[0], 0);
    }
      
    ep = ep->next;
  }

  tp = first_triangle();
  while (tp != NULL) {
    tp->v[0] = edge_start(tp->e1, tp->o1)->vxp;
    tp->v[1] = edge_start(tp->e2, tp->o2)->vxp;
    tp->v[2] = edge_start(tp->e3, tp->o3)->vxp;
    Polyp->flags = plflags & (PL_HASPCOL|PL_HASVCOL);
    Polyp->n_vertices = 3;
    Polyp->v = tp->v;
    /* computation is not exact here: assume triangle is small so center
       and vertex are very close together */
    if (IS_SHADED(shading)) {
      set_normal(&tp->e1->v1->V.pt, &tp->e1->v1->polar, (Point3 *)&Polyp->pn);
      Polyp->flags |= PL_HASPN;
    }
    if (IS_SMOOTH(shading)) {
      Polyp->flags |= PL_HASVN;
    }

    if (useshader) {
      (*shader)(1, &tp->v[0]->pt, (Point3 *)&Polyp->pn,
		&tp->e1->v1->V.vcol, &Polyp->pcol);
    } else {
      Polyp->pcol = tp->e1->v1->V.vcol;
    }

    Polyp++;
    tp = tp->next;
  }
   
  if (npolys) {
    keepflags = _mgc->astk->ap.flag;
    if (!cm_show_subdivision)
      _mgc->astk->ap.flag &= ~APF_EDGEDRAW;

    plflags = (plflags &~ (PL_HASPCOL|PL_HASVCOL))
      | (IS_SMOOTH(shading) ? PL_HASVCOL : PL_HASPCOL);
 
    mgpolylist(npolys, polys, nverts, verts, plflags);
    _mgc->astk->ap.flag = keepflags;
  }

  /* restore the current transform */
  mgpoptransform();

  if (polys) {
    OOGLFree(polys);
  }
  if (verts) {
    OOGLFree(verts);
  }
  
}

void refine()
{
  int maxsteps = cm_maxrefine;


  alldone = FALSE;
   
  /* should do edges at infinity here first */
   
  while (!alldone && maxsteps-- > 0) {
    alldone = TRUE;
    refine_once(edge_split);
  }
   
  return;
}

void refine_once(splitfunc split)
{
  struct edge *ep = first_edge(), *le = get_last_edge();
  struct triangle *tp = first_triangle(), *lt = get_last_triangle();

  /* split all long edges */
  while (ep != NULL) {
    split_edge(ep, split);
    if (ep == le) break;
    ep = ep->next;
  } 
   
  if (alldone) return;
   
  /* now split all triangles that have had edges split */
  while (tp != NULL) {
    split_triangle(tp);
    if (tp == lt) break;
    tp = tp->next;
  }

  return;
}

/* decides whether to split an edge and if so sets the split flag,
   creates a new edge and drops it into the structure */

void split_edge(struct edge *e, splitfunc split)
{
  struct vertex *mid;
   
  if (e->small) return;
  mid = (*split)(e, cm_cosmaxbend);
  if (mid == NULL) {
    e->split = FALSE;
    e->small = TRUE;
    return;
  }
  e->split = TRUE;
  e->other_half = new_edge(mid, e->v2, &e->polar);
  e->other_half->visible = e->visible;
  e->other_half->hascolor = e->hascolor;
  e->v2 = mid;
  alldone = FALSE;
   
  return;
}

struct vertex *edge_start(struct edge *e, int ori)
{
  return ori ? e->v1 : e->v2;
}

struct vertex *edge_mid(struct edge *e)
{
  return e->v2;
}

struct edge *first_half(struct edge *e, int ori)
{
  return ori ? e : e->other_half;
}

void split_triangle_at_one_edge(struct edge **e1, struct edge **e2, 
				struct edge **e3, int *o1, int *o2, int *o3, Poly *orig)
{
  struct edge *ne; 
   
  ne = new_edge_p(edge_mid(*e1), edge_start(*e3, *o3));

  new_triangle(first_half(*e1, !*o1), *e2,       ne, 
	       *o1,        *o2,       FALSE,  orig);
  *e1 = first_half(*e1, *o1);
  *e2 = ne;
  *o2 = TRUE;
   
  return;
}

void split_triangle_at_two_edges(struct edge **e1, struct edge **e2, 
				 struct edge **e3, int *o1, int *o2, int *o3, Poly *orig)
{
  struct edge *ne1, *ne2; 
   
  ne1 = new_edge_p(edge_mid(*e1), edge_start(*e3, *o3));
  ne2 = new_edge_p(edge_mid(*e1), edge_mid(*e2));

  new_triangle(first_half(*e1, !*o1), first_half(*e2, *o2), ne2, 
	       *o1,                   *o2,       FALSE, orig);
  new_triangle(first_half(*e2, !*o2), ne1,   ne2, 
	       *o2,        FALSE, TRUE, orig);
  *e1 = first_half(*e1, *o1);
  *e2 = ne1;
  *o2 = TRUE;
   
  return;
}

void split_triangle_at_three_edges(struct edge **e1, struct edge **e2, 
				   struct edge **e3, int *o1, int *o2, int *o3, Poly *orig)
{
  struct edge *ne1, *ne2, *ne3; 
   
  ne1 = new_edge_p(edge_mid(*e1), edge_mid(*e2));
  ne2 = new_edge_p(edge_mid(*e2), edge_mid(*e3));
  ne3 = new_edge_p(edge_mid(*e3), edge_mid(*e1));

  new_triangle(first_half(*e1, !*o1), first_half(*e2, *o2), ne1, 
	       *o1,                   *o2,       FALSE, orig);
  new_triangle(first_half(*e2, !*o2), first_half(*e3, *o3), ne2, 
	       *o2,                   *o3,       FALSE, orig);
  new_triangle(ne1, ne2, ne3, TRUE, TRUE, TRUE, orig);

  *e1 = first_half(*e1, *o1);
  *e2 = ne3;
  *o2 = FALSE;
  *e3 = first_half(*e3, !*o3);

  return;
}


void split_triangle(struct triangle *t)
{
  int magic;
  Poly *orig; 

  if (t->small) return;
  orig = t->orig_poly;
  magic = t->e1->split + 2 * t->e2->split + 4 * t->e3->split;
   
  switch (magic) {
  case 0:
    t->small = TRUE;
    break;
  case 1:
    split_triangle_at_one_edge(&t->e1, &t->e2, &t->e3, 
			       &t->o1, &t->o2, &t->o3, orig);
    break;
  case 2:
    split_triangle_at_one_edge(&t->e2, &t->e3, &t->e1, 
			       &t->o2, &t->o3, &t->o1, orig);
    break;
  case 4:
    split_triangle_at_one_edge(&t->e3, &t->e1, &t->e2, 
			       &t->o3, &t->o1, &t->o2, orig);
    break;
  case 3:
    split_triangle_at_two_edges(&t->e1, &t->e2, &t->e3, 
				&t->o1, &t->o2, &t->o3, orig);
    break;
  case 6:
    split_triangle_at_two_edges(&t->e2, &t->e3, &t->e1, 
				&t->o2, &t->o3, &t->o1, orig);
    break;
  case 5:
    split_triangle_at_two_edges(&t->e3, &t->e1, &t->e2, 
				&t->o3, &t->o1, &t->o2, orig);
    break;
  case 7:
    split_triangle_at_three_edges(&t->e1, &t->e2, &t->e3, 
				  &t->o1, &t->o2, &t->o3, orig);
    break;
  default:
    break;
  }
   
  return;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
