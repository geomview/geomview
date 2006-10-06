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
	bbox->flag = BBOX_P;
	bbox->minN = bbox->maxN = NULL;
	bbox->pdim = 3;
        GGeomInit (bbox, classp, BBOXMAGIC, NULL);
    } else {
	/* Check that exist is in fact a BBox. */
	bbox = exist;
    }

    while ((attr = va_arg (*a_list, int)))
	switch (attr) {
	case CR_FLAG:
	    bbox->flag = va_arg (*a_list, int );
	    break;
	case CR_MIN:
	    Pt3ToPt4(va_arg(*a_list, Point3 *), &bbox->min, 1);
	    bbox->pdim = 3;
	    break;
	case CR_MAX:
	    Pt3ToPt4(va_arg(*a_list, Point3 *), &bbox->max, 1);
	    bbox->pdim = 3;
	    break;
	case CR_4MIN:
	    bbox->min = *va_arg(*a_list, HPoint3 *);
	    bbox->geomflags |= VERT_4D;
	    bbox->pdim = 4;
	    break;
	case CR_4MAX:
	    bbox->max = *va_arg(*a_list, HPoint3 *);
	    bbox->geomflags |= VERT_4D;
	    bbox->pdim = 4;
	    break;
	case CR_NMIN:
	    tmp = va_arg(*a_list, HPointN *);
	    if (tmp != bbox->minN) { /* could happen, BBoxUnion3() ... */
		HPtNDelete(bbox->minN);
		bbox->minN = HPtNCreate(tmp->dim, tmp->v);
	    }	    
	    bbox->geomflags |= VERT_ND;
	    bbox->pdim  = bbox->minN->dim;
	    break;
	case CR_NMAX:
	    tmp = va_arg(*a_list, HPointN *);
	    if (tmp != bbox->maxN) { /* could happen, BBoxUnion3() ... */
		HPtNDelete(bbox->maxN);
		bbox->maxN = HPtNCreate(tmp->dim, tmp->v);
	    }	    
	    bbox->geomflags |= VERT_ND;
	    bbox->pdim  = bbox->maxN->dim;
	    break;
	default:
	    if (GeomDecorate (bbox, &copy, attr, a_list)) {
		OOGLError (0, "BBoxCreate: Undefined attribute: %d", attr);
		HPtNDelete(bbox->minN);
		HPtNDelete(bbox->maxN);
		OOGLFree (bbox);
		return NULL;
	    }
    }

    if (bbox->minN && bbox->maxN && bbox->minN->dim != bbox->maxN->dim) {
	OOGLError (0, "BBoxCreate: dimensions do of minN/maxN "
		   "vectors do not match: %d/%d",
		   bbox->minN->dim, bbox->maxN->dim);
	HPtNDelete(bbox->minN);
	HPtNDelete(bbox->maxN);
	OOGLFree (bbox);
	return NULL;
    }

    if (exist != NULL) return exist;
    
    return (bbox);
}

void BBoxDelete(BBox *bbox)
{
    if (bbox) {
	HPtNDelete(bbox->minN);
	HPtNDelete(bbox->maxN);
    }
}
