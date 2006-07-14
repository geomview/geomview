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

#include "geom.h"
#include "list.h"
#include "plutil.h"
#include "Clip.h"	/* Note: must include this *after* oogl includes */


Clip clip;

extern int span_vertices(Clip *clip, float *minp, float *maxp);

static char Usage[] = "\
Usage: clip [-v axisx,y,z,...] [-g value] [-l value] [-s nslices[,fraction]]\n\
	[-sph centerx,y,z,...] [-cyl centerx,y,z,...] [-e] [file.oogl]\n\
Reads an OOGL object from file.oogl (or stdin if omitted).\n\
Slices it against a (series of) planes whose normal vectors are given\n\
    by the -v argument (default 1,0,0,0,...).\n\
Given -g, selects portions where <point> dot <vector> > <pvalue>.\n\
Given -l, selects portions where <point> dot <vector> < <nvalue>.\n\
Given both, takes the portion lying between those two values.\n\
With -s, emits a series of slices spaced <spacing> units apart.\n\
With -e, emits just two numbers instead of geometry: range of function-values\n\
    which the object spans.\n\
-sph : Slice against a sphere, not a plane.  -g/-l gives radius.\n\
-cyl x,y,z... : Slice against a cylinder whose axis passes through x,y,z...\n\
	-v gives axis direction, -g/-l radius.\n";

static int parsevector(char *str, float data[], int maxdata)
{
    int i;
    char *p, *op;

    memset(data, 0, maxdata*sizeof(float));
    if(str == NULL)
	return -1;
    for(i = 0, p = str; i < maxdata && *p; i++) {
	op = p;
	data[i] = strtod(p, &p);
	if(p == op)
	    break;
	while(*p == ',' || *p == ' ') p++;
    }
    return i;
}

void
setclipat(Clip *clip, char *pstr, int dim, float *surf, void (*prepfunc)())
{
    float point[MAXDIM];
    float level;
    int n;

    n = parsevector(pstr, point, dim);

    if(n > 1) {
		    /* Given several numbers, consider them
		     * to be a point; choose a level-set
		     * going through that point.
		     */
	setClipPlane(clip, surf, 0);
	level = (*clip->clipfunc)(clip, point);
    } else {
	level = point[0];		/* If just one number, use exactly it */
    }
    setClipPlane(clip, surf, level);
    if(prepfunc)
	(*prepfunc)(clip);
}

/*
 * Clipping functions.
 * The default function, to clip against a plane, is in clip.c.
 */
static float origin[MAXDIM];

/*
 * Clip against a sphere.
 * Uses static variable "origin".
 */
static float sphere(Clip *clip, float *point)
{
  /* clip->surf[0..dim-1] are the coordinates of the center of the sphere. */
  int i;
  float r2 = 0, *s, *p;
  for(i = clip->dim, p = point, s = origin; --i >= 0; p++, s++)
    r2 += (*p - *s) * (*p - *s);
  return r2;
}


/*
 * Clip against a cylinder.
 * Uses both clip->surf, the cylinder's axis vector,
 * and static variable "origin", a point on the cylinder.
 */

static float cylinder(Clip *clip, float *point)
{
  /* clip->surf[0..dim-1] are the direction vector of the cylinder's axis;
   * clip->surf[dim..2*dim-1] are coordinates of a point on the axis.
   */
  int dim = clip->dim;
  int i;
  float r2 = 0, dot = 0, dx, *s, *p, *c;

  for(i=dim, p=point, c=origin, s=clip->surf; --i >= 0; p++, s++, c++)
    dot += (*p - *c) * *s;

  for(i=dim, p=point, c=origin, s=clip->surf; --i >= 0; p++, s++, c++) {
    dx = (*p - *c) - dot * *s;
    r2 += dx * dx;
  }
  return r2;
}


