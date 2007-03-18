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
#include "bezierP.h"

/* a hack: introduce new names of the form BEZUVM and BEZUVM_ST, where
	U	is u-degree of the polynomial and
	V	is v-degree of the polynomial and
	M	is the dimension of the points involved
*/

static int
bezierheader(IOBFILE *file, Bezier *bezier)
{
    int binary = 0;
    char *token;
		/* Parse {BBP|STBBP|BEZuvn[_ST]} [BINARY] */

    bezier->geomflags = BEZ_REMESH;
    bezier->dimn = 3;
    bezier->degree_u = bezier->degree_v = 3;	/* Default bicubic 3-D patch */

    bezier->nu = bezier->nv = 0;

    token = GeomToken(file);

    if(strncmp(token, "ST", 2) == 0) {
	bezier->geomflags |= BEZ_ST;
	token += 2;
    }
    if(*token == 'C') {
	bezier->geomflags |= BEZ_C;
	token++;
    }
    if(strncmp(token, "BEZ", 3) == 0) {
	int haveuvn = 0;
	token += 3;
	if(*token >= '0' && *token <= '9') {
	    haveuvn = 1;
	    bezier->degree_u = *token++ - '0';
	    bezier->degree_v = *token++ - '0';
	    bezier->dimn = *token++ - '0';
	}
	if(strcmp(token, "_ST") == 0) {
	    token += 3;
	    bezier->geomflags |= BEZ_ST;
	}
	if(!haveuvn) {
		/* [C]BEZ[_ST] u v n -- expect u, v, n as separate numbers */
	    if(iobfgetni(file, 1, &bezier->degree_u, 0) <= 0 ||
		    iobfgetni(file, 1, &bezier->degree_v, 0) <= 0 ||
		    iobfgetni(file, 1, &bezier->dimn, 0) <= 0)
		return -1;
	}
	if(bezier->degree_u <= 0 || bezier->degree_u > MAX_BEZ_DEGREE ||
	   bezier->degree_v <= 0 || bezier->degree_v > MAX_BEZ_DEGREE ||
	   bezier->dimn < 3 || bezier->dimn > MAX_BEZ_DIMN)
	    return -1;
    } else if(strcmp(token, "BBP") != 0) {
	return -1;
    }
    GeomAcceptToken();

    if(iobfnextc(file, 1) == 'B') {
	if(iobfexpectstr(file, "BINARY"))
	    return -1;
	binary = 1;
	if(iobfnextc(file, 1) == '\n')
	  (void) iobfgetc(file);		/* Toss \n */
    }

    bezier->CtrlPnts = NULL;
    bezier->mesh = NULL;
    return(binary);
}


List *
BezierListFLoad(IOBFILE *file, char *fname)
{
    Geom 	*bezierlist;
    Bezier 	proto, bez;
    int		totalfloats;
    int		nf, ok, c;
    int		binary;
    Geom	*geom;

    binary = bezierheader( file, &proto );
    if(binary < 0)
	return NULL;

    /* begin a list with empty list */
    bezierlist = NULL;

    for(;;) {
	/* get new space */
	ok = 0;
	bez = proto;		/* should be a call to GeomCopy */
				/* copy over header info */

	totalfloats = (proto.degree_u+1)*(proto.degree_v+1)*proto.dimn;
	bez.CtrlPnts = OOGLNewNE(float,totalfloats, "Bezier control pnts");

	nf = iobfgetnf(file, totalfloats, bez.CtrlPnts, binary);
	if(nf < totalfloats) {
	    if(nf != 0)
		break;	/* Incomplete array of control points -> error */

			/* Maybe another Bezier header follows.
			 * Take care not to call bezierheader(), which will
			 * invoke GeomToken(), unless we actually seem to have
			 * another letter following.  Without the isalpha()
			 * test, we might consume e.g. a close-brace which
			 * doesn't belong to us.
			 */
	    c = iobfnextc(file, 0);
	    if((isascii(c) && isalpha(c)) &&
			(binary = bezierheader( file, &proto )) >= 0)
		continue;	

			/* No.  If this doesn't look like the end of a
			 * Bezier list, it's an error.
			 */
	    if(c == EOF || c == CKET || c == ';')
		ok = 1;
	    break;
	}

	if (bez.geomflags & BEZ_ST) {
	  if(iobfgetnf(file, 8, (float *)bez.STCoords, binary) != 8)
		break;
	}
	if (bez.geomflags & BEZ_C) {
	    /* Load 4 colors, for the 4 corners of the patch, v-major order. */
	    if(iobfgetnf(file, 16, (float *)bez.c, binary) != 16)
		break;
	}

	/* successful read; append to list */
	geom = GeomCCreate (NULL,BezierMethods(),  CR_NOCOPY,
			    CR_FLAG, bez.geomflags | BEZ_REMESH,
			    CR_DEGU, bez.degree_u, CR_DEGV, bez.degree_v,
			    CR_DIM, bez.dimn, CR_POINT, bez.CtrlPnts,
			    CR_ST, bez.STCoords, CR_COLOR, bez.c, CR_END);
	if(bezierlist)
	    ListAppend(bezierlist, geom);
	else
	    bezierlist = GeomCCreate(NULL, BezierListMethods(),
				     CR_GEOM, geom, CR_END);
    }

    if(!ok) {
	OOGLSyntax(file, "Reading Bezier from \"%s\"", fname);
	GeomDelete(bezierlist);	/* Scrap everything on error */
	return NULL;
    }

    return (List *)bezierlist;
}

/*
 * Local Variables: ***
 * c-basic-offset: 4 ***
 * End: ***
 */
