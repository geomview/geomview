/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
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
Copyright (C) 1998-2000 Geometry Technologies, Inc.";
#endif

/*
 * Clipping routine.
 * Adapted from Daeron Meyer's Ginsu.
 * by Dan Krech and Stuart Levy, Geometry Center, 1994.
 */

#include "Clip.h"

#define EPS 0.00001

vertex *add_vertex(Clip * clip, vertex_list * vtxl, float *aCoord,
		   Color * c)
{
  vertex *head = vtxl->head;
  vertex *temp = head;

  head = (vertex *) obstack_alloc(&clip->obst, sizeof(vertex));
  head->coord = (float *) obstack_alloc(&clip->obst, clip->dim * sizeof(float));

  head->next = temp;
  memcpy(head->coord, aCoord, clip->dim * sizeof(float));

  head->c = *c;
  head->clip = 0;
  vtxl->head = head;
  return (head);
}

/* return true if all vertices have been clipped away */
bool find_unclipped_vertex(polyvtx_list * me, vertex ** vertex_set)
{
  pvtx *point;
  int brk = 0;

  point = me->head;

  do {
    brk = vertex_set[point->num]->clip;	/* Check polygon vertex list to see */
    if (brk)			/* if any still remain */
      point = point->next;
  } while (brk && (point != me->head));

  if ((point == me->head) && brk)	/* if we found no vertex, return true */
    return true;

  return false;			/* otherwise return false */
}


float dot(Clip * clip, float *point)
{
  int i;
  int dim = clip->dim;
  float sum = clip->surf[0];

  for (i = 0; i < dim; i++) {
    sum += point[i] * clip->surf[i + 1];
  }
  return sum;
}

void pcinterp(Clip * clip, float *result, float *p1, float *p2,
	      Color * cresult, Color * c1, Color * c2, float val1,
	      float val2)
{
  int i;
  int dim = clip->dim;
  float t = val1 / (val1 - val2);
  float unt;
  float val;

  /* Don't go too crazy in case of numerical error. */
  if (t < 0)
    t = 0;
  else if (t > 1)
    t = 1;
  unt = 1 - t;

  for (i = 0; i < dim; i++)
    result[i] = p1[i] * unt + p2[i] * t;
  if (clip->nonlinear) {
    float eps = EPS * fabs(val1 - val2);
    int limit = 7;
    float t1 = 0, t2 = 1, a;

    for (;;) {
      val = (*clip->clipfunc) (clip, result) - clip->level;
      if (fabs(val) < eps || --limit <= 0)
	break;
      if (val * val1 < 0) {
	t2 = t;
	val2 = val;
      } else {
	t1 = t;
	val1 = val;
      }
      a = val1 / (val1 - val2);	/*Subdivide interval */
      t = (1 - a) * t1 + a * t2;	/*Transform back to original interval */
      unt = 1 - t;
      for (i = 0; i < dim; i++)
	result[i] = p1[i] * unt + p2[i] * t;	/* Interpolate point */
    }
  }
  cresult->r = c1->r * unt + c2->r * t;
  cresult->g = c1->g * unt + c2->g * t;
  cresult->b = c1->b * unt + c2->b * t;
}

int clip_each_vertex(Clip * clip, polyvtx_list * me, vertex_list * vtxl,
		     vertex ** vertex_set)
{
  pvtx *temp, *next, **lastnext, *head;
  float *intersect;
  Color c, *c1, *c2;
  float *p1 = NULL;
  float *p2 = NULL;
  vertex *v1, *v2;

  intersect = malloc(clip->dim * sizeof(float));

  lastnext = &me->head;

  head = me->point = me->head;
  me->point->me = vertex_set[me->point->num];

  do {
    next = me->point->next;
    next->me = vertex_set[next->num];

    v1 = vertex_set[me->point->num];
    v2 = vertex_set[next->num];

    c1 = &v1->c;		/* Color of current vertex */
    c2 = &v2->c;		/* Color of next vertex */

    p1 = v1->coord;
    p2 = v2->coord;		/* Coordinates of next vtx */

    /*
     * Handle first case:
     *
     *
     *            o--------o <---- next vertex
     *           /        /
     *         /         /            side to clip
     *      -------------------------------  <------- clipping plane
     *      /          /
     *    /           /
     *   o-----------o   <--- current vertex
     */

    if (!v1->clip && v2->clip) {
      /* Calculate where line between the two points cuts clipping surface */
      pcinterp(clip, intersect, p1, p2, &c, c1, c2, v1->val, v2->val);

      /* Add vertex at intersection */
      temp = (pvtx *) obstack_alloc(&clip->obst, sizeof(pvtx));

      temp->me = add_vertex(clip, vtxl, intersect, &c);
      me->numvtx++;		/* point to the polygon's */
      temp->next = next;	/* vertex list. */
      me->point->next = temp;
      lastnext = &temp->next;
      me->point = next;
    } else
      /*
       * Handle second case:
       *
       *
       * next vertex ->   o--------o <---- current vertex
       *                 /        /
       *               /         /            side to clip
       *             /          /
       *    -------------------o-----------------  <------- clipping plane
       *          /           /
       *        /            /
       *       o------------o
       */

    if (v1->clip && v2->clip) {
      *lastnext = next;	/* simply delete the current */

      /*      free(me->point);*/
      me->point = NULL;

      me->numvtx--;
      me->point = next;
    } else
      /*
       * Handle third case:
       *
       *
       *                  o--------o <- current vertex
       *                 /        /
       *               /         /            side to clip
       *             /          /
       *    -------------------------------------  <------- clipping plane
       *          /           /
       *        /            /
       *       o------------o <- next vertex
       */

    if (v1->clip && !v2->clip) {
      pcinterp(clip, intersect, p1, p2, &c, c1, c2, v1->val, v2->val);

      temp = (pvtx *) obstack_alloc(&clip->obst, sizeof(pvtx));	/* Add vertex at the */

      temp->me = add_vertex(clip, vtxl, intersect, &c);
      temp->next = next;	/* intersection point to the */
      *lastnext = temp;	/* polygon's vertex list */

      lastnext = &temp->next;		/* from the list. */
      me->point = next;
    } /* fourth case: */
    else {			/* Both vertices unclipped. */
      /* Therefore, do nothing. */
      lastnext = &me->point->next;
      me->point = next;
    }

  } while (me->point != head);	/* Do this 'til we have checked all
				 * vertices in the circular linked
				 * list. */
  *lastnext = me->head;
  me->point = NULL;

  free(intersect);

  return me->numvtx;

}

