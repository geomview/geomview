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
 * NPolyList creation, editing, and deletion.
 */
#include "npolylistP.h"


/*
 * Free a NPolyList. 
 */

#include "npolylistP.h"

NPolyList *
NPolyListDelete( NPolyList *np )
{
  if (np == NULL)
    return NULL;

  OOGLFree(np->vi);
  OOGLFree(np->pv);
  OOGLFree(np->v);
  OOGLFree(np->vcol);
  OOGLFree(np->p[0].v);
  OOGLFree(np->p);
  OOGLFree(np->vl);

  np->vi   = NULL;
  np->pv   = NULL;
  np->v    = NULL;
  np->vcol = NULL;
  np->p    = NULL;
  np->vl   = NULL;
    
  return NULL;
}

NPolyList *
NPolyListCreate(NPolyList *exist, GeomClass *classp, va_list *a_list)
{
  NPolyList *pl;
  int *nvertperpol=NULL, *verts=NULL;
  int npolyflag=0, nvertflag=0, vertflag=0, pointflag=0, pointhomog = 0;
  float *v = NULL;
  ColorA *vc = NULL, *pc = NULL;
  int attr, copy=1;
  int numentries=0, numvertices=0;
  int i;
  int j,k=0;

  if (exist == NULL) {
    pl = OOGLNewE(NPolyList,"NPolyListCreate npolylist");
    memset(pl, 0, sizeof(NPolyList));
    GGeomInit(pl, classp, NPLMAGIC, NULL);
    pl->pdim = 4;	/* 3-D plus homogeneous component */
  } else {
    pl = exist;
  }

  while ((attr = va_arg(*a_list, int))) 
    switch (attr) {

    case CR_NOCOPY:
      OOGLError(0,"Note: NOCOPY option not used by NPolyListCreate()");
      break;

    case CR_FLAG:
      pl->geomflags = va_arg(*a_list, int);
      break;

    case CR_NPOLY:
      pl->n_polys = va_arg(*a_list, int);
      npolyflag = 1;
      break;

    case CR_NVERT:	/* number of verts of each polygon */
      nvertperpol = va_arg(*a_list, int*);
      nvertflag = 1;
      break;

    case CR_VERT:	/* indices of all verts of all polygons, concatenated */
			/* verts[] contains sum(nvertperpol[]) elements */
      verts = va_arg(*a_list, int*);
      vertflag = 1;
      break;

    case CR_DIM:
      pl->pdim = va_arg(*a_list, int) + 1;
      break;

    case CR_POINT4:	   /* CR_POINT4, <points including homog. components> */
      pointhomog = 1;
      /* Fall into CR_POINT */

    case CR_POINT:	   /* CR_POINT, <points without homog. components> */
	
      v = va_arg(*a_list, HPtNCoord *);
      pointflag = 1;
      break;

    case CR_COLOR:	   /* CR_COLOR, <ColorA's, one per vertex */
      pl->geomflags &= ~COLOR_ALPHA;
      vc = va_arg(*a_list, ColorA *);
      if (vc) {
	pl->geomflags |= PL_HASVCOL;
      }
      break;

    case CR_POLYCOLOR:
      pl->geomflags &= ~COLOR_ALPHA;
      pc = va_arg(*a_list, ColorA *);
      if (pc) {
	pl->geomflags |= PL_HASPCOL;
      }
      break;

    default:
      if (GeomDecorate(pl, &copy, attr, a_list)) {
	OOGLError(0,"Undefined PolyList option: %d", attr);
	if (!exist) GeomDelete((Geom *)pl);
	return NULL;
      }
    }

  if (!exist && (!npolyflag || !nvertflag || !vertflag || !pointflag)) {
    if (!npolyflag) OOGLError(0,"Must specify number of polygons");
    if (!nvertflag) OOGLError(0,"Must specify NVERT array");
    if (!vertflag) OOGLError(0,"Must specify VERT array");
    if (!pointflag) OOGLError(0,"Must specify vertices");
    if (pl->pdim < 5)OOGLError(0, "Dimension %d too small, "
			       "please use ordinary OFF format",
			       pl->pdim-1);
    GeomDelete((Geom *)pl);
    return NULL;
  }


  if(nvertflag && vertflag) {
    numentries = 0;
    for (i=0; i<pl->n_polys; i++) {
      numentries += nvertperpol[i];
    }
    for (i=0; i<numentries; i++) {
      if (verts[i] > numvertices) {
	numvertices = verts[i];
      }
    }
    pl->n_verts = numvertices + 1;
    if(pl->v) {
      OOGLFree(pl->v);
    }
    pl->v = OOGLNewNE(HPtNCoord,
		      pl->n_verts * pl->pdim, "polylist vertices");
    if (pl->vl) {
      OOGLFree(pl->vl);
    }
    pl->vl = OOGLNewNE(Vertex, pl->n_verts, "polylist vertex info");
  }
  if(pointflag) {
    /* if dimn == 3, copy over into 4-vector area and add a '1' in
     * the 4th coordinate */
    if (pointhomog) {
      memcpy(pl->v, v, pl->pdim*pl->n_verts*sizeof(HPtNCoord));
    } else {
      float *pv;
      for(i = pl->n_verts, pv = pl->v; --i >= 0; ) {
	*pv++ = 1.0;	/* Add homogenous '1' */
	for(j = 1; j < pl->pdim; j++)
	  *pv++ = *v++;
      }
    }
  }
  if(npolyflag) {
    if(pl->p) {
      if (pl->p[0].v) {
	OOGLFree(pl->p[0].v);
      }
      OOGLFree(pl->p);
    }
    pl->p = OOGLNewNE(Poly, pl->n_polys, "nPolyListCreate polygons");
    if (pl->pv) {
      OOGLFree(pl->pv);
    }
    pl->pv = OOGLNewNE(int, pl->n_polys, "nPolyListCreate polygon verts");
  }
  if(nvertflag && vertflag) {
    Vertex **vp;
    k = 0;
    for (i=0; i<pl->n_polys; i++) {
      pl->pv[i] = k;
      k += (pl->p[i].n_vertices = nvertperpol[i]);
    }
    if(pl->vi) {
      OOGLFree(pl->vi);
    }
    pl->vi = OOGLNewNE(int, numentries, "nPolyListCreate vertex indices");
    pl->nvi = k;
    memcpy(pl->vi, verts, k*sizeof(int));

    /* Also update the connectivity info stored in pl->p[] */
    vp = OOGLNewNE(Vertex *, numentries, "nPolyListCreate connectivity");
    for (i = 0; i < pl->n_polys; i++) {
      Poly *p = &pl->p[i];
      
      p->v  = vp;
      vp   += p->n_vertices;
    
      for (k = 0; k < p->n_vertices; k++) {
	p->v[k] = &pl->vl[pl->vi[pl->pv[i]+k]];
      }
    }
  }

  if(vc) {
    for (i = 0; i < pl->n_verts; i++) {
      pl->vcol[i] = pl->vl[i].vcol = vc[i];
      if (vc[i].a != 1.0) {
	pl->geomflags |= COLOR_ALPHA;
      }
    }
    pl->geomflags |= NPL_HASVLVCOL;
  }

  if (pc) {
    for (i = 0; i < pl->n_polys; i++) {
      pl->p[i].pcol = pc[i];
      if (pc[i].a != 1.0) {
	pl->geomflags |= COLOR_ALPHA;
      }
    }
  }

  return pl;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
