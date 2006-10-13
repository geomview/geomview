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

#include "bboxP.h"
#include "appearance.h"

/*--------------------------------------------------------------------------
 * Function:	BBoxCreate
 * Description: creates or edits a bounding box
 * Args:	BBox *exist: existing bbox to be edited, if using editing.
 *				NULL otherwise.
 *		GeomClass *classp: 
 *		a_list: List of option type - option pairs.  Option types
 *			are specified by integers, with 0 signifying end of
 *			list.
 * Returns:	Pointer to BBox created or edited.
 */

BBox *BBoxCreate (BBox *exist, GeomClass *classp, va_list *a_list)
{
  HPointN *tmp;
  BBox *bbox;
  int attr, copy = 1;

  if (exist == NULL) {
    bbox = OOGLNewE(BBox, "BBoxCreate BBox");
    bbox->min = HPtNCreate(4, NULL);
    bbox->max = HPtNCreate(4, NULL);
    bbox->pdim = 4;
    GGeomInit (bbox, classp, BBOXMAGIC, NULL);
  } else {
    /* FIXME: Check that exist is in fact a BBox. */
    bbox = exist;
  }

  while ((attr = va_arg (*a_list, int)))
    switch (attr) {
    case CR_FLAG:
	  
      break;
    case CR_MIN:
      Pt3ToPt4(va_arg(*a_list, Point3 *), (HPoint3 *)bbox->min->v, 1);
      if (bbox->pdim > 4) {
	bbox->min->v = OOGLRenewNE(HPtNCoord, bbox->min->v, 4, "renew HPointN");
	bbox->max->v = OOGLRenewNE(HPtNCoord, bbox->max->v, 4, "renew HPointN");
	bbox->min->dim = bbox->max->dim = bbox->pdim = 4;
      }
      break;
    case CR_MAX:
      Pt3ToPt4(va_arg(*a_list, Point3 *), (HPoint3 *)bbox->max->v, 1);
      if (bbox->pdim > 4) {
	bbox->min->v = OOGLRenewNE(HPtNCoord, bbox->min->v, 4, "renew HPointN");
	bbox->max->v = OOGLRenewNE(HPtNCoord, bbox->max->v, 4, "renew HPointN");
	bbox->min->dim = bbox->max->dim = bbox->pdim = 4;
      }
      break;
    case CR_4MIN:
      *(HPoint3 *)bbox->min->v = *va_arg(*a_list, HPoint3 *);
      bbox->geomflags |= VERT_4D;
      if (bbox->pdim > 4) {
	bbox->min->v = OOGLRenewNE(HPtNCoord, bbox->min->v, 4, "renew HPointN");
	bbox->max->v = OOGLRenewNE(HPtNCoord, bbox->max->v, 4, "renew HPointN");
	bbox->min->dim = bbox->max->dim = bbox->pdim = 4;
      }
      break;
    case CR_4MAX:
      *(HPoint3 *)bbox->max->v = *va_arg(*a_list, HPoint3 *);
      bbox->geomflags |= VERT_4D;
      if (bbox->pdim > 4) {
	bbox->min->v = OOGLRenewNE(HPtNCoord, bbox->min->v, 4, "renew HPointN");
	bbox->max->v = OOGLRenewNE(HPtNCoord, bbox->max->v, 4, "renew HPointN");
	bbox->min->dim = bbox->max->dim = bbox->pdim = 4;
      }
      break;
    case CR_NMIN:
      tmp = va_arg(*a_list, HPointN *);
      if (tmp != bbox->min) { /* == could happen, BBoxUnion3() ... */
	HPtNCopy(tmp, bbox->min);
	if (bbox->pdim != tmp->dim) {
	  bbox->max->v =
	    OOGLRenewNE(HPtNCoord, bbox->max->v, tmp->dim, "renew HPointN");
	  bbox->max->dim = bbox->pdim = tmp->dim;
	}
      }
      bbox->geomflags |= VERT_ND;
      break;
    case CR_NMAX:
      tmp = va_arg(*a_list, HPointN *);
      if (tmp != bbox->max) { /* == could happen, BBoxUnion3() ... */
	HPtNCopy(tmp, bbox->max);
	if (bbox->pdim != tmp->dim) {
	  bbox->min->v =
	    OOGLRenewNE(HPtNCoord, bbox->min->v, tmp->dim, "renew HPointN");
	  bbox->min->dim = bbox->pdim = tmp->dim;
	}
      }
      bbox->geomflags |= VERT_ND;
      break;
    default:
      if (GeomDecorate (bbox, &copy, attr, a_list)) {
	OOGLError (0, "BBoxCreate: Undefined attribute: %d", attr);
	HPtNDelete(bbox->min);
	HPtNDelete(bbox->max);
	OOGLFree (bbox);
	return NULL;
      }
    }

  if (exist != NULL) return exist;
    
  return (bbox);
}

void BBoxDelete(BBox *bbox)
{
  if (bbox) {
    HPtNDelete(bbox->min);
    HPtNDelete(bbox->max);
  }
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
