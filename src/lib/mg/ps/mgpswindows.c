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

#include <limits.h>
#include "mg.h"
#include "mgP.h"
#include "mgps.h"
#include "mgpsP.h"
#include "mgpstri.h"
#include "windowP.h"
#include "mgpswindows.h"

static mgps_sort *mgpssort = NULL;
static double curwidth = 1;

/*
   Function: mgps_initpsdevice
   Description: allocate space for the display list, initialize mgcontext
   Author: Daeron Meyer
*/
int mgps_initpsdevice()
{

  if (!mgpssort)
  {
    mgpssort = (mgps_sort *)malloc(sizeof(mgps_sort));
    mgpssort->primnum = 1000;
    VVINIT(mgpssort->primsort, int, mgpssort->primnum);
    vvneeds(&(mgpssort->primsort), mgpssort->primnum);
    VVINIT(mgpssort->prims, mgpsprim, mgpssort->primnum);
    vvneeds(&(mgpssort->prims), mgpssort->primnum);

    mgpssort->pvertnum = 2024;
    VVINIT(mgpssort->pverts, CPoint3, mgpssort->pvertnum);
    vvneeds(&(mgpssort->pverts), mgpssort->pvertnum);

  }
  _mgpsc->mysort = mgpssort;
  return 1;
}

/*
   Function: Xmg_newdisplaylist
   Description: initialize display list
   Author: Daeron Meyer
*/
void mgps_newdisplaylist(void)
{
    _mgpsc->mysort->cprim = 0;
    _mgpsc->mysort->cvert = 0;
    _mgpsc->mysort->maxverts = 0;
    _mgpsc->znudgeby = 0.0;
}

/*
   Function: mgps_add
   Description: add a primitive (polygon, vertex, line) to the display list
   Author: Daeron Meyer
*/
void mgps_add(int primtype, int numdata, void *data, void *cdata)
{
    HPoint3 *vt = (HPoint3 *)data;
    ColorA *colarray = (ColorA *)cdata;
    float *col = (float *)cdata;
    CPoint3 *vts;
    int i;

    static mgpsprim *prim;
    static ColorA color;
    static float average_depth;
    static int numverts;
    static int ecolor[3];
  
    switch (primtype)
    {
    case MGX_BGNLINE:
    case MGX_BGNSLINE:
	average_depth = 0.0;
	prim =
	    &(VVEC(_mgpsc->mysort->prims, mgpsprim)[_mgpsc->mysort->cprim]);
	
	if (primtype == MGX_BGNLINE)
	    prim->mykind = PRIM_LINE;
	else
	    prim->mykind = PRIM_SLINE;

	prim->index = _mgpsc->mysort->cvert;
	prim->depth = -100000; /* very far behind the viewer */
	numverts = 0;

	prim->ecolor[0] = ecolor[0];
	prim->ecolor[1] = ecolor[1];
	prim->ecolor[2] = ecolor[2];
	prim->ewidth = curwidth;

	VVEC(_mgpsc->mysort->primsort, int)[_mgpsc->mysort->cprim] =
	    _mgpsc->mysort->cprim;

	if (!(_mgc->has & HAS_S2O))
	    mg_findS2O();
	mg_findO2S();
	break;

    case MGX_BGNPOLY:
    case MGX_BGNSPOLY:
    case MGX_BGNEPOLY:
    case MGX_BGNSEPOLY:
	average_depth = 0.0;
	prim = &(VVEC(_mgpsc->mysort->prims, mgpsprim)
		 [_mgpsc->mysort->cprim]);
	
	switch(primtype)
	{
	case MGX_BGNPOLY:
	    prim->mykind = PRIM_POLYGON;
	    break;
	case MGX_BGNSPOLY:
	    prim->mykind = PRIM_SPOLYGON;
	    break;
	case MGX_BGNEPOLY:
	    prim->mykind = PRIM_EPOLYGON;
	    break;
	case MGX_BGNSEPOLY:
	    prim->mykind = PRIM_SEPOLYGON;
	    break;
	}

	prim->ewidth = curwidth;
	prim->index = _mgpsc->mysort->cvert;
	prim->depth = -100000; /* very far behind the viewer */
	numverts = 0;
	  
	VVEC(_mgpsc->mysort->primsort, int)[_mgpsc->mysort->cprim] =
	    _mgpsc->mysort->cprim;
	
	if (!(_mgc->has & HAS_S2O))
	    mg_findS2O();
	mg_findO2S();
	break;

    case MGX_VERTEX:
	for (i=0; i<numdata; i++)
	{
            vts = &(VVEC(_mgpsc->mysort->pverts, CPoint3)[_mgpsc->mysort->cvert]);
	    HPt3Transform(_mgc->O2S, &(vt[i]), (HPoint3 *) vts);
	    vts->drawnext = 1;

	    vts->vcol = color;
	    _mgpsc->mysort->cvert++; numverts++;

	    if (_mgpsc->mysort->cvert > _mgpsc->mysort->pvertnum)
	    {
		_mgpsc->mysort->pvertnum*=2;
		vvneeds(&(_mgpsc->mysort->pverts), _mgpsc->mysort->pvertnum);
	    }

	    if (vts->z > prim->depth)
		prim->depth = vts->z;
	    average_depth += vts->z;
	}
	break;
	
    case MGX_CVERTEX:
	for (i=0; i<numdata; i++)
	{
            vts = &(VVEC(_mgpsc->mysort->pverts, CPoint3)[_mgpsc->mysort->cvert]);
	    HPt3Transform(_mgc->O2S, &(vt[i]), (HPoint3 *) vts);
	    vts->drawnext = 1;

	    vts->vcol = colarray[i];

	    _mgpsc->mysort->cvert++;
	    numverts++;
	    if (_mgpsc->mysort->cvert > _mgpsc->mysort->pvertnum)
	    {
		_mgpsc->mysort->pvertnum*=2;
		vvneeds(&(_mgpsc->mysort->pverts), _mgpsc->mysort->pvertnum);
	    }

	    if (vts->z > prim->depth)
		prim->depth = vts->z;
	    average_depth += vts->z;
	  }
	  break;

    case MGX_COLOR:
	color = colarray[0];
	break;

    case MGX_ECOLOR:
	ecolor[0] = (int)(255.0 * col[0]);
	ecolor[1] = (int)(255.0 * col[1]);
	ecolor[2] = (int)(255.0 * col[2]);
	break;
	
    case MGX_END:
	prim->numvts = numverts;
	if (numverts > _mgpsc->mysort->maxverts)
	    _mgpsc->mysort->maxverts = numverts;
	average_depth += prim->depth;
	average_depth /= (float)(numverts+1);
	prim->depth = average_depth;

	prim->color[0] = (int)(255.0 * color.r);
	prim->color[1] = (int)(255.0 * color.g);
	prim->color[2] = (int)(255.0 * color.b);

	prim->ecolor[0] = ecolor[0];
	prim->ecolor[1] = ecolor[1];
	prim->ecolor[2] = ecolor[2];

	if ((prim->mykind = mgps_primclip(prim)) == PRIM_INVIS)
	    _mgpsc->mysort->cvert = prim->index;
	else
	{
	    _mgpsc->mysort->cvert = prim->index + prim->numvts;
	    _mgpsc->mysort->cprim++;
	}

	if (_mgpsc->mysort->cprim > _mgpsc->mysort->primnum)
	{
	    _mgpsc->mysort->primnum*=2;
	    vvneeds(&(_mgpsc->mysort->prims), _mgpsc->mysort->primnum);
	    vvneeds(&(_mgpsc->mysort->primsort), _mgpsc->mysort->primnum);
	}
	break;

    default:
	fprintf(stderr,"unknown type of primitive.\n");
	break;
    }
}

