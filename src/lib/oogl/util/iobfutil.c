/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2002-2007 Claus-Justus Heine
 *
 * This file is part of Geomview.
 * 
 * Geomview is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
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

/*
 * Geometry object routines, like in futil.c, but FILE replace with
 * IOBFILE implemented in iobuffer.c. This way we can seek back safely
 * on pipes, sockets, etc.
 *
 * Utility routines, useful for many objects
 *
 * int
 * iobfgetnf(iobf, nfloats, floatp, binary)
 *	Read an array of floats from a file in "ascii" or "binary" format.
 *	Returns number of floats successfully read, should = nfloats.
 *	"Binary" means "IEEE 32-bit floating-point" format.
 *
 * int
 * iobfgetni(IOBFILE *iobf, int nints, int *intsp, int binary)
 *	Read an array of ints from a file in "ascii" or "binary" format.
 *	Returns number of ints successfully read, should = nints.
 *	"Binary" means "32-bit big-endian" integer format.
 *
 * int
 * iobfgetns(IOBFILE *iobf, int nshorts, short *intsp, int binary)
 *	Read an array of shorts from a file in "ascii" or "binary" format.
 *	Returns number of shorts successfully read, should = nints.
 *	"Binary" means "16-bit big-endian" integer format.
 *
 * int
 * iobfexpectstr(IOBFILE *iobf, char *string)
 *	Expect the given string to appear immediately on file.
 *	Return 0 if the complete string is found,
 *	else the offset+1 of the last matched char within string.
 *	The first unmatched char is ungetc'd.
 *
 * int
 * iobfexpecttoken(IOBFILE *iobf, char *string)
 *	Expect the given string to appear on the iobf, possibly after
 *	skipping some white space and comments.
 *	Return 0 if found, else the offset+1 of last matched char in string.
 *	The first unmatched char is ungetc'd.
 *
 * int iobfnextc(IOBFILE *f, int flags)
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
 * int async_iobfnextc(IOBFILE *f, int flags)
 *	Like fnextc() above, but guarantees not to block if no data is
 *	immediately available.  It returns either an interesting character,
 *	EOF, or the special code NODATA (== -2).
 *      if fd == -1, then fileno(f) is used, otherwise fd.
 *
 * int async_iobfgetc(IOBFILE *f, false)
 *	Like getc(), but guarantees not to block.  Returns NODATA if
 *	nothing is immediately available.
 *
 * char *iobftoken(IOBFILE *f, int flags)
 *	Skips uninteresting characters with fnextc(f, flags),
 *	then returns a "token" - string of consecutive interesting characters.
 *	Returns NULL if EOF is reached with no token, or if
 *	flags specifies stopping at end-of-line and this is encountered with
 *	no token found.
 *	The token is effectively statically allocated and will be
 *	overwritten by the next ftoken() call.
 *
 * char *iobfdelimtok(char *delims, IOBFILE *f, int flags)
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
 * int iobfgettransform(IOBFILE *f, int ntransforms,
 *                      float *transforms, int binary)
 *	Reads 4x4 matrices from IOBFILE.  Returns the number of matrices found,
 *	up to ntransforms.  Returns 0 if no numbers are found.
 *	On finding incomplete matrices (not a multiple of 16 floats)
 *	returns -1, regardless of whether any whole matrices were found.
 *	Matrices are expected in the form used to transform a row vector
 *	multiplied on the left, i.e.  p * T -> p'; thus Euclidean translations
 *	appear in the last row.
 *
 * int iobfputtransform(IOBFILE *f, int ntransforms,
 *                      float *transforms, int binary)
 *	Writes 4x4 matrices to IOBFILE.  Returns the number written, i.e.
 *	ntransforms unless an error occurs.
 *
 * int iobfputnf(IOBFILE *f, int nfloats, float *fv, int binary)
 *	Writes 'nfloats' floats to the given file.  ASCII is in %g format,
 *	separated by blanks.
 *
 */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <sys/types.h>
