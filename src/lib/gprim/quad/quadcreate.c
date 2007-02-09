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

#include "quadP.h"

Quad *
QuadCreate (Quad *exist, GeomClass *classp, va_list *a_list)
{
    Quad *q;
    QuadP *p = (QuadP *)NULL;
    Point3 *p3;
    QuadN *n = (QuadN *)NULL;
    QuadC *c = (QuadC *)NULL;
    int attr, i;
    int copy = 1;

    if (exist == NULL) {
	q = OOGLNewE(Quad, "QuadCreate: new Quad");
        GGeomInit (q, classp, QUADMAGIC, NULL);
	q->geomflags = q->maxquad = 0;
        q->p = (QuadP *)NULL;
        q->n = (QuadN *)NULL;
        q->c = (QuadC *)NULL;
    } else {
	/* Check that exist is a Quad. */
	q = exist;
    }

    q->pdim = 4;

    while ((attr = va_arg(*a_list, int)))   /* parse argument list */
      switch (attr) {
	case CR_FLAG:
            q->geomflags = va_arg(*a_list, int);
	    break;

	case CR_NELEM:
	    q->maxquad = va_arg(*a_list, int);
	    break;

	case CR_POINT4:
	    p = va_arg(*a_list, QuadP *);
            if (exist) OOGLFree(q->p);
            if (p == NULL) {
               q->p = NULL;
               q->maxquad = 0;
            } else if (copy) {
		q->p = OOGLNewNE(QuadP,q->maxquad,"QuadCreate vertices");
		memcpy(q->p, p, q->maxquad * sizeof(QuadP));
            } else {
		q->p = p; /* caller relinquishes ownership of data */
	    }
	    break;

	case CR_POINT:
	    p3 = va_arg(*a_list, Point3 *);
            if (exist) OOGLFree(q->p);
            if (p3 == NULL) {
               q->p = NULL;
               q->maxquad = 0;
            } else if (copy) {
		q->p = OOGLNewNE(QuadP,q->maxquad,"QuadCreate vertices");
		Pt3ToHPt3(p3, &q->p[0][0], q->maxquad * 4);
            } else {
		Pt3ToHPt3(p3, &q->p[0][0], q->maxquad * 4);
	    }
	    break;

	case CR_NORMAL:
            q->geomflags &= ~QUAD_N;
            if (exist && q->n)
		OOGLFree(q->n);

	    n = va_arg(*a_list, QuadN *);
	    if (n == NULL) {
		q->n = NULL;
            } else if (copy) {
		q->n = OOGLNewNE(QuadN,q->maxquad,"QuadCreate normals");
		memcpy(q->n, n, q->maxquad*sizeof(QuadN));
		q->geomflags |= QUAD_N;
            } else {
		q->n = n;
		q->geomflags |= QUAD_N;
	    }
	    break;

	case CR_COLOR:
	    q->geomflags &= ~COLOR_ALPHA;
	    q->geomflags &= ~QUAD_C;
            if (exist && q->c)
		OOGLFree(q->c);

	    c = va_arg(*a_list, QuadC *);
	    if (c == NULL) {
		q->c = NULL;
            } else {
		q->geomflags |= QUAD_C;
		if (copy) {
		    q->c = OOGLNewNE(QuadC, q->maxquad, "QuadCreate: colors");
		    memcpy(q->c, c, q->maxquad*sizeof(QuadC));
		} else {
		    q->c = c;
		    q->geomflags |= QUAD_C;
		}
		for (i = 0; i < q->maxquad; i++) {
		    if (q->c[i][0].a < 1.0 ||
			q->c[i][1].a < 1.0 ||
			q->c[i][2].a < 1.0 ||
			q->c[i][3].a < 1.0) {
			q->geomflags |= COLOR_ALPHA;
		    }    
		}
	    }
	    break;

	default:
            if (GeomDecorate(q, &copy, attr, a_list)) {
	       OOGLError (0, "QuadCreate: Undefined option: %d",attr);
	       if (!exist) GeomDelete((Geom *)q);
	       return NULL;
            }
      }

    if ((q->p == NULL && q->maxquad > 0)
	||
        (q->p != NULL && q->maxquad <= 0)
	) {
	OOGLError (0,"QuadCreate: inconsistent number of quads");
	if (!exist) GeomDelete((Geom *)q);
	return NULL;
    }

    return (Quad *) q;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