/*
   Function: mgps_primcomp
   Description: Depth sort by comparing two primitives in a call from qsort()
		(painters algorithm, no subdiv of polygons)
   Author: Daeron Meyer
*/
int mgps_primcomp(const void *a, const void *b)
{
    mgpsprim *prim = VVEC(_mgpsc->mysort->prims, mgpsprim);

    if (prim[*(int*)a].depth < prim[*(int*)b].depth)
	return 1;
    else
	return -1;
}

/*
   Function: mgps_sortdisplaylist
   Description: Does depth sorting of primitives.
   Author: Daeron Meyer
*/
void mgps_sortdisplaylist(void)
{
    static int *primp;

    primp = VVEC(_mgpsc->mysort->primsort, int);
    qsort(primp, _mgpsc->mysort->cprim, sizeof(int), &mgps_primcomp);
}

/*
   Function: mgps_showdisplaylist
   Description: render display list to display
   Author: Daeron Meyer, Tim Rowley
*/
void mgps_showdisplaylist(FILE *outf)
{
    CPoint3 *vts;
    int ref, *primp;
    mgpsprim *prim, *prim2;
    static int width;
    static int height;

    WnGet(_mgc->win, WN_XSIZE, &width);
    WnGet(_mgc->win, WN_YSIZE, &height);

    MGPS_startPS(outf, &(_mgc->background), width/(double)height,
		 width, height);

    primp = VVEC(_mgpsc->mysort->primsort, int);
    prim2 = VVEC(_mgpsc->mysort->prims, mgpsprim);
    vts = VVEC(_mgpsc->mysort->pverts, CPoint3);

    for (ref = 0; ref < _mgpsc->mysort->cprim; ref++)
    {
	prim = &(prim2[primp[ref]]);
	switch (prim->mykind)
	{
	case PRIM_POLYGON:
	    MGPS_poly(vts+prim->index, prim->numvts, prim->color);
	    break;

	case PRIM_SPOLYGON:
	    MGPS_spoly(vts+prim->index, prim->numvts);
	    break;

	case PRIM_EPOLYGON:
	    MGPS_epoly(vts+prim->index, prim->numvts, prim->color,
		       prim->ewidth, prim->ecolor);
	    break;

	case PRIM_SEPOLYGON:
	    MGPS_sepoly(vts+prim->index, prim->numvts,
		       prim->ewidth, prim->ecolor);
	    break;
	    
	case PRIM_SLINE:
	    MGPS_spolyline(vts+prim->index, prim->numvts, prim->ewidth);
	    break;

	case PRIM_LINE:
	    MGPS_polyline(vts+prim->index, prim->numvts,
			  prim->ewidth, prim->ecolor);
	    break;
	    
	case PRIM_INVIS:
	    break;

	default:
	    break;
	}
    }
    MGPS_finishPS();
}

void
mgps_appearance( struct mgastk *ma, int mask )
{
    Appearance *ap = &(ma->ap);

    if (mask & APF_LINEWIDTH)
    {
	curwidth = ap->linewidth-1;
	_mgc->has &= ~HAS_POINT;
    }

    if (mask & APF_SHADING)
    {
	if (IS_SHADED(ap->shading) && ma->shader != NULL) {
	    /* Use software shader if one exists and user wants lighting */
	    ma->flags |= MGASTK_SHADER;
	} else {
	    /* No software shading, just use raw colors */
	    ma->flags &= ~MGASTK_SHADER;
	}
    }
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
    
