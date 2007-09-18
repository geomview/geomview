/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
 * Copyright (C) 2007 Claus-Justus Heine
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

/* (picked "name" (gx gy gz) (vx vy vz) (x1 y1 z1 x2 y2 z2)) */

#include <stdio.h>
#include <stdlib.h>
#include "lisp.h"
#include "pickfunc.h"
#include "3d.h"
#include "handleP.h"
#include "ooglutil.h"

void deflittlebox(FILE *fp, float size);
void norm(FILE *fp);

int verbose = 0;

void handle_pick(FILE *fp, int picked, HPoint3 *gotten,
		 bool vert, HPoint3 *v, bool edge, HPoint3 e[])
{
  static bool first = true;
  HPoint3 got;

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
      HPoint3 e0, e1;

      HPt3Dehomogenize(&e[0], &e0);
      HPt3Dehomogenize(&e[1], &e1);

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
    first = false;
  }
  fprintf(fp, ")\n");
  fflush(fp);
}

void handle_ND_pick(FILE *fp, int picked, HPtNCoord *gotten, int dim,
		    bool vert, HPtNCoord *v, bool edge, HPtNCoord *e)
{
  static bool first = true;
  HPointN got, e0, e1;
  TransformN *trans;
  int i;

  got.flags = e0.flags = e1.flags = 0;
  got.dim = e0.dim = e1.dim = dim;
  got.v = gotten;
  e0.v  = e;
  e1.v  = e+dim;

  HPtNDehomogenize(&got, &got);
  fprintf(fp, "(progn\n");
  if (!picked) {
    fprintf(fp,"(geometry \"pick\" { LIST } )\n");
  } else {
    trans = TmNTranslateOrigin(NULL, &got);
    fprintf(fp,"(ND-xform-set pick ");
    TmNPrint(fp, trans);
    fprintf(fp, ")\n");
    fprintf(fp,"(geometry \"pick\"\n");
    if (vert) fprintf(fp, "{ appearance { material { diffuse 1 0 1 } }\n");
    else fprintf(fp, "{ appearance { material { diffuse 1 1 0 } }\n");
    fprintf(fp, "  { LIST { :littlebox }\n");

    if (edge && !vert) {
      HPtNDehomogenize(&e0, &e0);
      HPtNDehomogenize(&e1, &e1);
      HPtNComb(-1.0, &got, 1.0, &e0, &e0);
      HPtNComb(-1.0, &got, 1.0, &e1, &e1);
      fprintf(fp,
	      "{\n"
	      "  appearance { material { diffuse 0 1 1 } }\n"
	      "  LIST {\n"
	      "    { INST\n");
      trans = TmNTranslateOrigin(NULL, &e0);
      TmNPrint(fp, trans);
      fprintf(fp,
	      "      geom :littlebox\n"
	      "    }\n"
	      "    { INST\n");
      trans = TmNTranslateOrigin(NULL, &e1);
      TmNPrint(fp, trans);
      fprintf(fp,
	      "      geom :littlebox\n"
	      "    }\n"
	      "    { nSKEL %d\n"
	      "      2 1\n", dim-1);
      fprintf(fp,
	      "     ");
      for (i = 1; i < dim; i++) {
	fprintf(fp, " %10.7f", e0.v[i]);
      }
      fprintf(fp,
	      "\n     ");
      for (i = 1; i < dim; i++) {
	fprintf(fp, " %10.7f", e1.v[i]);
      }
      fprintf(fp,
	      "\n      2 0 1 1 1 0 1\n"
	      "    }\n"
	      "  }\n"
	      "}\n");
    }
    fprintf(fp, "}})\n");
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
	    face, fn,
	    ppath, ppn,
	    vi,
	    ei, ein,
	    fi,
{	    
  int picked = pn > 0;

  handle_pick(stdout, picked, &point, vn>0, &vertex, en>0, edge);
  if (verbose)
    handle_pick(stderr, picked, &point, vn>0, &vertex, en>0, edge);
  return Lt;
},
{	      
  int picked = pn > 0;

  handle_ND_pick(stdout, picked, point, pn, vn>0, vertex, en>0, edge);
  if (verbose)
    handle_ND_pick(stderr, picked, point, pn, vn>0, vertex, en>0, edge);
  return Lt;
})

void init(void)
{
  LInit();
  LDefun("pick", Lpick, Hpick);
}

void pickability(FILE *fp)
{
  fprintf(fp, "(interest (pick world * * * * nil nil nil nil nil))\n");
  fflush(fp);
}

int main(int argc, char *argv[])
{
  Lake *lake;
  LObject *lit, *val;
  IOBFILE *fromgv;

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

  lake = LakeDefine(fromgv = iobfileopen(stdin), stdout, NULL);
  while (!iobfeof(fromgv)) {
    lit = LSexpr(lake);
    val = LEval(lit);
    LFree(lit);
    LFree(val);
  }

  return 0;
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

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
