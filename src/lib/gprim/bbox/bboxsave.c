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

BBox *BBoxFSave(BBox *bbox, FILE *f, char *fname)
{
    if ((bbox->geomflags & VERT_4D) && bbox->minN == NULL) {
	fprintf(f, "4BBOX\n%g %g %g %g\n%g %g %g %g\n",
		bbox->min.x, bbox->min.y, bbox->min.z, bbox->min.w,
	bbox->max.x, bbox->max.y, bbox->max.z, bbox->max.w);
    } else if (bbox->minN == NULL) {
	fprintf(f, "BBOX\n%g %g %g\n%g %g %g\n",
		bbox->min.x, bbox->min.y, bbox->min.z,
		bbox->max.x, bbox->max.y, bbox->max.z);
    } else if (bbox->geomflags & VERT_4D) {
	int i;
	
	fprintf(f, "4nBBOX %d\n", bbox->pdim);
	for (i = 0; i < bbox->pdim; i++) {
	    fprintf(f, " %g", bbox->minN->v[i]);
	}
	fprintf(f, "\n");
	for (i = 0; i < bbox->pdim; i++) {
	    fprintf(f, " %g", bbox->maxN->v[i]);
	}
    } else {
	int i;
	
	fprintf(f, "nBBOX %d\n", bbox->pdim-1);
	for (i = 0; i < bbox->pdim; i++) {
	    fprintf(f, " %g", bbox->minN->v[i]);
	}
	fprintf(f, "\n");
	for (i = 0; i < bbox->pdim-1; i++) {
	    fprintf(f, " %g", bbox->maxN->v[i]);
	}
    }
    return bbox;
}
