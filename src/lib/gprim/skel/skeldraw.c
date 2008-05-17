/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2006-2007 Claus-Justus Heine
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
 * ROUTINE DESCRIPTION:
 *	Draw a Vect (collection of vectors).
 * 
 */

#include "mgP.h"
#include "skelP.h"
#include "hpointn.h"
#include "transformn.h"
#include "cmodel.h"

#include <stdlib.h>
#ifndef alloca
#include <alloca.h>
#endif

#define MAXPLINE 32	/* Temp buffer size.  Must be a power of 2! */

static void
draw_projected_skel(mgNDctx *NDctx, Skel *s, int flags,
		    int penultimate, int hascolor)
{
  HPointN *h;
  float *op;
  HPoint3 *np, *newp;
  Skline *l;
  ColorA *c, *lastcolor, *newc;
  int i, colored = 0;
  HPoint3 tv[MAXPLINE];
  ColorA tc[MAXPLINE];
  mgNDmapfunc mapHPtN = NDctx->mapHPtN;
  Appearance *ap = &_mgc->astk->ap;

#if !NO_ALLOCA
  newp = (HPoint3 *)alloca(s->nvert*sizeof(HPoint3));
#else
  newp = OOGLNewNE(HPoint3, s->nvert, "projected SKEL points");
#endif

  h = HPtNCreate(s->pdim, NULL);
  if (ap->flag & APF_KEEPCOLOR) {
    colored = 0;
  } else {
    HPoint3 dummyv;
    ColorA dummyc;
    /* Dummy transform to determine whether we have ND colors or not */
    colored = mapHPtN(NDctx, h, &dummyv, &dummyc);
  }

  if (colored) {
    newc = (ColorA *)alloca(s->nvert*sizeof(ColorA));
  } else {
    newc = s->vc;
  }

  /* VERT_4D is only honoured here if pdim == 4 */
  if (s->pdim == 4) {
    for (i = 0, op = s->p, np = newp; i < s->nvert; i++, op += s->pdim, np++) {
      if (s->geomflags & VERT_4D) {
	Pt4ToHPtN((HPoint3 *)op, h);
      } else {
	HPt3ToHPtN((HPoint3 *)op, NULL, h);
      }
      if (colored) {
	mapHPtN(NDctx, h, np, &newc[i]);
      } else {
	mapHPtN(NDctx, h, np, NULL);
      }
    }
  } else { /* Real ND case, i.e. pdim >= 5 */
    float *hdata= h->v;

    for (i = 0, op = s->p, np = newp; i < s->nvert; i++, op += s->pdim, np++) {
      h->v = op;
      if (colored) {
	mapHPtN(NDctx, h, np, &newc[i]);
      } else {
	mapHPtN(NDctx, h, np, NULL);
      }
    }
    h->v = hdata;
  }
  HPtNDelete(h);

  if (!hascolor) {
    colored = 0;
  } else if (newc) {
    colored = 1;
  }

  lastcolor = (ColorA *)(void *)&_mgc->astk->mat.edgecolor;
  for(i = 0, l = s->l, c = colored ? newc : s->c; i < s->nlines; i++, l++) {
    int nleft = l->nv;
    int *vleft = &s->vi[l->v0];
    int j;

    if(l->nc > 0 && hascolor)
      lastcolor = &s->c[l->c0];
    while(nleft > MAXPLINE) {
      if(colored) {
	for(np = tv, c = tc, j = MAXPLINE; --j >= 0; vleft++) {
	  *np++ = newp[*vleft];
	  *c++ = newc[*vleft];
	}
	mgpolyline(MAXPLINE, tv, MAXPLINE, tc, flags);
      } else {
	for(np = tv, j = MAXPLINE; --j >= 0; vleft++)
	  *np++ = newp[*vleft];
	mgpolyline(MAXPLINE, tv, 1, lastcolor, flags);
      }
      nleft -= MAXPLINE-1;
      vleft--;
      flags = 6;
    }
    if(colored) {
      for(np = tv, c = tc, j = nleft; --j >= 0; vleft++) {
	*np++ = newp[*vleft];
	*c++ = newc[*vleft];
      }
      flags = (i < penultimate) ? 6 : 2;
      mgpolyline(nleft, tv, nleft, tc, flags);
    } else {
      for(np = tv, j = nleft; --j >= 0; vleft++)
	*np++ = newp[*vleft];
      flags = (i < penultimate) ? 6 : 2;
      mgpolyline(nleft, tv, 1, lastcolor, flags);
    }
  }
#if !!NO_ALLOCA
  OOGLFree(newp);
#endif
}

Skel *SkelDraw(Skel *s)
{
  int i, hascolor;
  Skline *l;
  int flags, penultimate;
  ColorA *lastcolor=NULL;
  HPoint3 tv[MAXPLINE];
  ColorA tc[MAXPLINE];
  mgNDctx *NDctx = NULL;

  /* Don't draw if vect-drawing is off. */
  if (s == NULL || (_mgc->astk->ap.flag & APF_VECTDRAW) == 0)
    return NULL;
    
  /* draw in conformal model if necessary */
  if (_mgc->space & TM_CONFORMAL_BALL) {
    cmodel_clear(_mgc->space);
    /* cm_read_skel(s); */
    cmodel_draw(0);
    return s;
  }

  hascolor = !(_mgc->astk->mat.override & MTF_EDGECOLOR);

  penultimate = s->nlines - 2;
  flags = penultimate>0 ? 4 : 0;	/* 4: not last mbr of batch of lines */

  mgctxget(MG_NDCTX, &NDctx);

  if (NDctx) {
    draw_projected_skel(NDctx, s, flags, penultimate, hascolor);
    return s;
  }

  lastcolor = (ColorA *)(void *)&_mgc->astk->mat.edgecolor;
  for (i = 0, l = s->l; i < s->nlines; i++, l++) {
    int nleft = l->nv;
    int *vleft = &s->vi[l->v0];
    int j;

    if (l->nc > 0 && hascolor)
      lastcolor = &s->c[l->c0];
    while (nleft > MAXPLINE) {
      for (j = 0; j < MAXPLINE; j++) {
	if (hascolor && s->vc)
	  tc[j] = s->vc[*vleft];
	tv[j] = *(HPoint3 *)&(s->p)[(*vleft++)*s->pdim];
	if (s->pdim < 4) {
	  if (s->pdim < 3) {
	    tv[j].y = 0;
	  }
	  tv[j].z = 0;
	  tv[j].w = 1;
	}
      }
      if (hascolor && s->vc)
	mgpolyline(MAXPLINE, tv, MAXPLINE, tc, flags);
      else
	mgpolyline(MAXPLINE, tv, 1, lastcolor, flags);
      nleft -= MAXPLINE-1;
      vleft--;
      flags = 6;
    }
    for(j = 0; j < nleft; j++) {
      if (hascolor && s->vc)
	tc[j] = s->vc[*vleft];
      tv[j] = *(HPoint3 *)&(s->p)[(*vleft++)*s->pdim];
      if (s->pdim < 4) {
	if (s->pdim < 3) {
	  tv[j].y = 0;
	}
	tv[j].z = 0;
	tv[j].w = 1;
      }
    }
    flags = (i < penultimate) ? 6 : 2;
    if (hascolor && s->vc)
      mgpolyline(nleft, tv, nleft, tc, flags);
    else
      mgpolyline(nleft, tv, 1, lastcolor, flags);
  }
  return s;
}


/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