#include <math.h>

/* Try to get the prototype for select */
#if HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#if HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ooglutil.h"

#include "iobuffer.h"

#undef getc
#define getc gobble
#define ungetc gobble
#define fgetc  gobble

#ifndef WORDS_BIGENDIAN
static inline int gv_ntohl(unsigned int v) {
  return (((v >> 24) & 0x000000FF) |
	  ((v << 24) & 0xFF000000) |
	  ((v >>  8) & 0x0000FF00) |
	  ((v <<  8) & 0x00FF0000));
}
static inline short gv_ntohs(unsigned short s) {
  return (((s >> 8) & 0x00FF) | ((s << 8) & 0xFF00));
}
#else
static inline int gv_ntohl(unsigned int v) {
  return v;
}
static inline short gv_ntohs(unsigned short s) {
  return s;
}
#endif

static inline short gv_htons(unsigned short s)
{
  return gv_ntohs(s);
}
static inline int gv_htonl(unsigned int v) {
  return gv_ntohl(v);
}

int iobfnextc(IOBFILE *f, int flags)
{
  int c;

  c = iobfgetc(f);
  for(;;) {
    switch(c) {
    case EOF:
      return EOF;

    case ' ':
    case '\t':
      break;			/* Always skip blanks and tabs */

    case '#':
      if(flags & 2)		/* 2: stop on comments, else skip */
	goto fim;

      while((c = iobfgetc(f)) != '\n' && c != EOF)
	;
      continue;		/* Rescan this c */

    case '\n':
      if(!(flags & 1))	/* 1: stop on \n's, else skip them */
	break;
      /* flags&1 => fall into default */

    default:
    fim:
      iobfungetc(c, f);
      return c;
    }

    c = iobfgetc(f);
  }
}

/* Read an array of white-space-separated floats from file 'f' in
 * ASCII, fast.  Returns the number successfully read.
 */
int iobfgetnf(IOBFILE *f, int maxf, float *fv, int binary)
{
  int ngot;
  float v = 0;
  int c = EOF;
  long n;
  int s, es, nd, any;

  if(binary) {
#if WORDS_BIGENDIAN
    /* Easy -- our native floating point == big-endian IEEE */
    return iobfread((char *)fv, sizeof(float), maxf, f);
#else /* not native big-endian IEEE */
    union {
      int   wi;
      float wf;
    } w;
    for(n=0; n<maxf && iobfread((char *)&w,sizeof(float),1,f) > 0; n++) {
      w.wi = gv_ntohl(w.wi);
      fv[n] = w.wf;
    }
    return n;
#endif /* not native big-endian IEEE */
  }

  /* Read ASCII format floats */
  for(ngot = 0; ngot < maxf; ngot++) {
    if(iobfnextc(f, 0) == EOF)
      return(ngot);
    n = 0;
    s = 0;
    nd = 0;
    any = 0;
    if((c = iobfgetc(f)) == '-') {
      s = 1;
      c = iobfgetc(f);
    }
    while(c >= '0' && c <= '9') {
      n = n*10 + c - '0';
      nd++;
      if(n >= 214748364) {	/* 2^31 / 10 */
	v = any ? v*pow(10.0, nd) + (float)n : (float)n;
	nd = n = 0;
	any = 1;
      }
      c = iobfgetc(f);
    }
    v = any ? v*pow(10.0, nd) + (float)n : (float)n;
    any += nd;
    if(c == '.') {
      nd = n = 0;
      while((c = iobfgetc(f)) >= '0' && c <= '9') {
	n = n*10 + c - '0';
	nd++;
	if(n >= 214748364) {
	  v += (float)n / pow(10.0f, nd);
	  n = 0;
	}
      }
      v += (float)n / pow(10.0, nd);
    }
    if(any == 0 && nd == 0)
      break;
    if(c == 'e' || c == 'E') {
      es = nd = 0;
      switch(c = iobfgetc(f)) {
      case '-':
	es = 1;	/* And fall through */
      case '+':
	c = iobfgetc(f);
      }
      n = 0;
      while(c >= '0' && c <= '9') {
	n = n*10 + c - '0';
	nd++;
	c = iobfgetc(f);
      }
      if(nd == 0)
	break;
      if(es) v /= pow(10.0, n);
      else v *= pow(10.0, n);
    }
    fv[ngot] = s ? -v : v;
  }
  if(c!=EOF) iobfungetc(c, f);
  return(ngot);
}

