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
#include "discgrpP.h"

BBox *
DiscGrpBound(DiscGrp *discgrp, Transform T, TransformN *TN)
{
    BBox *geombbox;
    Transform Tnew;
    GeomIter *it;
#ifdef BADVELOCITY	/* strange flying mode velocity comps require this */
    int nels;
#endif

    (void)TN;

    if( discgrp == NULL)
	return NULL;

    if (T == NULL)
	T = TM_IDENTITY;

    if (discgrp->geom == NULL)  return NULL;

    it = GeomIterate( (Geom *)discgrp, DEEP );
    geombbox = NULL;

    /* this is a horrible hack to allow us to continue to fly around at
     * reasonable speeds when in centercam mode, since the speed is
     * proportional to the bbox of the world.  Solution: we only look
     * at the identity element's bbox */
#ifdef BADVELOCITY	/* strange flying mode velocity comps require this */
    if (discgrp->attributes & DG_SPHERICAL || discgrp->flag & DG_CENTERCAM) {
        nels = discgrp->big_list->num_el;
        discgrp->big_list->num_el = 1;
	}
#endif

    while(NextTransform(it, Tnew) > 0) {
	BBox *box;

	TmConcat( Tnew,  T, Tnew );
	if((box = (BBox *)GeomBound(discgrp->geom, Tnew, NULL)) != NULL) {
	    if(geombbox) {
		BBoxUnion3(geombbox, box, geombbox);
		GeomDelete((Geom *)box);
	    } else
		geombbox = box;
	}
    }
#ifdef BADVELOCITY
    if (discgrp->attributes & DG_SPHERICAL || discgrp->flag & DG_CENTERCAM)
	discgrp->big_list->num_el = nels;
#endif
    return geombbox;
}

