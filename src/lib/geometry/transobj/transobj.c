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


/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "transform.h"
#include "transobj.h"
#include "handleP.h"

/* 
 * Operations on TransObj objects.
 */

void
TransPosition(TransObj *tobj, Transform T)	/* Get transform into T */
{ TmCopy(tobj->T, T); }

void
TransTransformTo(TransObj *tobj, Transform T)	/* Set transform from T */
{ TmCopy(T, tobj->T); }

void
TransDelete(TransObj *tobj)
{
    if(tobj == NULL)
	return;
    if(tobj->magic != TRANSMAGIC) {
	OOGLWarn("Internal warning: TransDelete'ing non-TransObj %x (%x != %x)",
	    tobj, tobj->magic, TRANSMAGIC);
	return;
    }
    if(tobj != NULL && RefDecr((Ref *)tobj) <= 0)
	OOGLFree(tobj);
}

TransObj *
TransCreate( Transform T )
{
    TransObj *tobj = OOGLNewE(TransObj, "TransObj");
    RefInit((Ref*)tobj, TRANSMAGIC);
    if(T != TMNULL) TmCopy(T, tobj->T);
    return tobj;
}


/*
 * Communications
 */

HandleOps TransOps = {
	"trans",
	(int ((*)()))TransStreamIn,
	(int ((*)()))TransStreamOut,
	(void ((*)()))TransDelete,
	NULL,
	NULL
};

void
TransUpdate( Handle **hp, Ref *ignored, Transform Tfixme )
{
    register Handle *h = *hp;

    if(h != NULL && h->object != NULL)
	TmCopy( ((TransObj *)(h->object))->T, Tfixme );
}


int
TransStreamIn( Pool *p, Handle **hp, Transform T )
{
    Handle *h = NULL;
    Handle *hname = NULL;
    register TransObj *tobj = NULL;
    char *w, *raww;
    int c;
    int more = 0;
    int brack = 0;
    FILE *inf;

    if(p == NULL || (inf = PoolInputFile(p)) == NULL)
	return 0;


    do {
	more = 0;
	switch(c = fnextc(inf, 0)) {
	case '{': brack++; fgetc(inf); break;
	case '}': if(brack--) fgetc(inf); break;
	case 't':
	    if(fexpectstr(inf, "transform"))
		return 0;
	    more = 1;
	    break;

	case 'd':
	    if(fexpectstr(inf, "define"))
		return 0;
	    hname = HandleAssign(ftoken(inf, 0), &TransOps, NULL);
	    break;

	case '<':
	case ':':
	case '@':
	    fgetc(inf);
	    w = fdelimtok("{}()", inf, 0);
	    if(c == '<' && HandleByName(w, &TransOps) == NULL) {
		w = findfile(PoolName(p), raww = w);
		if(w == NULL) {
		    OOGLSyntax(inf, "Reading transform from \"%s\": can't find file \"%s\"",
			PoolName(p), raww);
		}
	    }
	    h = HandleReferringTo(c, w, &TransOps, NULL);
	    if(h)
		tobj = (TransObj *)h->object;
	    break;

	default:
	    /*
	     * Anything else should be a 4x4 matrix
	     */
	    if(tobj == NULL)
		tobj = TransCreate( TMNULL );
	    if(fgettransform(inf, 1, &tobj->T[0][0], 0) <= 0)
		return 0;
	}
    } while(brack || more);

    if(hname != NULL) {
	if(tobj)
	    HandleSetObject(hname, (Ref *)tobj);
	h = hname;
    }

    if(h == NULL && tobj != NULL && p->ops == &TransOps)
	h = HandleAssign(PoolName(p), &TransOps, (Ref *)tobj);


    if(h != NULL && tobj != NULL)
	HandleSetObject(h, (Ref *)tobj);

    if(tobj != NULL && T != TMNULL)
	TmCopy(tobj->T, T);

    if(h != NULL && hp != NULL) {
	if(*hp != h) {
	    if(*hp != NULL)
		HandlePDelete(hp);
	    *hp = h;
	}
    } else {
	TransDelete(tobj);	/* Maintain ref count */
    }

    return (h != NULL || tobj != NULL);
}

int
TransStreamOut( Pool *p, Handle *h, Transform T )
{
    int putdata;
    FILE *outf;

    if((outf = PoolOutputFile(p)) == NULL)
	return 0;

    fprintf(outf, "transform {\n");

    putdata = PoolStreamOutHandle( p, h, T != TMNULL );
    if(putdata)
	fputtransform(outf, 1, &T[0][0], 0);
    fputs("}\n", outf);
    return !ferror(outf);
}
