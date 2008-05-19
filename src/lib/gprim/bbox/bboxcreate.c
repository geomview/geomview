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

DEF_FREELIST(BBox);

void BBoxFreeListPrune(void)
{
  FreeListNode *old;
  size_t size = 0;

  HPtNFreeListPrune();
  while (BBoxFreeList) {
    old = BBoxFreeList;
    BBoxFreeList = old->next;
    OOGLFree(old);
    size += sizeof(BBox);
  }
  OOGLWarn("Freed %ld bytes.\n", size);
}

#ifndef max
# define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
# define min(a,b) ((a) < (b) ? (a) : (b))
#endif

/* Retrieve min/max/center of bbox */
int BBoxGet(BBox *bbox, int attr, void *attrp)
{
  HPoint3 min3, max3;

  switch (attr) {
  case CR_FLAG: *(int *)attrp = 0; break;
  case CR_MIN:
    HPtNToHPt3(bbox->min, NULL, &min3);
    HPt3Dehomogenize(&min3, &min3);
    memcpy(attrp, &min3, sizeof(Point3));
    break;
  case CR_MAX:
    HPtNToHPt3(bbox->max, NULL, &max3);
    HPt3Dehomogenize(&max3, &max3);
    memcpy(attrp, &max3, sizeof(Point3));
    break;
  case CR_4MIN: HPtNToHPt3(bbox->min, NULL, (HPoint3 *)attrp); break;
  case CR_4MAX: HPtNToHPt3(bbox->max, NULL, (HPoint3 *)attrp); break;
  case CR_NMIN: *(HPointN **)attrp = HPtNCopy(bbox->min, NULL); break;
  case CR_NMAX: *(HPointN **)attrp = HPtNCopy(bbox->max, NULL); break;
  case CR_CENTER: HPtNToHPt3(bbox->center, NULL, (HPoint3 *)attrp); break;
  case CR_NCENTER: *(HPointN **)attrp = HPtNCopy(bbox->center, NULL); break;
  default:
    return -1;
  }
  return 1;
}

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
  HPoint3 min3, max3;
  int min3p = 0, max3p = 0;
  HPointN *minN = NULL, *maxN = NULL;
  int minNp = 0, maxNp = 0;
  BBox *bbox;
  int attr, copy = 1, need_update = 0;

  if (exist == NULL) {
    FREELIST_NEW(BBox, bbox);
    GGeomInit(bbox, classp, BBOXMAGIC, NULL);
    bbox->freelisthead = &BBoxFreeList;
    bbox->min = HPtNCreate(4, NULL);
    bbox->max = HPtNCreate(4, NULL);
    bbox->center = HPtNCreate(4, NULL);
  } else {
    /* FIXME: Check that exist is in fact a BBox. */
    bbox = exist;
  }

  while ((attr = va_arg (*a_list, int))) {
    switch (attr) {
    case CR_FLAG:
      break;
    case CR_MIN:
      if (minNp || maxNp) {
	OOGLError(1, "BBoxCreate(): ND minimum already specified.\n");
	goto error;
      }
      Pt3ToHPt3(va_arg(*a_list, Point3 *), &min3, 1);
      min3p = need_update = 1;
      break;
    case CR_MAX:
      if (minNp || maxNp) {
	OOGLError(1, "BBoxCreate(): ND maximum already specified.\n");
	goto error;
      }
      Pt3ToHPt3(va_arg(*a_list, Point3 *), &max3, 1);
      max3p = need_update = 1;
      break;
    case CR_4MIN:
      if (minNp || maxNp) {
	OOGLError(1, "BBoxCreate(): ND minimum already specified.\n");
	goto error;
      }
      min3 = *va_arg(*a_list, HPoint3 *);
      min3p = need_update = 1;
      break;
    case CR_4MAX:
      if (minNp || maxNp) {
	OOGLError(1, "BBoxCreate(): ND maximum already specified.\n");
	goto error;
      }
      max3 = *va_arg(*a_list, HPoint3 *);
      max3p = need_update = 1;
      break;
    case CR_NMIN:
      if (min3p || max3p) {
	OOGLError(1, "BBoxCreate(): 3D minimum already specified.\n");
	goto error;
      }
      minN = va_arg(*a_list, HPointN *);
      minNp = need_update = 1;
      break;
    case CR_NMAX:
      if (min3p || max3p) {
	OOGLError(1, "BBoxCreate(): 3D maximum already specified.\n");
	goto error;
      }
      maxN = va_arg(*a_list, HPointN *);
      maxNp = need_update = 1;
      break;
    default:
      if (GeomDecorate (bbox, &copy, attr, a_list) && exist == NULL) {
	OOGLError(0, "BBoxCreate: Undefined attribute: %d", attr);
	HPtNDelete(bbox->min);
	HPtNDelete(bbox->max);
	HPtNDelete(bbox->center);
	FREELIST_FREE(BBox, bbox);
	return NULL;
      }
    }
  }

  /* The VERT_4D flag is set by GeomDecorate(), so it is only now that
   * we can decide what to do.
   */

  if (need_update) {
    int pdim = bbox->pdim;

    if (min3p && max3p) {
      if (bbox->geomflags & VERT_4D)
	pdim = 5;
      else
	pdim = 4;
    }
    if (minNp) {
      pdim = max(pdim, minN->dim);
    }
    if (maxNp) {
      pdim = max(pdim, maxN->dim);
    }
    if (bbox->min->dim != pdim) {
      bbox->min->v = OOGLRenewNE(HPtNCoord, bbox->min->v, pdim,
				 "Renew min coords");
      bbox->min->dim = pdim;
    }
    if (bbox->max->dim != pdim) {
      bbox->max->v = OOGLRenewNE(HPtNCoord, bbox->max->v, pdim,
				 "Renew max coords");
      bbox->max->dim = pdim;
    }
    if (min3p) {
      if (bbox->geomflags & VERT_4D)
	Pt4ToHPtN(&min3, bbox->min);
      else
	HPt3ToHPtN(&min3, NULL, bbox->min);
    }
    if (max3p) {
      if (bbox->geomflags & VERT_4D)
	Pt4ToHPtN(&max3, bbox->max);
      else
	HPt3ToHPtN(&max3, NULL, bbox->max);
    }
    if (minNp) {
      HPtNCopy(minN, bbox->min);
    }
    if (maxNp) {
      HPtNCopy(maxN, bbox->max);
    }
    bbox->pdim = pdim;
    bbox->center = BBoxCenterND(bbox, bbox->center);
  }

  bbox->geomflags &= ~VERT_4D; /* bboxes are always N-dim */

  if (exist != NULL) return exist;
    
  return bbox;

 error:
  if (exist == NULL) {
    HPtNDelete(bbox->min);
    HPtNDelete(bbox->max);
    HPtNDelete(bbox->center);
    FREELIST_FREE(BBox, bbox);
  }
  return NULL;
}

void BBoxDelete(BBox *bbox)
{
  if (bbox) {
    HPtNDelete(bbox->min);
    HPtNDelete(bbox->max);
    HPtNDelete(bbox->center);
  }
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
