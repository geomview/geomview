/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2006-2007 Claus-Justus Heine
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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips,
 * Celeste Fowler */

#ifndef PICKPDEF
#define	PICKPDEF

/*
 * Private definitions for picking.
 */

#include "pick.h"
#include "ooglutil.h"

struct Pick {
    Point3 got;

    float thresh;

    int want;		/* Fields wanted */
    int  found;		/* Fields found */

    vvec gcur;              /* Path to the current primitive - 
			       used for recursion */
    vvec gpath;		/* Path to picked primitive */
    Geom *gprim;		/* Picked primitive */

    HPoint3 v;		/* picked vertex, if any */
    int vi;			/* index of picked vertex */

    HPoint3 e[2];		/* endpoints of picked edge, if any */
    int ei[2];		/* indices of endpoints of picked edge */

    Transform Tprim;	/* gprim -> screen */
    Transform Tmirp;
    Transform Tw;
    Transform Tself;

    /* for ND-viewing. */
    TransformN *TprimN;
    TransformN *TmirpN;
    int        axes[4];     /* the relevant sub-space of the output of 
			     * TprimN.
			     */
    TransformN *TwN;
    TransformN *TselfN;

    HPoint3 *f;		/* array of vertices of picked face, if any */
    int fn;			/* number of vertices in array f */
    int fi;			/* index of picked face */

    Transform Ts2n;		/* NDC to screen tfm */
    Transform Tc2n;		/* NDC to camera tfm */
    Transform Tw2n;		/* NDC to world (global) tfm */
    float x0, y0;		/* NDC coords of original pick */
};

/* 
 * PickFace() is used to test a face for a pick and modify the pick 
 * structure if necessary. 
 *   n_verts is the number of vertices in the face.
 *   verts is the list of vertices in the face.
 *   pick is the pick structure.
 *   ap is the appearance (not currently used).
 * The following fields of the pick structure will be filled in:
 *   found - what has been found.
 *   vi - index of the picked vertex in the verts array.
 *   ei - indices of the endpoints of the picked edge in the verts array.
 *   f - freed if non-NULL and set to NULL
 * PickFace() returns nonzero if something was picked.  The calling 
 * program is responsible for using the data returned in the pick 
 * structure to fill in the actual points and correct the indices.
 */
int PickFace(int n_verts, Point3 *verts, Pick *pick, Appearance *ap);

/*
 * PickFillIn() is used to fill in the fields of a pick structure once
 * a pick has been found.  
 *   pick is the pick structure.
 *   n_verts is the number of points in the face.
 *   got is the interesection of the pick ray with the face.
 *   vertex is the index of the picked vertex (-1 if none).
 *   edge is the index of the first vertex of the hit line (-1 if none).
 *   ep is the picked point on the edge.
 * The routine fills in the following fields of the pick structure:
 *   got - filled in with the parameter got. 
 *   found - filled in with what was found.
 *   vi - the index of the picked vertex. 
 *   ei - the indices of the endpoints of the picked edge. 
 *   f - freed if non-null and set to null.
 *   fn - the number of vertices in the face
 *   gpath - the path to the picked primitive (copy of gcur)
 * The rest of the fields are left unchanged.
 * Returns pick->found.
 */
int PickFillIn(Pick *pick, int n_verts, Point3 *got, int vertex, 
	       int edge, Appearance *ap);


#endif /*PICKPDEF*/

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
