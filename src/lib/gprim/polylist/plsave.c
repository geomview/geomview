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

#include "polylistP.h"

PolyList *
PolyListFSave(PolyList *polylist, FILE *outf, char *fname)
{
	int i, n;
	Poly *p;
	Vertex **vp, *v;

	(void)fname;

	/* We don't really know the number of edges and it's a pain to count.
	 * Assume Euler number 2.
	 */
	fprintf(outf, "%s%s%s%sOFF\n%d %d %d\n",
		&"ST"[polylist->geomflags & PL_HASST ? 0 : 2],
		&"C"[polylist->geomflags & PL_HASVCOL ? 0 : 1],
		&"N"[polylist->geomflags & PL_HASVN ? 0 : 1],
		&"4"[polylist->geomflags & VERT_4D ? 0 : 1],
		polylist->n_verts, polylist->n_polys,
		polylist->n_verts + polylist->n_polys - 2);

	for(i = polylist->n_verts, v = polylist->vl; --i >= 0; v++) {
	  if (polylist->geomflags & VERT_4D)
	    fprintf(outf, "\n%.8g %.8g %.8g %.8g",
		    v->pt.x, v->pt.y, v->pt.z, v->pt.w);
	  else 
	    fprintf(outf, "\n%.8g %.8g %.8g", v->pt.x, v->pt.y, v->pt.z);
	    if(polylist->geomflags & PL_HASVN)
		fprintf(outf, "  %.8g %.8g %.8g", v->vn.x, v->vn.y, v->vn.z);
	    if(polylist->geomflags & PL_HASVCOL)
		fprintf(outf, "  %g %g %g %g",
			v->vcol.r, v->vcol.g, v->vcol.b, v->vcol.a);
	    if(polylist->geomflags & PL_HASST)
		fprintf(outf, "  %.8g %.8g", v->st.s, v->st.t);
	}

	fputc('\n', outf);
	for(i = polylist->n_polys, p = polylist->p; --i >= 0; p++) {
	    n = p->n_vertices;
	    fprintf(outf, "\n%d	", n);
	    for(vp = p->v; --n >= 0; vp++)
		    fprintf(outf, " %d", (int)((*vp) - polylist->vl));
	    if ((polylist->geomflags & PL_HASPCOL)) {
	      fprintf(outf, "\t%g %g %g %g",
		      p->pcol.r, p->pcol.g, p->pcol.b, p->pcol.a);
	    }
	}
	fputc('\n', outf);

	return (ferror(outf) ? NULL : polylist);

}
