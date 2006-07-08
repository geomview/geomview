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

#include "bezierP.h"

/*
 * Bezier creation, deletion and editing.
 */
Bezier *
BezierCopy( register Bezier *ob )
{
	register Bezier *b;
	register int n;

	if(ob == NULL) return NULL;

	b = OOGLNewE(Bezier, "new Bezier");

	*b = *ob;	/* Copy all fields */
	GGeomInit(b, BezierMethods(), BEZIERMAGIC, NULL);

	if(b->flag & BEZ_ST) {
	    if(ob->STCords == NULL) {
               OOGLError(0,"Inconsistency in BEZ_ST field of flag");
	       b->flag &= ~BEZ_ST;
            } else {
		b->STCords = OOGLNewNE(float, 4*2, "Bezier ST coords");
		memcpy(b->STCords, ob->STCords, 4*2*sizeof(float));
	    }
	} else
	    b->STCords = NULL;
	
	if(ob->CtrlPnts != NULL) {
	    n = (b->degree_u + 1) * (b->degree_v + 1) * b->dimn;
	    b->CtrlPnts = OOGLNewNE(float, n, "Bezier control points");
	    memcpy(b->CtrlPnts, ob->CtrlPnts, n * sizeof(float));
	}

	if(b->flag & BEZ_REMESH)
	    b->mesh = (Mesh *) GeomCCreate (NULL, MeshMethods(), CR_END);
	else if(ob->mesh != NULL)
	    b->mesh = (Mesh *) GeomCopy((Geom *)ob->mesh);

	return(b);
}

void
BezierDelete( Bezier *bezier )
{
	if(bezier) {
	    if(bezier->CtrlPnts != NULL)
		OOGLFree(bezier->CtrlPnts);
	    if(bezier->STCords != NULL)
		OOGLFree(bezier->STCords);
	    if(bezier->mesh != NULL)
		GeomDelete((Geom *)bezier->mesh);
	}
}

/* ZZZ: note:  BezierCreate doesn't observe the copy directive: always
copies pointers */
Bezier *
BezierCreate ( Bezier *exist, GeomClass *classp, va_list *a_list )
{
    register Bezier *bezier;
    int attr, copy = 1;
    ColorA *color = NULL;
    int i;

    if (exist == NULL) {
	bezier = OOGLNewE(Bezier, "BezierCreate Bezier");
	memset(bezier, 0, sizeof(Bezier));
        GGeomInit (bezier, classp, BEZIERMAGIC, NULL);
	bezier->flag = BEZ_P;
	bezier->mesh = NULL;
	bezier->CtrlPnts = NULL;
	bezier->STCords = NULL;
	bezier->nu = bezier->nv = 0;	/* no mesh yet */
    } else {
	/* Check that exist is a Bezier. */
	bezier = exist;
    }

    while ((attr = va_arg (*a_list, int))) switch (attr) {
	case CR_FLAG:
	    bezier->flag = va_arg (*a_list, int);
	    break;
	case CR_DEGU:
	    bezier->degree_u = va_arg (*a_list, int);
	    break;
	case CR_DEGV:
	    bezier->degree_v = va_arg (*a_list, int);
	    break;
	case CR_DIM:
	    bezier->dimn = va_arg (*a_list, int);
	    break;
	case CR_NU:
	    bezier->nu = va_arg (*a_list, int);
	    break;
	case CR_NV:
	    bezier->nv = va_arg (*a_list, int);
	    break;
	case CR_POINT:
	    bezier->CtrlPnts = va_arg (*a_list, float *);
	    break;
	case CR_MESH:
	    bezier->mesh = va_arg (*a_list, Mesh *);
	    break;
	case CR_ST:
	    bezier->STCords = va_arg(*a_list, float *);
	    break;
	case CR_COLOR:
	    color = va_arg (*a_list, ColorA *);
	    if (color != NULL) for (i=0; i<4; i++) bezier->c[i] = color[i];
	    break;
	default:
	    if (GeomDecorate (bezier, &copy, attr, a_list)) {
		OOGLError (0, "BezierCreate: undefined option: %d", attr);
		OOGLFree (bezier);
		return NULL;
	    }
    }

    if (bezier->dimn > MAX_BEZ_DIMN) {
	OOGLError (0, "BezierCreate: dimension (%d) too high.", bezier->dimn);
	OOGLFree (bezier);
	return NULL;
    }

    if (exist != NULL) return exist;

    return bezier;
}
