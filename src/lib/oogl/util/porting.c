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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */


/*
 * Routines for porting to brain-damaged operating systems.
 * Contents:
 *    strdup(str)
 *    putenv(name
 */

#include <stdio.h>

#ifdef NeXT
#include <stdlib.h>
#include <string.h>

char *
strdup( char *str )
{
    char *s;
    int len;

    if(str == NULL)
	return NULL;
    len = strlen(str);
    s = malloc(len+1);
    memcpy(s, str, len+1);
    return s;
}


/*
 * The following copyright notice applies to everything in this
 * file from here on down.
 */

/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getenv.c	5.5 (Berkeley) 6/27/88";
#endif /* LIBC_SCCS and not lint */

#include <stdio.h>

/*
 * _findenv --
 *	Returns pointer to value associated with name, if any, else NULL.
 *	Sets offset to be the offset of the name/value combination in the
 *	environmental array, for use by setenv(3)/putenv(3) and unsetenv(3).
 *	Explicitly removes '=' in argument name.
 */
static char *
_findenv(name, offset)
	register char *name;
	int *offset;
{
	extern char **environ;
	register int len;
	register char **P, *C;

	for (C = name, len = 0; *C && *C != '='; ++C, ++len);
	for (P = environ; *P; ++P)
		if (!strncmp(*P, name, len))
			if (*(C = *P + len) == '=') {
				*offset = P - environ;
				return(++C);
			}
	return(NULL);
}
#if defined(LIBC_SCCS) && !defined(lint)
/* static char sccsid[] = "@(#)setenv.c	5.2 (Berkeley) 6/27/88"; */
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>

/*
 * putenv --
 *	Put a string of the form "name=value" into the environment.
 *	[Adapted from BSD routine setenv(name, value, rewrite).]
 */
putenv(register char *name)
{
	extern char **environ;
	static int alloced;			/* if allocated space before */
	register char *value;
	register char *C;
	int l_value, offset;

	value = strchr(name, '=');
	value = value ? value+1 : name;
	l_value = strlen(value);
	if ((C = _findenv(name, &offset))) {	/* find if already exists */
		if (strlen(C) >= l_value) {	/* old larger; copy over */
			while (*C++ = *value++);
			return(0);
		}
	}
	else {					/* create new slot */
		register int	cnt;
		register char	**P;

		for (P = environ, cnt = 0; *P; ++P, ++cnt);
		if (alloced) {			/* just increase size */
			environ = (char **)realloc((char *)environ,
			    (u_int)(sizeof(char *) * (cnt + 2)));
			if (!environ)
				return(-1);
		}
		else {				/* get new space */
			alloced = 1;		/* copy old entries into it */
			P = (char **)malloc((u_int)(sizeof(char *) *
			    (cnt + 2)));
			if (!P)
				return(-1);
			memcpy(P, environ, cnt * sizeof(char *));
			environ = P;
		}
		environ[cnt + 1] = NULL;
		offset = cnt;
	}
	for (C = name; *C && *C != '='; ++C);	/* no `=' in name */
	if (!(environ[offset] =			/* name + `=' + value */
	    malloc((u_int)((int)(C - name) + l_value + 2))))
		return(-1);
	for (C = environ[offset]; (*C = *name++) && *C != '='; ++C);
	for (*C++ = '='; *C++ = *value++;);
	return(0);
}

/*
 * unputenv(name) --
 *	Delete environmental variable "name".
 */
void
unputenv(name)
	char	*name;
{
	extern	char	**environ;
	register char	**P;
	int	offset;

	while (_findenv(name, &offset))		/* if set multiple times */
		for (P = &environ[offset];; ++P)
			if (!(*P = *(P + 1)))
				break;
}

#endif /*NeXT*/


#ifdef IRIX3	/* Define -DIRIX3 in makefiles/mk.sgi  to enable this.  */

/*
 * Define some routines not present in Irix 3.x:
 *	GLXwinset()	(used by mg libraries, if they're told to)
 *	alloca()	(used in N-D projection, software shading, etc.) 
 */