/* Special case for Mark Levi */
float leviarctan(Clip *clip, float *point)
{
  /* On input, point[0] .. point[2] are x,y,z coordinates of the point to test.
   * Assigning w <- point[1] and T <- point[2]
   * we compute x(w,T) on the surface
   *   x = constant - 2T +/- arctan(w)
   * and return the difference between x(w,t) and our point's x.
   *
   * The sign of arctan is taken from clip->surf[0]; this is
   * the number given to the "-v" command-line argument.  So, using
   *  "-a -v 1,21.99"  means x(w,T) = 7pi - 2T - arctan(w), and
   *  "-a -v -1,21.99" means x(w,T) = 7pi - 2T + arctan(w).
   */

    return clip->surf[1] - 2*point[2] - clip->surf[0] * atan(point[1])
		- point[0];
}

/*
 * For some level-set functions we'd rather precompute the square of the
 * given value.  They set prepfunc = squared.
 */
static void squared(Clip *clip)
{
    clip->level *= clip->level;
}

/* For the cylinder, we want to make the axis be a unit vector,
 * so we can easily project it out.  Want to square the level value, too.
 */
static void squared_normalized(Clip *clip)
{
    int dim = clip->dim;
    int i;
    float len = 0, *p;
    for(i = dim, p = clip->surf; --i >= 0; p++)
	len += *p * *p;
    len = sqrt(len);
    if(len > 0) {
	for(i = dim, p = clip->surf; --i >= 0; p++)
	    *p /= len;
    }

    /* And square the radius */
    clip->level *= clip->level;
}


