/* Copyright (C) 2006 Claus-Justus Heine
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

#include "transformn.h"
#include "ntransobj.h"
#include "handleP.h"
#include "freelist.h"

DEF_FREELISTHEAD(TransformN);
DEF_FREELISTHEAD(HPointN);

/* 
 * Operations on NTransObj objects.
 */

void
NTransPosition(TransformN *ntobj, TransformN *T) /* Get transform into T */
{
    TmNCopy(ntobj, T);
}

void
NTransTransformTo(TransformN *ntobj, TransformN *T) /* Set transform from T */
{
    TmNCopy(T, ntobj);
}

void
NTransDelete(TransformN *ntobj)
{
    if (ntobj == NULL) {
	return;
    }
    if (ntobj->magic != TMNMAGIC) {
	OOGLWarn("Internal warning: "
		 "NTransDelete'ing non-TransformN %x (%x != %x)",
		 ntobj, ntobj->magic, TMNMAGIC);
	return;
    }
    if (ntobj != NULL && RefDecr((Ref *)ntobj) <= 0) {
	TmNDelete(REFGET(TransformN, ntobj));
    }
}

TransformN *
NTransCreate(TransformN *T)
{
    TransformN *ntobj;

    if (T) {
	ntobj = TmNCopy(T, NULL);
    } else {
	ntobj = TmNCreate(0, 0, NULL);
    }

    return ntobj;
}


/*
 * Communications
 */

HandleOps NTransOps = {
	"ntrans",
	(int ((*)()))NTransStreamIn,
	(int ((*)()))NTransStreamOut,
	(void ((*)()))NTransDelete,
	NULL,
	NULL
};

int
NTransStreamIn(Pool *p, Handle **hp, TransformN **ntobjp)
{
    Handle *h = NULL;
    Handle *hname = NULL;
    TransformN *ntobj = NULL;
    TransformN *tmp = NULL;
    char *w, *raww;
    int c;
    int more = 0;
    int brack = 0;
    IOBFILE *inf;

    if (p == NULL || (inf = PoolInputFile(p)) == NULL)
	return 0;

    do {
	more = 0;
	switch(c = iobfnextc(inf, 0)) {
	case '{': brack++; iobfgetc(inf); break;
	case '}': if (brack--) iobfgetc(inf); break;
	case 'n':
	    if (iobfexpectstr(inf, "ntransform"))
		return 0;
	    more = 1;
	    break;

	case 'd':
	    if (iobfexpectstr(inf, "define")) {
		return 0;
	    }
	    hname = HandleCreateGlobal(iobftoken(inf, 0), &NTransOps);
	    break;

	case '<':
	case ':':
	case '@':
	    iobfgetc(inf);
	    w = iobfdelimtok("{}()", inf, 0);
	    if (c == '<' && (h = HandleByName(w, &NTransOps)) == NULL) {
		w = findfile(PoolName(p), raww = w);
		if (w == NULL) {
		    OOGLSyntax(inf,
			       "Reading ntransform from \"%s\": "
			       "can't find file \"%s\"",
			       PoolName(p), raww);
		}
	    }
	    h = HandleReferringTo(c, w, &NTransOps, NULL);
	    if (h) {
		ntobj = (TransformN *)HandleObject(h);
		/* Increment the ref. count. This way we can call
		 * HandleDelete() and NTransDelete() independently.
		 */
		RefIncr((Ref*)ntobj);
	    }
	    break;
	default:
	    /*
	     * Anything else should be the ntransform data.
	     */
	    if (ntobj != NULL) {
		tmp = ntobj;
	    }
	    ntobj = TmNRead(inf, 0);
	    if (ntobj == NULL) {
		OOGLSyntax(inf,
			   "Reading ntransform from \"%s\": "
			   "can't read ntransform data", PoolName(p));
	    } else if (tmp) {
		TmNCopy(ntobj, tmp);
		TmNDelete(ntobj);
		ntobj = tmp;
	    }
	    break;
	}
    } while (brack || more);

    if (hname != NULL) {
	if (ntobj != NULL) {
	    HandleSetObject(hname, (Ref *)ntobj);
	}
	if (h) {
	    /* HandleReferringTo() has passed the ownership to use, so
	     * delete h because we do not need it anymore.
	     */
	    HandleDelete(h);
	}
	h = hname;
    }

    /* Pass the ownership of h and ntobj to the caller if requested */

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
    if (ntobjp != NULL) {
	if (*ntobjp) {
	    NTransDelete(*ntobjp);
	}
	*ntobjp = ntobj;
    } else if(ntobj) {
	NTransDelete(ntobj);
    }

    return (h != NULL || ntobj != NULL);
}

int
NTransStreamOut(Pool *p, Handle *h, TransformN *T)
{
    int i, j;
    int idim = T->idim, odim = T->odim;
    FILE *outf;

    if ((outf = PoolOutputFile(p)) == NULL) {
	return 0;
    }

    fprintf(outf, "ntransform {\n");
    PoolIncLevel(p, 1);
    if (PoolStreamOutHandle(p, h, T != NULL)) {
	PoolFPrint(p, outf, "%d %d\n", idim, odim);
	for(i = 0; i < idim; i++) {
	    PoolFPrint(p, outf, "");
	    for(j = 0; j < odim; j++) {
		fprintf(outf, "%10.7f ", T->a[i*odim+j]);
	    }
	    fprintf(outf, "\n");
	}
    }
    PoolIncLevel(p, -1);
    PoolFPrint(p, outf, "}\n");
    return !ferror(outf);
}

/*
 * Local Variables: ***
 * c-basic-offset: 4 ***
 * End: ***
 */
