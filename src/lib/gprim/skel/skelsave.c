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

 /*
  * Geometry Routines
  * 
  * Geometry Supercomputer Project
  * 
  * ROUTINE DESCRIPTION:  Save a skel object to a file.
  * 
  */

#include "skelP.h"

Skel *
SkelFSave(Skel *s, FILE *f)
{
	int i, j, d;
	float *p;
	int *vp;
	Skline *l;

	/* This one just saves in ASCII format */

	if(s == NULL || f == NULL)
		return NULL;

	d = s->geomflags & VERT_4D ? s->dim : s->dim-1;
	if (s->geomflags & VERT_4D)
	    fprintf(f, "4");
	fprintf(f, s->dim==4 ? "SKEL" : "nSKEL %d", d);
	fprintf(f, "\n%d %d\n\n", s->nvert, s->nlines);

	for(i = 0, p = s->p; i < s->nvert; i++, p += s->dim) {
	    fputnf(f, d, p, 0);
	    fputc('\n', f);
	}
	fputc('\n', f);

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
