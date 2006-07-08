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

#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
#include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "mg.h"
#include "bezierP.h"

Bezier *
BezierDraw( bezier )
    Bezier *bezier;
{
    Appearance *ap;
    
    ap = mggetappearance();
    if(mgfeature(MGF_BEZIER)>0) {
	mgbezier( bezier->degree_u, bezier->degree_v , bezier->dimn,
		  bezier->CtrlPnts, bezier->STCords, 
		  bezier->flag & BEZ_C ? bezier->c : NULL );
    } else {
	if (ap->valid & APF_DICE) {
	    bezier->nu = ap->dice[0];
	    bezier->nv = ap->dice[1];
	}
	if (bezier->mesh == NULL ||
	    bezier->mesh->nu != bezier->nu ||
	    bezier->mesh->nv != bezier->nv) 
		bezier->flag |= BEZ_REMESH; 
    
	if (bezier->flag & BEZ_REMESH) {
		BezierReDice(bezier);
	}  
	GeomDraw( (Geom *)bezier->mesh );
    }
        
    return bezier;
}

