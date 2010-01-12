/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2006 Claus-Justus Heine
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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "listP.h"
#include "mg.h"
#include "bsptreeP.h"

#define INITIAL_ALLOC_SIZE (1 << 5)

List *ListDraw(List *list)
{
  List *l;
  char *lpath = NULL;
  int lpathlen, allocsz;

  GeomMakePath(list, 'L', path, pathlen);

  list->geomflags &= ~GEOM_ALPHA;

  /* Use alloca() for moderately large lists, fall back to malloc
   * afterwards.
   */
  allocsz = INITIAL_ALLOC_SIZE;
  if (pathlen < allocsz) {
    lpath = alloca(allocsz);
    memcpy(lpath, path, pathlen+1);
  } else {
    lpath = path;
  }

  for (lpathlen = pathlen+1, l = list; l != NULL; l = l->cdr, ++lpathlen) {

    if (lpathlen >= allocsz) {
      /* give up on alloca */
      bool doFree = allocsz > INITIAL_ALLOC_SIZE;
      while (lpathlen >= allocsz) {
	allocsz <<= 1;
      }
      path = OOGLNewNE(char, allocsz, "List PATH");
      memcpy(path, lpath, lpathlen-1);
      if (doFree) {
	OOGLFree(lpath);
      }
      lpath = path;
    }

    lpath[lpathlen-1] = 'l';
    lpath[lpathlen] = '\0';

    if (l->car) {
      l->car->ppath = lpath;
      l->car->ppathlen = lpathlen;
      GeomDraw(l->car);
      if (l->car->geomflags & GEOM_ALPHA) {
	list->geomflags |= GEOM_ALPHA;
      }
    }
  }

  if (allocsz > INITIAL_ALLOC_SIZE) {
    OOGLFree(lpath);
  }

  return list;
}

List *ListBSPTree(List *list, BSPTree *bsptree, int action)
{
  List *l;
  char *lpath = NULL;
  int lpathlen, allocsz;

  GeomMakePath(list, 'L', path, pathlen);

  switch (action) {
  case BSPTREE_CREATE:
    
    /* Use alloca() for moderately large lists, fall back to malloc
     * afterwards.
     */
    allocsz = INITIAL_ALLOC_SIZE;
    if (pathlen < allocsz) {
      lpath = alloca(allocsz);
      memcpy(lpath, path, pathlen+1);
    } else {
      lpath = path;
    }

    for (lpathlen = pathlen+1, l = list; l != NULL; l = l->cdr, ++lpathlen) {

      if (lpathlen >= allocsz) {
	/* give up on alloca */
	bool doFree = allocsz > INITIAL_ALLOC_SIZE;
	while (lpathlen >= allocsz) {
	  allocsz <<= 1;
	}
	path = OOGLNewNE(char, allocsz, "List PATH");
	memcpy(path, lpath, lpathlen-1);
	if (doFree) {
	  OOGLFree(lpath);
	}
	lpath = path;
      }

      lpath[lpathlen-1] = 'l';
      lpath[lpathlen] = '\0';

      if (l->car) {
	l->car->ppath = lpath;
	l->car->ppathlen = lpathlen;
	GeomBSPTree(l->car, bsptree, action);
      }
      HandleRegister(&l->carhandle, (Ref *)l, bsptree, BSPTreeInvalidate);
    }

    if (allocsz > INITIAL_ALLOC_SIZE) {
      OOGLFree(lpath);
    }

    return list;
    
  case BSPTREE_DELETE:

    /* Use alloca() for moderately large lists, fall back to malloc
     * afterwards.
     */
    allocsz = INITIAL_ALLOC_SIZE;
    if (pathlen < allocsz) {
      lpath = alloca(allocsz);
      memcpy(lpath, path, pathlen+1);
    } else {
      lpath = path;
    }

    for (lpathlen = pathlen+1, l = list; l != NULL; l = l->cdr, ++lpathlen) {

      if (lpathlen >= allocsz) {
	/* give up on alloca */
	bool doFree = allocsz > INITIAL_ALLOC_SIZE;
	while (lpathlen >= allocsz) {
	  allocsz <<= 1;
	}
	path = OOGLNewNE(char, allocsz, "List PATH");
	memcpy(path, lpath, lpathlen-1);
	if (doFree) {
	  OOGLFree(lpath);
	}
	lpath = path;
      }
      
      lpath[lpathlen-1] = 'l';
      lpath[lpathlen] = '\0';

      if (l->car) {
	l->car->ppath = lpath;
	l->car->ppathlen = lpathlen;
	GeomBSPTree(l->car, bsptree, action);
      }
      HandleUnregisterJust(&l->carhandle, (Ref *)l, bsptree, BSPTreeInvalidate);
    }

    if (allocsz > INITIAL_ALLOC_SIZE) {
      OOGLFree(lpath);
    }

    return list;
    
  case BSPTREE_ADDGEOM:

    /* Use alloca() for moderately large lists, fall back to malloc
     * afterwards.
     */
    allocsz = INITIAL_ALLOC_SIZE;
    if (pathlen < allocsz) {
      lpath = alloca(allocsz);
      memcpy(lpath, path, pathlen+1);
    } else {
      lpath = path;
    }

    for (lpathlen = pathlen+1, l = list; l != NULL; l = l->cdr, ++lpathlen) {

      if (lpathlen >= allocsz) {
	/* give up on alloca */
	bool doFree = allocsz > INITIAL_ALLOC_SIZE;
	while (lpathlen >= allocsz) {
	  allocsz <<= 1;
	}
	path = OOGLNewNE(char, allocsz, "List PATH");
	memcpy(path, lpath, lpathlen-1);
	if (doFree) {
	  OOGLFree(lpath);
	}
	lpath = path;
      }
      
      lpath[lpathlen-1] = 'l';
      lpath[lpathlen] = '\0';

      if (l->car) {
	l->car->ppath = lpath;
	l->car->ppathlen = lpathlen;
	GeomBSPTree(l->car, bsptree, action);
      }
    }

    if (allocsz > INITIAL_ALLOC_SIZE) {
      OOGLFree(lpath);
    }

    return list;
    
  default:
    return NULL;
  }
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
