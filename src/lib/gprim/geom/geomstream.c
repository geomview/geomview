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

#include <stdio.h>
#include <string.h>
#include "geomclass.h"
#include "streampool.h"
#include "handleP.h"

#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#if HAVE_IO_H
# include <io.h> /* _WIN32 */
#endif

#include <signal.h>

#if POPEN_ACCEPTS_RB
# define POPEN_RB "rb"
#else
# define POPEN_RB "r"
#endif

HandleOps GeomOps = {
	"geom",
	(int ((*)()))GeomStreamIn,
	(int ((*)()))GeomStreamOut,
	(void ((*)()))GeomDelete,
	NULL,		/* resync */
	NULL,		/* close pool */
};

Geom *
GeomLoad(char *fname)
{
    IOBFILE *inf = iobfopen(fname, "rb");
    Geom *g;

    if (inf == NULL) {
	OOGLError(0, "GeomLoad: can't open %s: %s", fname, sperror());
	return NULL;
    }
    g = GeomFLoad(inf, fname);
    iobfclose(inf);
    return g;
}

Geom *
GeomFLoad(IOBFILE *inf, char *fname)
{
    Pool *p;
    Geom *g = NULL;

    p = PoolStreamTemp(fname, inf, NULL, 0, NULL);
    GeomStreamIn(p, NULL, &g);
    PoolDelete(p);
    return g;
}

int
GeomFLoadEmbedded( Geom **gp, Handle **hp, IOBFILE *inf, char *fname )
{
    Pool *p;
    int nope;

    p = PoolStreamTemp(fname, inf, NULL, 0, NULL);
    nope = GeomStreamIn(p, hp, gp);
    PoolDelete(p);
    return !nope;
}

Geom *
GeomSave(Geom *g, char *fname)
{
    Pool *p;
    int ok;
    FILE *outf;

    if ((outf = fopen(fname, "wb")) == NULL) {
	OOGLError(0, "GeomSave: Can't open %s: %s", fname, sperror());
	return NULL;
    }
    p = PoolStreamTemp(fname, NULL, outf, 1, &GeomOps);
    if (p == NULL) {
	OOGLError(0, "GeomSave: Can't open %s: %s", fname, sperror());
	return NULL;
    }
    PoolSetOType(p, PO_DATA);
    ok = GeomStreamOut(p, NULL, g);
    PoolClose(p);
    PoolDelete(p);
    return ok ? g : NULL;
}

Geom *
GeomFSave(Geom *g, FILE *outf, char *fname)
{
    Pool *p;
    int ok;

    p = PoolStreamTemp(fname, NULL, outf, 1, NULL);
    PoolSetOType(p, PO_DATA);
    PoolIncLevel(p,1);
    ok = GeomStreamOut(p, NULL, g);
    PoolDelete(p);
    return ok ? g : NULL;
}

Geom *
GeomFSaveEmbedded( Geom *g, Handle *handle, FILE *outf, char *fname )
{
    Pool *p;
    int ok;

    p = PoolStreamTemp(fname, NULL, outf, 1, NULL);
    PoolSetOType(p, PO_HANDLES);
    PoolIncLevel(p, 1);		/* Enforce level > 0 to get { braces } */
    ok = GeomStreamOut(p, handle, g);
    PoolDelete(p);
    return ok ? g : NULL;
}

int
GeomEmbedPrefix(int c)
{ return (c == '{' || c == '=' || c == '@' || c == '<'); }

static char *geomtoken;

char *
GeomToken(IOBFILE *f)
{
    if (geomtoken)
	return geomtoken;
    geomtoken = iobfdelimtok("{}()<:@=", f, 0);
    return geomtoken ? geomtoken : "";
}

void
GeomAcceptToken()
{
    geomtoken = NULL;
}

vvec geomtransl;
int comment_translators = 0;
struct GeomTranslator {
    int prefixlen;
    char *prefix;
    char *cmd;
};

void
GeomAddTranslator(char *prefix, char *cmd)
{
    struct GeomTranslator *gt;
    int i;
    if (VVCOUNT(geomtransl) == 0)
	VVINIT(geomtransl, struct GeomTranslator, 4);
    cmd = cmd && cmd[0] ? strdup(cmd) : "";
    if (prefix[0] == '#')
	comment_translators = 1;
    gt = VVEC(geomtransl, struct GeomTranslator);
    for (i = VVCOUNT(geomtransl); --i >= 0; gt++) {
	if (strcmp(prefix, gt->prefix) == 0) {
	    if (gt->cmd) OOGLFree(gt->cmd);
	    gt->cmd = cmd[0]!='\0' ? cmd : NULL;
	    return;
	}
    }
    gt = VVAPPEND(geomtransl, struct GeomTranslator);
    gt->prefixlen = strlen(prefix);
    gt->prefix = strdup(prefix);
    gt->cmd = cmd[0]!='\0' ? cmd : NULL;
}

