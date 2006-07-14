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

#include <stdio.h>
#include <stdlib.h>
#include "cmodelP.h"

/* created from link_list.c */

struct vertex_block {
   struct vertex_block *next;
   struct vertex block[ vertexBLOCKSIZE ];
   };

static struct vertex_block first_vertex_block, *curr_vertex_block;
static struct vertex *last_vertex;

static int vx_count = 0;

int vertex_count()
{
   return vx_count;
   }

/* once off initialization at the start */

void initialize_vertexs()
{
   curr_vertex_block = &first_vertex_block;
   first_vertex_block.next = NULL;
   last_vertex = first_vertex_block.block;
   first_vertex_block.block->next = NULL;
   vx_count = 0;
   return;
   }

struct vertex *first_vertex()
{
   return first_vertex_block.block->next;
   }

void clear_all_vertexs()
{
   struct vertex_block *this_block, *tmp;
   
   /* free each block */
   this_block = first_vertex_block.next;
   while(this_block != NULL) {
      tmp = this_block->next;
      free(this_block);
      this_block = tmp;
      }
   
   initialize_vertexs();
   return;
   }

struct vertex_block *new_vertex_block()
{
   struct vertex_block *n_block;
   
   curr_vertex_block->next = n_block = 
      (struct vertex_block *)malloc(sizeof(struct vertex_block));
   n_block->next = NULL;
   return curr_vertex_block = n_block;
   }

struct vertex *new_vertex(Point3 *pt, struct vertex *v1, struct vertex *v2)
{
   struct vertex *n_vertex;
   
   /* allocate a new vertex and link it in */
   n_vertex = last_vertex + 1;
   if (n_vertex - curr_vertex_block->block >= vertexBLOCKSIZE)
      n_vertex = (new_vertex_block())->block;

   last_vertex->next = n_vertex;
   n_vertex->next = NULL;
   last_vertex = n_vertex;
   vx_count++;
   
   /* now initialize n_vertex */
   Pt3Copy(pt, (Point3 *)(void *)&n_vertex->V.pt);
   n_vertex->V.pt.w = 1.;
   /* interpolate between two vertex colors */
   if (v2 != NULL) {
      n_vertex->V.vcol.r = .5 * v1->V.vcol.r + .5 * v2->V.vcol.r;
      n_vertex->V.vcol.g = .5 * v1->V.vcol.g + .5 * v2->V.vcol.g;
      n_vertex->V.vcol.b = .5 * v1->V.vcol.b + .5 * v2->V.vcol.b;
      n_vertex->V.vcol.a = .5 * v1->V.vcol.a + .5 * v2->V.vcol.a;
      } 
   else
      n_vertex->V.vcol = v1->V.vcol;
   n_vertex->visible = FALSE;
   HPt3Copy(&v1->polar, &n_vertex->polar);
   
   return n_vertex;
   }

struct vertex *simple_new_vertex(HPoint3 *pt, ColorA *col)
{
   struct vertex *n_vertex;
   
   /* allocate a new vertex and link it in */
   n_vertex = last_vertex + 1;
   if (n_vertex - curr_vertex_block->block >= vertexBLOCKSIZE)
      n_vertex = (new_vertex_block())->block;

   last_vertex->next = n_vertex;
   n_vertex->next = NULL;
   last_vertex = n_vertex;
   vx_count++;
   
   /* now initialize n_vertex */
   HPt3Copy(pt, &n_vertex->V.pt);
   n_vertex->V.vcol = *col;
   n_vertex->visible = FALSE;
   /* polar is not used in this case */
      
   return n_vertex;
   }

struct edge_block {
   struct edge_block *next;
   struct edge block[ edgeBLOCKSIZE ];
   };

static struct edge_block first_edge_block, *curr_edge_block;
static struct edge *last_edge;

/* once off initialization at the start */

void initialize_edges()
{
   curr_edge_block = &first_edge_block;
   first_edge_block.next = NULL;
   last_edge = first_edge_block.block;
   first_edge_block.block->next = NULL;
   return;
   }

struct edge *first_edge()
{
   return first_edge_block.block->next;
   }

struct edge *get_last_edge()
{
   return last_edge;
   }

void clear_all_edges()
{
   struct edge_block *this_block, *tmp;
   
   /* free each block */
   this_block = first_edge_block.next;
   while(this_block != NULL) {
      tmp = this_block->next;
      free(this_block);
      this_block = tmp;
      }
   
   initialize_edges();
   return;
   }

struct edge_block *new_edge_block()
{
   struct edge_block *n_block;
   curr_edge_block->next = n_block = 
      (struct edge_block *)malloc(sizeof(struct edge_block));
   n_block->next = NULL;
   return curr_edge_block = n_block;
   }

struct edge *new_edge(struct vertex *v1, struct vertex *v2, HPoint3 *polar)		      
{
   struct edge *n_edge;
   
   /* allocate a new edge and link it in */
   n_edge = last_edge + 1;
   if (n_edge - curr_edge_block->block >= edgeBLOCKSIZE)
      n_edge = (new_edge_block())->block;

   last_edge->next = n_edge;
   n_edge->next = NULL;
   last_edge = n_edge;
   
   /* now initialize n_edge */
   n_edge->v1 = v1;
   n_edge->v2 = v2;
   HPt3Copy(polar, &n_edge->polar);
   n_edge->small = n_edge->split = n_edge->visible = n_edge->hascolor = FALSE;
   n_edge->other_half = NULL;
   
   return n_edge;
   }

/* created from link_list.c */

struct triangle_block {
   struct triangle_block *next;
   struct triangle block[ triangleBLOCKSIZE ];
   };

static struct triangle_block first_triangle_block, *curr_triangle_block;
static struct triangle *last_triangle;

static int tri_count = 0;

int triangle_count()
{
   return tri_count;
   }

/* once off initialization at the start */

void initialize_triangles()
{
   curr_triangle_block = &first_triangle_block;
   first_triangle_block.next = NULL;
   last_triangle = first_triangle_block.block;
   first_triangle_block.block->next = NULL;
   tri_count = 0;
   return;
   }

struct triangle *first_triangle()
{
   return first_triangle_block.block->next;
   }

struct triangle *get_last_triangle()
{
   return last_triangle;
   }

void clear_all_triangles()
{
   struct triangle_block *this_block, *tmp;
   
   /* free each block */
   this_block = first_triangle_block.next;
   while(this_block != NULL) {
      tmp = this_block->next;
      free(this_block);
      this_block = tmp;
      }
   
   initialize_triangles();
   return;
   }

struct triangle_block *new_triangle_block()
{
   struct triangle_block *n_block;
   
   curr_triangle_block->next = n_block = 
      (struct triangle_block *)malloc(sizeof(struct triangle_block));
   n_block->next = NULL;
   return curr_triangle_block = n_block;
   }

struct triangle *new_triangle(struct edge *e1, struct edge *e2, 
   struct edge *e3, int o1, int o2, int o3, Poly *orig)
{
   struct triangle *t;
   
   /* allocate a new triangle and link it in */
   t = last_triangle + 1;
   if (t - curr_triangle_block->block >= triangleBLOCKSIZE)
      t = (new_triangle_block())->block;

   last_triangle->next = t;
   t->next = NULL;
   last_triangle = t;
   
   /* now initialize t */
   t->small = FALSE;
   t->e1 = e1;
   t->e2 = e2;
   t->e3 = e3;
   t->o1 = o1;
   t->o2 = o2;
   t->o3 = o3;
   t->orig_poly = orig;
   ++tri_count;
   
   return t;
   }

