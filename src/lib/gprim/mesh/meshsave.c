/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
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

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Geometry Technologies, Inc.";


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "meshP.h"

Mesh           *
MeshFSave(m, outf)
Mesh           *m;
FILE           *outf;
{
	int             i, j;
	register HPoint3 *p = m->p;
	register Point3 *n = m->n;
	register ColorA *c = m->c;
	register Point3 *u = m->u;

	if (!outf || !m)
		return NULL;
	if (m->n == NULL)
		m->flag &= ~MESH_N;
	if (m->c == NULL)
		m->flag &= ~MESH_C;
	if (m->u == NULL)
		m->flag &= ~MESH_U;

	if (m->flag & MESH_C) fputc('C', outf);
	if (m->flag & MESH_N) fputc('N', outf);
	if (m->flag & MESH_Z) fputc('Z', outf);
	if (m->geomflags & VERT_4D) fputc('4', outf);
	if (m->flag & MESH_U) fputc('U', outf);
	if (m->flag & MESH_UWRAP) fputc('u', outf);
	if (m->flag & MESH_VWRAP) fputc('v', outf);
	if (m->flag & MESH_BINARY)    /* Hack -- should be sent by context */
	{
	    fprintf(outf, "MESH BINARY\n");
	    fwrite(&m->nu, 4, 1, outf);
	    fwrite(&m->nv, 4, 1, outf);
	    for (i = 0; i < m->nv; i++) {
		for (j = 0; j < m->nu; j++) {
		    if (m->flag & MESH_Z) fwrite(&p->z, 4, 1, outf);
		    else if (m->flag & MESH_4D) fwrite(p, 4, 4, outf);
		    else fwrite(p, 4, 3, outf);
		    p++;
		    if (m->flag & MESH_N) { fwrite(n, 4, 3, outf); n++; }
		    if (m->flag & MESH_C) { fwrite(c, 4, 4, outf); c++; }
		    if (m->flag & MESH_U) { fwrite(u, 4, 3, outf); u++; }
		}
	    }
	}
	else
	{
	    fprintf(outf, "MESH\n%d %d\n", m->nu, m->nv);
	    for (i = 0; i < m->nv; i++) {
		for (j = 0; j < m->nu; j++) {
		    if(!(m->flag & MESH_Z))
			fprintf(outf, "%g %g ", p->x, p->y);
		    fprintf(outf, "%g ", p->z);
		    if (m->geomflags & VERT_4D)
		        fprintf(outf, "%g ", p->w);
		    p++;
		    if (m->flag & MESH_N) {
			    fprintf(outf, " %g %g %g ", n->x, n->y, n->z);
			    n++;
		    }
		    if (m->flag & MESH_C) {
			    fprintf(outf, " %.3g %.3g %.3g %.3g ",
				c->r, c->g, c->b, c->a);
			    c++;
		    }
		    if (m->flag & MESH_U) {
			    fprintf(outf, " %g %g %g", u->x, u->y, u->z);
			    u++;
		    }
		    fputc('\n', outf);
		}
		fputc('\n', outf);
	    }
	}

	return m;
}


Mesh           *
MeshSave(m, name)
Mesh           *m;
char           *name;
{
	Mesh           *tm;
	FILE           *outf;

	outf = fopen(name, "w");
	if (!outf)
	{
		perror(name);
		return NULL;
	}
	tm = MeshFSave(m, outf);
	fclose(outf);

	return (m);
}
