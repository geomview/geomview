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

#include "config.h"

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";

/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

/* $Header: /home/mbp/geomview-git/geomview-cvs/geomview/src/lib/oogl/util/futil.c,v 1.9 2004/03/15 22:05:35 rotdrop Exp $ */

/*
 * Geometry object routines
 *
 * Utility routines, useful for many objects
 *
 * int
 * fgetnf(file, nfloats, floatp, binary)
 *	Read an array of floats from a file in "ascii" or "binary" format.
 *	Returns number of floats successfully read, should = nfloats.
 *	"Binary" means "IEEE 32-bit floating-point" format.
 *
 * int
 * fgetni(FILE *file, int nints, int *intsp, int binary)
 *	Read an array of ints from a file in "ascii" or "binary" format.
 *	Returns number of ints successfully read, should = nints.
 *	"Binary" means "32-bit big-endian" integer format.
 *
 * int
 * fgetns(FILE *file, int nshorts, short *intsp, int binary)
 *	Read an array of shorts from a file in "ascii" or "binary" format.
 *	Returns number of shorts successfully read, should = nints.
 *	"Binary" means "16-bit big-endian" integer format.
 *
 * int
 * fexpectstr(FILE *file, char *string)
 *	Expect the given string to appear immediately on file.
 *	Return 0 if the complete string is found,
 *	else the offset+1 of the last matched char within string.
 *	The first unmatched char is ungetc'd.
 *
 * int
 * fexpecttoken(FILE *file, char *string)
 *	Expect the given string to appear on the file, possibly after
 *	skipping some white space and comments.
 *	Return 0 if found, else the offset+1 of last matched char in string.
 *	The first unmatched char is ungetc'd.
 *
 * int fnextc(FILE *f, int flags)
 *	Advances f to the next "interesting" character and
 *	returns it.  The returned char is ungetc'ed so the next getc()
 *	will yield the same value.
 *	Interesting depends on flags:
 *	  0 : Skip blanks, tabs, newlines, and comments (#...\n).
 *	  1 : Skip blanks, tabs, and comments, but newlines are interesting
 *		(including the \n that terminates a comment).
 *	  2 : Skip blanks, tabs, and newlines, but stop at #.
 *	  3 : Skip blanks and tabs but stop at # or \n.
 *
 * int async_fnextc(FILE *f, int flags, int fd)
 *	Like fnextc() above, but guarantees not to block if no data is
 *	immediately available.  It returns either an interesting character,
 *	EOF, or the special code NODATA (== -2).
 *      if fd == -1, then fileno(f) is used, otherwise fd.
 *
 * int async_getc(FILE *f, int fd)
 *	Like getc(), but guarantees not to block.  Returns NODATA if
 *	nothing is immediately available.
 *
 * char *ftoken(FILE *f, int flags)
 *	Skips uninteresting characters with fnextc(f, flags),
 *	then returns a "token" - string of consecutive interesting characters.
 *	Returns NULL if EOF is reached with no token, or if
 *	flags specifies stopping at end-of-line and this is encountered with
 *	no token found.
 *	The token is effectively statically allocated and will be
 *	overwritten by the next ftoken() call.
 *
 * char *fdelimtok(char *delims, FILE *f, int flags)
 *	Like ftoken(), but specially handles the characters in delims[].
 *	If one appears at the beginning of the token, it's returned as 
 *	a single-character token.
 *	If a member of delims[] appears after other characters have been
 *	accumulated, it's considered to terminate the token.
 *	So successive calls to fdelimtok("()", f, 0)
 *	on a file containing  (fred smith)
 *	would return "(", "fred", "smith", and ")".
 *	Behavior is undefined if one of the predefined delimiters
 *	(white space or #) appears in delims[].  Explicit quoting
 *	(with ", ' or \) overrides detection of delimiters.
 *
 * int fgettransform(FILE *f, int ntransforms, float *transforms, int binary)
 *	Reads 4x4 matrices from FILE.  Returns the number of matrices found,
 *	up to ntransforms.  Returns 0 if no numbers are found.
 *	On finding incomplete matrices (not a multiple of 16 floats)
 *	returns -1, regardless of whether any whole matrices were found.
 *	Matrices are expected in the form used to transform a row vector
 *	multiplied on the left, i.e.  p * T -> p'; thus Euclidean translations
 *	appear in the last row.
 *
 * int fputtransform(FILE *f, int ntransforms, float *transforms, int binary)
 *	Writes 4x4 matrices to FILE.  Returns the number written, i.e.
 *	ntransforms unless an error occurs.
 *
 * int fputnf(FILE *f, int nfloats, float *fv, int binary)
 *	Writes 'nfloats' floats to the given file.  ASCII is in %g format,
 *	separated by blanks.
 *
 * FILE *fstropen(str, len, mode)
 *	Opens a string (buffer) as a "file".
 *	Mode is "r" or "w" as usual.
 *	Reads should return EOF on encountering end-of-string,
 *	writes past end-of-string should also yield an error return.
 *	fclose() should be used to free the FILE after use.
 */

