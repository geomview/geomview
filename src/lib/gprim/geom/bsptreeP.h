/* Copyright (C) 2006 Claus-Justus Heine 
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
#ifndef _GV_BSPTREE_P_H_
#define _GV_BSPTREE_P_H_

#include "geomclass.h"
#include "bsptree.h"

#include "obstack.h"

struct Poly;

typedef struct PolyListNode PolyListNode;
struct PolyListNode
{
  PolyListNode *next;
  struct Poly  *poly;
  Point3       *pn; /* If we split a polygon we compute the normal on
		     * the parent to avoid numerical instabilities
		     * caused by increasingly degenerated polygons.
		     */
#if 0
  /* To really support merging different geometries into a single BSP
   * tree we would need to keep track of the possibly different
   * apperances. Only afterwards we can support INSTs and LISTs. TODO.
   */
  Appearance   *ap;
#endif
};

typedef struct BSPTreeNode BSPTreeNode;
struct BSPTreeNode
{
  BSPTreeNode  *front;
  BSPTreeNode  *back;
  HPoint3      plane;
  PolyListNode *polylist;
};

struct BSPTree {
  BSPTreeNode    *tree;     /* The root of the BSPtree itself */
  Geom           *geom;     /* The top-level Geom we belong to */
  TransformPtr   T;         /* INST support: transform polygons during
			     * tree generation, i.e. before adding
			     * them to init_lpl.
			     */
  PolyListNode   *init_lpl; /* While tree == NULL elements can be
			     * added to this list
			     */
  struct obstack obst;  /* Scratch space for new polygons etc */
};

#endif

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
