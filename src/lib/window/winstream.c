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


/* Authors: Stuart Levy, Tamara Munzner, Mark Phillips */

#include "windowP.h"

int WnStreamIn( Pool *, Handle **, WnWindow ** );
int WnStreamOut( Pool *, Handle *, WnWindow * );

HandleOps WindowOps = {
	"window",
	(int ((*)()))WnStreamIn,
	(int ((*)()))WnStreamOut,
	(void ((*)()))WnDelete,
	NULL,
	NULL
};

static struct winkeyword {
	char *kw;
	int flag;
} wn_kw[] = {
	{ "window", 0 },
	{ "define", 0 },
	{ "size", WNF_HASSIZE },
	{ "position", WNF_HASPREF },
	{ "noborder", WNF_NOBORDER },
	{ "border", WNF_NOBORDER },
	{ "pixelaspect", WNF_HASPIXASPECT },
	{ "resize", WNF_ENLARGE|WNF_SHRINK },
	{ "curpos", WNF_HASCUR },
	{ "viewport", WNF_HASVP },
	{ "title", WNF_HASNAME },
};

/* See the comments in src/lib/gprim/geom/geomstream.c for the logic
 * behind HandleDelete(), RefIncr(), RefDecr() etc.
 */
int
WnStreamIn( Pool *p, Handle **hp, WnWindow **wp )
{
    Handle *h = NULL;
    Handle *hname = NULL;
    WnWindow *win = NULL;
    IOBFILE *inf;
    char *w, *raww, *err, *title;
    int i, c;
    int more, unset;
    int brack = 0;
    static char poserr[] = "xmin xmax ymin ymax";

    if(p == NULL || (inf = PoolInputFile(p)) == NULL)
	return 0;

    win = WnCreate(WN_END);

    do {
	more = unset = 0;
	switch(c = iobfnextc(inf, 0)) {
	case '{': brack++; iobfgetc(inf); break;
	case '}': if(brack--) iobfgetc(inf); break;

	case '<':
	case ':':
	case '@':
	    iobfgetc(inf);
	    w = iobfdelimtok("{}()", inf, 0);
	    if(c == '<' && (h = HandleByName(w, &WindowOps)) == NULL) {
		w = findfile(PoolName(p), raww = w);
		if(w == NULL) {
		    OOGLSyntax(inf, "Reading window from \"%s\": can't find file \"%s\"",
			PoolName(p), raww);
		    break;
		}
	    } else if (h) {
		HandleDelete(h); /* undo HandleByName() */
	    }
	    h = HandleReferringTo(c, w, &WindowOps, NULL);
	    if(h) {
		win = (WnWindow *)HandleObject(h);
		RefIncr((Ref *)win);
	    }
	    break;

	case '-':
	    iobfgetc(inf); unset = 1;	/* and fall into... */

	default:
	    w = iobfdelimtok("{}()", inf, 0);
	    if(w == NULL) goto error;
	    for(i = sizeof(wn_kw)/sizeof(wn_kw[0]); strcmp(w, wn_kw[--i].kw); )
		if(i == 0) {
		    OOGLSyntax(inf,
			"Reading window from \"%s\": unknown keyword \"%s\"",
			PoolName(p), w);
		    goto error;
		}

	    win->changed |= wn_kw[i].flag;
	    if(unset) {
		win->flag &= ~wn_kw[i].flag;
		continue;
	    }
	    win->flag |= wn_kw[i].flag;
	    switch(i) {
	    case 0: more = 1; break;		/* window */
	    case 1:				/* define */
		hname = HandleCreateGlobal(iobftoken(inf, 0), &WindowOps);
		more = 1;
		break;
	    case 2:				/* size */
		err = "xsize ysize";
		if(iobfgetni(inf, 1, (int *)(void *)&win->xsize, 0) <= 0 ||
		   iobfgetni(inf, 1, (int *)(void *)&win->ysize, 0) <= 0)
		    goto expect;
		break;
	    case 3:				/* position */
		err = poserr;
		if(iobfgetni(inf, 4, (int *)&win->pref, 0) != 4)
		    goto expect;
		break;

		/* "noborder" and "resize" are just flag manipulation */

	    case 5:				/* border */
		win->flag &= ~wn_kw[i].flag;
		break;
	    case 6:				/* pixelaspect */
		err = "pixel-aspect-ratio(X/Y)";
		if(iobfgetnf(inf, 1, &win->pixaspect, 0) <= 0)
		    goto expect;
		break;
	    case 8:				/* curpos */
		err = poserr;
		if(iobfgetni(inf, 4, (int *)&win->cur, 0) != 4)
		    goto expect;
		break;
	    case 9:				/* viewport */
		err = poserr;
		if(iobfgetni(inf, 4, (int *)&win->viewport, 0) != 4)
		    goto expect;
		break;
	    case 10:
		err = "window title string";
		title = iobfdelimtok("{}()", inf, 0);
		if(title == NULL)
		    goto expect;
		WnSet(win, WN_NAME, title, WN_END);
		break;
	    }
	    break;

	
	 expect:
	    OOGLSyntax(inf, "Reading window from \"%s\": \"%s\" expects %s",
		PoolName(p), w, err);
	 error:
	    if(win && !h)
		WnDelete(win);
	    return 0;
	}
    } while (brack || more);

    if (hname != NULL) {
	if (win) {
	    HandleSetObject(hname, (Ref *)win);
	}
	if (h) {
	    HandleDelete(h);
	}
	h = hname;
    }

    /* Pass the ownership of h and win to the caller if requested */

    if (hp != NULL) {
	/* pass on ownership of the handle h to the caller of this function */
	if (*hp != NULL) {
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
    if (wp != NULL) {
	if (*wp != NULL) {
	    WnDelete(*wp);
	}
	*wp = win;
    } else if(win) {
	WnDelete(win);
    }

    return (h != NULL || win != NULL);
}

int
WnStreamOut( Pool *p, Handle *h, WnWindow *win )
{
    int i;
    WnPosition *wp;
    FILE *f;

    if((f = PoolOutputFile(p)) == NULL)
	return 0;

    fprintf(f, "window {");

    if(PoolStreamOutHandle( p, h, win != NULL)) {
	for(i = 2; i < (int)(sizeof(wn_kw)/sizeof(wn_kw[0])); i++) {
	    if(win->flag & wn_kw[i].flag) {
		if(wn_kw[i].flag & WNF_HASNAME)
		    continue;
		fprintf(f, " %s", wn_kw[i].kw);
		switch(i) {
		case 2: fprintf(f, " %d %d", win->xsize, win->ysize);
			break;
		case 3: wp = &win->pref; goto dowp;
		case 6: fprintf(f, " %g", win->pixaspect); break;
		case 10: break;
		case 8: wp = &win->cur; goto dowp;
		case 9: wp = &win->viewport;	/* Fall into dowp */
		
		    dowp:
			fprintf(f, " %d %d %d %d",
				wp->xmin, wp->xmax, wp->ymin, wp->ymax);
			break;
		}
	    }
	}
    }
    fputs(" }\n", f);
    return 1;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