#if HAVE_FMEMOPEN
/*
 * fmemopen is GNU stuff, AFAIK. _AND_, PLEASE NOTE NOTE NOTE it makes
 * a _MAJOR_ difference on 64bit-systems whether a functions returns
 * int or "long int" _AND_ which bit-lengths its arguments have.
 *
 * Also, defining _GNU_SOURCE should not do any harm on non-GNU
 * systems :)
 */
# define _GNU_SOURCE
#endif
#include <stdio.h>
#include <sys/types.h>

#if defined(unix) || defined(__unix)
# include <sys/time.h>
# ifndef NeXT
#  include <unistd.h>
# endif
# ifdef AIX
#  include <sys/select.h>
# endif
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ooglutil.h"


#ifndef WORDS_BIGENDIAN
#  if HAVE_NETINET_IN_H
#  include <netinet/in.h>	/* for ntohl(), etc. */
# else

int ntohl(unsigned int v) {
   return (((v>>24)&0xFF) | ((v>>8)&0xFF00) | ((v<<8)&0xFF0000) | ((v&0xFF)<<24));
}
int ntohs(unsigned int s) {
   return (((s>>8)&0xFF) | ((s&0xFF)<<8));
}
#define htonl(v)  ntohl(v)
#define htons(v)  ntohs(v)
# endif
#endif

#ifdef __hpux	/* Hack names of stdio buffer fields */
#define _base __base
#define _ptr __ptr
#define _cnt __cnt
#endif 

#if defined(__FreeBSD__) || defined(__CYGWIN__)
#define _base _bf._base
#define _ptr _p
#define _cnt _r
#endif

#if defined(__linux__) && !defined(_STDIO_USES_IOSTREAM)
	/* Handle older (pre-1.0) Linux stdio fields,
	 * making them look like later ones.
	 */
#define	 _IO_read_base	_base
#define  _IO_read_ptr	_gptr
#define	 _IO_read_end	_egptr
#endif

	/* Peer into a stdio buffer, check whether it has data available
	 * for reading.  Almost portable given the common stdio ancestry.
	 */
#ifdef __linux__
#define F_HASDATA(f)  ((f)->_IO_read_ptr < (f)->_IO_read_end)
#else
#define F_HASDATA(f)  ((f)->_cnt > 0)
#endif


int
fnextc(FILE *f, int flags)
{
	register int c;

	c = getc(f);
	for(;;) {
	    switch(c) {
	    case EOF:
		return(EOF);

	    case ' ':
	    case '\t':
		break;			/* Always skip blanks and tabs */

	    case '#':
		if(flags & 2)		/* 2: stop on comments, else skip */
		    goto fim;

		while((c = getc(f)) != '\n' && c != EOF)
		    ;
		continue;		/* Rescan this c */

	    case '\n':
		if(!(flags & 1))	/* 1: stop on \n's, else skip them */
		    break;
					/* flags&1 => fall into default */

	    default:
	     fim:
		ungetc(c, f);
		return(c);
	    }

	    c = getc(f);
	}
}


float f_pow10[11] = { 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10 };

#define fPow10(n)  ((unsigned)(n) > 10 ? fpow10(n) : f_pow10[n])

float
fpow10(int n)
{
	register int k;
	register float v;

	if((k = n) < 0)
		k = -k;
	v = f_pow10[k & 7];
	if(k >= 8) {
		float unit = 1e8;

		k >>= 3;
		for(;;) {
			if(k & 1)
				v *= unit;
			if((k >>= 1) == 0)
				break;
			unit *= unit;
		}
	}
	return(n < 0 ? 1.0/v : v);
}

