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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "geomclass.h"
#include "mg.h"
#include "bsptreeP.h"

void GeomDrawNodeDataFreeListPrune(void)
{
  NodeDataFreeListPrune();
}

static inline void maybe_tag_appearance(Geom *geom, const Appearance *ap)
{
  NodeData *data;

  if (GeomHasAlpha(geom, ap)) {
    data = GeomNodeDataCreate(geom, NULL);
    if (data->tagged_ap) {
      mguntagappearance(data->tagged_ap);
      data->tagged_ap = NULL;
    }
    data->tagged_ap = mgtagappearance();
  } else if ((data = GeomNodeDataByPath(geom, NULL)) && data->tagged_ap) {
    mguntagappearance(data->tagged_ap);
    data->tagged_ap = NULL;
  }
}

Geom *GeomDraw(Geom *geom)
{
  if (geom && geom->Class->draw) {
    const Appearance *ap;

    if (geom->bsptree != NULL) {
      mgNDctx *NDctx = NULL;
      mgctxget(MG_NDCTX, &NDctx);
      if (NDctx != NULL) {
	NDctx->bsptree = geom->bsptree;
	BSPTreeSet(NDctx->bsptree, BSPTREE_ONESHOT, true, BSPTREE_END);
	BSPTreeSetId(geom->bsptree);
      }
    }

    if (geom->ap != NULL) {
      mgpushappearance();
      ap = mgsetappearance(geom->ap, 1);	/* Merge into inherited ap */
    } else {
      ap = mggetappearance();
    }
    maybe_tag_appearance(geom, ap);
    
    (*geom->Class->draw)(geom);

    if(geom->ap != NULL) {
      mgpopappearance();
    }

    if (geom->bsptree != NULL && (geom->geomflags & GEOM_ALPHA)) {
      GeomBSPTreeDraw(geom);
    }
  }

  return geom;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
