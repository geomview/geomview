/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2007 Claus-Justs Heine
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
 * ROUTINE DESCRIPTION:  Create and load a vert object from a file.
 * 
 */

#include <string.h>
#include "skelP.h"

Skel *SkelFLoad(IOBFILE *file, char *fname)
{
  Skel *s;
  int dimn = 3, pdim = 4;
  bool nd = false, vc = false, binary = false;
  char *token;
  float *vp;
  Skline *lp;
  int i, k;
  ColorA *cp;
  static ColorA black = { 0,0,0,1 };
  vvec colors;
  vvec verts;

  if (file == NULL) return NULL;

  token = GeomToken(file);

  if (*token == 'C') {
    vc = true;
    token++;
  }
  if(*token == '4') {		/* Really means "homogeneous coords" */
    dimn = 4;
    token++;
  }
  if(*token == 'n') {
    nd = true;
    token++;
  }
  if (strcmp(token, "SKEL"))
    return NULL;

  if(nd) {
    if(iobfgetni(file, 1, &pdim, 0) <= 0) {
      OOGLSyntax(file,
		 "Reading nSKEL from \"%s\": Expected dimension", fname);
      return NULL;
    }
    if (pdim < 4) {
      OOGLSyntax(file, "Reading nSKEL from \"%s\": dimension %d < 4",
		 fname, pdim);
      return NULL;
    }
    ++pdim;
  }
  if(iobfnextc(file, 1) == 'B' && iobfexpectstr(file, "BINARY") == 0) {
    binary = true;
    if(iobfnextc(file, 1) != '\n')	/* Expect \n after BINARY */
      return NULL;
    (void) iobfgetc(file);		/* Toss \n */
  }

  s = OOGLNewE(Skel, "SkelFLoad: skel");

  GGeomInit(s, SkelMethods(), SKELMAGIC, NULL);
  s->geomflags = (dimn == 4) ? VERT_4D : 0;
  if (vc) {
    s->geomflags |= VERT_C;
  }
  s->pdim = pdim;
  s->p = NULL;
  s->l = NULL;
  s->c = NULL;
  s->vc = NULL;
  s->vi = NULL;
  s->nvi = s->nc = 0;

  if(iobfgetni(file, 1, &s->nvert, binary) <= 0 ||
     iobfgetni(file, 1, &s->nlines, binary) <= 0) {
    OOGLSyntax(file, "Reading SKEL from \"%s\": expected nvertices, nlines");
    goto bogus;
  }

  s->p = OOGLNewNE(float, s->nvert * s->pdim, "SKEL vertices");
  s->l = OOGLNewNE(Skline, s->nlines, "SKEL lines");
  if (vc) {
    s->vc = OOGLNewNE(ColorA, s->nvert, "SKEL vertex colors");
  }
  VVINIT(colors, ColorA, 10);
  VVINIT(verts, int, 40);

  if (pdim == 4) {
    for (i = 0, vp = s->p; i < s->nvert; i++, vp += 4) {
      vp[3] = 1.0; /* homogeneous component */
      if (iobfgetnf(file, dimn, vp, binary) < dimn) {
	OOGLSyntax(file,
		   "Reading SKEL from \"%s\": error reading vertex %d of %d",
		   fname, i, s->nvert);
	goto bogus;
      }
      if (vc) {
	if (iobfgetnf(file, 4, (float *)&s->vc[i], binary) < 4) {
	  OOGLSyntax(file,
		     "Reading SKEL from \"%s\": "
		     "error reading vertex color %d of %d",
		     fname, i, s->nvert);
	  goto bogus;
	}
      }
    }
  } else {
    dimn = dimn == 4 ? s->pdim : s->pdim - 1;
    for (i = 0, vp = s->p; i < s->nvert; i++, vp += s->pdim) {
      vp[0] = 1.0; /* homogeneous component */
      if(iobfgetnf(file, dimn, vp + (pdim == dimn+1), binary) < dimn) {
	OOGLSyntax(file,
		   "Reading SKEL from \"%s\": error reading vertex %d of %d",
		   fname, i, s->nvert);
	goto bogus;
      }
      if (vc) {
	if (iobfgetnf(file, 4, (float *)&s->vc[i], binary) < 4) {
	  OOGLSyntax(file,
		     "Reading SKEL from \"%s\": "
		     "error reading vertex color %d of %d",
		     fname, i, s->nvert);
	  goto bogus;
	}
      }
    }
  }

  for (i = 0, lp = s->l; i < s->nlines; i++, lp++) {
    /* Read one polyline
     */
    if(iobfgetni(file, 1, &lp->nv, binary) <= 0 || lp->nv <= 0) {
      OOGLSyntax(file,
		 "Reading SKEL from \"%s\": "
		 "expected vertex count on polyline %d of %d",
		 fname, i, s->nlines);
      goto bogus;
    }
    /* Add its vertex-indices to our global list
     */
    lp->v0 = k = VVCOUNT(verts);
    VVCOUNT(verts) += lp->nv;
    vvneeds(&verts, VVCOUNT(verts));
    if(iobfgetni(file, lp->nv, VVEC(verts, int) + k, binary) < lp->nv) {
      OOGLSyntax(file, "Reading SKEL from \"%s\": expected %d vertex indices on polyline %d of %d", fname, lp->nv, i, s->nlines);
      goto bogus;
    }

    /* Check for color (sigh) by testing for end of line. */
    vvneeds(&colors, VVCOUNT(colors)+1);
    cp = &VVEC(colors, ColorA)[VVCOUNT(colors)];
    *cp = black;
    /* Pick up the color, if any.
     * In ASCII format, take whatever's left before end-of-line
     */
    if (!binary) {
      for(k = 0; k < 4 && iobfnextc(file, 1) != '\n'; k++) {
	if(iobfgetnf(file, 1, ((float *)cp)+k, 0) < 1) {
	  OOGLSyntax(file, "Reading SKEL from \"%s\": polyline %d of %d: expected color component",
		     fname, i, s->nlines);
	  goto bogus;
	}
      }
    } else {
      int ncol;

      if (iobfgetni(file, 1, &ncol, 1) <= 0
	  || iobfgetnf(file, ncol, (float *)cp, 1) < ncol) {
	OOGLSyntax(file, "Reading binary SKEL from \"%s\": "
		   "polyline %d of %d: expected color component",
		   fname, i, s->nlines);
	goto bogus;
      }
      k = ncol;
    }

    lp->c0 = VVCOUNT(colors);
    if (k > 0) {
      lp->nc = 1;
      VVCOUNT(colors)++;
      s->geomflags |= FACET_C;
    } else {
      lp->nc = 0;
    }
  }

  /* Note we don't vvfree() the verts and colors arrays --
   * we simply use the pointers to data they've already malloced, if any.
   */
  s->nvi = VVCOUNT(verts);
  s->vi = VVEC(verts, int);
  s->nc = VVCOUNT(colors);
  s->c = VVEC(colors, ColorA);
  return s;

 bogus:
  GeomDelete((Geom *)s);
  return NULL;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