/*
 * Read an array of white-space-separated floats from file 'f' in ASCII, fast.
 * Returns the number successfully read.
 */
int
fgetnf(register FILE *f, int maxf, float *fv, int binary)
{
	int ngot;
	float v;
	register int c = EOF;
	register long n;
	int s, es, nd, any;

	if(binary) {
#if WORDS_BIGENDIAN
		/* Easy -- our native floating point == big-endian IEEE */
		return fread((char *)fv, sizeof(float), maxf, f);
#else /* not native big-endian IEEE */
		long w;
		for(n=0; n<maxf && fread((char *)&w,sizeof(long),1,f) > 0; n++)
		    *(long *)&fv[n] = ntohl(w);
		return n;
#endif /* not native big-endian IEEE */
	}

	/* Read ASCII format floats */
	for(ngot = 0; ngot < maxf; ngot++) {
		if(fnextc(f, 0) == EOF)
			return(ngot);
		n = 0;
		s = 0;
		nd = 0;
		any = 0;
		if((c = getc(f)) == '-') {
			s = 1;
			c = getc(f);
		}
		while(c >= '0' && c <= '9') {
			n = n*10 + c - '0';
			nd++;
			if(n >= 214748364) {	/* 2^31 / 10 */
				v = any ? v*fPow10(nd) + (float)n : (float)n;
				nd = n = 0;
				any = 1;
			}
			c = getc(f);
		}
		v = any ? v*fPow10(nd) + (float)n : (float)n;
		any += nd;
		if(c == '.') {
			nd = n = 0;
			while((c = getc(f)) >= '0' && c <= '9') {
				n = n*10 + c - '0';
				nd++;
				if(n >= 214748364) {
					v += (float)n / fPow10(nd);
					n = 0;
				}
			}
			v += (float)n / fPow10(nd);
		}
		if(any == 0 && nd == 0)
			break;
		if(c == 'e' || c == 'E') {
			es = nd = 0;
			switch(c = getc(f)) {
			case '-':
				es = 1;	/* And fall through */
			case '+':
				c = getc(f);
			}
			n = 0;
			while(c >= '0' && c <= '9') {
				n = n*10 + c - '0';
				nd++;
				c = getc(f);
			}
			if(nd == 0)
				break;
			if(es) v /= fPow10(n);
			else v *= fPow10(n);
		}
		fv[ngot] = s ? -v : v;
	}
	if(c!=EOF) ungetc(c, f);
	return(ngot);
}


int
fgetni(register FILE *f, int maxi, int *iv, int binary)
{
	int ngot;
	register int c = EOF;
	register long n;
	int s, any;

	if(binary) {
#if WORDS_BIGENDIAN
		/* Easy -- our native floating point == big-endian IEEE */
		return fread((char *)iv, sizeof(int), maxi, f);
#else /* not native big-endian int's */
		long w;
		for(n = 0; n < maxi && fread(&w,4,1,f) > 0; n++)
		    iv[n] = ntohl(w);
		return n;
#endif /* not native big-endian int's */
	}

	/* Read ASCII format floats */
	for(ngot = 0; ngot < maxi; ngot++) {
		if(fnextc(f, 0) == EOF)
			return(ngot);
		n = 0;
		s = 0;
		any = 0;
		if((c = getc(f)) == '-') {
			s = 1;
			c = getc(f);
		}
		while(c >= '0' && c <= '9') {
			n = n*10 + c - '0';
			any = 1;
			c = getc(f);
		}
		if(!any)
			break;
		iv[ngot] = s ? -n : n;
	}
	if(c!=EOF) ungetc(c, f);
	return(ngot);
}

int
fgetns(register FILE *f, int maxs, short *sv, int binary)
{
	int ngot;
	register int c = EOF;
	register long n;
	int s, any;

	if(binary) {
#if WORDS_BIGENDIAN
		/* Easy -- our native floating point == big-endian IEEE */
		return fread((char *)sv, sizeof(short), maxs, f);
#else /* not native big-endian int's */
		short w;
		for(n = 0; n < maxs && fread(&w,2,1,f) > 0; n++)
		    sv[n] = ntohs(w);
		return n;
#endif /* not native big-endian int's */
	}

	/* Read ASCII format floats */
	for(ngot = 0; ngot < maxs; ngot++) {
		if(fnextc(f, 0) == EOF)
			return(ngot);
		n = s = any = 0;
		if((c = getc(f)) == '-') {
			s = 1;
			c = getc(f);
		}
		while(c >= '0' && c <= '9') {
			n = n*10 + c - '0';
			any = 1;
			c = getc(f);
		}
		if(!any)
			break;
		sv[ngot] = s ? -n : n;
	}
	if(c!=EOF) ungetc(c, f);
	return(ngot);
}