void clip_init(Clip * clip)
{
  clip->polyhedron.numpoly = 0;
  clip->polyhedron.head = NULL;
  clip->polyhedron.has = 0;

  clip->polyvertex.numvtx = 0;
  clip->polyvertex.head = NULL;

  clip->side = CLIP_LE;
  clip->level = 0.0;

  clip->clipfunc = dot;

  clip->surf = NULL;

  obstack_init(&clip->obst);
}

void clip_destroy(Clip * clip)
{
#ifdef notyet
  poly *po, *nextpo;
  vertex *v, *nextv;

  for (po = clip->polyhedron.head; po != NULL; po = nextpo) {
    nextpo = po->next;
    free(po);
  }
  for (v = clip->polyvertex.head; v != NULL; v = nextv) {
    nextv = v->next;
    free(v);
  }
#endif
  clip->polyhedron.numpoly = 0;
  clip->polyhedron.head = NULL;
  clip->polyhedron.has = 0;

  clip->polyvertex.numvtx = 0;
  clip->polyvertex.head = NULL;

  obstack_free(&clip->obst, NULL);
  obstack_init(&clip->obst);
}

void setClipPlane(Clip * clip, float *coeff, float level)
{
  clip->surf = realloc(clip->surf, (clip->dim + 1) * sizeof(float));
  memcpy(clip->surf, coeff, (clip->dim + 1) * sizeof(float));
  clip->level = level;
}

void setSide(Clip * clip, enum clip_side side)
{
  clip->side = side;
}

/* Find vertices which are on the wrong side of the plane; mark them
 * as clipped. Vertices on the plane are marked as unclipped.
 */
void clip_vertices(Clip * clip)
{
  vertex *point;

  for (point = clip->polyvertex.head; point != NULL; point = point->next) {
    point->val = (*clip->clipfunc) (clip, point->coord) - clip->level;

    point->clip = (clip->side == CLIP_LE) ?
	(point->val > -EPS) : (point->val < EPS);
  }
}

/* Find the range of function values over all vertices */
int span_vertices(Clip * clip, float *minp, float *maxp)
{
  vertex *point;
  float min, max, val;

  *minp = *maxp = 0;
  point = clip->polyvertex.head;
  if (point == NULL)
    return 0;
  min = max = (*clip->clipfunc) (clip, point->coord);
  for (point = clip->polyvertex.head; point != NULL; point = point->next) {
    val = (*clip->clipfunc) (clip, point->coord);
    if (clip->side != CLIP_LE)
      val = -val;
    if (min > val)
      min = val;
    else if (max < val)
      max = val;
  }
  *minp = min;
  *maxp = max;
  return 1;
}


void clip_polygons(Clip * clip, vertex_list * vtxl)
{
  poly *point;

  vertex **vertex_set;

  vertex_set = (vertex **) obstack_alloc(&clip->obst, vtxl->numvtx * sizeof(vertex));

  {
    int count = 0;
    vertex *vert;
    vert = vtxl->head;
    while (vert != NULL) {
      vertex_set[count] = vert;
      vert = vert->next;
      count++;
    }
  }

  point = clip->polyhedron.head;
  clip->polyhedron.point = point;
  while (point != NULL) {
    point->clipped = find_unclipped_vertex(point->me, vertex_set);

    if (!point->clipped) {
      /* at least some vertices remain; clip or keep the polygon */
      point->numvtx = clip_each_vertex(clip, point->me, vtxl, vertex_set);
    }

    point = point->next;	/* the list. */
  }
  clip->polyhedron.point = point;
}


void refresh_vertex_list(Clip * clip, vertex_list * vtxl)
{
  int count = 0;

  vtxl->point = vtxl->head;
  while (vtxl->point != NULL) {
    if (!vtxl->point->clip) {
      vtxl->point->num = count;	/* Count number of vertices remaining */
      count++;			/* after clip and give each vertex a */
    }				/* new reference number. */
    vtxl->point = vtxl->point->next;
  }
  vtxl->numvtx = count;
}

void refresh_poly_list(Clip * clip, poly_list * pl)
{
  int count = 0;
  poly *point;
  point = pl->head;
  while (point != NULL) {
    if (!point->clipped)	/* Count number of polygons remaining */
      count++;			/* after clip. */
    point = point->next;
  }
  pl->numpoly = count;
}

void do_clip(Clip * clip)
{
  if (clip->polyvertex.numvtx != 0) {
    clip_vertices(clip);	/* mark appropriate vertices as clipped */
    clip_polygons(clip, &clip->polyvertex);	/* modify or remove clipped polygons */
    refresh_vertex_list(clip, &clip->polyvertex);
    /* rearrange pointers into vertex list */
    refresh_poly_list(clip, &clip->polyhedron);
    /* rearrange points into polygon list */
  }
}
