/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2002-2006 Claus-Justus Heine
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

/* $Header: /home/mbp/geomview-git/geomview-cvs/geomview/src/lib/oogl/util/futil.c,v 1.13 2006/07/14 09:57:57 rotdrop Exp $ */

/*
 * Geometry object routines. These routines have their "back-seekable"
 * counterparts in iobfutil.c, the routines below are just kept for
 * convenience for add-on modules.
 *
 * int fputtransform(FILE *f, int ntransforms, float *transforms, int binary)
 *	Writes 4x4 matrices to FILE.  Returns the number written, i.e.
 *	ntransforms unless an error occurs.
 *
 * int fputnf(FILE *f, int nfloats, float *fv, int binary)
 *	Writes 'nfloats' floats to the given file.  ASCII is in %g format,
 *	separated by blanks.
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
 */

#include <stdio.h>
#include <sys/types.h>
#include <math.h>
#include <ctype.h>
#include "ooglutil.h"

#ifndef WORDS_BIGENDIAN
static inline int ntohl(unsigned int v) {
  return (((v >> 24) & 0x000000FF) |
	  ((v << 24) & 0xFF000000) |
	  ((v >>  8) & 0x0000FF00) |
	  ((v <<  8) & 0x00FF0000));
}
static inline short ntohs(unsigned short s) {
  return (((s >> 8) & 0x00FF) | ((s << 8) & 0xFF00));
}
#else
static inline int ntohl(unsigned int v) {
  return v;
}
static inline short ntohs(unsigned short s) {
  return s;
}
#endif

static inline short htons(unsigned short s)
{
  return ntohs(s);
}
static inline int htonl(unsigned int v) {
  return ntohl(v);
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
	register int i, j, n;
	register float *p;

	if(binary) {
#if WORDS_BIGENDIAN
	    return fwrite(trans, 4*4*sizeof(float), ntrans, file);
#else
	    union {
		    int   wi;
		    float wf;
	    } w;
	    for (i = 0; i < ntrans; i++) {
		    for (j = 0; j < 16; j++) {
			    w.wf = trans[i*16 + j];
			    w.wi = htonl(w.wi);
			    if (fwrite(&w, sizeof(float), 1, file) != 1) {
				    return i;
			    }
		    }
	    }
	    return i;
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

/* Read an array of white-space-separated floats from file 'f' in
 * ASCII, fast.  Returns the number successfully read.
 */
int fgetnd(FILE *f, int maxd, double *dv, int binary)
{
  int ngot;
  double v = 0;
  register int c = EOF;
  register long n;
  int s, es, nd, any;

  if(binary) {
#if WORDS_BIGENDIAN
    /* Easy -- our native floating point == big-endian IEEE */
    return fread((char *)dv, sizeof(double), maxf, f);
#else /* not native big-endian IEEE */
    union {
      int    wi[2];
      double wd;
    } w;
    int tmp;
    for(n=0; n<maxd && fread((char *)&w,sizeof(double),1,f) > 0; n++) {
      tmp     = ntohl(w.wi[0]);
      w.wi[0] = ntohl(w.wi[1]);
      w.wi[1] = tmp;
      dv[n] = w.wd;
    }
    return n;
#endif /* not native big-endian IEEE */
  }

  /* Read ASCII format floats */
  for(ngot = 0; ngot < maxd; ngot++) {
    if(fnextc(f, 0) == EOF)
      return(ngot);
    n = 0;
    s = 0;
    nd = 0;
    any = 0;
    if((c = fgetc(f)) == '-') {
      s = 1;
      c = fgetc(f);
    }
    while(c >= '0' && c <= '9') {
      n = n*10 + c - '0';
      nd++;
      if(n >= 214748364) {	/* 2^31 / 10 */
	v = any ? v*pow(10.0, nd) + (float)n : (float)n;
	nd = n = 0;
	any = 1;
      }
      c = fgetc(f);
    }
    v = any ? v*pow(10.0, (double)nd) + (float)n : (float)n;
    any += nd;
    if(c == '.') {
      nd = n = 0;
      while((c = fgetc(f)) >= '0' && c <= '9') {
	n = n*10 + c - '0';
	nd++;
	if(n >= 214748364) {
	  v += (float)n / pow(10.0, (double)nd);
	  n = 0;
	}
      }
      v += (float)n / pow(10.0, (double)nd);
    }
    if(any == 0 && nd == 0)
      break;
    if(c == 'e' || c == 'E') {
      es = nd = 0;
      switch(c = fgetc(f)) {
      case '-':
	es = 1;	/* And fall through */
      case '+':
	c = fgetc(f);
      }
      n = 0;
      while(c >= '0' && c <= '9') {
	n = n*10 + c - '0';
	nd++;
	c = fgetc(f);
      }
      if(nd == 0)
	break;
      if(es) v /= pow(10.0, (double)n);
      else v *= pow(10, n);
    }
    dv[ngot] = s ? -v : v;
  }
  if(c!=EOF) ungetc(c, f);
  return(ngot);
}

/*
 * Read an array of white-space-separated floats from file 'f' in ASCII, fast.
 * Returns the number successfully read.
 */
int
fgetnf(register FILE *f, int maxf, float *fv, int binary)
{
	int ngot;
	float v = 0;
	register int c = EOF;
	register long n;
	int s, es, nd, any;

	if(binary) {
#if WORDS_BIGENDIAN
		/* Easy -- our native floating point == big-endian IEEE */
		return fread((char *)fv, sizeof(float), maxf, f);
#else /* not native big-endian IEEE */
		union {
			int   wi;
			float wf;
		} w;
		for(n=0; n<maxf && fread((char *)&w,sizeof(float),1,f) > 0; n++) {
			w.wi = ntohl(w.wi);
			fv[n] = w.wf;
		}
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
				v = any ? v*pow(10.0, nd) + (float)n : (float)n;
				nd = n = 0;
				any = 1;
			}
			c = getc(f);
		}
		v = any ? v*pow(10.0, nd) + (float)n : (float)n;
		any += nd;
		if(c == '.') {
			nd = n = 0;
			while((c = getc(f)) >= '0' && c <= '9') {
				n = n*10 + c - '0';
				nd++;
				if(n >= 214748364) {
					v += (float)n / pow(10.0, nd);
					n = 0;
				}
			}
			v += (float)n / pow(10.0, nd);
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
			if(es) v /= pow(10.0, n);
			else v *= pow(10.0, n);
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
		int w;
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