/*
 * Check for a string on a file.
 * If found, return 0.
 * If not, return the offset of the last matched char +1
 * and ungetc the failed char so the caller can see it.
 */
int
fexpectstr(register FILE *file, char *str)
{
	register char *p = str;
	register int c;

	while(*p != '\0') {
	    if((c = getc(file)) != *p++) {
		if(c != EOF)
		    ungetc(c, file);
		return(p - str);
	    }
	}
	return 0;
}

/*
 * Check for a string on a file, skipping leading blanks.
 */
int
fexpecttoken(register FILE *file, char *str)
{
	(void) fnextc(file, 0);
	return fexpectstr(file, str);
}

int fescape(FILE *f)
{
    int n, k, c = fgetc(f);

    switch(c) {
	case 'n': return '\n';
	case 'b': return '\b';
	case 't': return '\t';
	case 'r': return '\r';
    }
    if(c < '0' || c > '7')
	return c;
    
    n = c-'0';  k = 2;
    while((c = fgetc(f)) >= '0' && c <= '7') {
	n = (n*8) | (c-'0');
	if(--k <= 0)
	    return n;
    }
    if(c != EOF) ungetc(c, f);
    return n;
}

/*
 * Get a token, return a string or NULL.
 * Tokens may be "quoted" or 'quoted'; backslashes accepted.
 * The string is statically allocated and should be copied if
 * needed before the next call to ftoken().
 */
char *
ftoken(FILE *file, int flags)
{
	static char *token = NULL;
	static int troom = 0;
	register int c;
	register char *p;
	register int term;

	if((term = fnextc(file, flags)) == EOF)
	    return NULL;

	if(token == NULL) {
	    troom = 50;
	    token = malloc(troom * sizeof(char));
	    if(token == NULL)
		return NULL;
	}

	p = token;
	switch(term) {
	case '"':
	case '\'':
	    (void) fgetc(file);
	    for(;;) { 
		if((c = getc(file)) == EOF || c == term)
		    break;
		else if(c == '\\')
		    c = fescape(file);
		*p++ = c;
		if(p == &token[troom]) {
		    token = realloc(token, troom * 2);
		    if(token == NULL)
			return NULL;
		    p = &token[troom];
		    troom *= 2;
		}
	    }
	    break;

	default:
	    if(isspace(term))
		return NULL;
	    while((c = getc(file)) != EOF && !isspace(c)) {
		if(c == '\\')
		    c = fescape(file);
		*p++ = c;
		if(p == &token[troom]) {
		    token = realloc(token, troom * 2);
		    if(token == NULL)
			return NULL;
		    p = &token[troom];
		    troom *= 2;
		}
	    }
	    break;
	}
	*p = '\0';
	return token;
}


/*
 * Get a token, return a string or NULL.
 * Tokens may be "quoted" or 'quoted'; backslashes accepted.
 * The string is statically allocated and should be copied if
 * needed before the next call to ftoken().
 */
char *
fdelimtok(char *delims, FILE *file, int flags)
{
	static char *token = NULL;
	static int troom = 0;
	register int c;
	register char *p;
	register char *q;
	register int term;

	if((term = fnextc(file, flags)) == EOF)
	    return NULL;

	if(token == NULL) {
	    troom = 50;
	    token = malloc(troom * sizeof(char));
	    if(token == NULL)
		return NULL;
	}

	p = token;
	switch(term) {
	case '"':
	case '\'':
	    (void) fgetc(file);
	    for(;;) { 
		if((c = getc(file)) == EOF || c == term)
		    break;
		else if(c == '\\')
		    c = fescape(file);
		*p++ = c;
		if(p == &token[troom]) {
		    token = realloc(token, troom * 2);
		    if(token == NULL)
			return NULL;
		    p = &token[troom];
		    troom *= 2;
		}
	    }
	    break;

	default:
	    if(isspace(term))
		return NULL;
	    while((c = getc(file)) != EOF && !isspace(c)) {
		if(c == '\\')
		    c = fescape(file);
		*p = c;
		if(++p == &token[troom]) {
		    token = realloc(token, troom * 2);
		    if(token == NULL)
			return NULL;
		    p = &token[troom];
		    troom *= 2;
		}
		for(q = delims; *q && c != *q; q++)
		    ;
		if(*q) {
		    if(p > token+1) {
			p--;
			ungetc(c, file);
		    }
		    break;
		}
	    }
	    break;
	}
	*p = '\0';
	return token;
}


