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
#ifndef _GV_BSPTREE_H_
#define _GV_BSPTREE_H_

#include "geom.h"

typedef struct BSPTree BSPTree;

extern BSPTree *BSPTreeCreate(BSPTree *tree, Geom *object);
extern void BSPTreeAddObject(BSPTree *bsp_tree, Geom *object);
extern void BSPTreeFinalize(BSPTree *bsp);
extern void BSPTreeFree(BSPTree *bsp);
extern void BSPTreeFreeTree(const BSPTree *tree);
extern void BSPTreeSetAppearance(Geom *object);
extern BSPTree *BSPTreeSet(BSPTree *tree, int attr1, ...);

/* Geometries attached to INSTs or LISTs can be deleted underneath the
 * BSP-tree; so INSTs and LISTs must register a callback with their
 * handles.
 */
extern void BSPTreeInvalidate(Handle **chp, Ref *unused, BSPTree *tree);

/* Attributes for BSPTreeSet */
#define BSPTREE_ONESHOT 42001
#define BSPTREE_END     42999

#endif

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
