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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef _GV_REFERENCE_H_
#define _GV_REFERENCE_H_

#include "ooglutil.h"
#include "dbllist.h"

/*
 * Public definitions for Reference & Communications package
 */

#define REFERENCEFIELDS				\
  unsigned magic;				\
  int ref_count;				\
  DblListNode handles

typedef struct Ref  {
  REFERENCEFIELDS;
} Ref ;

#define REFCNT(obj)       RefCount((Ref *)(obj))
#define	REFINCR(obj)      RefIncr((Ref *)(obj))
#define	REFDECR(obj)      RefDecr((Ref *)(obj))
#define REFGET(type, obj) (REFINCR((obj)), (type *)(obj))
#define REFPUT(obj)       REFDECR(obj)

/* Initializes a Reference */
static inline void RefInit(Ref *ref, int magic)
{
  ref->ref_count = 1;
  ref->magic = magic;
  DblListInit(&ref->handles);
}

/* Returns current ref count */
static inline int RefCount(Ref *ref)
{
  return ref != NULL ? ref->ref_count : 0;
}

/* Increments count, returns object */
static inline int RefIncr(Ref *ref)
{
  if (ref != NULL) {
    return ++ref->ref_count;
  } else {
    return 0;
  }
}

/* Decrements count, returns it  */
static inline int RefDecr(Ref *ref)
{
  if (ref == NULL) {
    return 0;
  }
  if (--ref->ref_count < 0) {
    OOGLError(1, "RefDecr: ref %x count %d < 0!", ref, ref->ref_count);
    abort();
  }
  return ref->ref_count;
}

/* Returns magic number */
static inline int RefMagic(Ref *ref) {
  return ref->magic;
}

#endif

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
