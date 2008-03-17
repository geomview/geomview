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


/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "transform.h"
#include "transobj.h"
#include "handleP.h"
#include "freelist.h"

/* 
 * Operations on TransObj objects.
 */

void
TransPosition(TransObj *tobj, Transform T)	/* Get transform into T */
{
    TmCopy(tobj->T, T);
}

void
TransTransformTo(TransObj *tobj, Transform T)	/* Set transform from T */
{
    TmCopy(T, tobj->T);
}

static DEF_FREELIST(TransObj);

void TransDelete(TransObj *tobj)
{
    if(tobj == NULL) {
	return;
    }
    if(tobj->magic != TRANSMAGIC) {
	OOGLWarn("Internal warning: TransDelete'ing non-TransObj %x (%x != %x)",
	    tobj, tobj->magic, TRANSMAGIC);
	return;
    }
    if(tobj != NULL && RefDecr((Ref *)tobj) <= 0) {
	FREELIST_FREE(TransObj, tobj);
    }
}

TransObj *
TransCreate(Transform T)
{
    TransObj *tobj;

    FREELIST_NEW(TransObj, tobj);

    RefInit((Ref*)tobj, TRANSMAGIC);
    if (T != NULL) {
	TmCopy(T, tobj->T);
    }
    return tobj;
}


/*
 * Communications
 */

HandleOps TransOps = {
	"trans",
	(int ((*)()))TransObjStreamIn,
	(int ((*)()))TransObjStreamOut,
	(void ((*)()))TransDelete,
	NULL,
};

void TransUpdate(Handle **hp, Ref *ignored, Transform Tfixme)
{
    Handle *h = *hp;

    (void)ignored;
    if (h != NULL && h->object != NULL) {
	TmCopy(((TransObj *)(h->object))->T, Tfixme);
    }
}


int
TransStreamIn(Pool *p, Handle **hp, Transform T)
{
    TransObj *tobj = NULL;
    
    if (TransObjStreamIn(p, hp, &tobj)) {
	if (tobj) {
	    TmCopy(tobj->T, T);
	    TransDelete(tobj);
	}
	return true;
    }
    return false;
}

int
TransObjStreamIn(Pool *p, Handle **hp, TransObj **tobjp)
{
    Handle *h = NULL;
    Handle *hname = NULL;
    TransObj *tobj = NULL;
    char *w, *raww;
    int c;
    int more = 0;
    int brack = 0;
    IOBFILE *inf;

    if(p == NULL || (inf = PoolInputFile(p)) == NULL) {
	return 0;
    }

    do {
	more = 0;
	switch(c = iobfnextc(inf, 0)) {
	case '{': brack++; iobfgetc(inf); break;
	case '}': if(brack--) iobfgetc(inf); break;
	case 't':
	    if(iobfexpectstr(inf, "transform")) {
		return 0;
	    }
	    more = 1;
	    break;

	case 'd':
	    if(iobfexpectstr(inf, "define")) {
		return 0;
	    }
	    w = iobftoken(inf, 0);
	    hname = HandleCreateGlobal(w, &TransOps);
	    break;

	case '<':
	case ':':
	case '@':
	    iobfgetc(inf);
	    w = iobfdelimtok("{}()", inf, 0);
	    if (c == '<' && (h = HandleByName(w, &TransOps)) == NULL) {
		w = findfile(PoolName(p), raww = w);
		if(w == NULL) {
		    OOGLSyntax(inf, "Reading transform from \"%s\": can't find file \"%s\"",
			PoolName(p), raww);
		}
	    } else if (h) {
		HandleDelete(h);
	    }
	    h = HandleReferringTo(c, w, &TransOps, NULL);
	    if (h) {
		/* Increment the ref. count. This way we can call
		 * HandleDelete() and TransDelete() independently.
		 */
		tobj = REFGET(TransObj, HandleObject(h));
	    }
	    break;

	default:
	    /*
	     * Anything else should be a 4x4 matrix
	     */
	    if(tobj == NULL) {
		tobj = TransCreate(NULL);
	    }
	    if(iobfgettransform(inf, 1, &tobj->T[0][0], 0) <= 0) {
		return 0;
	    }
	}
    } while(brack || more);

    if(hname != NULL) {
	if (tobj) {
	    HandleSetObject(hname, (Ref *)tobj);
	}
	if (h) {
	    /* HandleReferringTo() has passed the ownership to use, so
	     * delete h because we do not need it anymore.
	     */
	    HandleDelete(h);
	}
	h = hname;
    }

    /* Pass the ownership of h and tobj to the caller if requested */

    if (hp != NULL) {
	/* pass on ownership of the handle h to the caller of this function */
	if (*hp) {
	    if (*hp != h) {
		HandlePDelete(hp);
	    } else {
		HandleDelete(*hp);
	    }
	}
	*hp = h;
    } else if (h) {
	/* Otherwise delete h because we are its owner. Note that
	 * HandleReferringTo() has passed the ownership of h to us;
	 * explicitly defined handles (hdefine and define constructs)
	 * will not be deleted by this call.
	 */
	HandleDelete(h);
    }

    /* same logic as for hp */
    if (tobjp != NULL) {
	if (*tobjp) {
	    TransDelete(*tobjp);
	}
	*tobjp = tobj;
    } else if(tobj) {
	TransDelete(tobj);
    }


    return (h != NULL || tobj != NULL);
}

int TransObjStreamOut(Pool *p, Handle *h, TransObj *tobj)
{
    return TransStreamOut(p, h, tobj->T);
}

int
TransStreamOut(Pool *p, Handle *h, Transform T)
{
    FILE *outf;

    if ((outf = PoolOutputFile(p)) == NULL) {
	return false;
    }

    fprintf(outf, "transform {\n");
    PoolIncLevel(p, 1);
    if (PoolStreamOutHandle(p, h, true)) {
	fputtransform(outf, 1, &T[0][0], 0);
    }
    PoolIncLevel(p, -1);
    PoolFPrint(p, outf, "}\n");

    return !ferror(outf);
}

TransObj *TransObjFSave(TransObj *t, FILE *outf, char *fname)
{
  Pool *p;
  int ok;

  p = PoolStreamTemp(fname, NULL, outf, 1, NULL);
  PoolSetOType(p, PO_DATA);
  PoolIncLevel(p, 1);
  ok = TransObjStreamOut(p, NULL, t);
  PoolDelete(p);
  return ok ? t : NULL;
}

TransformPtr TransFSave(Transform T, FILE *outf, char *fname)
{
  Pool *p;
  int ok;

  p = PoolStreamTemp(fname, NULL, outf, 1, NULL);
  PoolSetOType(p, PO_DATA);
  PoolIncLevel(p, 1);
  ok = TransStreamOut(p, NULL, T);
  PoolDelete(p);
  return ok ? T : NULL;
}

/*
 * Local Variables: ***
 * c-basic-offset: 4 ***
 * End: ***
 */