/*
 * Load one or more Transforms from a file.
 * Return 1 on success, 0 on failure.
 */
int
fgettransform(FILE *file, int ntrans, float *trans /* float trans[ntrans][4][4] */, int binary)
{
	register float *T;
	int nt;

	for(nt = 0; nt < ntrans; nt++) {
	    T = trans + 16*nt;
	    switch(fgetnf(file, 16, T, binary)) {
	    case 16:
		break;

	    case 0:
		return nt;

	    default:
		return -1;
	    }
	}
	return ntrans;
}

int
fputnf(FILE *file, int count, float *v, int binary)
{
	register int i;
	if(binary) {
#if WORDS_BIGENDIAN
	  return fwrite(v, sizeof(float), count, file);
#else
	  long w;
	  for(i = 0; i < count; i++) {
	    w = htonl(*(long *)&v[i]);
	    fwrite(&w, sizeof(float), 1, file);
	  }
	  return count;
#endif
	}

	fprintf(file, "%g", v[0]);
	for(i = 1; i < count; i++)
	    fprintf(file, " %g", v[i]);
	return count;
}

int
fputtransform(FILE *file, int ntrans, float *trans, int binary)
{
	register int i, n;
	register float *p;

	if(binary) {
#if WORDS_BIGENDIAN
	    return fwrite(trans, 4*4*sizeof(float), ntrans, file);
#else
	OOGLError(1, "fputtransform: need code to handle binary writes for this architecture.");
	return 0;
#endif
	}

	/* ASCII. */

	for(n = 0; n < ntrans; n++) {
	    p = trans + n*16;
	    for(i = 0; i < 4; i++, p += 4) {
		fprintf(file, "  %12.8g  %12.8g  %12.8g  %12.8g\n",
		    p[0], p[1], p[2], p[3]);
	    }
	    if(ferror(file))
		return n;
	    fprintf(file, "\n");
	}
	return ntrans;
}

/*
 * Given a file pointer, return a string attempting to show the context
 * of its current position.  If no data is available, returns the empty string.
 */
char *
fcontext(register FILE *f)
{
    static char *cont = NULL;
    static char dflt[] = "";
    char buf[1024];
    int npre, npost, nlpre, nlpost, tab, len;
    int predots = 4, postdots = 4;
    register char *p, *q;
    char *lastline, *lastnonblank;
    char *base, *ptr;
    int cnt;

    if(f == NULL)
	return dflt;
    if(feof(f))
	return "> END OF FILE\n";
#ifdef __linux__
    base = (char *)f->_IO_read_base;
    ptr = (char *)f->_IO_read_ptr;
    cnt = ptr - base;
#else
    base = (char *)f->_base;
    ptr = (char *)f->_ptr;
    cnt = f->_cnt;
#endif
    if(cnt <= 0 || base == NULL || ptr == NULL)
	return dflt;

    p = ptr;
    for(npre = nlpre = 0; --p >= base && npre < 128; npre++) {
	if(*p == '\n') {
	    if(++nlpre > 2 || npre > 60) {
		predots = 0;
		break;
	    }
	} else if(*p & 0x80 || *p == 0)		/* binary data? */
	    break;
    }
    strcpy(buf, "> ... ");
    q = buf + 2 + predots;
    tab = 2 + predots;
    for(p = ptr - npre; p < ptr; ) {
	switch(*q++ = *p++) {
	case '\n': case '\r':	*q++ = '>'; *q++ = ' '; tab = 2; break;
	case '\t':		tab += 8-(tab&7); break;
	default:		tab++;
	}
    }
    len = npre;
    npost = nlpost = 0;
    lastline = lastnonblank = q;
    for(p = ptr;  p < ptr + cnt && len < 128;  len++, q++) {
	*q = *p++;
	if(*q == '\n') {
	    if(nlpost == 0) {
		while(--tab > 0) *++q = '-';	/* Point ---^ to error */
		*++q = '^'; *++q = '\n';
	    }
	    if((++nlpost >= 2 || len > 80) && len > 32) {
		postdots = 0;
		break;
	    }
	    lastline = q;
	    *++q = '>'; *++q = ' ';
	} else if(*q & 0x80 || *q == 0)		/* Binary data */
	    break;
	else if(isprint(*q))
	    lastnonblank = q;
    }
    if(postdots && lastnonblank < lastline) {
	q = lastline;		/* Suppress trailing white space */
	postdots = 0;		/* to avoid final ``> ...'' */
    }
    strcpy(q, " ...\n" + 4-postdots);
    if(nlpost == 0) {
	q += strlen(q);
	while(--tab > 0) *q++ = '-';
	strcpy(q, "^\n");
    }
    if(cont) free(cont);
    return (cont = strdup(buf));
}

