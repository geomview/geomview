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

/* 
 * Operations on NTransObj objects.
 */

void
NTransPosition(NTransObj *ntobj, TransformN *T)	/* Get transform into T */
{
    if (ntobj->T)
	TmNCopy(ntobj->T, T);
}

void
NTransTransformTo(NTransObj *ntobj, TransformN *T) /* Set transform from T */
{
    TmNCopy(T, ntobj->T);
}

void
NTransDelete(NTransObj *ntobj)
{
    if(ntobj == NULL)
	return;
    if(ntobj->magic != NTRANSMAGIC) {
	OOGLWarn("Internal warning: "
		 "NTransDelete'ing non-NTransObj %x (%x != %x)",
		 ntobj, ntobj->magic, NTRANSMAGIC);
	return;
    }
    if(ntobj != NULL && RefDecr((Ref *)ntobj) <= 0) {
	if (ntobj->T) {
	    TmNDelete(ntobj->T);
	}
	OOGLFree(ntobj);
    }
}

NTransObj *
NTransCreate( TransformN *T )
{
    NTransObj *ntobj = OOGLNewE(NTransObj, "NTransObj");
    RefInit((Ref*)ntobj, NTRANSMAGIC);
    if(T != NULL) {
	ntobj->T = TmNCopy(T, ntobj->T);
    } else {
	ntobj->T = NULL;
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

void
NTransUpdate( Handle **hp, Ref *ignored, TransformN **Tfixme )
{
    Handle *h = *hp;

    if(h != NULL && h->object != NULL) {
	*Tfixme = TmNCopy( ((NTransObj *)(h->object))->T, *Tfixme );
    }
}


int
NTransStreamIn(Pool *p, Handle **hp, TransformN **Tptr)
{
    Handle *h = NULL;
    Handle *hname = NULL;
    NTransObj *ntobj = NULL;
    char *w, *raww;
    int c;
    int more = 0;
    int brack = 0;
    int idim, odim;
    IOBFILE *inf;

    if(p == NULL || (inf = PoolInputFile(p)) == NULL)
	return 0;

    do {
	more = 0;
	switch(c = iobfnextc(inf, 0)) {
	case '{': brack++; iobfgetc(inf); break;
	case '}': if(brack--) iobfgetc(inf); break;
	case 'n':
	    if(iobfexpectstr(inf, "ntransform"))
		return 0;
	    more = 1;
	    break;

	case 'd':
	    if(iobfexpectstr(inf, "define"))
		return 0;
	    hname = HandleAssign(iobftoken(inf, 0), &NTransOps, NULL);
	    break;

	case '<':
	case ':':
	case '@':
	    iobfgetc(inf);
	    w = iobfdelimtok("{}()", inf, 0);
	    if(c == '<' && HandleByName(w, &NTransOps) == NULL) {
		w = findfile(PoolName(p), raww = w);
		if(w == NULL) {
		    OOGLSyntax(inf,
			       "Reading ntransform from \"%s\": "
			       "can't find file \"%s\"",
			       PoolName(p), raww);
		}
	    }
	    h = HandleReferringTo(c, w, &NTransOps, NULL);
	    if(h)
		ntobj = (NTransObj *)h->object;
	    break;
	default:
	    /*
	     * Anything else should be a the ntransform data.
	     */
	    if(ntobj == NULL)
		ntobj = NTransCreate(NULL);
	    ntobj->T = TmNRead(inf);
	    if (ntobj->T == NULL) {
		NTransDelete(ntobj);
		ntobj = NULL;
	    }
	}
    } while(brack || more);

    if(hname != NULL) {
	if(ntobj != NULL)
	    HandleSetObject(hname, (Ref *)ntobj);
	h = hname;
    }

    if(h == NULL && ntobj != NULL && p->ops == &NTransOps)
	h = HandleAssign(PoolName(p), &NTransOps, (Ref *)ntobj);


    if(h != NULL && ntobj != NULL)
	HandleSetObject(h, (Ref *)ntobj);

    if(ntobj != NULL && ntobj->T != NULL && Tptr != NULL)
	*Tptr = TmNCopy(ntobj->T, *Tptr);

    if(h != NULL && hp != NULL) {
	if(*hp != h) {
	    if(*hp != NULL)
		HandlePDelete(hp);
	    *hp = h;
	}
    } else {
	NTransDelete(ntobj);	/* Maintain ref count */
    }

    return (h != NULL || ntobj != NULL);
}

int
NTransStreamOut( Pool *p, Handle *h, TransformN *T )
{
    int putdata;
    FILE *outf;

    if((outf = PoolOutputFile(p)) == NULL)
	return 0;

    putdata = PoolStreamOutHandle( p, h, T != NULL );
    if(putdata)
	TmNPrint(outf, T);
    return !ferror(outf);
}

/*
 * Local Variables: ***
 * c-basic-offset: 4 ***
 * End: ***
 */
