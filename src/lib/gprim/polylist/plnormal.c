/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2006 Claus-Justus Heine
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

/*
 * Compute the normals to each surface in a polylist.
 */

#include "polylistP.h"

void PolyNormal(Poly *p, Point3 *nu_av, int fourd, int evert,
		int *flagsp, int *first_concave)
{
  int flags = 0, n;
  Vertex **vp;
  Vertex *v1, *v2, *v3;
  Pt3Coord w1, w2, w3, len;
  Point3 nu;

  if (first_concave){
    *first_concave = 0;
  }
  
  nu_av->x = nu_av->y = nu_av->z = 0.0;
  if((n = p->n_vertices) >= 3) {
    v1 = p->v[n-2];
    v2 = p->v[n-1];
    vp = p->v;

    if (fourd) {
#undef ANTI_4D
#define ANTI_4D(P,Q)							\
      ((v2->pt.P*w2 - v1->pt.P*w1) * (v3->pt.Q*w3 - v1->pt.Q*w1) -	\
       (v2->pt.Q*w2 - v1->pt.Q*w1) * (v3->pt.P*w3 - v1->pt.P*w1))
      
      w1 = !fzero(v1->pt.w) ? 1.0 / v1->pt.w : 1.0;
      w2 = !fzero(v2->pt.w) ? 1.0 / v2->pt.w : 1.0;
      do {
	v3 = *vp++;
	w3 = !fzero(v3->pt.w) ? 1.0 / v3->pt.w : 1.0;
	nu.x = ANTI_4D(y,z);
	nu.y = ANTI_4D(z,x);
	nu.z = ANTI_4D(x,y);
	if (!fneg(Pt3Dot(&nu, nu_av))) {
	  Pt3Add(nu_av, &nu, nu_av);
	} else {
	  Pt3Sub(nu_av, &nu, nu_av);
	  flags |= POLY_CONCAVE;
	  if (first_concave) {
	    *first_concave = p->n_vertices - n;
	    first_concave = NULL;
	  }
	}
	v1 = v2;
	w1 = w2;
	v2 = v3;
	w2 = w3;
      } while(--n > 0);
#undef ANTI_4D
    } else {
#undef ANTI
#define ANTI(P,Q)						\
      ((v2->pt.P - v1->pt.P) * (v3->pt.Q - v1->pt.Q) -		\
       (v2->pt.Q - v1->pt.Q) * (v3->pt.P - v1->pt.P))

      do {
	v3 = *vp++;
	nu.x = ANTI(y,z);
	nu.y = ANTI(z,x);
	nu.z = ANTI(x,y);
	/*Pt3Unit(&nu);*/
	if (!fneg(Pt3Dot(&nu, nu_av))) {
	  Pt3Add(nu_av, &nu, nu_av);
	} else {
	  Pt3Sub(nu_av, &nu, nu_av);
	  flags |= POLY_CONCAVE;
	  if (first_concave) {
	    *first_concave = p->n_vertices - n;
	    first_concave = NULL;
	  }
	}
	/*Pt3Unit(nu_av);*/
	v1 = v2;
	v2 = v3;
      } while(--n > 0);
#undef ANTI
    }
  }

  len = Pt3Length(nu_av);

#if 0
  static Pt3Coord min_len = 1e8;

  if (!fzero(len) && len < min_len) {
    min_len = len;
    fprintf(stderr, "min_len: %e\n", min_len);
  }
#endif

  if (fzero(len)) {
    /* degenerated */
    flags |= POLY_NOPOLY;
  } else {
    if(evert)
      len = -len;
    Pt3Mul(1.0/len, nu_av, nu_av);

    if (flagsp && (n = p->n_vertices) > 3) {
      /* determine whether this polygon is flat or not */
      Point3 diff;
      v1 = p->v[n-1];
      vp = p->v;

      do {
	v2 = *vp++;
	if (fourd) {
	  HPt3SubPt3(&v2->pt, &v1->pt, &diff);
	} else {
	  Pt3Sub((Point3 *)(void *)&v2->pt,
		 (Point3 *)(void *)&v1->pt, &diff);
	}
	if (!fzero(Pt3Dot(nu_av, &diff))) {
	  p->flags |= POLY_NONFLAT;
	  break;
	}
	v1 = v2;
      } while (--n > 0);
    }
  }

  if (flagsp) {
    *flagsp |= flags;
  }

}