#if USE_SEEKPIPE

/* this one need the fopencookie() stuff from glibc but does _not_ twiddle
 * with libio interna.
 */

struct stdio_mark {
    FILE   *file;
    fpos_t pos;
};

struct stdio_mark *stdio_setmark(struct stdio_mark *mark, FILE *file)
{
    if (!mark) {
	mark = malloc(sizeof(struct stdio_mark));
    }
    mark->file = file;
    fgetpos(file, &mark->pos);
    return mark;
}

int stdio_seekmark(struct stdio_mark *mark)
{
    return fsetpos(mark->file, &mark->pos) == 0;
}

void stdio_freemark(struct stdio_mark *mark)
{
	free(mark);
}
#endif

#if USE_IO_MARKER
/* Using the functions above is a no-no but works. It is left a
 * compilation option whether these are used (see configure)
 */
struct stdio_mark {
    struct _IO_marker marker;
};

extern void _IO_init_marker __P ((struct _IO_marker *, _IO_FILE *));
extern void _IO_remove_marker __P ((struct _IO_marker *));
extern int _IO_seekmark __P ((_IO_FILE *, struct _IO_marker *, int));

struct stdio_mark *stdio_setmark(struct stdio_mark *mark, FILE *file)
{
	if (!mark) {
		mark = malloc(sizeof(struct stdio_mark));
	}
	_IO_init_marker(&mark->marker, file);
	return mark;
}

int stdio_seekmark(struct stdio_mark *mark)
{
	return _IO_seekmark(mark->marker._sbuf, &mark->marker, 0) == 0;
}
void stdio_freemark(struct stdio_mark *mark)
{
	_IO_remove_marker(&mark->marker);
	free(mark);
}
#endif

#if HAVE_FMEMOPEN
FILE *fstropen(char *mem, int len, char *mode)
{
  return (FILE *)fmemopen(mem, len, mode);
}
#else
# define USE_FSTROPEN_1 1
#endif

#if !USE_SEEKPIPE && !USE_IO_MARKER

#if defined(__linux__) 

#if 0 /* really outdated */
struct stdio_mark *stdio_setmark(struct stdio_mark *m, FILE *f)
{ return CC_stdio_setmark__FP10stdio_markP8_IO_FILE(m, f); }

int stdio_seekmark(struct stdio_mark *mark)
{ return CC_stdio_seekmark__FP10stdio_mark(mark) != EOF; }

void stdio_freemark(struct stdio_mark *mark)
{ CC_stdio_freemark__FP10stdio_mark(mark); }
#else

	/* More with vanilla stdio:
	 * attempt to seek backwards (backtrack) on a stream.
	 * Use true seeking where possible, otherwise attempt to
	 * fiddle with the buffer pointers and pray it's still in the buffer
	 * (sigh).   [GNU libc has real facilities for doing this!]
	 */
struct stdio_mark {
  FILE *f;
  off_t fpos;
  FILE fcopy;
  char fchars[8];
};

struct stdio_mark *
stdio_setmark(register struct stdio_mark *mark, register FILE *stream)
{
    if(mark == NULL)
	mark = OOGLNewE(struct stdio_mark, "stdio_setmark mark");
    mark->f = stream;

    mark->fpos = isatty(fileno(stream)) ? -1 : ftello(stream);
    mark->fcopy = *stream;
    memcpy(mark->fchars, mark->f->_IO_read_base, sizeof(mark->fchars));
    return mark;
}

