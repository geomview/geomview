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

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

/*
 * OOGL PolyList and NPolyList wrappers for clipping routine.
 * Adapted from Daeron Meyer's Ginsu
 * by Dan Krech and Stuart Levy, Geometry Center, 1994.
 */

#include "polylistP.h"
#include "npolylistP.h"

#include "Clip.h"


void readVerts(Clip *clip, vertex_list *pv, Geom *g, int isnd)
{
    int i;
    vertex *point = (vertex *)malloc(pv->numvtx * sizeof(vertex));
    ColorA *colors;
    int colored = clip->polyhedron.has & HAS_VC;

    memset(point, 0, pv->numvtx * sizeof(vertex));

    pv->head = point;
      
#define pl ((PolyList *)g)
#define npl ((NPolyList *)g)

    for(i = 0; i < pv->numvtx; i++, point++) {
	point->next = point+1;
	if (isnd) {
	    point->coord = &npl->v[i*(clip->dim+1)];
	    colors = &npl->vcol[i];
	} else {
	    point->coord = &pl->vl[i].pt.x;
	    colors = &pl->vl[i].vcol;
	}
	if (colored) {
	    point->c = *(Color *)colors;
	}
    }
    point--;
    point->next = NULL;
    pv->point = point;
#undef pl
#undef npl
}

void readNDPoly(poly *p, NPolyList *pl, int polyNum)
{
  int i;
  pvtx *corner = NULL;
  polyvtx_list *pv = (polyvtx_list *)malloc(sizeof(polyvtx_list));
  pvtx **prevp = &pv->head;
  Poly *np = &pl->p[polyNum];

  p->me = pv;
  pv->numvtx = np->n_vertices;
  for(i = 0; i < np->n_vertices; i++) {
    corner = (pvtx *)malloc(sizeof(pvtx));
    corner->num = pl->vi[i + pl->pv[i]];
    *prevp = corner;
    prevp = &corner->next;
  }
  *prevp = pv->head;
  pv->point = corner;

  p->c.r = np->pcol.r;
  p->c.g = np->pcol.g;
  p->c.b = np->pcol.b;
}
   
  

void readPolyvtx(polyvtx_list *pv, int numvx, PolyList *pl, int polyNum)
{
  int i;
  pvtx **prevp = &pv->head;
  pvtx *point = NULL;
  Poly *p = &pl->p[polyNum];

  pv->numvtx = numvx;
  for (i = 0; i < pv->numvtx; i++)
  {
    point = (pvtx *)malloc(sizeof(pvtx));
    point->num = (p->v[i] - pl->vl);
    *prevp = point;
    prevp = &point->next;
  }
  *prevp = pv->head;
  pv->point = point;

  return;
}

void readPoly(poly *p, PolyList *pl, int polyNum)
{
  p->numvtx = pl->p[polyNum].n_vertices;

  p->me = (polyvtx_list *)malloc(sizeof(polyvtx_list));
  readPolyvtx(p->me, p->numvtx, pl, polyNum);	/* read each vertex ref number. */

  p->c.r = pl->p[polyNum].pcol.r;
  p->c.g = pl->p[polyNum].pcol.g;
  p->c.b = pl->p[polyNum].pcol.b;
}

void readPolys(Clip *clip, poly_list *ph, Geom *g, int isnd)
{
  int i;
  poly *point;

#define pl ((PolyList *)g)
#define npl ((NPolyList *)g)

  ph->numpoly = isnd ? npl->n_polys : pl->n_polys;
  point = (poly *)malloc(ph->numpoly * sizeof(poly));
  ph->head = point;
  for(i = 0; i < ph->numpoly; i++, point++) {
    point->next = point+1;
    if (isnd) readNDPoly(point, npl, i);
    else readPoly(point, pl, i);
  }
  point--;
  point->next = NULL;
  ph->point = point;
#undef pl
#undef npl
  return;
}


