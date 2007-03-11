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

List *ListDraw(List *list)
{
  List *l;

  GeomMakePath(list, 'L', path, pathlen);

  list->geomflags &= ~GEOM_ALPHA;

  for (l = list; l != NULL; l = l->cdr) {
    int lpathlen = pathlen+1;
    char *lpath = alloca(lpathlen+1);

    memcpy(lpath, path, pathlen);
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
    path = lpath;
    pathlen = lpathlen;
  }

  return list;
}

List *ListBSPTree(List *list, BSPTree *bsptree, int action)
{
  List *l;

  GeomMakePath(list, 'L', path, pathlen);

  switch (action) {
  case BSPTREE_CREATE:
    for (l = list; l != NULL; l = l->cdr) {
      int lpathlen = pathlen+1;
      char *lpath = alloca(lpathlen+1);
      
      memcpy(lpath, path, pathlen);
      lpath[lpathlen-1] = 'l';
      lpath[lpathlen] = '\0';

      if (l->car) {
	l->car->ppath = lpath;
	l->car->ppathlen = lpathlen;
	GeomBSPTree(l->car, bsptree, action);
      }
      HandleRegister(&l->carhandle, (Ref *)l, bsptree, BSPTreeInvalidate);

      path = lpath;
      pathlen = lpathlen;
    }
    return list;
    
  case BSPTREE_DELETE:
    for (l = list; l != NULL; l = l->cdr) {
      int lpathlen = pathlen+1;
      char *lpath = alloca(lpathlen+1);
      
      memcpy(lpath, path, pathlen);
      lpath[lpathlen-1] = 'l';
      lpath[lpathlen] = '\0';

      if (l->car) {
	l->car->ppath = lpath;
	l->car->ppathlen = lpathlen;
	GeomBSPTree(l->car, bsptree, action);
      }
      HandleUnregisterJust(&l->carhandle, (Ref *)l, bsptree, BSPTreeInvalidate);

      path = lpath;
      pathlen = lpathlen;
    }
    return list;
    
  case BSPTREE_ADDGEOM:
    for (l = list; l != NULL; l = l->cdr) {
      int lpathlen = pathlen+1;
      char *lpath = alloca(lpathlen+1);
      
      memcpy(lpath, path, pathlen);
      lpath[lpathlen-1] = 'l';
      lpath[lpathlen] = '\0';

      if (l->car) {
	l->car->ppath = lpath;
	l->car->ppathlen = lpathlen;
	GeomBSPTree(l->car, bsptree, action);
      }

      path = lpath;
      pathlen = lpathlen;
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
