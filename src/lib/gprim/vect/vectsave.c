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
  * ROUTINE DESCRIPTION:  Save a vect object to a file.
  * 
  */

#include "vectP.h"

Vect *
VectFSave(v, f)
	Vect *v;
	FILE *f;
{
	int i;
	HPoint3 *p;
	ColorA *c;

	/* This one just saves in ASCII format */

	if(v == NULL || f == NULL)
		return NULL;

	if (v->geomflags & VERT_4D)
	    fprintf(f, "4");
	fprintf(f, "VECT\n%d %d %d\n", v->nvec, v->nvert, v->ncolor);
	for(i = 0; i < v->nvec; i++)
		fprintf(f, "%d\n", v->vnvert[i]);
	
	fputc('\n', f);
	for(i = 0; i < v->nvec; i++)
		fprintf(f, "%d\n", v->vncolor[i]);

	fputc('\n', f);
	if  (v->geomflags & VERT_4D)	{
	    for(i = v->nvert, p = v->p; --i >= 0; p++)
		fprintf(f, "%g %g %g %g\n", p->x, p->y, p->z, p->w);
	    }
	else	{
	    for(i = v->nvert, p = v->p; --i >= 0; p++)
		fprintf(f, "%g %g %g\n", p->x, p->y, p->z);
	    }

	fputc('\n', f);
	for(i = v->ncolor, c = v->c; --i >= 0; c++)
		fprintf(f, "%.3g %.3g %.3g %.3g\n", c->r, c->g, c->b, c->a);

	return ferror(f) ? NULL : v;
}
