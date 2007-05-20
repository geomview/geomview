/* Copyright (C) 2006-2007 Claus-Justus Heine 
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
#include "nodedata.h"
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

  const void   **tagged_app; /* pointer to shared storage for the
			      * return value of mgtagappearance().
			      */
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
  BSPTreeNode  *tree;     /* The root of the BSPtree itself */
  Geom         *geom;     /* The top-level Geom we belong to */
  bool         oneshot;   /* Tree is destroyed after drawing; useful
			   * for location/oring != LOCAL INSTs and
			   * during ND-drawing. The oneshot flag is
			   * reset after it took affect.
			   */
  PolyListNode   *init_lpl; /* While tree == NULL elements can be
			     * added to this list
			     */
  TransformPtr Tid;       /* INST support: to support absolute
			   * positioning (location and origin !=
			   * LOCAL) we need the position of the
			   * top-level geometry (the one who owns the
			   * tree, i.e. tree->geom).
			   */
  TransformPtr   Tidinv;    /* Inverse of Tid, only computed on demand */
  TransformPtr   T;         /* INST support: transform polygons during
			     * tree generation, i.e. before adding
			     * them to init_lpl.
			     */
  Transform      Tdual;     /* We need the dual of T to transform the normals
			     * correctly ( y^t x = 0 <=> y^t T T^{-tr}x = 0).
			     */
  TransformPtr   TxT;       /* texture transform */
  const void **tagged_app;

  struct obstack obst;  /* Scratch space for new polygons etc */
};

/* if we have an appearance (but no handle to an appearance ) which
 * overrides the transparency flag to false _or_ a material which
 * overrides the alpha value to 1.0 (and no own color spec with alpha
 * != 1.0) then we need not add ourselves to the BSP-tree: no facet of
 * this Geom will ever be translucent.
 *
 * This does, of course, not work with INSTs or LISTs, but only with
 * "atomic" Geom's
 */
static inline bool never_translucent(Geom *geom)
{
  Appearance *ap = geom->ap;

  return (ap && !geom->aphandle &&
	  (((ap->override & APF_FACEDRAW) && (ap->flag & APF_FACEDRAW) == 0)
	   ||
	   ((ap->override & APF_TRANSP) && (ap->flag & APF_TRANSP) == 0)
	   ||
	   ((geom->geomflags & COLOR_ALPHA) == 0 && ap->mat &&
	    (ap->mat->override & MTF_ALPHA) && ap->mat->diffuse.a == 1.0)));
}

/* Load the current transform form the MG-layer into tree->Tid */
static inline void BSPTreeSetId(BSPTree *tree)
{
  Transform T;

  if (tree->geom == NULL || tree != tree->geom->bsptree) {
    abort();
  }
      
  /* make sure the top-level geom has per-node data */
  GeomNodeDataCreate(tree->geom, NULL);

  mggettransform(T);
  if (memcmp(T, TM_IDENTITY, sizeof(Transform)) != 0) {
    tree->Tid = obstack_alloc(&tree->obst, sizeof(Transform));
    TmCopy(T, tree->Tid);
  } else {
    tree->Tid = TM_IDENTITY;
  }
  tree->Tidinv = NULL;
}

static inline const void **BSPTreePushAppearance(BSPTree *bsptree, Geom *geom)
{
  NodeData *data = GeomNodeDataCreate(geom, NULL);
  const void **tagged_app = bsptree->tagged_app;

  bsptree->tagged_app = &data->tagged_ap;
  return tagged_app;
}

static inline void
BSPTreePopAppearance(BSPTree *bsptree, const void **old_tagged_app)
{
  if (bsptree != NULL && old_tagged_app != NULL) {
    bsptree->tagged_app = old_tagged_app;
  }
}

static inline TransformPtr BSPTreePushTransform(BSPTree *tree, TransformPtr T)
{
  TransformPtr old_T;

  if (tree != NULL) {
    old_T = tree->T;
    tree->T = T;
    if (T != TM_IDENTITY) {
      TmDual(T, tree->Tdual);
    }
    return old_T;
  } else {
    return TM_IDENTITY;
  }
}

static inline void BSPTreeSetTransform(BSPTree *tree, TransformPtr T)
{
  if (tree != NULL) {
    tree->T = T;
    if (T != TM_IDENTITY) {
      TmDual(T, tree->Tdual);
    }
  }
}

static inline void BSPTreePopTransform(BSPTree *tree, TransformPtr old_T)
{
  if (tree != NULL) {
    tree->T = old_T;
    if (old_T != TM_IDENTITY) {
      TmDual(old_T, tree->Tdual);
    }
  }
}

static inline TransformPtr BSPTreePushTxTransform(BSPTree *tree, TransformPtr T)
{
  TransformPtr old_TxT;

  if (tree != NULL) {
    old_TxT = tree->TxT;
    tree->TxT = T;
    return old_TxT;
  } else {
    return TM_IDENTITY;
  }
}

static inline void BSPTreeSetTxTransform(BSPTree *tree, TransformPtr T)
{
  if (tree != NULL) {
    tree->TxT = T;
  }
}

static inline void BSPTreePopTxTransform(BSPTree *tree, TransformPtr old_T)
{
  if (tree != NULL) {
    tree->TxT = old_T;
  }
}

#endif

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