void setGeom(struct clip *clip, void *aGeom)
{
    char *classname;
    int isnd;

    clip_destroy(clip);
    if (aGeom==NULL) {
	return;		/* clip_destroy() already created null object */
    }
    classname = GeomName(aGeom);
    clip->polyhedron.has = 0;
    if (strcmp(classname, "polylist") == 0) {
	PolyList *pl = (PolyList *)aGeom;
	if (pl->geomflags & PL_HASVCOL)
	    clip->polyhedron.has = HAS_VC;
	if (pl->geomflags & PL_HASPCOL)
	    clip->polyhedron.has |= HAS_PC;
	isnd = 0;
	clip->dim = pl->geomflags & VERT_4D ? 4 : 3;
	clip->polyvertex.numvtx = pl->n_verts;
	isnd = 0;
    } else if (strcmp(classname, "npolylist") == 0) {
	NPolyList *npl = (NPolyList *)aGeom;

	clip->dim = npl->pdim-1;
	if (clip->dim >= MAXDIM-1) {
	    fprintf(stderr, "clip: can't handle objects of dimension %d\n\
(change MAXDIM, now %d, and recompile)\n", clip->dim, MAXDIM);
	    exit(1);
	}
	if (npl->geomflags & PL_HASVCOL)
	    clip->polyhedron.has = HAS_VC;
	if (npl->geomflags & PL_HASPCOL)
	    clip->polyhedron.has |= HAS_PC;
	isnd = 1;
	clip->polyvertex.numvtx = npl->n_verts;
	isnd = 1;
    } else {
	fprintf(stderr, "clip: can't handle object of type '%s'\n", classname);
	exit(1);
    }
    readVerts(clip, &clip->polyvertex, aGeom, isnd);
    readPolys(clip, &clip->polyhedron, aGeom, isnd);
}

void *getGeom(Clip *clip)
{
  int i = 0, j = 0;
  float *points, *ptp;
  ColorA *colors = NULL, *ctp;
  ColorA *poly_color = NULL;
  int *vertex_counts;
  int *vertex_indices;
  vertex *vert;
  poly *point;
  int total = 0;
  Geom *aGeom=NULL;
  int dim = clip->dim;
  int hdim = (dim == 3 || dim == 4) ? 4 : dim+1;
  int has = clip->polyhedron.has;

  if (clip->polyvertex.numvtx==0) {
      return NULL;
  }
  point = clip->polyhedron.head;
  while (point != NULL)	{		/* For each polygon: */
    if (!point->clipped) {		/* if it still exists */
	total += point->me->numvtx;
    }
    point = point->next;
  }
  points = (float *)malloc(hdim*clip->polyvertex.numvtx*sizeof(*points));
  vertex_counts = (int *)malloc(clip->polyhedron.numpoly*sizeof(int));
  if (has & HAS_PC)
      poly_color = (ColorA *)malloc(clip->polyhedron.numpoly*sizeof(ColorA));
  vertex_indices = (int *)malloc(total*sizeof(int));
  if (has & HAS_VC)
      colors = (ColorA *)malloc(clip->polyvertex.numvtx*sizeof(ColorA));

  ptp = points;
  ctp = colors;
  for(vert=clip->polyvertex.head; vert != NULL; vert = vert->next) {
    if (!vert->clip) {
	memcpy(ptp, vert->coord, dim*sizeof(float));
	ptp += dim;
	if (hdim > dim) *ptp++ = 1.;
	if (has & HAS_VC) {
	    *(Color *)ctp = vert->c;
	    ctp->a = 1;
	    ctp++;
	}
    }
  }

  point = clip->polyhedron.head;
  i = 0; j = 0;
  ctp = poly_color;
  while (point != NULL)			/* For each polygon: */
  {
    if (!point->clipped)		/* if it still exists */
    {
	pvtx *temp;
	pvtx *pnt;
	
	temp = point->me->head;
	pnt = point->me->head;
	vertex_counts[i] = point->me->numvtx;
	if (has & HAS_PC) {
	    *(Color *)ctp = point->c;
	    ctp->a = 1;
	    ctp++;
	}
	i++;
	do
	{
	  vertex_indices[j] = pnt->me->num;
	  j++;
				/* Write out each vertex reference number. */
	  pnt = pnt->next;
	} while (pnt != temp);

    }
    point = point->next;
  }
  if (dim == 3 || dim == 4) {
    aGeom = GeomCreate("polylist",
	CR_NPOLY, clip->polyhedron.numpoly,
	CR_POINT4, points,
	CR_NVERT, vertex_counts,
	CR_VERT, vertex_indices,
	CR_END);
  } else {
    aGeom = GeomCreate("npolylist",
	CR_DIM, dim,
	CR_NPOLY, clip->polyhedron.numpoly,
	CR_POINT4, points,
	CR_NVERT, vertex_counts,
	CR_VERT, vertex_indices,
	CR_END);
  }
  if (has & HAS_PC)
      GeomSet(aGeom, CR_POLYCOLOR, poly_color, CR_END);
  if (has & HAS_VC)
      GeomSet(aGeom, CR_COLOR, colors, CR_END);
  free(points);
  free(vertex_counts);
  free(vertex_indices);
  if (colors) free(colors);
  if (poly_color) free(poly_color);
  return aGeom;
}
