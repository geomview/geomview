/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2006 Claus-Justus Heine
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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips, Claus-Justus Heine */

/*
 * Save a NPolyList (in .off format).
 */

#include "npolylistP.h"

NPolyList *
NPolyListFSave(NPolyList *pl, FILE *outf, char *fname)
{
  int i, k;
  HPtNCoord *v;
  ColorA *c;

  (void)fname;

  /* We don't really know the number of edges and it's a pain to count.
   * Assume Euler number 2.
   */
  if (pl->pdim == 4) {
    fprintf(outf, "%s%s%sOFF\n%d %d %d\n",
	    pl->geomflags & PL_HASST ? "ST" : "",
	    pl->geomflags & PL_HASVCOL ? "C" : "",
	    pl->geomflags & VERT_4D ? "4" : "",
	    pl->n_verts, pl->n_polys,
	    0);
  } else {
    fprintf(outf, "%s%s%snOFF %d\n%d %d %d\n",
	    pl->geomflags & PL_HASST ? "ST" : "",
	    pl->geomflags & PL_HASVCOL ? "C" : "",
	    pl->geomflags & VERT_4D ? "4" : "",
	    pl->pdim - 1,
	    pl->n_verts, pl->n_polys,
	    0);
  }

  for(i = 0, v = pl->v, c = pl->vcol; i < pl->n_verts; ++i, ++c) {
    if(pl->geomflags & VERT_4D) {
      if (pl->pdim == 4) {
	float denom = *v++;
	for(k = pl->pdim-1; --k >= 0; ) {
	  fprintf(outf, "%.8g ", *v++);
	}
	fprintf(outf, "%.8g ", denom);
      } else {
	for(k = pl->pdim; --k >= 0; )
	  fprintf(outf, "%.8g ", *v++);
      }
    } else {
      float denom = *v++;
      for(k = pl->pdim; --k > 0; )
	fprintf(outf, "%.8g ", *v++/denom);
    }
    
    if(pl->geomflags & PL_HASVCOL) {
      fprintf(outf, "  %.8g %.8g %.8g %.8g", c->r, c->g, c->b, c->a);
    }

    if (pl->geomflags & PL_HASST) {
      fprintf(outf, "  %.8g %.8g", pl->vl[i].st.s, pl->vl[i].st.t);
    }
    fputc('\n', outf);
  }

  fputc('\n', outf);
  for(i = 0; i < pl->n_polys; i++) {
    Poly *p = &pl->p[i];
    fprintf(outf, "\n%d	", p->n_vertices);
    for(k = 0; k < p->n_vertices; k++)
      fprintf(outf, " %d", pl->vi[pl->pv[i] + k]);
    if((pl->geomflags & PL_HASPCOL)) {
      fprintf(outf, "\t%.8g %.8g %.8g %.8g",
	      p->pcol.r, p->pcol.g, p->pcol.b, p->pcol.a);
    }
  }
  fputc('\n', outf);

  return (ferror(outf) ? NULL : pl);
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: *
 */

