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
QuadFSave(Quad *q, FILE *f, char *fname)
{
	int i;
	HPoint3 *p;
	Point3 *n;
	ColorA *c;

	(void)fname;

	if(q == NULL || f == NULL)
		return(NULL);
	if(q->geomflags & QUAD_C) fputc('C', f);
	if(q->geomflags & QUAD_N) fputc('N', f);
	if(q->geomflags & VERT_4D) fputc('4', f);
	fprintf(f, "QUAD\n");
	p = &q->p[0][0];
	n = (q->geomflags & QUAD_N) ? &q->n[0][0] : NULL;
	c = (q->geomflags & QUAD_C) ? &q->c[0][0] : NULL;
	for(i = 4 * q->maxquad; --i >= 0 && !ferror(f); ) {
	    if (q->geomflags & VERT_4D) fprintf(f, "%g %g %g %g",
			p->x, p->y, p->z, p->w);
	    else  fprintf(f, "%g %g %g",
			p->x, p->y, p->z);
	    p++;
	    if(n) {
		fprintf(f, "  %g %g %g",
			n->x, n->y, n->z);
		n++;
	    }
	    if(c) {
		fprintf(f, "  %.3g %.3g %.3g %.3g",
			c->r, c->g, c->b, c->a);
		c++;
	    }
	    fputc('\n', f);
	    if((i & 3) == 0)
		fputc('\n', f);
	}
	return(ferror(f) ? NULL : q);
}