#include "ooglutil.h"
#include <alloca.h>

GLXwinset() {
	OOGLError(1, "Hey, somebody called GLXwinset()!");
}

/*
	alloca -- (mostly) portable public-domain implementation -- D A Gwyn

	This implementation of the PWB library alloca() function,
	which is used to allocate space off the run-time stack so
	that it is automatically reclaimed upon procedure exit, 
	was inspired by discussions with J. Q. Johnson of Cornell.

	It should work under any C implementation that uses an
	actual procedure stack (as opposed to a linked list of
	frames).  There are some preprocessor constants that can
	be defined when compiling for your specific system, for
	improved efficiency; however, the defaults should be okay.

	The general concept of this implementation is to keep
	track of all alloca()-allocated blocks, and reclaim any
	that are found to be deeper in the stack than the current
	invocation.  This heuristic does not reclaim storage as
	soon as it becomes invalid, but it will do so eventually.

	As a special case, alloca(0) reclaims storage without
	allocating any.  It is a good idea to use alloca(0) in
	your main control loop, etc. to force garbage collection.
*/
#ifndef lint
static char	SCCSid[] = "@(#)alloca.c	1.1";	/* for the "what" utility */
#endif

#ifdef emacs
#include "config.h"
#ifdef static
/* actually, only want this if static is defined as ""
   -- this is for usg, in which emacs must undefine static
   in order to make unexec workable
   */
#ifndef STACK_DIRECTION
you
lose
-- must know STACK_DIRECTION at compile-time
#endif /* STACK_DIRECTION undefined */
#endif /*static*/
#endif /*emacs*/

#ifdef X3J11
typedef void	*pointer;		/* generic pointer type */
#else
typedef char	*pointer;		/* generic pointer type */
#endif

#define	NULL	0			/* null pointer constant */

extern void	free();
extern pointer	malloc();

/*
	Define STACK_DIRECTION if you know the direction of stack
	growth for your system; otherwise it will be automatically
	deduced at run-time.

	STACK_DIRECTION > 0 => grows toward higher addresses
	STACK_DIRECTION < 0 => grows toward lower addresses
	STACK_DIRECTION = 0 => direction of growth unknown
*/

#ifndef STACK_DIRECTION
#define	STACK_DIRECTION	0		/* direction unknown */
#endif

#if STACK_DIRECTION != 0

#define	STACK_DIR	STACK_DIRECTION	/* known at compile-time */

#else	/* STACK_DIRECTION == 0; need run-time code */

static int	stack_dir;		/* 1 or -1 once known */
#define	STACK_DIR	stack_dir

static void
find_stack_direction (/* void */)
{
  static char	*addr = NULL;	/* address of first
				   `dummy', once known */
  auto char	dummy;		/* to get stack address */

  if (addr == NULL)
    {				/* initial entry */
      addr = &dummy;

      find_stack_direction ();	/* recurse once */
    }
  else				/* second entry */
    if (&dummy > addr)
      stack_dir = 1;		/* stack grew upward */
    else
      stack_dir = -1;		/* stack grew downward */
}

#endif	/* STACK_DIRECTION == 0 */

/*
	An "alloca header" is used to:
	(a) chain together all alloca()ed blocks;
	(b) keep track of stack depth.

	It is very important that sizeof(header) agree with malloc()
	alignment chunk size.  The following default should work okay.
*/

#ifndef	ALIGN_SIZE
#define	ALIGN_SIZE	sizeof(double)
#endif

typedef union hdr
{
  char	align[ALIGN_SIZE];	/* to force sizeof(header) */
  struct
    {
      union hdr *next;		/* for chaining headers */
      char *deep;		/* for stack depth measure */
    } h;
} header;

/*
	alloca( size ) returns a pointer to at least `size' bytes of
	storage which will be automatically reclaimed upon exit from
	the procedure that called alloca().  Originally, this space
	was supposed to be taken from the current stack frame of the
	caller, but that method cannot be made to work for some
	implementations of C, for example under Gould's UTX/32.
*/

