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

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

#include <geomclass.h>
#include <bezierP.h>
#include <meshP.h>
#include <listP.h>
#include <stdlib.h>

static void transpose(void *src, void *dst, int nu, int nv, int elsize)
{
    char *tmp = (src == dst) ? malloc(nu*nv*elsize) : dst;
    char *p = src;
    int i,j;

    for(j = 0; j < nv; j++) {
	for(i = 0; i < nu; i++) {
	    memcpy(tmp + (i*nu + j) * elsize, p, elsize);
	    p += elsize;
	}
    }
    if(src == dst) {
	memcpy(dst, tmp, nu*nv*elsize);
	free(tmp);
    }
}

int main(int argc, char *argv[])
{
    Geom *g;
    List *l;
    Bezier *b;
    char *fname;
    int t;
    int n = 5;
    int evert = 0;

    if(argc <= 1) {
	fprintf(stderr, "Usage: %s  [[-]N]  [file.bez]\n\
Dices a BEZ file to a list of MESHes, each diced NxN.\n\
If N negative, everts each patch.\n", argv[0]);
	exit(1);
    }

    if(argc > 1)
	n = atoi(argv[1]);
    if(n < 0)
	evert = 1;

    g = (argc > 2 && strcmp(argv[2],"-"))
      ? GeomLoad(fname = argv[2])
      : GeomFLoad(iobfileopen(stdin), fname = "stdin");

    if(g == NULL) {
	fprintf(stderr, "%s: Couldn't read geometry from %s\n", argv[0], fname);
	exit(1);
    }
    if(strcmp(GeomName(g), "list") && strcmp(GeomName(g), "bezierlist")) {
	fprintf(stderr, "%s: input must be a bezierlist, not a %s\n",
		argv[0], GeomName(g));
	exit(1);
    }

    GeomDice(g, abs(n), abs(n));
    printf("{ LIST\n");
    for(l = (List *)g; l; l = l->cdr) {
	if(strcmp(GeomName(l->car), "bezier"))
	    continue;
	b = (Bezier *)l->car;
	if(evert) {
	    transpose(b->CtrlPnts, b->CtrlPnts,
		b->degree_u+1, b->degree_v+1, b->dimn*sizeof(float));
	    t = b->degree_u;
	    b->degree_u = b->degree_v;
	    b->degree_v = t;
	}
	MeshFSave( BezierReDice(b), stdout );
    }
    printf("}\n");
    exit(0);
}
	
