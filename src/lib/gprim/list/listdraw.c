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

#if 0
  if (list->bsptree == NULL) {
    BSPTreeCreate((Geom *)list);
    for (l = list; l != NULL; l = l->cdr) {
      if (l->bsptree) {
	abort();
      }
      l->bsptree = list->bsptree;
    }
  }
#endif

  for (l = list; l != NULL; l = l->cdr) {
    GeomDraw(l->car);
  }

#if 0
  /* If we have a private BSP-tree, then draw it now. Software shading
   * with transparency will not work, to be fixed.
   */
  if (list->bsptree->geom == (Geom *)list) {
    if (list->bsptree->tree == NULL) {
      BSPTreeFinalize(list->bsptree);
    }
    mgbsptree(list->bsptree);
  }
#endif
  
  return list;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
