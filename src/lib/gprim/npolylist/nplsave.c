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

/*
 * Save a PolyList (in .off format).
 */

#include "npolylistP.h"

NPolyList *
NPolyListFSave(NPolyList *pl, FILE *outf, char *fname)
{
	int i, k;
	NPoly *p;
	float *v;
	ColorA *c;

	/* We don't really know the number of edges and it's a pain to count.
	 * Assume Euler number 2.
	 */
	fprintf(outf, "%s%snOFF %d\n%d %d %d\n",
		&"C"[pl->flags & PL_HASVCOL ? 0 : 1],
		&"4"[pl->geomflags & VERT_4D ? 0 : 1],
		pl->pdim - 1,
		pl->n_verts, pl->n_polys,
		0);

	for(i = pl->n_verts, v = pl->v, c = pl->vcol; --i >= 0; ) {
	  if(pl->geomflags & VERT_4D) {
	    for(k = pl->pdim; --k >= 0; )
		fprintf(outf, "%g ", *v++);
	  } else {
	    float denom = v[0];
	    for(k = pl->pdim; --k > 0; )
		fprintf(outf, "%g ", *++v/denom);
	  }
	  if(pl->flags & PL_HASVCOL) {
	    fprintf(outf, "  %g %g %g %g", c->r, c->g, c->b, c->a);
	    c++;
	  }
	  fputc('\n', outf);
	}

	fputc('\n', outf);
	for(i = pl->n_polys, p = pl->p; --i >= 0; p++) {
	    fprintf(outf, "\n%d	", p->n_vertices);
	    for(k = 0; k < p->n_vertices; k++)
		fprintf(outf, " %d", pl->vi[p->vi0 + k]);
	    if((pl->flags & (PL_HASPCOL|PL_HASVCOL)) == PL_HASPCOL) {
		fprintf(outf, "\t%g %g %g %g",
			p->pcol.r, p->pcol.g, p->pcol.b, p->pcol.a);
	    }
	}
	fputc('\n', outf);

	return (ferror(outf) ? NULL : pl);
}