/* Read an array of white-space-separated floats from file 'f' in
 * ASCII, fast.  Returns the number successfully read.
 */
int iobfgetnd(IOBFILE *f, int maxd, double *dv, int binary)
{
  int ngot;
  double v = 0;
  int c = EOF;
  long n;
  int s, es, nd, any;

  if(binary) {
#if WORDS_BIGENDIAN
    /* Easy -- our native floating point == big-endian IEEE */
    return iobfread((char *)dv, sizeof(double), maxd, f);
#else /* not native big-endian IEEE */
    union {
      int    wi[2];
      double wd;
    } w;
    int tmp;
    for(n=0; n<maxd && iobfread((char *)&w,sizeof(double),1,f) > 0; n++) {
      tmp     = gv_ntohl(w.wi[0]);
      w.wi[0] = gv_ntohl(w.wi[1]);
      w.wi[1] = tmp;
      dv[n] = w.wd;
    }
    return n;
#endif /* not native big-endian IEEE */
  }

  /* Read ASCII format floats */
  for(ngot = 0; ngot < maxd; ngot++) {
    if(iobfnextc(f, 0) == EOF)
      return(ngot);
    n = 0;
    s = 0;
    nd = 0;
    any = 0;
    if((c = iobfgetc(f)) == '-') {
      s = 1;
      c = iobfgetc(f);
    }
    while(c >= '0' && c <= '9') {
      n = n*10 + c - '0';
      nd++;
      if(n >= 214748364) {	/* 2^31 / 10 */
	v = any ? v*pow(10.0, nd) + (float)n : (float)n;
	nd = n = 0;
	any = 1;
      }
      c = iobfgetc(f);
    }
    v = any ? v*pow(10.0, (double)nd) + (float)n : (float)n;
    any += nd;
    if(c == '.') {
      nd = n = 0;
      while((c = iobfgetc(f)) >= '0' && c <= '9') {
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
      switch(c = iobfgetc(f)) {
      case '-':
	es = 1;	/* And fall through */
      case '+':
	c = iobfgetc(f);
      }
      n = 0;
      while(c >= '0' && c <= '9') {
	n = n*10 + c - '0';
	nd++;
	c = iobfgetc(f);
      }
      if(nd == 0)
	break;
      if(es) v /= pow(10.0, (double)n);
      else v *= pow(10.0, n);
    }
    dv[ngot] = s ? -v : v;
  }
  if(c!=EOF) iobfungetc(c, f);
  return(ngot);
}

int
iobfgetni(IOBFILE *f, int maxi, int *iv, int binary)
{
  int ngot;
  int c = EOF;
  long n;
  int s, any;

  if(binary) {
#if WORDS_BIGENDIAN
    /* Easy -- our native floating point == big-endian IEEE */
    return iobfread((char *)iv, sizeof(int), maxi, f);
#else /* not native big-endian int's */
    int w;
    for(n = 0; n < maxi && iobfread(&w,4,1,f) > 0; n++)
      iv[n] = gv_ntohl(w);
    return n;
#endif /* not native big-endian int's */
  }

  /* Read ASCII format floats */
  for(ngot = 0; ngot < maxi; ngot++) {
    if(iobfnextc(f, 0) == EOF)
      return(ngot);
    n = 0;
    s = 0;
    any = 0;
    if((c = iobfgetc(f)) == '-') {
      s = 1;
      c = iobfgetc(f);
    }
    while(c >= '0' && c <= '9') {
      n = n*10 + c - '0';
      any = 1;
      c = iobfgetc(f);
    }
    if(!any)
      break;
    iv[ngot] = s ? -n : n;
  }
  if(c!=EOF) iobfungetc(c, f);
  return(ngot);
}

int
iobfgetns(IOBFILE *f, int maxs, short *sv, int binary)
{
  int ngot;
  int c = EOF;
  long n;
  int s, any;

  if(binary) {
#if WORDS_BIGENDIAN
    /* Easy -- our native floating point == big-endian IEEE */
    return iobfread((char *)sv, sizeof(short), maxs, f);
#else /* not native big-endian int's */
    short w;
    for(n = 0; n < maxs && iobfread(&w,2,1,f) > 0; n++)
      sv[n] = gv_ntohs(w);
    return n;
#endif /* not native big-endian int's */
  }

  /* Read ASCII format floats */
  for(ngot = 0; ngot < maxs; ngot++) {
    if(iobfnextc(f, 0) == EOF)
      return(ngot);
    n = s = any = 0;
    if((c = iobfgetc(f)) == '-') {
      s = 1;
      c = iobfgetc(f);
    }
    while(c >= '0' && c <= '9') {
      n = n*10 + c - '0';
      any = 1;
      c = iobfgetc(f);
    }
    if(!any)
      break;
    sv[ngot] = s ? -n : n;
  }
  if(c!=EOF) iobfungetc(c, f);
  return(ngot);
}

/*
 * Check for a string on a file.
 * If found, return 0.
 * If not, return the offset of the last matched char +1
 * and iobfungetc the failed char so the caller can see it.
 */
int
iobfexpectstr(IOBFILE *iobf, char *str)
{
  char *p = str;
  int c;

  while(*p != '\0') {
    if((c = iobfgetc(iobf)) != *p++) {
      if(c != EOF)
	iobfungetc(c, iobf);
      return(p - str);
    }
  }
  return 0;
}

/*
 * Check for a string on a iobf, skipping leading blanks.
 */
int
iobfexpecttoken(IOBFILE *iobf, char *str)
{
  (void) iobfnextc(iobf, 0);
  return iobfexpectstr(iobf, str);
}

int iobfescape(IOBFILE *f)
{
  int n, k, c = iobfgetc(f);

  switch (c) {
  case 'n': return '\n';
  case 'b': return '\b';
  case 't': return '\t';
  case 'r': return '\r';
  }
  if (c < '0' || c > '7')
    return c;
    
  n = c-'0';  k = 2;
  while ((c = iobfgetc(f)) >= '0' && c <= '7') {
    n = (n*8) | (c-'0');
    if(--k <= 0)
      return n;
  }
  if (c != EOF) iobfungetc(c, f);
  return n;
}

/*
 * Get a token, return a string or NULL.
 * Tokens may be "quoted" or 'quoted'; backslashes accepted.
 * The string is statically allocated and should be copied if
 * needed before the next call to ftoken().
 */
char *
iobfquotedelimtok(const char *delims, IOBFILE *iobf, int flags, int *quote)
{
  static char *token = NULL;
  static int troom = 0;
  int c;
  char *p;
  const char *q;
  int term;

  if((term = iobfnextc(iobf, flags)) == EOF)
    return NULL;

  if(token == NULL) {
    troom = 50;
    token = malloc(troom * sizeof(char));
    if(token == NULL)
      return NULL;
  }

  p = token;
  switch (term) {
  case '"':
  case '\'':
    *quote = term;
    (void)iobfgetc(iobf);
    for(;;) { 
      if((c = iobfgetc(iobf)) == EOF || c == term)
	break;
      else if (c == '\\')
	c = iobfescape(iobf);
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
    *quote = '\0';
    if(isspace(term))
      return NULL;
    while((c = iobfgetc(iobf)) != EOF && !isspace(c)) {
      if(c == '\\')
	c = iobfescape(iobf);
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
	  iobfungetc(c, iobf);
	}
	break;
      }
    }
    break;
  }
  *p = '\0';
  return token;
}

