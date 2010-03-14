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

#include "ndmeshP.h"

static void ign_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  size_t result;
  
  result = fwrite(ptr, size, nmemb, stream);
}

NDMesh *
NDMeshFSave(NDMesh *m, FILE *outf)
{
  int  i, j, k, wdim, offset;
  HPointN **p = m->p;
  ColorA *c = m->c;
  TxST *u = m->u;

  if (!outf || !m)
    return NULL;
  if (m->c == NULL)
    m->geomflags &= ~MESH_C;
  if (m->u == NULL)
    m->geomflags &= ~MESH_U;
  wdim = m->pdim;

  if (m->geomflags & MESH_C) fputc('C', outf);
  if (m->geomflags & VERT_4D) {
    fputc('4', outf);
    offset = 0;
  } else {
    --wdim;
    offset = 1;
  }
  if (m->geomflags & MESH_U) fputc('U', outf);
  if (m->geomflags & MESH_UWRAP) fputc('u', outf);
  if (m->geomflags & MESH_VWRAP) fputc('v', outf);
  /* dim is always pdim-1, even for 4nMESH */
  fprintf(outf, "nMESH %d", m->pdim-1);
  if (m->geomflags & MESH_BINARY) {   /* Hack -- should be sent by context */
    fprintf(outf, "BINARY\n");
    ign_fwrite(&m->mdim, sizeof(int), m->meshd, outf);
    for (i = 0; i < m->mdim[1]; i++) {
      for (j = 0; j < m->mdim[0]; j++) {
	ign_fwrite((*p)->v+offset, sizeof(float), wdim, outf);
	p++;
	if (m->geomflags & MESH_C) { ign_fwrite(c, 4, 4, outf); c++; }
	if (m->geomflags & MESH_U) { ign_fwrite(u, 4, 3, outf); u++; }
      }
    }
  } else {
    fprintf(outf, "\n%d %d\n", m->mdim[0], m->mdim[1]);
    for (i = 0; i < m->mdim[1]; i++) {
      for (j = 0; j < m->mdim[0]; j++) {
	for(k = 0; k < wdim; k++)
	  fprintf(outf, "%g ", (*p)->v[k+offset]);
	p++;
	if (m->geomflags & MESH_C) {
	  fprintf(outf, " %.3g %.3g %.3g %.3g ",
		  c->r, c->g, c->b, c->a);
	  c++;
	}
	if (m->geomflags & MESH_U) {
	  fprintf(outf, " %g %g 0", u->s, u->t);
	  u++;
	}
	fputc('\n', outf);
      }
      fputc('\n', outf);
    }
  }

  return m;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