int
stdio_seekmark(register struct stdio_mark *mark)
{
     /* Efficient no-op if we're seeking to where we already are */
    if(memcmp(mark->f, &mark->fcopy, sizeof(*mark->f)) == 0 &&
	memcmp(mark->fchars, mark->f->_IO_read_base, sizeof(mark->fchars)) == 0) {
	    return 1;
    }

    if(mark->fpos == -1 || fseeko(mark->f, mark->fpos, SEEK_SET) == -1) {
	/* Maybe it's a pipe or socket, so seeks fail.
	 * Try repositioning inside the stdio buffer.
	 * This is a stdio-dependent kludge, but might work.
	 */
	if(mark->f->_IO_read_base == mark->fcopy._IO_read_base &&
	   memcmp(mark->fchars,
		  mark->f->_IO_read_base,
		  sizeof(mark->fchars)) == 0) {
		    /* Buffer looks unchanged.  Reset pointers. */
	    *(mark->f) = mark->fcopy;
	} else {
	    return 0;	/* Failed */
	}
    }
    return 1;		/* Succeeded */
}

void
stdio_freemark(struct stdio_mark *mark)
{
    OOGLFree(mark);
}

#endif

	/* End Linux (GNU libc, more or less) */

#else  /* Roughly vanilla stdio */

	/* More with vanilla stdio:
	 * attempt to seek backwards (backtrack) on a stream.
	 * Use true seeking where possible, otherwise attempt to
	 * fiddle with the buffer pointers and pray it's still in the buffer
	 * (sigh).   [GNU libc has real facilities for doing this!]
	 */
struct stdio_mark {
	FILE *f;
	long fpos;
	FILE fcopy;
	char fchars[8];
};

struct stdio_mark *
stdio_setmark(register struct stdio_mark *mark, register FILE *stream)
{
    if(mark == NULL)
	mark = OOGLNewE(struct stdio_mark, "stdio_setmark mark");
    mark->f = stream;

    mark->fpos = isatty(fileno(stream)) ? -1 : ftell(stream);
    mark->fcopy = *stream;
    memcpy(mark->fchars, mark->f->_base, sizeof(mark->fchars));
    return mark;
}

int
stdio_seekmark(register struct stdio_mark *mark)
{
     /* Efficient no-op if we're seeking to where we already are */
    if(memcmp(mark->f, &mark->fcopy, sizeof(*mark->f)) == 0 &&
	memcmp(mark->fchars, mark->f->_base, sizeof(mark->fchars)) == 0) {
	    return 1;
    }

    if(mark->fpos == -1 || fseek(mark->f, mark->fpos, 0) == -1) {
	/* Maybe it's a pipe or socket, so seeks fail.
	 * Try repositioning inside the stdio buffer.
	 * This is a stdio-dependent kludge, but might work.
	 */
	if(mark->f->_base == mark->fcopy._base &&
	   memcmp(mark->fchars, mark->f->_base, sizeof(mark->fchars)) == 0) {
		    /* Buffer looks unchanged.  Reset pointers. */
	    *(mark->f) = mark->fcopy;
	} else {
	    return 0;	/* Failed */
	}
    }
    return 1;		/* Succeeded */
}

void
stdio_freemark(struct stdio_mark *mark)
{
    OOGLFree(mark);
}

#endif  /* vanilla stdio */

#endif /* !USE_IO_MARKER && !USE_SEEKPIPE */

/***************************************************************************/

#if !USE_FSTROPEN_1 && !HAVE_FMEMOPEN

#if defined(AIX) || defined(__osf__) || defined(__hpux) || defined(__FreeBSD__

#define USE_FSTROPEN_1 1

#else /* Even more nearly vanilla stdio */

/*
 * This one is stdio dependent, but seems to work on a fair variety of
 * implementations.
 */
FILE *
fstropen(char *str, int len, char *mode)
{
	FILE *f;

	f = fopen("/dev/null", mode);	/* How else do I get a FILE *? */
	if(f == NULL)
	    return NULL;
	setbuf(f, NULL);
	close(fileno(f));

#ifdef __hpux
	f->__fileL = f->__fileH = -1;
#else
	f->_file = -1;
#endif
	f->_cnt = len;
	f->_ptr = f->_base = (unsigned char *) str;
	f->_flag &= ~(_IONBF|_IOMYBUF|_IOEOF|_IOERR);
	return f;
}
#endif /* non-AIX, non-OSF stdio */

