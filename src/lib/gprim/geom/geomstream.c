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

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include <stdio.h>
#include <string.h>
#include "geomclass.h"
#include "streampool.h"
#include "handleP.h"

#if defined(unix) || defined(__unix)
# include <unistd.h>
#else	/* _WIN32 */
# include <io.h>
#endif

#include <signal.h>


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
    FILE *inf = fopen(fname, "rb");
    Geom *g;

    if(inf == NULL) {
	OOGLError(0, "GeomLoad: can't open %s: %s", fname, sperror());
	return NULL;
    }
    g = GeomFLoad(inf, fname);
    fclose(inf);
    return g;
}

Geom *
GeomFLoad(FILE *inf, char *fname)
{
    Pool *p;
    Geom *g = NULL;

    p = PoolStreamTemp(fname, inf, 0, NULL);
    GeomStreamIn(p, NULL, &g);
    PoolDelete(p);
    return g;
}

int
GeomFLoadEmbedded( Geom **gp, Handle **hp, FILE *inf, char *fname )
{
    Pool *p;
    int nope;

    p = PoolStreamTemp(fname, inf, 0, NULL);
    nope = GeomStreamIn(p, hp, gp);
    PoolDelete(p);
    return !nope;
}

Geom *
GeomSave(Geom *g, char *fname)
{
    Pool *p;
    int ok;

    p = PoolStreamTemp(fname, NULL, 1, &GeomOps);
    if(p == NULL) {
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

    p = PoolStreamTemp(fname, outf, 1, NULL);
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

    p = PoolStreamTemp(fname, outf, 1, NULL);
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
GeomToken(FILE *f)
{
    char *s;
    if(geomtoken)
	return geomtoken;
    geomtoken = fdelimtok("{}()<:@=", f, 0);
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
    register struct GeomTranslator *gt;
    int i;
    if(VVCOUNT(geomtransl) == 0)
	VVINIT(geomtransl, struct GeomTranslator, 4);
    cmd = cmd && cmd[0] ? strdup(cmd) : "";
    if(prefix[0] == '#')
	comment_translators = 1;
    gt = VVEC(geomtransl, struct GeomTranslator);
    for(i = VVCOUNT(geomtransl); --i >= 0; gt++) {
	if(strcmp(prefix, gt->prefix) == 0) {
	    if(gt->cmd) OOGLFree(gt->cmd);
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
    FILE *tf, *pf = PoolInputFile(p);
    long pos = ftell(pf) - strlen(prefix);
    int ok, oldstdin;
    Pool *tp;
    void (*oldchld)();

#if defined(unix) || defined(__unix)
    /* Rewind file descriptor to previous position */
    if(lseek(fileno(pf), pos, 0) < 0) {
	OOGLError(1, "%s: can only use external format-translators on disk files", PoolName(p));
	return 0;
    }
    oldstdin = dup(0);
    close(0);
    dup(fileno(pf));
    oldchld = signal(SIGCHLD, SIG_DFL);
    tf = popen(cmd, "r");
    close(0);
    if(oldstdin > 0) {
	dup(oldstdin);
	close(oldstdin);
    }
    tp = PoolStreamTemp(PoolName(p), tf, 0, &GeomOps);
    ok = GeomStreamIn(tp, hp, gp);
    pclose(tf);
    PoolClose(tp);
    signal(SIGCHLD, oldchld);
    fseek(pf, 0, 2);  /* Seek input to EOF to avoid confusion */
    return ok;
#else
   /* non-unix -- give up */
    return 0;
#endif
}

int
GeomStreamIn(Pool *p, Handle **hp, Geom **gp)
{
    FILE *f;
    Handle *h = NULL;
    Handle *hname = NULL;
    Geom *g = NULL;
    Handle *aphandle = NULL;
    Appearance *ap = NULL;
    GeomClass *Class;
    void *it;
    struct stdio_mark *mark = NULL;
    int i, first;
    int empty = 1, braces = 0;
    int defining = 0;
    Geom **tgp;
    Handle **thp;
    int c;
    char *w, *raww, *tail;
    int brack = 0;
    int more;

    if(p == NULL || (f = PoolInputFile(p)) == NULL)
	return 0;

    /* Hack for external support for additional file formats, VRML/Inventor in
     * particular, which alas use a # convention for its header, so we must
     * bypass default comment parsing.
     * If first character is a #, grab first line as a token and
     * match against known patterns.
     */
    if(comment_translators) {
	struct GeomTranslator *gt;
	char prefix[256];
	if((c = getc(f)) == '#') {
	    prefix[0] = '#';
	    fgets(prefix+1, sizeof(prefix)-1, f);
	    gt = VVEC(geomtransl, struct GeomTranslator);
	    for(i = VVCOUNT(geomtransl); --i >= 0; gt++) {
		if(strncmp(gt->prefix, prefix, gt->prefixlen) == 0) {
		    return GeomInvokeTranslator(p, prefix, gt->cmd, hp, gp);
		}
	    }
	    /* Nope, not one we recognize.  Finish consuming comment line
	     * in case the prefix[] buffer didn't hold it all.
	     */
	    if(strchr(prefix, '\n') == NULL)
		while((c = fgetc(f)) != '\n' && c != EOF)
		    ;
	} else if(c != EOF)
	    ungetc(c, f);
    }

    /* Skip a semicolon if it's the first thing we see -- 'stuff' compatibility.
     */
    if(fnextc(f, 0) == ';')
	fgetc(f);

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
	    w = fdelimtok("{}()", f, 0);
	    /*
	     * Consider doing a path search.
	     * Do this before calling HandleReferringTo()
	     * to prevent spurious error messages.
	     */
	    if(c == '<' && (h = HandleByName(w, &GeomOps)) == NULL && w[0] != '/') {
		w = findfile(PoolName(p), raww = w);
		if(w == NULL) {
		    OOGLSyntax(PoolInputFile(p),
			"Error reading \"%s\": can't find file \"%s\"",
			PoolName(p), raww);
		}
	    }
	    h = HandleReferringTo(c, w, &GeomOps, NULL);
	    if(h != NULL) {
		g = (Geom *)HandleObject(h);
		RefIncr((Ref*)g);
	    }
	    break;

	case '{':
	    brack++;
	    break;
	case '}':
	    if(brack--) braces = 1;
	    else ungetc(c, f);
	    break;

	case '=':
	    more = 1;
	    break;

	default:
	    if(strcmp(w, "geom") == 0) {
		more = 1;
		break;
	    }
	    if(strcmp(w, "define") == 0) {
		more = 1;
		hname = HandleCreate( ftoken(PoolInputFile(p), 0), &GeomOps );
		defining = 1;
		break;
	    }
	    if(strcmp(w, "appearance") == 0) {
		more = 1;
		ap = ApFLoad(ap, PoolInputFile(p), PoolName(p));
		break;
	    }

	    /* Stuff token in the cache so all geom readers can see it */
	    geomtoken = w;

		
	    /*
	     * Load literal object.
	     * First try to guess object type from its file name.
	     */
	    empty = 0;
	    mark = stdio_setmark( NULL, f );

	    Class = GeomFName2Class( PoolName(p) );

	    g = NULL;
	    first = 1;
	    tgp = gp ? gp : &g;
	    thp = hp ? hp : &h;

	    if(Class) {
		if(Class->import)
		    g = (*Class->import)(p);
		else if(Class->fload)
		    g = (*Class->fload)(PoolInputFile(p), PoolName(p));
		first = 0;
		if(g)
		    break;
	    }

	    /*
	     * If not, try all known classes.
	     */
	    GeomKnownClassInit();	/* Ensure all classes entered */

	    it = GeomClassIterate();
	    while (g == NULL && (Class = GeomNextClass(&it)) != NULL) {
		if(Class->import == NULL && Class->fload == NULL)
		    continue;
		/*
		 * Try to seek back to our initial position.
		 */
		if(!first && !stdio_seekmark(mark)) {
		    /* No luck.  Might as well give up right now. */
		    OOGLSyntax(PoolInputFile(p),
			"Error reading \"%s\": can't seek back far enough (on pipe?)",
				PoolName(p));
			break;
		}

		if(Class->import)
		    g = (*Class->import)(p);
		else if(Class->fload)
		    g = (*Class->fload)(PoolInputFile(p), PoolName(p));
		first = 0;
	    }
	    geomtoken = NULL;
	    if(g == NULL) {
		stdio_freemark(mark);
		return 0;
	    }
	}
    } while(brack > 0 || more);


    if(hname != NULL) {
	if(g)
	    HandleSetObject(hname, (Ref *)g);
	h = hname;
    }

    /*
     * Leave results where we're told to
     */
    if(ap != NULL || aphandle != NULL) {
	/* If we're given an appearance and a reference to a handle,
	 * insert an INST to hang the appearance onto.
	 * (But still allow  { define X  appearance { ... } } to bind
	 * an appearance to a handle.  Just disallow it when referring to
	 * an existing handle.)
	 */
	if(h != NULL && !defining) {
	   g = GeomCreate("inst", CR_HANDLE_GEOM, h, g, CR_END);
	   HandleDelete(h);	/* Decr ref count; the INST now owns it. */
	   h = NULL;
	}
	else if(g == NULL)
	   g = GeomCreate("inst", CR_END);
	if(g->ap)
	    ApDelete(g->ap);
	if(g->aphandle)
	    HandlePDelete(&g->aphandle);
	g->ap = ap;
	g->aphandle = aphandle;
    }
    if(h != NULL && hp != NULL && *hp != h) {
	if(*hp)
	    HandlePDelete(hp);
	*hp = h;
    }
    if(g != NULL && gp != NULL && *gp != g) {
	if(*gp != NULL)
	    GeomDelete(*gp);
	*gp = g;
    } else if(g)		/* Maintain ref count */
	GeomDelete(g);

    if(mark != NULL)
	stdio_freemark(mark);
    return (g != NULL || h != NULL || (empty && braces));
}


int
GeomStreamOut(Pool *p, Handle *h, Geom *g)
{
    int putdata;
    int brack;

    if(PoolOutputFile(p) == NULL)
	  return 0;

    if(g == NULL && h != NULL && h->object != NULL)
	g = (Geom *)h->object;

    if(g == NULL && h == NULL) {
	fprintf(PoolOutputFile(p), "{ }\n");
	return 1;
    }

    brack = (p->level > 0 || (g && (g->ap || g->aphandle)) || h != NULL);

    if(brack)
	fprintf(PoolOutputFile(p), "{ ");

    if(p->otype & 4) fprintf(PoolOutputFile(p), "# %d refs\n", g->ref_count); /* debug */

    if(g && (g->ap || g->aphandle))
	ApFSave(g->ap, g->aphandle, PoolOutputFile(p), PoolName(p));

    putdata = PoolStreamOutHandle( p, h, g != NULL );
    if(g != NULL && putdata) {
	PoolIncLevel(p, 1);
	if(g->Class->export)
	    (*g->Class->export)(g, p);
	else if(g->Class->fsave)
	    (*g->Class->fsave)(g, PoolOutputFile(p), PoolName(p));
	PoolIncLevel(p, -1);
    }
    if(brack)
	fprintf(PoolOutputFile(p), "}\n");
    return !ferror(PoolOutputFile(p));
}
