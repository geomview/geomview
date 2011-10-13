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

#if defined(unix) || defined(__unix) || defined(__unix__) /* Don't try to compile for Windows */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ooglutil.h"
#include <signal.h>

#if POPEN_ACCEPTS_RB
# define POPEN_RB "rb"
#else
# define POPEN_RB "r"
#endif

#define FBUFSIZ 1024
#define INITSIZ 10

/*-----------------------------------------------------------------------
 * Function:	*ooglglob
 * Description:	C-shell globbing
 * Args:	*s: the string to glob
 * Returns:	pointer to array of pointers to expanded strings; ends
 *		   with NULL
 * Author:	mbp
 * Date:	Tue Aug 25 12:42:21 1992
 * Notes:	Invokes a subshell (/bin/csh) to do the work.
 *		Caller should free the returned value by
 *		first calling ooglblkfree() then free():
 *
 *		char **g;
 *		g = ooglglob(string);
 *		...
 *		ooglblkfree(g);
 *		free(g);
 */
char **ooglglob(char *s)
{
  FILE *fp;
  char cmd[FBUFSIZ];
  vvec vp;
  char *c;
#ifdef SIGCHLD
	/* Avoid NeXT pclose() bug: don't catch subprocess' SIGCHLD */
  void (*oldsigchld)() = signal(SIGCHLD, SIG_DFL);
#endif

  sprintf(cmd, GLOB_SHELL"\"echo %s\" 2>&1", s);
  fp = popen(cmd, POPEN_RB);
  
  if (fp == NULL) {
    OOGLError(1, "Could not popen(\"%s\", \"r\"): %s\n", cmd, sperror());
    return NULL;
  }

  VVINIT(vp, char *, INITSIZ);
  while (!feof(fp))
    if ((c=ftoken(fp, 2)) != NULL) *VVAPPEND(vp,char*) = strdup(c);
  *VVAPPEND(vp,char*) = NULL;
  vvtrim(&vp);

  pclose(fp);
#ifdef SIGCHLD
  signal(SIGCHLD, oldsigchld);
#endif
  return VVEC(vp,char*);
}

void
ooglblkfree(av0)
     char **av0;
{
  char **av = av0;
  while (*av)
    free(*av++);
}

#endif /*unix*/
