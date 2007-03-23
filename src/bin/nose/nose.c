/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
 *
 * This file is part of Geomview.
 * 
 * Geomview is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * Geomview is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Geomview; see the file COPYING.  If not, write to the
 * Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA,
 * or visit http://www.gnu.org.
 */

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Geometry Technologies, Inc.";

/* (picked "name" (gx gy gz) (vx vy vz) (x1 y1 z1 x2 y2 z2)) */

#include <stdio.h>
#include "lisp.h"
#include "pickfunc.h"
#include "3d.h"
#include "handleP.h"
#include "ooglutil.h"

void deflittlebox(FILE *fp, float size);
void norm(FILE *fp);

int verbose = 0;

handle_pick(FILE *fp, int picked, HPoint3 *gotten,
	    int vert, HPoint3 *v, int edge, HPoint3 e[])
{
  static int first = 1;
  HPoint3 got, e0, e1;

  HPt3Dehomogenize(&e[0], &e0);
  HPt3Dehomogenize(&e[1], &e1);
  HPt3Dehomogenize(gotten, &got);
  fprintf(fp, "(progn\n");
  if (!picked) {
    fprintf(fp,"(geometry \"pick\" { LIST } )\n");
  } else {
    fprintf(fp,"(xform-set pick { 1 0 0 0  0 1 0 0  0 0 1 0  %g %g %g 1 })\n",
	got.x, got.y, got.z);
    fprintf(fp,"(geometry \"pick\"\n");
    if (vert) fprintf(fp, "{ appearance { material { diffuse 1 0 1 } }\n");
    else fprintf(fp, "{ appearance { material { diffuse 1 1 0 } }\n");
    fprintf(fp, "  { LIST { :littlebox }\n");

    if (edge && !vert) {
	e0.x -= got.x; e0.y -= got.y; e0.z -= got.z;
	e1.x -= got.x; e1.y -= got.y; e1.z -= got.z;
	fprintf(fp,"{ appearance { material { diffuse 0 1 1 } }\n\
  LIST\n\
   { INST transform 1 0 0 0 0 1 0 0 0 0 1 0 %f %f %f 1 geom :littlebox }\n\
   { INST transform 1 0 0 0 0 1 0 0 0 0 1 0 %f %f %f 1 geom :littlebox }\n\
   { VECT\n\
	  1 2 1\n\
	  2\n\
	  1\n\
	  %f %f %f\n\
	  %f %f %f\n\
	  1 1 0 1\n\
   }\n\
  }\n",
	     e0.x, e0.y, e0.z,
	     e1.x, e1.y, e1.z,
	     e0.x, e0.y, e0.z,
	     e1.x, e1.y, e1.z);
    }
    fprintf(fp,"    }\n  }\n)\n");
  }
  if (first) {
    fprintf(fp, "(pickable \"pick\" no)\n");
    first = 0;
  }
  fprintf(fp, ")\n");
  fflush(fp);
}

DEFPICKFUNC("(pick COORDSYS GEOMID G V E F P VI EI FI)",
	    coordsys,
	    id,
	    point, pn,
	    vertex, vn,
	    edge, en,
	    face, fn, 10,
	    ppath, ppn, 50,
	    vi,
	    ei, ein,
	    fi,
{	    
  int picked = pn > 0;
  handle_pick(stdout, picked, &point, vn>0, &vertex, en>0, edge);
  if (verbose)
    handle_pick(stderr, picked, &point, vn>0, &vertex, en>0, edge);
  return Lt;
})


init()
{
  LInit();
  LDefun("pick", Lpick, Hpick);
}

pickability(FILE *fp)
{
  fprintf(fp, "(interest (pick world * * * * nil nil nil nil nil))\n");
  fflush(fp);
}

main()
{
  Lake *lake;
  LObject *lit, *val;
  extern char *getenv();

  verbose = (getenv("VERBOSE_NOSE") != NULL);

  fprintf(stdout, "(progn\n");
  init();
  deflittlebox(stdout, 0.04);
  if (verbose) deflittlebox(stderr, 0.04);

  pickability(stdout);
  if (verbose) pickability(stderr);

  norm(stdout);
  if (verbose) norm(stderr);

  fprintf(stdout, ")\n");
  fflush(stdout);

  lake = LakeDefine(stdin, stdout, NULL);
  while (!feof(stdin)) {
    lit = LSexpr(lake);
    val = LEval(lit);
    LFree(lit);
    LFree(val);
  }
}

void
norm(FILE *fp)
{
  fprintf(fp, "(geometry \"pick\" { LIST } )\n");
  fprintf(fp, "(normalization \"pick\" none)\n");
  fprintf(fp, "(bbox-draw \"pick\" off)\n");
  fflush(fp);
}


void
deflittlebox(FILE *fp, float size)
{
  fprintf(fp,"\
  (read geometry { define littlebox {\n\
	INST\n\
	  transform %f 0 0 0\n\
		    0 %f 0 0\n\
		    0 0 %f 0\n\
		    0 0 0  1\n\
	  geom\n\
	  OFF\n\
	  8 6 12\n\
\n\
	  -.5 -.5 -.5	# 0   \n\
	   .5 -.5 -.5	# 1   \n\
	   .5  .5 -.5	# 2   \n\
	  -.5  .5 -.5	# 3   \n\
	  -.5 -.5  .5	# 4   \n\
	   .5 -.5  .5	# 5   \n\
	   .5  .5  .5	# 6   \n\
	  -.5  .5  .5	# 7   \n\
\n\
	  4 0 1 2 3\n\
	  4 4 5 6 7\n\
	  4 2 3 7 6\n\
	  4 0 1 5 4\n\
	  4 0 4 7 3\n\
	  4 1 2 6 5\n\
	  }})\n", size, size, size);
}
