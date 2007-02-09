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
  BSPTreeNode    *tree;     /* The root of the BSPtree itself */
  Geom           *geom;     /* The top-level Geom we belong to */
  int            geomflags; /* COLOR_ALPHA set if any component of the
			     * tree has an alpha channel. Filled by
			     * BSPTreeSetAppearance().
			     */

  TransformPtr   T;         /* INST support: transform polygons during
			     * tree generation, i.e. before adding
			     * them to init_lpl.
			     */
  PolyListNode   *init_lpl; /* While tree == NULL elements can be
			     * added to this list
			     */
  const void **tagged_app;

  struct obstack obst;  /* Scratch space for new polygons etc */
};

static inline const void **BSPTreePushAppearance(Geom *geom,
						 const Appearance *ap)
{
  if (!ap) {
    ap = geom->ap;
  }
  if (geom->bsptree != NULL && ap != NULL) {
    const void **tagged_app = geom->bsptree->tagged_app;
    geom->bsptree->tagged_app = &geom->tagged_ap;
    return tagged_app;
  } else {
    return NULL;
  }
}

static inline void BSPTreePopAppearance(Geom *geom, const void **old_tagged_app)
{
  if (geom->bsptree != NULL && old_tagged_app != NULL) {
    geom->bsptree->tagged_app = old_tagged_app;
  }
}

#endif

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
