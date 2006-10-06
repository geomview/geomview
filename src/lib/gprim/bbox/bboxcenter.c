/* Copyright (C) 2006 Claus-Justus Heine
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

#include "bboxP.h"

HPoint3 *BBoxCenter(BBox *bbox, HPoint3 *center)
{
    static HPoint3 space;

    if (!center)
	center = &space;

    if (bbox->geomflags & VERT_4D) {
	center->x = (bbox->min.x + bbox->max.x) / 2.0;
	center->y = (bbox->min.y + bbox->max.y) / 2.0;
	center->z = (bbox->min.z + bbox->max.z) / 2.0;
	center->w = (bbox->min.w + bbox->max.w) / 2.0;
    } else {
	HPt3Dehomogenize(&bbox->min, &bbox->min);
	HPt3Dehomogenize(&bbox->max, &bbox->max);
	center->x = (bbox->min.x + bbox->max.x) / 2.0;
	center->y = (bbox->min.y + bbox->max.y) / 2.0;
	center->z = (bbox->min.z + bbox->max.z) / 2.0;
	center->w = 1.0;
    }

    return center;
}

HPointN *BBoxCenterND(BBox *bbox, HPointN *center)
{
    int i;

    if (!center)
	center = HPtNCreate(bbox->pdim, NULL);

    if (bbox->pdim < 5) {
	HPt3Coord center4[4];
	HPt3Coord *min = (HPt3Coord *)&bbox->min;
	HPt3Coord *max = (HPt3Coord *)&bbox->max;

	for (i = 0; i < 4; i++) {
	    center4[i] = 0.5 * (min[i] + max[i]);
	}
	Pt4ToHPtN((HPoint3 *)center4, center);
    } else {
	HPtNDehomogenize(bbox->minN, bbox->minN);
	HPtNDehomogenize(bbox->maxN, bbox->maxN);
	for (i = 0; i < bbox->pdim-1; i++) {
	    center->v[i] = 0.5 * (bbox->minN->v[i] + bbox->maxN->v[i]);
	}
	center->v[i] = 1.0;
    }

    return center;
}

/*
 * Local Variables: ***
 * c-basic-offset: 4 ***
 * End: ***
 */
