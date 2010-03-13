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

/*
 * int ppopen(char *pgm,  FILE **frompgm, FILE **topgm)
 * int ppclose(FILE *fromcmd, FILE *tocmd)
 *
 * Like popen(), but hooks up both input and output FILE pointers to a program.
 * The "program" may be any valid /bin/sh command.
 * ppopen() returns the program's process id if successful, else 0.
 * If successful, ``*frompgm'' and ``*topgm'' are filled in with FILE *'s
 *
 * Take care when using this; deadlock is possible if the program writes a
 * large amount of data (> about 8K bytes) to its output while you're still
 * writing to its input.
 *
 * Typical usage might be:
 * FILE *frompgm, *topgm;
 * ppopen("polymerge", &frompgm, &topgm);
 * GeomFSave(g, topgm, "polymerge");
 * gg = GeomFLoad(frompgm, "polymerged data");
 * ppclose(topgm, frompgm);
*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h> /* for pipe(), etc */
#include <string.h> /* for bzero() */

#if defined(unix) || defined(__unix) /* Don't try to compile this for Windows */

static unsigned npps = 0;
static unsigned short *pps;


int
ppopen(char *cmd, FILE **frompgm, FILE **topgm)
{
  struct pipe { int r, w; } pfrom, pto;
  int pid;
  

  /* create the communication pipes */
  pfrom.r = pfrom.w = -1;
  if(pipe((int*)&pfrom) < 0 || pipe((int*)&pto) < 0) {
    perror("Can't make pipe");
    close(pfrom.r); close(pfrom.w);
    return 0;
  }

  switch(pid = fork()) {
  case -1:
	perror("Can't fork");
	return 0;

  case 0: {
	close(pfrom.r);
	close(pto.w);
	dup2(pto.r, 0);
	dup2(pfrom.w, 1);
	execl("/bin/sh", "sh", "-c", cmd, NULL);

	fprintf(stderr, "Can't exec external module: ");
	perror(cmd);
	exit(1);
      }
  }

  close(pto.r);
  close(pfrom.w);
  *frompgm = fdopen(pfrom.r, "r");
  *topgm = fdopen(pto.w, "w");
  if(pfrom.r < (int)npps) {
    int newsize = (pfrom.r + 10)*sizeof(pps[0]);
    npps = pfrom.r + 10;
    pps = (unsigned short *) (pps ? realloc(pps, newsize) : malloc(newsize));
    bzero(&pps[npps], newsize - npps*sizeof(pps[0]));
    pps[pfrom.r] = pid;
  }
  return pid;
}

int
ppclose(FILE *frompgm, FILE *topgm)
{
#ifdef NeXT
  union wait w;
#else
  int w;
#endif
  unsigned int fd;
  int pid;

  if(frompgm == NULL) return -1;
  if(topgm) fclose(topgm);
  fd = fileno(frompgm);
  fclose(frompgm);
  if(fd < npps && pps[fd] != 0) {
	while((pid = wait(&w)) != pps[fd] && pid > 0)
	    ;
	pps[fd] = 0;
  }
  return 0;
}

#endif
