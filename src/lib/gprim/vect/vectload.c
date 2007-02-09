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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

 /*
  * Geometry Routines
  * 
  * Geometry Supercomputer Project
  * 
  * ROUTINE DESCRIPTION:  Create and load a vect object from a file.
  * 
  */

#include "vectP.h"

Vect *
VectFLoad(IOBFILE *file, char *fname)
{
    Vect *v;
    int	 binary = 0, dimn = 3;
    char *token;
    int i;
    static char badvert[] = "Reading VECT from \"%s\": bad %dth vertex (of %d)";

    if (file == NULL) return NULL;

    token = GeomToken(file);
    if(*token == '4') {
	dimn = 4;
	token++;
    }

    if(strcmp(token, "VECT"))
	return NULL;

    if(iobfnextc(file, 1) == 'B') {
	if(iobfexpectstr(file, "BINARY"))
	    return NULL;
	binary = 1;
	if(iobfnextc(file, 1) == '\n')
	    (void) iobfgetc(file);		/* Toss \n */
    }

    v = OOGLNewE(Vect, "VectFLoad: Vect");

    GGeomInit(v, VectMethods(), VECTMAGIC, NULL);
    v->geomflags = (dimn == 4) ? VERT_4D : 0;
    v->geomflags = 0;
    v->vnvert = NULL;
    v->vncolor = NULL;
    v->p = NULL;
    v->c = NULL;

    if(iobfgetni(file, 1, &v->nvec, binary) <= 0 ||
       iobfgetni(file, 1, &v->nvert, binary) <= 0 ||
       iobfgetni(file, 1, &v->ncolor, binary) <= 0) {
	OOGLSyntax(file, "Reading VECT from \"%s\": can't read header counts", fname);
	goto bogus;
    }
    if(!vSane(v)) {
	OOGLSyntax(file,
	 "Reading VECT from \"%s\": inconsistent VECT header counts %d %d %d",
		fname, v->nvec, v->nvert, v->ncolor);
	goto bogus;
    }

    v->vnvert = OOGLNewNE(short, 2*v->nvec, "VECT nvec counts");
    v->p = OOGLNewNE(HPoint3, v->nvert, "VECT vertices");
    v->c = OOGLNewNE(ColorA, (v->ncolor>0) ? v->ncolor : 1, "VECT colors");

    v->vncolor = v->vnvert + v->nvec;

    if((i = iobfgetns(file, v->nvec, v->vnvert, binary)) < v->nvec) {
	OOGLSyntax(file,
	 "Reading VECT from \"%s\": bad vertex count in %d'th polyline (of %d)",
		fname, i, v->nvec);
	goto bogus; 
    }
    if((i = iobfgetns(file, v->nvec, v->vncolor, binary)) < v->nvec) {
	OOGLSyntax(file,
	 "Reading VECT from \"%s\": bad color count in %d'th polyline (of %d)",
		fname, i, v->nvec);
	goto bogus;
    }
    /* if the points are 3D points, we have to convert them to the native
	4D data structure */
    if (dimn == 3)	{
	HPoint3 *p;

	for(i = v->nvert, p = v->p; --i >= 0; p++) {
	    if (iobfgetnf(file, 3, (float *)p, binary) < 3) {
		OOGLSyntax(file, badvert, fname, v->nvert - i, v->nvert);
		goto bogus;
	    }
	    p->w = 1;
	  }
	}
    else {
	i = iobfgetnf(file, 4*v->nvert, (float *)v->p, binary);
	if(i < 4*v->nvert) {
	    OOGLSyntax(file, badvert, fname, i/4, v->nvert);
	    goto bogus;
	}
      }
    if (v->ncolor > 0 &&
	(i = iobfgetnf(file, 4*v->ncolor, (float *)v->c, binary)) < 4*v->ncolor) {
	    OOGLSyntax(file, "Reading VECT from \"%s\": bad %dth color (of %d)",
		fname, i/4, v->ncolor);
	    goto bogus;
	}

    if(!VectSane(v)) {
	OOGLError(0, "Reading VECT from \"%s\": VECT polyline/color counts inconsistent with header", fname);
	goto bogus;
    }

    return v;

  bogus:
    GeomDelete((Geom *)v);
    return NULL;
}
