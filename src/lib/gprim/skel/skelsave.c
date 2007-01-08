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
 * Geometry Routines
 * 
 * Geometry Supercomputer Project
 * 
 * ROUTINE DESCRIPTION:  Save a skel object to a file.
 * 
 */

#include "skelP.h"

Skel *SkelFSave(Skel *s, FILE *f)
{
  int i, j, d, o;
  float *p;
  int *vp;
  Skline *l;

  /* This one just saves in ASCII format */

  if(s == NULL || f == NULL)
    return NULL;

  d = s->geomflags & VERT_4D ? s->pdim : s->pdim-1;
  o = s->geomflags & VERT_4D ? 0 : 1;
  if (s->vc)
    fprintf(f, "C");
  if (s->geomflags & VERT_4D)
    fprintf(f, "4");
  fprintf(f, s->pdim==4 ? "SKEL" : "nSKEL %d", s->pdim-1);
  fprintf(f, "\n%d %d\n\n", s->nvert, s->nlines);
	
  if (s->pdim == 4) {
    for (i = 0, p = s->p; i < s->nvert; i++, p += s->pdim) {
      fputnf(f, d, p, 0);
      if (s->vc) {
	fputc(' ', f);
	fputnf(f, 4, (float *)&s->vc[i], 0);
      }
      fputc('\n', f);
    }
    fputc('\n', f);
  } else {
    for(i = 0, p = s->p; i < s->nvert; i++, p += s->pdim) {
      fputnf(f, d, p + o, 0);
      if (s->vc) {
	fputc(' ', f);
	fputnf(f, 4, (float *)&s->vc[i], 0);
      }
      fputc('\n', f);
    }
    fputc('\n', f);
  }

  for(i = 0, l = s->l; i < s->nlines; i++, l++) {
    fprintf(f, "%d\t", l->nv);
    for(j = 0, vp = &s->vi[l->v0]; j < l->nv; j++, vp++)
      fprintf(f, "%d ", *vp);
    if(l->nc > 0) {
      fputc('\t', f);
      fputnf(f, 4, &s->c[l->c0].r, 0);
    }
    fputc('\n', f);
  }
  return ferror(f) ? NULL : s;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
