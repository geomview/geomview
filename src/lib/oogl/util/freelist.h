/*
 * Copyright (C) 2007 Claus-Justus Heine

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; see the file COPYING.  If not, write to the
 Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 */

/* Support for allocating objects from a free-list (instead of using
 * malloc() small fixed-size objects which are allocated frequently).
 */

#ifndef _GV_FREE_LIST_H_
#define _GV_FREE_LIST_H_

typedef struct FreeListNode
{
  struct FreeListNode *next;
} FreeListNode;

#define DEF_FREELISTHEAD(type) FreeListNode *type##FreeList

#define DEF_FREELIST(type)			\
  DEF_FREELISTHEAD(type);			\
  typedef union type##FreeNode {		\
    FreeListNode node;				\
    type data;					\
  } type##FreeNode

#define FREELIST_NEW(type, name)		\
  if (type##FreeList == NULL) {			\
    name = OOGLNewE(type, #type);		\
    memset(name, 0, sizeof(type));		\
  } else {					\
    type##FreeNode *reused;			\
    reused = (type##FreeNode *)type##FreeList;	\
    type##FreeList = reused->node.next;		\
    name = &reused->data;			\
  }

#define FREELIST_FREE(type, name)			\
{							\
  type##FreeNode *reuse = (type##FreeNode *)(name);	\
  reuse->node.next = type##FreeList;			\
  type##FreeList = &reuse->node;			\
}

#endif

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