char *iobfdelimtok(const char *delims, IOBFILE *iobf, int flags)
{
  int tmp;
  return iobfquotedelimtok(delims, iobf, flags, &tmp);
}
/*
 * Get a token, return a string or NULL.
 * Tokens may be "quoted" or 'quoted'; backslashes accepted.
 * The string is statically allocated and should be copied if
 * needed before the next call to ftoken().
 */
char *
iobfquotetoken(IOBFILE *iobf, int flags, int *quote)
{
  return iobfquotedelimtok("", iobf, flags, quote);
}

char *
iobftoken(IOBFILE *iobf, int flags)
{
  return iobfdelimtok("", iobf, flags);
}

/*
 * Load one or more Transforms from a file.
 * Return 1 on success, 0 on failure.
 */
int
iobfgettransform(IOBFILE *iobf, int ntrans, float *trans /* float trans[ntrans][4][4] */, int binary)
{
  float *T;
  int nt;

  for(nt = 0; nt < ntrans; nt++) {
    T = trans + 16*nt;
    switch(iobfgetnf(iobf, 16, T, binary)) {
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

/*
 * Given a file pointer, return a string attempting to show the context
 * of its current position.  If no data is available, returns the empty string.
 */
#define CONTEXT_SIZE 256

char *
iobfcontext(IOBFILE *f)
{
  static char *cont = NULL;
  static char dflt[] = "";
  char buf[1024];
  int npre, nlpre, nlpost, tab, len;
  int predots = 4, postdots = 4;
  char *p, *q;
  char *lastline, *lastnonblank;
  char base[CONTEXT_SIZE], *ptr;
  int cnt;

  if (f == NULL)
    return dflt;
  if (iobfeof(f)) {
    return "> END OF IOBFILE\n";
  }

  cnt = iobfgetbuffer(f, base, sizeof(base), -1);

  if(cnt <= 0) {
    return dflt;
  }

  ptr = base + cnt;
  p = ptr;
  for(npre = nlpre = 0; --p >= base && npre < CONTEXT_SIZE; npre++) {
    if(*p == '\n') {
      if(++nlpre > 2 || npre > 60) {
	predots = 0;
	break;
      }
    } else if(*p & 0x80 || *p == 0) { /* binary data? */
      break;
    }
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
  nlpost = 0;
  lastline = lastnonblank = q;
  for(p = ptr;  p < ptr + cnt && len < CONTEXT_SIZE;  len++, q++) {
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
  if(cont) {
    free(cont);
  }
  return (cont = strdup(buf));
}

/***************************************************************************/

int
iobfhasdata(IOBFILE *f)
{
  return iobfgetbuffer(f, NULL, 0, 1) > 0;
}

int
async_iobfgetc(IOBFILE *f)
{
#if HAVE_SELECT
  fd_set fds;
  int fd;
  static struct timeval notime = { 0, 0 };

  fd = iobfileno(f);

  if(iobfhasdata(f))
    return iobfgetc(f);
  if (fd < 0)
    return NODATA;
  FD_ZERO(&fds);
  FD_SET(fd, &fds);
  if(select(fd+1, &fds, NULL, NULL, &notime) == 1)
    return iobfgetc(f);
  return NODATA;
#else
  return iobfgetc(f);
#endif
}

int
async_iobfnextc(IOBFILE *f, int flags)
{
  int c;

  c = async_iobfgetc(f);
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

      while((c = iobfgetc(f)) != '\n' && c != EOF)
	;
      continue;			/* Rescan this c */

    case '\n':
      if(!(flags & 1))		/* 1: stop on \n's, else skip them */
	break;
      /* flags&1 => fall into default */

    default:
    fim:
      iobfungetc(c, f);
      return(c);
    }

    c = async_iobfgetc(f);
  }
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