PolyList *
PolyListComputeNormals(PolyList *polylist, int need)
{
  int	   i, n, fourd, evert;
  Poly	   *p;
  Vertex   **vp;
  Pt3Coord len;
  Point3   nu_av;

  if (!polylist) return NULL;

  need &= ~polylist->geomflags;
  fourd = (polylist->geomflags & VERT_4D) != 0;
  evert = (polylist->geomflags & PL_EVNORM) != 0;

  if (need & (PL_HASPN|PL_HASPFL)) {
    /* Create per-polygon normals */
    for (i = polylist->n_polys, p = polylist->p; --i >= 0; p++) {
      if((n = p->n_vertices) >= 3) {
	if (need & PL_HASPFL) {
	  p->flags = 0;
	  PolyNormal(p, &nu_av, evert, fourd, &p->flags, NULL);
	} else {
	  PolyNormal(p, &nu_av, evert, fourd, NULL, NULL);
	}
	if (need & PL_HASPN) {
	  p->pn = nu_av;
	}
      }
    }
  }

  if (need & PL_HASVN) {

#if 0
    for(i = polylist->n_verts, v = polylist->vl; --i >= 0; v++) {
      v->vn.x = v->vn.y = v->vn.z = 0.0;
    }
    for(i = polylist->n_polys, p = polylist->p; --i >= 0; p++) {
      for(n = p->n_vertices, vp = p->v; --n >= 0; vp++) {
	v = *vp;
	if (fneg(Pt3Dot(&v->vn, &p->pn))) {
	  Pt3Sub(&p->pn, &v->vn, &v->vn);
	} else {
	  Pt3Add(&p->pn, &v->vn, &v->vn);
	}
      }
    }
    for(i = polylist->n_verts, v = polylist->vl; --i >= 0; v++) {
      len = Pt3Length(&v->vn);
      if(fpos(len)) {
	if(polylist->flags & PL_EVNORM)
	  len = -len;
	Pt3Mul(1.0/len, &v->vn, &v->vn);
      }
    }
#else
    int *e_idx =
      OOGLNewNE(int, polylist->n_verts+1, "Adjacent edge indexes");
    HPoint3 **edges;
    int n_edges = 0;
    Vertex *vl;

#define V_IDX(elem, base)						\
    (int)((long)((char *)(elem) - (char *)(base))/sizeof(*elem))

    memset(e_idx, 0, polylist->n_verts*sizeof(int));
    for(i = polylist->n_polys, p = polylist->p; --i >= 0; p++) {
      for(n = p->n_vertices, vp = p->v; --n >= 0; vp++) {
	int v_idx = V_IDX(*vp, polylist->vl);
	e_idx[v_idx] += 2;
	n_edges += 2;
      }
    }
    
    edges = OOGLNewNE(HPoint3 *, n_edges, "Adjacent edges");
    n_edges  = e_idx[0];
    e_idx[0] = 0;
    edges[e_idx[0]] = (void *)(long)(n_edges-1);
    for (i = 1; i < polylist->n_verts; i++) {
      int v_n_edges = e_idx[i];
      e_idx[i] = n_edges;
      n_edges += v_n_edges;
      edges[e_idx[i]] = (void *)(long)(v_n_edges-1);
    }
    e_idx[i] = n_edges;

    for(i = polylist->n_polys, p = polylist->p; --i >= 0; p++) {
      int v_idx, cnt, j;

      vp = p->v;
      v_idx = V_IDX(*vp, polylist->vl);
      cnt = (int)(long)edges[e_idx[v_idx]];
      edges[e_idx[v_idx]+cnt--] = &vp[p->n_vertices-1]->pt;
      edges[e_idx[v_idx]+cnt--] = &vp[1]->pt;
      if (cnt > 0) {
	edges[e_idx[v_idx]] = (void *)(long)cnt;
      }
      for (j = 1, ++vp; j < p->n_vertices-1; ++j, ++vp) {
	v_idx = V_IDX(*vp, polylist->vl);
	cnt = (int)(long)edges[e_idx[v_idx]];
	edges[e_idx[v_idx]+cnt--] = &vp[1]->pt;
	edges[e_idx[v_idx]+cnt--] = &vp[-1]->pt;
	if (cnt > 0) {
	  edges[e_idx[v_idx]] = (void *)(long)cnt;
	}
      }
      v_idx = V_IDX(*vp, polylist->vl);
      cnt = (int)(long)edges[e_idx[v_idx]];
      edges[e_idx[v_idx]+cnt--] = &p->v[0]->pt;
      edges[e_idx[v_idx]+cnt--] = &vp[-1]->pt;
      if (cnt > 0) {
	edges[e_idx[v_idx]] = (void *)(long)cnt;
      }
    }

    for (i = 0, vl = polylist->vl; i < polylist->n_verts; ++i, ++vl) {
      bool patch_end;
      Point3 patch_nu;
      HPt3Coord w = 0.0;
      int v_idx, j, k;

      v_idx = V_IDX(vl, polylist->vl);

      vl->vn.x = vl->vn.y = vl->vn.z = 0.0;
      if (fourd) {
	w = 1.0/vl->pt.w;
      }

      patch_nu.x = patch_nu.y = patch_nu.z = 0.0;
      patch_end = false;
      for (j = e_idx[v_idx]; j < e_idx[v_idx+1]; j += 2) {
	HPoint3 *swap;
	HPoint3 *p1, *p2;
	Point3 nu;
	HPt3Coord w1, w2;

	p1 = edges[j];
	p2 = edges[j+1];

	if (fourd) {
#undef ANTI_4D
#define ANTI_4D(P, Q)						\
	  ((p1->P*w1 - vl->pt.P*w) * (p2->Q*w2 - vl->pt.Q*w) -	\
	   (p1->Q*w1 - vl->pt.Q*w) * (p2->P*w2 - vl->pt.P*w))

	  w1 = 1.0/p1->w;
	  w2 = 1.0/p2->w;
	  
	  nu.x = ANTI_4D(y,z);
	  nu.y = ANTI_4D(z,x);
	  nu.z = ANTI_4D(x,y);
#undef ANTI_4D
	} else {
#undef ANTI
#define ANTI(P, Q)					\
	  ((p1->P - vl->pt.P) * (p2->Q - vl->pt.Q) -	\
	   (p1->Q - vl->pt.Q) * (p2->P - vl->pt.P))

	  nu.x = ANTI(y,z);
	  nu.y = ANTI(z,x);
	  nu.z = ANTI(x,y);
#undef ANTI
	}
	Pt3Add(&patch_nu, &nu, &patch_nu);

	/* Try to find the next adjacent edges pair and give it the
	   proper orientation.
	 */
	if (j+2 < e_idx[v_idx+1]) {
	  if (edges[j+2] == edges[j+1]) {
	    /* ok */
	  } else if (edges[j+3] == edges[j+1]) {
	    swap = edges[j+2]; edges[j+2] = edges[j+3]; edges[j+3] = swap;
	  } else {
	    for (k = j + 4; k < e_idx[v_idx+1]; k += 2) {
	      if (edges[k] == edges[j+1]) {
		swap = edges[j+2]; edges[j+2] = edges[k];   edges[k]   = swap;
		swap = edges[j+3]; edges[j+3] = edges[k+1]; edges[k+1] = swap;
		break;
	      } else if (edges[k+1] == edges[j+1]) {
		swap = edges[j+2]; edges[j+2] = edges[k+1]; edges[k+1] = swap;
		swap = edges[j+3]; edges[j+3] = edges[k];   edges[k] = swap;
		break;
	      }
	    }
	    if (k >= e_idx[v_idx+1]) {
	      patch_end = true;
	    }
	  }
	} else {
	  patch_end = true;
	}
	
	if (patch_end) {
	  /* try to match the orientation with the normal of the
	   * previous patch-component (if any).
	   */
	  if (fneg(Pt3Dot(&patch_nu, &vl->vn))) {
	    Pt3Sub(&vl->vn, &patch_nu, &vl->vn);
	  } else {
	    Pt3Add(&vl->vn, &patch_nu, &vl->vn);
	  }
	  patch_nu.x = patch_nu.y = patch_nu.z = 0.0;
	  patch_end = false;
	}
      }
      len = Pt3Length(&vl->vn);
      if(len > 0.0) {
	if(polylist->geomflags & PL_EVNORM)
	  len = -len;
	Pt3Mul(1.0/len, &vl->vn, &vl->vn);
      }
    }
    OOGLFree(edges);
    OOGLFree(e_idx);
#endif
  }
  
  polylist->geomflags |= need;

  return polylist;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
