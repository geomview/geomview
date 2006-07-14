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

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

/*
 * anytooff.main.c
 * author:  Celeste Fowler
 * date:  June 12, 1992
 */

#include <stdio.h>
#include <stdlib.h>
#include "geom.h"
#include "appearance.h"
#include "transform.h"
#include "transformn.h"
#include "plutil.h"

static char Usage[] = "\
Usage: %s [-n] [-nd] [-3d] [-non] [-noc] [filename ...]\n\
Converts OOGL file(s) to OFF (or nOFF) format.  Writes to standard output.\n\
Reads from stdin if no file(s) specified.\n\
Produces the union of all file(s) if several are specified.  Options:\n\
  -n Compute normals.\n\
  -nd allow emitting N-dimensional nOFF object if input is N-D.\n\
  -3d only emit OFF objects; project down to 3-space\n\
  -non No normals.  Omit them even if input objects had them.\n\
  -noc No colors.   Omit them even if input objects had them.\n";

int main(int argc, char *argv[]) {
  Geom *g;
  PLData *pd;
  int i;
  int want = 0, unwant = 0;
  char *prog = argv[0];

  while(argc > 1 && (argv[1][0] == '-') && argv[1][1] != '\0') {
    char *arg = &argv[1][1];
    int len = strlen(arg);
    int bit = 0;
    int wanted = 1;
    if((strncmp(arg, "no", 2) == 0 && arg[2] != 'r')) {
	arg += 2;
	len -= 2;
	wanted = 0;
    }
    if(!strncmp(arg, "normals", len)) {
	bit = PL_N;
    } else if(!strcasecmp(arg, "nd")) {
	bit = PL_ND;
    } else if(!strcasecmp(arg, "3d")) {
	bit = PL_ND, wanted = !wanted;
    } else if(!strncasecmp(arg, "colors", len)) {
	bit = PL_C;
    } else {
	fprintf(stderr, "%s: unknown option %s\n", prog, argv[1]);
	fprintf(stderr, Usage, prog);
	exit(1);
    }
    if(wanted) {
	want |= bit;
	unwant &= ~bit;
    } else {
	want &= ~bit;
	unwant |= bit;
    }
    argc--, argv++;
  }

  if(argc <= 1) {
    argc = 2;
    argv[1] = "-";
  }

  pd = NULL;
  for(i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-") == 0) {
      g = GeomFLoad(iobfileopen(stdin), "standard input");
    } else {
      g = GeomLoad(argv[i]);
    }
    pd = AnyGeomToPLData(g, TM_IDENTITY, NULL, NULL, pd);
  }

  /* Get the data.
   * If they explicitly asked for normals, force them to be recomputed
   * by including PL_N in both "unwant" and "want".
   */
  g = PLDataToGeom(pd, want, unwant | (want & PL_N) );
  GeomFSave(g, stdout, NULL);
  return g != NULL && !ferror(stdout) ? 0 : 1;
}