int
GeomInvokeTranslator(Pool *p, char *prefix, char *cmd, Handle **hp, Geom **gp)
{
    IOBFILE *tf, *pf = PoolInputFile(p);
    long pos = iobftell(pf) - strlen(prefix);
    int ok, oldstdin;
    Pool *tp;
    void (*oldchld)();

#if defined(unix) || defined(__unix)
    /* Rewind file descriptor to previous position */
    if (iobfseek(pf, pos, SEEK_SET) < 0) {
	OOGLError(1, "%s: can only use external format-translators on disk files", PoolName(p));
	return 0;
    }
    oldstdin = dup(0);
    close(0);
    if (dup(iobfileno(pf)) < 0) {
	/* ignorance */
    }
    oldchld = signal(SIGCHLD, SIG_DFL);
    tf = iobpopen(cmd, POPEN_RB);
    close(0);
    if (oldstdin > 0) {
	if (dup(oldstdin) < 0) {
	    /* ignorance */
	}
	close(oldstdin);
    }
    tp = PoolStreamTemp(PoolName(p), tf, NULL, 0, &GeomOps);
    ok = GeomStreamIn(tp, hp, gp);
    iobpclose(tf);
    PoolClose(tp);
    PoolDelete(tp);
    signal(SIGCHLD, oldchld);
    iobfseek(pf, 0, SEEK_END);  /* Seek input to EOF to avoid confusion */
    return ok;
#else
   /* non-unix -- give up */
    return 0;
#endif
}

int
GeomStreamIn(Pool *p, Handle **hp, Geom **gp)
{
    IOBFILE *f;
    Handle *h = NULL;
    Handle *hname = NULL;
    Geom *g = NULL;
    Handle *aphandle = NULL;
    Appearance *ap = NULL;
    GeomClass *Class;
    void *it;
    int i, first;
    int empty = 1, braces = 0;
    int c;
    char *w, *raww;
    int brack = 0;
    int more;

    if (p == NULL || (f = PoolInputFile(p)) == NULL)
	return 0;

    /* Hack for external support for additional file formats, VRML/Inventor in
     * particular, which alas use a # convention for its header, so we must
     * bypass default comment parsing.
     * If first character is a #, grab first line as a token and
     * match against known patterns.
     */
    if (comment_translators) {
	struct GeomTranslator *gt;
	char prefix[256];
	if ((c = iobfgetc(f)) == '#') {
	    prefix[0] = '#';
	    iobfgets(prefix+1, sizeof(prefix)-1, f);
	    gt = VVEC(geomtransl, struct GeomTranslator);
	    for (i = VVCOUNT(geomtransl); --i >= 0; gt++) {
		if (strncmp(gt->prefix, prefix, gt->prefixlen) == 0) {
		    return GeomInvokeTranslator(p, prefix, gt->cmd, hp, gp);
		}
	    }
	    /* Nope, not one we recognize.  Finish consuming comment line
	     * in case the prefix[] buffer didn't hold it all.
	     */
	    if (strchr(prefix, '\n') == NULL)
		while((c = iobfgetc(f)) != '\n' && c != EOF)
		    ;
	} else if (c != EOF)
	    iobfungetc(c, f);
    }

    /* Skip a semicolon if it's the first thing we see -- 'stuff' compatibility.
     */
    if (iobfnextc(f, 0) == ';')
	iobfgetc(f);

    do {
	more = 0;
	geomtoken = NULL;	/* Fetch a new token */
	w = GeomToken(f);
	c = w[0];
	geomtoken = NULL;	/* Clear cache unless we know we'll need it */
	switch(c) {
	case '<':
	case ':':
	case '@':
	    w = iobfdelimtok("{}()", f, 0);
	    /* Consider doing a path search.
	     * Do this before calling HandleReferringTo()
	     * to prevent spurious error messages.
	     */
	    if (c == '<' &&
	       (h = HandleByName(w, &GeomOps)) == NULL && w[0] != '/') {
		w = findfile(PoolName(p), raww = w);
		if (w == NULL) {
		    OOGLSyntax(f,
			"Error reading \"%s\": can't find file \"%s\"",
			PoolName(p), raww);
		}
	    } else if (h) {
		/* HandleByName() increases the ref. count s.t. the
		 * caller of HandleByName() owns the returned handle.
		 */
		HandleDelete(h);
	    }
	    h = HandleReferringTo(c, w, &GeomOps, NULL);
	    if (h != NULL) {
		/* Increment the ref. count. This way we can call
		 * HandleDelete() and GeomDelete() independently.
		 */
		g = REFGET(Geom, HandleObject(h));
	    }
	    break;

	case '{':
	    brack++;
	    break;
	case '}':
	    if (brack--) braces = 1;
	    else iobfungetc(c, f);
	    break;

	case '=':
	    more = 1;
	    break;

	default:
	    if (strcmp(w, "geom") == 0) {
		more = 1;
		break;
	    }
	    if (strcmp(w, "define") == 0) {
		more = 1;
		hname = HandleCreateGlobal(iobftoken(f, 0), &GeomOps);
		break;
	    }
	    if (strcmp(w, "appearance") == 0) {
		more = 1;
		if (!ApStreamIn(p, &aphandle, &ap)) {
		    OOGLSyntax(f, "%s: appearance definition expected",
			       PoolName(p));
		    GeomDelete(g);
		    return false;
		}
		break;
	    }

	    /* Stuff token in the cache so all geom readers can see it */
	    geomtoken = w;

		
	    /*
	     * Load literal object.
	     * First try to guess object type from its file name.
	     */
	    empty = 0;
	    PoolSetMark(p);

	    Class = GeomFName2Class( PoolName(p) );

	    g = NULL;
	    first = 1;

	    if (Class) {
		if (Class->import)
		    g = (*Class->import)(p);
		else if (Class->fload)
		    g = (*Class->fload)(f, PoolName(p));
		first = 0;
		if (g)
		    break;
	    }

	    /*
	     * If not, try all known classes.
	     */
	    GeomKnownClassInit();	/* Ensure all classes entered */

	    it = GeomClassIterate();
	    while (g == NULL && (Class = GeomNextClass(&it)) != NULL) {
		if (Class->import == NULL && Class->fload == NULL)
		    continue;
		/*
		 * Try to seek back to our initial position.
		 */
		if (!first && !PoolSeekMark(p)) {
		    /* No luck.  Might as well give up right now. */
		    OOGLSyntax(f, "Error reading \"%s\": PoolSetMark() failed.",
			       PoolName(p));
			break;
		}

		if (Class->import)
		    g = (*Class->import)(p);
		else if (Class->fload)
		    g = (*Class->fload)(f, PoolName(p));
		first = 0;
	    }
	    geomtoken = NULL;
	    if (g == NULL) {
		PoolClearMark(p);
		return 0;
	    }
	}
    } while (brack > 0 || more);

    /* Leave results where we're told to */

    if (ap != NULL || aphandle != NULL) {
	Geom *container;
	/* If we're given an appearance and a reference to a handle,
	 * insert an INST to hang the appearance onto.
	 * (But still allow  { define X  appearance { ... } } to bind
	 * an appearance to a handle.  Just disallow it when referring to
	 * an existing handle.)
	 */
	if (h != NULL) {
	    container = GeomCreate("inst", CR_HANDLE_GEOM, h, g, CR_END);
	    HandleDelete(h); /* Decr ref count; the INST now owns it. */
	    GeomDelete(g);
	    g = container;
	    h = NULL;
	} else if (g == NULL) {
	    g = GeomCreate("inst", CR_END);
	}
	if (g->ap) {
	    ApDelete(g->ap);
	}
	if (g->aphandle) {
	    HandlePDelete(&g->aphandle);
	}
	g->ap = ap;
	g->aphandle = aphandle;
	if (g->aphandle) {
	    HandleRegister(&g->aphandle, (Ref *)g, &g->ap, HandleUpdRef);
	}
    }

    if (hname != NULL) {
	if (g) {
	    HandleSetObject(hname, (Ref *)g);
	}
	if (h) {
	    /* HandleReferringTo() has passed the ownership to us, so
	     * delete h because we do not need it anymore.
	     */
	    HandleDelete(h);
	}
	h = hname;
    }

    /* Pass the ownership of h and g to the caller if requested */

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
    if (gp != NULL) {
	if (*gp != NULL) {
	    GeomDelete(*gp);
	}
	*gp = g;
    } else if (g) {
	GeomDelete(g);
    }

    PoolClearMark(p);

    return (g != NULL || h != NULL || (empty && braces));
}


