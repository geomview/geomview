/* Copyright (C) 2007 Claus-Justus Heine 
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
#ifndef _GV_NODEDATA_H_
#define _GV_NODEDATA_H_

#include "geomclass.h"
#include "mg.h"

/* A geometry hierarchy may refer aribitrarily often to the same
 * Geometry object via "Handle" references. In some cases, however, we
 * need data which is attached to a node in the hierarchy tree. For
 * the time being the only case is the "tagged apperance" handle used
 * to refer to appearances out of BSPTree nodes.
 *
 * "path" is a unique tag and is computed during hierarchy
 * traversal. 'L' stands for a list element, 'I' for Inst.
 *
 * L -> (L) -> nil
 * |     |
 * G1    I
 *       |
 *       G2
 *
 * The leaf-nodes of the hierarchy will have the paths "L" and
 * "LI". This should be a unique id for earch hierarchy node, because
 * it records the history of the path through the hierarchy to the
 * node. Only the head of a list is included in the path, so the
 * second id is "LI" and not "LLI" in the example above.
 */
typedef struct NodeData {
  DblListNode   node;       /* Link into Geom->pernode */
  char          *ppath;     /* The path to this geom in the hierarchy (copy).*/
  const void    *tagged_ap; /* tagged appearance pointer */
  const BSPTree *node_tree; /* BSP-tree we belong to, if any */
} NodeData;

#define GeomMakePath(geom, c, path, pathlen)		\
  int pathlen = (geom)->ppathlen+1;			\
  char *path = alloca(pathlen+1);			\
  							\
  memcpy(path, (geom)->ppath, (geom)->ppathlen);	\
  path[pathlen-1] = c;					\
  path[pathlen] = '\0'

/* Transfer one Geom's per-node data to another Geom. Priamrily meant
 * as hack during ND-drawing
 */
static inline void GeomNodeDataMove(Geom *from, Geom *to)
{
  DblListMove(&from->pernode, &to->pernode);
  to->ppath = from->ppath;
  to->ppathlen = from->ppathlen;
}

static inline NodeData *GeomNodeDataByPath(Geom *geom, const char *ppath)
{
  NodeData *pos;
  
  if (!ppath) {
    ppath = geom->ppath ? geom->ppath : "";
  }
  DblListIterateNoDelete(&geom->pernode, NodeData, node, pos) {
    if (strcmp(pos->ppath, ppath) == 0) {
      return pos;
    }
  }

  return NULL;
}

extern DEF_FREELIST(NodeData);

static inline NodeData *GeomNodeDataCreate(Geom *geom, const char *ppath)
{
  NodeData *data;

  if (!ppath) {
    ppath = geom->ppath ? geom->ppath : "";
  }
  data = GeomNodeDataByPath(geom, ppath);
  if (data == NULL) {
    FREELIST_NEW(NodeData, data);
    data->ppath = strdup(ppath);
    data->tagged_ap = NULL;
    data->node_tree = NULL;
    DblListAdd(&geom->pernode, &data->node);
  }

  return data;
}

static inline void GeomNodeDataPrune(Geom *geom)
{
  NodeData *data, *data_next;

  DblListIterate(&geom->pernode, NodeData, node, data, data_next) {
    DblListDelete(&data->node);
    if (data->tagged_ap) {
      mguntagappearance(data->tagged_ap);
    }
    if (data->node_tree) {
      BSPTreeFreeTree(data->node_tree);
    }
    if (data->ppath) {
	free(data->ppath);
	data->ppath = NULL;
    }
    FREELIST_FREE(NodeData, data);
  }
}

static inline void NodeDataFreeListPrune(void)
{
  FreeListNode *old;
  size_t size = 0;
  
  while (NodeDataFreeList) {
    old = NodeDataFreeList;
    NodeDataFreeList = old->next;
    OOGLFree(old);
    size += sizeof(NodeData);
  }
  OOGLWarn("Freed %ld bytes.\n", size);
}

#endif