static header *last_alloca_header = NULL; /* -> last alloca header */

pointer
alloca (size)			/* returns pointer to storage */
     unsigned	size;		/* # bytes to allocate */
{
  auto char	probe;		/* probes stack depth: */
  register char	*depth = &probe;

#if STACK_DIRECTION == 0
  if (STACK_DIR == 0)		/* unknown growth direction */
    find_stack_direction ();
#endif

				/* Reclaim garbage, defined as all alloca()ed storage that
				   was allocated from deeper in the stack than currently. */

  {
    register header	*hp;	/* traverses linked list */

    for (hp = last_alloca_header; hp != NULL;)
      if (STACK_DIR > 0 && hp->h.deep > depth
	  || STACK_DIR < 0 && hp->h.deep < depth)
	{
	  register header	*np = hp->h.next;

	  free ((pointer) hp);	/* collect garbage */

	  hp = np;		/* -> next header */
	}
      else
	break;			/* rest are not deeper */

    last_alloca_header = hp;	/* -> last valid storage */
  }

  if (size == 0)
    return NULL;		/* no allocation required */

  /* Allocate combined header + user data storage. */

  {
    register pointer	new = malloc (sizeof (header) + size);
    /* address of header */

    ((header *)new)->h.next = last_alloca_header;
    ((header *)new)->h.deep = depth;

    last_alloca_header = (header *)new;

    /* User storage begins just after header. */

    return (pointer)((char *)new + sizeof(header));
  }
}
#endif /*IRIX3*/


#if (defined(__hpux) && defined(__GNUC__))

#include <math.h>

	/* HP-UX native cc does have finite() and acosh() in its library,
	 * but gcc (at least our 2.5.8) doesn't find it there.
	 */
int finite(double v) { return (!isinf(v) && !isnan(v)); }

double acosh(double c) {
    /* cosh(x) = (e^x + e^-x)/2
     * u = e^x
     * u^2 - 2c u + 1 = 0
     * u = c +/- sqrt(c^2 - 1)
     */
    return log(c + sqrt(c*c - 1));
}
#endif /*hpux gcc*/

#ifdef SVR4
  /* Solaris lacks bcopy, bzero */

void bcopy(char *src, char *dst, int len) { memcpy(dst, src, len); }
void bzero(char *mem, int len)            { memset(mem, 0, len); }

#endif


#ifdef _WIN32
/* Visual C++ lacks strcasecmp()! */

int strcasecmp(char *a, char *b)
{
  int ca, cb;
  while((ca = *a) && (cb = *b)) {
    if(ca != cb) {
	if(ca >= 'A' && ca <= 'Z') ca += 'a' - 'A';
	if(cb >= 'A' && cb <= 'Z') cb += 'a' - 'A';
	if(ca != cb)
	    return ca - cb;
    }
    a++, b++;
  }
  return 0;
}

int strncasecmp(char *a, char *b, int n)
{
  int ca, cb;
  while(--n >= 0 && (ca = *a) && (cb = *b)) {
    if(ca != cb) {
	if(ca >= 'A' && ca <= 'Z') ca += 'a' - 'A';
	if(cb >= 'A' && cb <= 'Z') cb += 'a' - 'A';
	if(ca != cb)
	    return ca - cb;
    }
    a++, b++;
  }
  return 0;
}

int finite(double v)
{
  return (! (v <= 0.0 || v > 0.0));
}

double acosh(double c) {
    /* cosh(x) = (e^x + e^-x)/2
     * u = e^x
     * u^2 - 2c u + 1 = 0
     * u = c +/- sqrt(c^2 - 1)
     */
    return log(c + sqrt(c*c - 1));
}

FILE *popen(char *name, char *mode) {
    fprintf(stderr, "Call to stub popen(%s, %s)!\n", name, mode);
    return NULL;
}

#endif /*_WIN32*/