#endif /* !USE_FSTROPEN_1 && !HAVE_FMEMOPEN */

/***************************************************************************/

#if USE_FSTROPEN_1

/* The stdio-buf-hacking code below doesn't work on some systems, so
 * just ship the data through a pipe.  Small (8K?) size limit.
 */
FILE *fstropen(char *str, int len, char *mode)
{
   int pfd[2];
   if(pipe(pfd) < 0)
    return NULL;
   if(mode[0] == 'w') {
	OOGLError(0, "fstropen(..., %d, \"%s\"): Sorry, can't handle write mode",
		len, mode);
	return fdopen(dup(2), "wb");
   }
   if(len >= 8192) {
	OOGLError(0, "fstropen(\"%.32s...\", %d, \"%s\"): Sorry, can't deal with more than 8192-byte strings");
	len = 4096;
   }
   write(pfd[1], str, len);
   close(pfd[1]);
   return fdopen(pfd[0], "rb");
}

#endif

/***************************************************************************/

int
fhasdata(FILE *f)
{
    return F_HASDATA(f);
}


int
async_getc(register FILE *f)
{
    return async_getc_fd(f, -1);
}

int
async_getc_fd(register FILE *f, int fd)
{

#if defined(unix) || defined(__unix)

    fd_set fds;
    static struct timeval notime = { 0, 0 };

    if (fd < 0) {
	fd = fileno(f);
    }

    if(F_HASDATA(f))
	return getc(f);
    if (fd < 0)
	return NODATA;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    if(select(fd+1, &fds, NULL, NULL, &notime) == 1)
	return fgetc(f);
    return NODATA;
#else
    return getc(f);
#endif
}

int
async_fnextc(register FILE *f, register int flags)
{
    return async_fnextc_fd(f, flags, -1);
}

int
async_fnextc_fd(register FILE *f, register int flags, int fd)
{
    register int c;

    c = async_getc_fd(f, fd);
    for(;;) {
	switch(c) {
	case EOF:
	case NODATA:
	    return(c);

	case ' ':
	case '\t':
	    break;			/* Always skip blanks and tabs */

	case '#':
	    if(flags & 2)		/* 2: stop on comments, else skip */
		goto fim;

	    while((c = getc(f)) != '\n' && c != EOF)
		;
	    continue;			/* Rescan this c */

	case '\n':
	    if(!(flags & 1))		/* 1: stop on \n's, else skip them */
		break;
				    	/* flags&1 => fall into default */

	default:
	 fim:
	    ungetc(c, f);
	    return(c);
	}

	c = async_getc_fd(f, fd);
    }
}

#if defined(__linux__) && defined(i386)
/* This is really just for debugging.  Linux kernel/gdb don't support
 * access to floating-point registers yet, so we use this kludge,
 * along with a .gdbinit file like:

define qfp
set $junk = fpudump()
set $tos = (fps.status >> 11) & 7
set $control = fps.control
set $status = fps.status
set $tags = fps.tag
set $stags = (($tags*65537) >> ($tos*2)) & 0xFFFF
printf "fcw %x  status %x fSP %d tags %x->%x %d %d %d %d %d %d %d %d\n", \
	$control, $status, $tos, \
	$tags, $stags, $stags&3, ($stags>>2)&3, ($stags>>4)&3, ($stags>>6)&3, \
	($stags>>8)&3, ($stags>>10)&3, ($stags>>12)&3, ($stags>>14)&3
end

define fs
qfp
echo About to: 
x/i $pc
stepi
qfp
echo Next: 
x/i $pc
end

define ion
set control0=1
set control1=0
qfp
end

define ioff
set control0=0
set control1=1
qfp
end
 */
static struct env387 
{
  unsigned short control;
  unsigned short r0;
  unsigned short status;
  unsigned short r1;
  unsigned short tag;
  unsigned short r2;
  unsigned long eip;
  unsigned short code_seg;
  unsigned short opcode;
  unsigned long operand;
  unsigned short operand_seg;
  unsigned short r3;
  unsigned char regs[8][10];
} fps;
static int control0 = 0, control1 = 0, status0 = 0;
void
fpudump() {
  __asm__("fnsave fps");
  fps.control &= ~control0;
  fps.control |= control1;
  fps.status &= ~status0;
  __asm__("frstor fps");
}
#endif
