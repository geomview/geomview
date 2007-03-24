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

 Simple doubly linked list implementation. Somewhat inspired from the
 stuff used in the Linux kernel.

 */

#ifndef _GV_LIST_H_
#define _GV_LIST_H_

typedef struct dbllistnode 
{
  struct dbllistnode *next;
  struct dbllistnode *prev;
} DblListNode;

/* DblListNode Head = DBLLISTINIT(Head); */
#define DBLLISTINIT(name, head) { &name.head, &name.head }
#define DBLLIST(name) DblListNode name = { &name, &name }

/* list is empty if the list head is the only member */
static inline void DblListInit(DblListNode *head)
{
  head->next = head;
  head->prev = head;
}

static inline bool DblListEmpty(DblListNode *head)
{
  return head->next == head;
}

/* add in front, right after head */
static inline void DblListAdd(DblListNode *head, DblListNode *node)
{
  node->next       = head->next;
  head->next->prev = node;
  head->next       = node;
  node->prev       = head;
}

/* add at the tail, right before head */
static inline void DblListAddTail(DblListNode *head, DblListNode *node)
{
  node->prev       = head->prev;
  head->prev->next = node;
  head->prev       = node;
  node->next       = head;
}

/* One nice feature of doubly linked lists is that one does not need a
 * pointer to the head to do a deletion.
 */
static inline void DblListDelete(DblListNode *node)
{
  node->next->prev = node->prev;
  node->prev->next = node->next;
  DblListInit(node);
}

/* Copy an entire DblList, i.e. reparent it, give it a new head.
 */
static inline void DblListMove(DblListNode *fromhead, DblListNode *tohead)
{
  fromhead->next->prev = tohead;
  fromhead->prev->next = tohead;
  tohead->next = fromhead->next;
  tohead->prev = fromhead->prev;
}

/* Given a list-node "node" construct a pointer to the containing
 * structure of type "ctype" which has a member with name "nodename"
 * (of type DblListNode).
 */
#define DblListContainer(node, ctype, nodename)			\
  ((ctype *)((char *)(node) - (char *)&((ctype *)0)->nodename))

/* Iterate over the list; head is the list-head, ctype the type of the
 * containing structure, nodemember the name of the list-head inside
 * ctype, pos is the loop variable and must be of type ctype, next is
 * a temporary helper variable to protect against deletion of list
 * members.
 */
#define DblListIterate(head, ctype, nodename, pos, nextp)		\
  for (pos = DblListContainer((head)->next, ctype, nodename),		\
	 nextp = DblListContainer(pos->nodename.next, ctype, nodename);	\
       pos->nodename.next != (head)->next;				\
       pos = nextp,							\
	 nextp = DblListContainer(pos->nodename.next, ctype, nodename))


#define DblListIterateNoDelete(head, ctype, nodename, pos)		\
  for (pos = DblListContainer((head)->next, ctype, nodename);		\
       pos->nodename.next != (head)->next;				\
       pos = DblListContainer(pos->nodename.next, ctype, nodename))

#endif

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
