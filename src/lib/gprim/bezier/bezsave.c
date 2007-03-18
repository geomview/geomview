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

#include "bezierP.h"
#include "listP.h"

List *
BezierListSave(bezierlist, fname)
    List *bezierlist;
    char *fname;
{
    List *q;
    FILE *f;

    if((f = fopen(fname, "w")) == NULL)
	return NULL;
    q = BezierListFSave(bezierlist, f);
    fclose(f);
    return q;
}

List *
BezierListFSave(bezierlist, f)
    List *bezierlist;
    FILE *f;
{
    List *bl;
    Bezier *bez;
    float *p;
    int u, v;
    int dimwas = -1, uwas = -1, vwas = -1, flagwas = -1;

    for(bl = bezierlist; bl != NULL; bl = bl->cdr) {
	if(bl->car == NULL)
	    continue;
	if((bez = (Bezier *)(bl->car)) == NULL)
	    continue;

	if(bez->magic != BEZIERMAGIC) {
	    GeomError(1/*warning-mismatch*/, "BezierListFSave: Non-Bezier object on BezierList: %x magic %x",
		bez, bez->magic);
	    continue;
	}

	if(bez->dimn != dimwas || bez->geomflags != flagwas
	   || bez->degree_u != uwas || bez->degree_v != vwas) {

	    if(bez->dimn == 3 && bez->degree_u == 3 && bez->degree_v == 3
	       && !(bez->geomflags & BEZ_C)) {
		    fputs(bez->geomflags & BEZ_ST ? "STBBP" : "BBP", f);
	    } else {
		if(bez->geomflags & BEZ_C)
		    fputc('C', f);
		fprintf(f, "BEZ%c%c%c", bez->degree_u + '0',
					bez->degree_v + '0',
					bez->dimn + '0');
		if(bez->geomflags & BEZ_ST)
		    fputs("_ST", f);
	    }
	    dimwas = bez->dimn;
	    flagwas = bez->geomflags;
	    uwas = bez->degree_u;
	    vwas = bez->degree_v;
	}

	fputc('\n', f);

	p = bez->CtrlPnts;
	for(v = 0; v <= bez->degree_v; v++) {
	    fputc('\n', f);
	    for(u = 0; u <= bez->degree_u; u++) {
		if(bez->dimn == 4)
		    fprintf(f, "%11.8g ", *p++);
		fprintf(f, "%11.8g %11.8g %11.8g\n", p[0], p[1], p[2]);
		p += 3;
	    }
	}
	if(bez->geomflags & BEZ_ST) {
	    fputc('\n', f);
	    for(u = 0; u < 4; u++)
		fprintf(f, "%8g %8g  ", bez->STCoords[u].s, bez->STCoords[u].t);
	}
	if(bez->geomflags & BEZ_C && bez->c != NULL) {
	    fputc('\n', f);
	    for(u = 0, p = (float *)bez->c; u < 4; u++, p += 4)
		fprintf(f, "%6g %6g %6g %6g\n", p[0], p[1], p[2], p[3]);
	}
    }
    return bezierlist;

}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