int main(int argc, char *argv[])
{
    Geom *g, *clipped;
    float surf[MAXDIM];
    float (*func)() = NULL;
    void (*prepfunc)() = NULL;
    char *lestr = NULL, *gestr = NULL;
    char *type;
    int slices = 0;
    int nonlinear = 0;
    int extent = 0;
    float slicev[2];
    IOBFILE *inf = iobfileopen(stdin);


    if(argc <= 1) {
	fputs(Usage, stderr);
	exit(1);
    }

    clip_init(&clip);

    memset(surf, 0, sizeof(surf));
    surf[0] = 1;

    while(argc > 1) {
	if(strncmp(argv[1], "-g", 2) == 0 && argc>2) {
	    if(gestr != NULL) {
		fprintf(stderr, "clip: can only specify one -g option.\n");
		exit(1);
	    }
	    gestr = argv[2];
	    argc -= 2, argv += 2;
	} else if(strncmp(argv[1], "-l", 2) == 0 && argc>2) {
	    if(lestr != NULL) {
		fprintf(stderr, "clip: can only specify one -l option.\n");
		exit(1);
	    }
	    lestr = argv[2];
	    argc -= 2, argv += 2;

	} else if(strcmp(argv[1], "-v") == 0 && argc > 2) {
	    if(parsevector(argv[2], surf, MAXDIM) <= 0) {
		fprintf(stderr, "-v: expected x,y,z,... surface parameters\n\
(plane normal or cylinder axis)\n");
		exit(1);
	    }
	    argc -= 2, argv += 2;

	} else if(strcmp(argv[1], "-e") == 0) {
	    extent = 1;
	    argc--, argv++;

	} else if(strcmp(argv[1], "-a") == 0) {
	    func = leviarctan;
	    nonlinear = 1;
	    argc--, argv++;

	} else if(strncmp(argv[1], "-sph", 4) == 0) {
	    func = sphere;
	    prepfunc = squared;
	    nonlinear = 1;
	    if(parsevector(argv[2], origin, COUNT(origin)) <= 0) {
		fprintf(stderr, "clip -sph: expected x,y,z,... of sphere's center.\n");
		exit(1);
	    }
	    argc -= 2, argv += 2;

	} else if(strncmp(argv[1], "-cyl", 4) == 0) {
	    func = cylinder;
	    prepfunc = squared_normalized;
	    nonlinear = 1;
	    if(parsevector(argv[2], origin, COUNT(origin)) <= 0) {
		fprintf(stderr, "clip -cyl: expected x,y,z,... of a point on the cylinder's axis\n");
		exit(1);
	    }
	    argc -= 2, argv += 2;

	} else if(strcmp(argv[1], "-s") == 0) {
	    slices = parsevector(argv[2], slicev, 2);
	    if(slices <= 0 || slicev[0] <= 1) {
		fprintf(stderr, "clip -s: expected -s nribbons  or  nribbons,gap-fraction; must have >= 2 ribbons\n");
		exit(1);
	    } else if(slices == 1)
		slicev[1] = .5;
	    argc -= 2, argv += 2;

	} else if(argv[1][0] == '-') {
	    fprintf(stderr, "Unknown option '%s'.  Run 'clip' with no arguments for help.\n",
		argv[1]);
	    exit(1);
	} else {
	    break;
	}
    }

    if(lestr == NULL && gestr == NULL && slices == 0 && extent == 0) {
	fprintf(stderr, "clip: must specify at least one of -l or -g or -s or -e options.\n");
	exit(2);
    }

    if(argc > 2) {
	fprintf(stderr, "clip: can only handle one object\n");
	exit(2);
    }

    if(argc > 1 && strcmp(argv[1], "-") != 0) {
	g = GeomLoad(argv[1]);
    } else {
	argv[1] = "standard input";
	g = GeomFLoad(inf, "stdin");
    }

    type = GeomName(g);
    if(type!=NULL && strcmp(type, "polylist") && strcmp(type, "npolylist")) {
	Geom *newg = AnyToPL(g, TM3_IDENTITY);
	GeomDelete(g);
	g = newg;
    }

    if(g == NULL) {
	fprintf(stderr, "clip: Error loading OOGL object from %s\n", argv[1]);
	exit(1);
    }

    if(func != NULL)
	clip.clipfunc = func;
    clip.nonlinear = nonlinear;

    if(lestr != NULL) {
	setGeom(&clip, g);
	setclipat(&clip, lestr, clip.dim, surf, prepfunc);
	setSide(&clip, CLIP_LE);
	do_clip(&clip);
	clipped = getGeom(&clip);
	GeomDelete(g);
	g = clipped;
    }

    if(gestr != NULL) {
	setGeom(&clip, g);
	setclipat(&clip, gestr, clip.dim, surf, prepfunc);
	setSide(&clip, CLIP_GE);
	do_clip(&clip);
	clipped = getGeom(&clip);
	GeomDelete(g);
	g = clipped;
    }


    if(slices > 0) {
	float min, max, v, step;
	int i;
	Geom *piece, *whole = NULL;
	char lim[64];

	setGeom(&clip, g);
	setclipat(&clip, "0", clip.dim, surf, prepfunc);
	if(span_vertices(&clip, &min, &max) != 0) {
	    /* Nonempty */
	    /* Interpolate linearly in real space -- invert prepfunc */
	    if(prepfunc == squared || prepfunc == squared_normalized) {
		min = sqrt(min);
		max = sqrt(max);
	    }
	    step = (max - min) / (slicev[0] + slicev[1] - 1);
	    sprintf(lim, "%g", min + slicev[1]*step);
	    setclipat(&clip, lim, clip.dim, surf, prepfunc);
	    setSide(&clip, CLIP_LE);
	    do_clip(&clip);
	    whole = GeomCreate("list", CR_GEOM, getGeom(&clip), CR_END);
	    for(i = 1, v = min + step; i < slicev[0]; i++, v += step) {
		setGeom(&clip, g);
		sprintf(lim, "%g", v + slicev[1]*step);
		setclipat(&clip, lim, clip.dim, surf, prepfunc);
		setSide(&clip, CLIP_LE);
		do_clip(&clip);
		piece = getGeom(&clip);

		setGeom(&clip, piece);
		sprintf(lim, "%g", v);
		setclipat(&clip, lim, clip.dim, surf, prepfunc);
		setSide(&clip, CLIP_GE);
		do_clip(&clip);
		piece = getGeom(&clip);
		if(piece)
		    whole = ListAppend(whole, piece);
	    }
	}
	g = whole;
    }

    if(extent != 0) {
	float min, max;
	setGeom(&clip, g);
	setclipat(&clip, "0", clip.dim, surf, prepfunc);
	if(span_vertices(&clip, &min, &max) != 0) {
	    if(prepfunc == squared || prepfunc == squared_normalized) {
		min = sqrt(min);
		max = sqrt(max);
	    }
	}
	printf("%g %g\n", min, max);
    } else {
	GeomFSave(g, stdout, "stdout");
    }
    exit(0);
}
