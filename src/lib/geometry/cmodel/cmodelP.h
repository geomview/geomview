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
#ifndef PMH
#define PMH

#include "point3.h"
#include "hpoint3.h"
#include "polylistP.h"
#include "color.h"
#include "vectP.h"
#include "quadP.h"
#include "cmodel.h"

/* decls for cmodel_data.c */

struct vertex {
   Vertex V, *vxp;
   int visible;
   HPoint3 polar;
   struct vertex *next;
   };

#undef small	/* Blast Windows.  How can they co-opt "small"? */

struct edge {
   struct vertex *v1, *v2;
   HPoint3 polar;
   int small, 	/* says the edge is too short to consider splitting */           
       visible, /* says we should display this edge */
       hascolor,
       split; 	/* says we just split this edge, this is one half ...*/
   struct edge *other_half,  /* .. and the other half is here  */
      *next;
   };

struct triangle {
   int small,	/* says the triangle has 3 short edges */
      o1, o2, o3;
   struct edge *e1, *e2, *e3;
   Poly *orig_poly;
   Vertex *v[3]; /* only used at the end */
   struct triangle *next;
   };

#define edgeBLOCKSIZE 120

void initialize_edges(void);
void clear_all_edges(void);
struct edge *new_edge(struct vertex *v1, struct vertex *v2, HPoint3 *polar);
struct edge *first_edge(void);
struct edge *get_last_edge(void);

#define triangleBLOCKSIZE 80

int triangle_count(void);
void initialize_triangles(void);
void clear_all_triangles(void);
struct triangle *new_triangle(struct edge *e1, struct edge *e2,
   struct edge *e3, int o1, int o2, int o3, Poly *p);
struct triangle *first_triangle(void);
struct triangle *get_last_triangle(void);

#define vertexBLOCKSIZE 40

void initialize_vertexs(void);
void clear_all_vertexs(void);
int vertex_count(void);
struct vertex *new_vertex(Point3 *pt, struct vertex *v1, struct vertex *v2);
struct vertex *simple_new_vertex(HPoint3 *pt, ColorA *col);
struct vertex *first_vertex(void);
struct vertex *get_last_vertex(void);

/* decls for cm_geometry.c */

/* any function of type "splitfunc" takes an edge, decides whether to 
   split it or not and then returns either a pointer to the midpoint or
   a null pointer.
*/
typedef struct vertex *(*splitfunc)(struct edge *e, double cosmaxbend);
void projective_to_conformal(int curv, HPoint3 *proj, Transform T,
				Point3 *poinc);
void projective_vector_to_conformal(int curv, HPoint3 *pt,  Point3 *v,
				    Transform T, 
				    Point3 *ppt, Point3 *pv);
struct vertex * edge_split(struct edge *e, double cosmaxbend);
void edge_polar_point(int curv, const Point3 *a, const Point3 *b, HPoint3 *p);
void triangle_polar_point(int curv,
			  const Point3 *a, const Point3 *b, const Point3 *c, 
			  HPoint3 *p);

int vertex_count(void);
int triangle_count(void);

#define FALSE 0
#define TRUE 1

#endif
