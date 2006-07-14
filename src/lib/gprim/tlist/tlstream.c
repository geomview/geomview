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

#include <stdlib.h>
#include <ctype.h>
#include "tlistP.h"
#include "inst.h"
#include "list.h"
#include "handleP.h"
#include "transobj.h"

Geom *
TlistImport( Pool *p )
{
    Geom *subtlist = NULL;
    Geom *list = NULL;
    Geom *groupinst = NULL;
    Geom *geom = NULL;
    Geom *g = NULL;
    Transform tfm;
    Handle *geomhandle = NULL, *tfmhandle = NULL, *subtlisthandle = NULL;
    char *token;
    static char errfmt[] = "Reading TLIST from \"%s\": %s";
    int i, nel;
    vvec v;
    vvec vinst;
    int brack = 0;
    int binary = 0;
    int sawheader = 1;
    float *tp;
    float preread = 0;
    int got_preread = 0;
    IOBFILE *file;
    Transform TT[100];


    if((file = PoolInputFile(p)) == NULL)
	return NULL;

    token = GeomToken(file);
    if(strcmp(token, "GROUP") == 0 || strcmp(token, "TLIST") == 0) {
	sawheader = 1;
    } else {
	if(*token == '\0')
	    return NULL;
	if(*token == '-' || isdigit(*token) || *token == '.')
	    preread = strtod(token, &token);
	if(*token != '\0')
	    return NULL;
	got_preread = 1;
    }
    VVINIT(vinst, Geom *, 7);
    if (sawheader && (iobfnextc(file, 1) == 'B')) {
	if(iobfexpectstr(file, "BINARY")) return NULL;
	if(iobfnextc(file, 1) == '\n') (void) iobfgetc(file);

	/*
	 * TLIST BINARY ... \n
	 * <number of tlist elements as a 32-bit int>
	 * <that many 4x4 matrices>
	 */
	if(iobfgetni(file, 1, &nel, binary) != 1 || nel < 0 || nel > 9999999)
	    return NULL;
	VVINIT(v, Transform, 1);
	vvneeds(&v, nel);
	if((VVCOUNT(v) = iobfgettransform(file, nel,
				(float *)VVEC(v,Transform), 1)) < nel) {
	    vvfree(&v);
	    OOGLError(0, "Reading binary TLIST from \"%s\": got only %d of %d transforms expected",
		PoolName(p), VVCOUNT(v), nel);
	    return NULL;
	}
    } else {
	VVINIT(v, Transform, 100);
	vvuse(&v, TT, 100);
    }

    for(;;) {
	switch(iobfnextc(file, 0)) {
	case ':': case '<': case '@': case 'd':
	tfmobject:
	    if(TransStreamIn(p, &tfmhandle, tfm)) {
		*VVAPPEND(vinst, Geom *) = GeomCCreate(NULL, InstMethods(),
			    CR_AXIS, tfm, CR_AXISHANDLE, tfmhandle, CR_END);
	    }
	    break;

	case CBRA: iobfgetc(file); brack++; break;
	case CKET:
	    if(--brack >= 0) {
		iobfgetc(file);
		break;
	    }
	    /* If brackets matched, fall into... */

	case EOF: case ';': case ')':
		goto done;

	case 't':
	    iobfgetc(file);
	    if(iobfexpectstr(file, /*t*/"ransform") == 0)
		goto tfmobject;
	    if(iobfexpecttoken(file, /*t*/"list"))
		goto syntax;
	    if(!GeomStreamIn(p, &subtlisthandle, &subtlist)) {
		OOGLSyntax(file, errfmt, PoolName(p), "failed reading 'tlist'");
		return NULL;
	    }
	    break;

	case 'u':
	case 'g':
	    if(iobfexpecttoken(file, "unit") && iobfexpecttoken(file, "geom"))
		goto syntax;
	    /*
	     * "GROUP" Compatibility mode -- create an Inst comprising our
	     * Tlist and the given object.
	     */
	    if(!GeomStreamIn(p, &geomhandle, &geom))
		OOGLSyntax(file, errfmt, PoolName(p), "failed reading 'geom'");
	    else
		groupinst = GeomCCreate(NULL, InstMethods(),
		    CR_HANDLE_GEOM, geomhandle, geom, CR_END);
	    break;

	default:
	    tp = VVAPPEND(v, float);
	    if(got_preread) {
		got_preread = 0;
		*tp++ = preread;
		if(iobfgetnf(file, 15, tp, 0) == 15)
		    break;
	    } else {
		if(iobfgettransform(file, 1, tp, 0) > 0)
		    break;
	    }
	    /* Else fall into... */

	 syntax:
	    if(sawheader)
		OOGLSyntax(file, errfmt, PoolName(p), "unknown keyword");
	    vvfree(&v);
	    return NULL;
	}
    }
  done:
    if(brack > 0)
	OOGLSyntax(file, errfmt, PoolName(p), "unclosed '{'");  /* } */

    if(VVCOUNT(v) > 0) {
	vvtrim(&v);
	*VVAPPEND(vinst, Geom *) = GeomCCreate(NULL,TlistMethods(),
	    CR_NOCOPY, CR_NELEM, VVCOUNT(v), CR_ELEM, VVEC(v,float), CR_END);
    }

    for(i = 0, list = g = NULL; i < VVCOUNT(vinst); i++) {
	g = VVEC(vinst, Geom *)[i];
	GeomSet(g, CR_HANDLE_GEOM, subtlisthandle, subtlist, CR_END);
	if(VVCOUNT(vinst) > 1)
	    list = ListAppend(list, g);
    }
    if(list) g = list;

    if(groupinst) {
	GeomSet(groupinst, CR_TLIST, g, CR_END);
	g = groupinst;
    }

    return g;
}

int
TlistExport( Tlist *tl, Pool *p )
{
    FILE *outf;

    if(tl == NULL || tl->elements == NULL
			|| (outf = PoolOutputFile(p)) == NULL)
	return 0;
    fprintf(outf, "TLIST\n");
    if(fputtransform(outf, tl->nelements, (float *)tl->elements, 0)
		!= tl->nelements)
	return 0;
   if(tl->tlist != NULL) {
	fprintf(outf, " tlist ");
	GeomStreamOut( p, tl->tlisthandle, tl->tlist );
   }
   return 1;
}
