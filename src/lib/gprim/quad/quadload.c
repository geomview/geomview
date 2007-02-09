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

#include <ctype.h>
#include "quadP.h"

static int
getquads(IOBFILE *file, Quad *pquad, int off, int binary, int dimn)
{
    HPoint3 *p;
    Point3 *n;
    ColorA *c;
    int k;

    p = &pquad->p[off][0];
    n = (pquad->geomflags & QUAD_N) ? &pquad->n[off][0] : NULL;
    c = (pquad->geomflags & QUAD_C) ? &pquad->c[off][0] : NULL;
    for (k = 4 * (pquad->maxquad - off); --k >= 0; ) {
	if (iobfgetnf(file, dimn, (float *)p, binary) < dimn)
	    break;
	/* set the w-coordinate if the points are 3 dim'nal */
	if (dimn == 3) p->w = 1.0;
	p++;
	if (n != NULL) {
	    if(iobfgetnf(file, 3, (float *)n, binary) < 3)
		return -1;
	    n++;
	}
	if (c != NULL) {
	    if (iobfgetnf(file, 4, (float *)c, binary) < 4)
		return -1;
	    if (c->a < 1.0)
		pquad->geomflags |= COLOR_ALPHA;
	    c++;
	}
    }
    k++;
    if (k % 4 != 0)
	return(-1);
    return pquad->maxquad - k/4;
}


Quad *
QuadFLoad( IOBFILE *file, char *fname )
{
    Quad q;
    int binary = 0;
    char *token;
    int ngot;
    int dimn = 3;

    q.geomflags = 0;
    q.p = NULL;
    q.n = NULL;
    q.c = NULL;

    token = GeomToken(file);

    /* Parse [C][N][4]{QUAD|POLY}[ BINARY]\n */

    if(*token == 'C') {
	q.geomflags = QUAD_C;
	token++;
    }
    if(*token == 'N') {
	q.geomflags |= QUAD_N;
	token++;
    }
    if(*token == '4') {
	q.geomflags |= VERT_4D;
	dimn = 4;
	token++;
    }

    if(strcmp(token, "POLY") && strcmp(token, "QUAD")) {
	return NULL;
    }
    /* Got valid header, now read the file. */
    if(iobfnextc(file, 1) == 'B' && iobfexpectstr(file, "BINARY") == 0) {
	binary = 1;
	if(iobfnextc(file, 1) != '\n') {	/* Expect \n after BINARY */
            OOGLSyntax(file,"QuadFLoad: bad QUAD file header on %s", fname);
	    return NULL;
        }
	(void) iobfgetc(file);		/* Toss \n */
    }

    if(binary) {
	/*
	 * Binary case is easy.
	 * Read the q count, then the P, N and C arrays.
	 * Error if we get less than the q count promised.
	 */
	if(iobfgetni(file, 1, &q.maxquad, 1) <= 0) 
	    return NULL;
	if(q.maxquad <= 0 || q.maxquad > 10000000) {
	    OOGLError(0, "Reading QUAD BINARY from \"%s\": incredible q count 0x%x",
		fname, q.maxquad);
	    return NULL;
	}
	q.p = OOGLNewNE(QuadP, q.maxquad, "QuadFLoad: vertices");
	if(q.geomflags & QUAD_N) q.n = OOGLNewNE(QuadN, q.maxquad, "QuadFLoad: normals");
	if(q.geomflags & QUAD_C) q.c = OOGLNewNE(QuadC, q.maxquad, "QuadFLoad: colors");
	ngot = getquads(file, &q, 0, 1, dimn);
	if(ngot != q.maxquad) {
	    OOGLFree(q.p);
	    if(q.n) OOGLFree(q.n);
	    if(q.c) OOGLFree(q.c);
	    goto fail;
	}
    } else {
	/*
	 * ASCII case is harder.
	 * Guess how many quads we need, and realloc() if we need more.
	 */
#define  TRYQUADS	1000

	QuadP qp[TRYQUADS];
	QuadN qn[TRYQUADS];
	QuadC qc[TRYQUADS];
	vvec vp, vn, vc;

	VVINIT(vp, QuadP, TRYQUADS);
	vvuse(&vp, qp, TRYQUADS);
	if(q.geomflags & QUAD_N) {
	    VVINIT(vn, QuadN, TRYQUADS);
	    vvuse(&vn, qn, TRYQUADS);
	}
	if(q.geomflags & QUAD_C) {
	    VVINIT(vc, QuadC, TRYQUADS);
	    vvuse(&vc, qc, TRYQUADS);
	}
	q.maxquad = TRYQUADS;
	ngot = 0;
	for(;;) {
	    q.p = VVEC(vp, QuadP);
	    if(q.geomflags & QUAD_N) q.n = VVEC(vn, QuadN);
	    if(q.geomflags & QUAD_C) q.c = VVEC(vc, QuadC);

	    ngot = getquads(file, &q, ngot, 0, dimn);

	    VVCOUNT(vp) = VVCOUNT(vn) = VVCOUNT(vc) = ngot;

	    if(ngot < q.maxquad)
		break;

	    q.maxquad *= 2;
	    vvneeds(&vp, q.maxquad);
	    if(q.geomflags & QUAD_N) vvneeds(&vn, q.maxquad);
	    if(q.geomflags & QUAD_C) vvneeds(&vc, q.maxquad);
	}
	if(ngot <= 0) {
	    vvfree(&vp);
	    vvfree(&vn);
	    vvfree(&vc);
	    goto fail;
	}

	/* Release unneeded storage */

	q.maxquad = ngot;
	vvtrim(&vp);
	q.p = VVEC(vp, QuadP);
	if (q.geomflags & QUAD_N) {
	    vvtrim(&vn);
	    q.n = VVEC(vn, QuadN);
	}
	if (q.geomflags & QUAD_C) {
	    vvtrim(&vc);
	    q.c = VVEC(vc, QuadC);
	}
    }

    return (Quad *) GeomCCreate(NULL, QuadMethods(),
	CR_4D, (dimn == 4) ? 1 : 0,
	CR_NOCOPY, CR_FLAG, q.geomflags, CR_NELEM, q.maxquad,
	CR_POINT4, q.p, CR_NORMAL, q.n, CR_COLOR, q.c, 0);

  fail:
    OOGLSyntax(file, "Error reading QUADs in \"%s\"", fname);
    return NULL;
}