int
GeomStreamOut(Pool *p, Handle *h, Geom *g)
{
    int brack;

    if (PoolOutputFile(p) == NULL)
	  return 0;

    if (g == NULL && h != NULL && h->object != NULL)
	g = (Geom *)h->object;

    if (g == NULL && h == NULL) {
	fprintf(PoolOutputFile(p), "{ }\n");
	return 1;
    }

    brack = true ||
	(p->level > 0 || (g && (g->ap || g->aphandle)) || h != NULL);

    if (brack) {
	fprintf(PoolOutputFile(p), "{");
	PoolIncLevel(p, 1);
    }

    if (p->otype & 4) {
	fprintf(PoolOutputFile(p), " # %d refs\n", g->ref_count); /* debug */
    } else {
	fprintf(PoolOutputFile(p), "\n");
    }

    if (g && (g->ap || g->aphandle)) {
	PoolPrint(p, ""); /* use the proper indentation */
	ApStreamOut(p, g->aphandle, g->ap);
    }

    if (PoolStreamOutHandle(p, h, g != NULL)) {
	if (g->Class->export)
	    (*g->Class->export)(g, p);
	else if (g->Class->fsave)
	    (*g->Class->fsave)(g, PoolOutputFile(p), PoolName(p));
    }

    if (brack) {
	PoolIncLevel(p, -1);
	PoolPrint(p, "}\n");
    }

    return !ferror(PoolOutputFile(p));
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
