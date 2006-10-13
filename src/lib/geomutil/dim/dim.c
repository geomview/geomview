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

/* Get the dimension of the space in which a Geom lives */
#include "geomclass.h"

#include "skelP.h"
#include "ndmeshP.h"
#include "polylistP.h"
#include "npolylistP.h"
#include "bezierP.h"
#include "quadP.h"
#include "meshP.h"
#include "vectP.h"
#include "listP.h"
#include "discgrpP.h"
#include "instP.h"
#include "hpoint3.h"
#include "point3.h"

#include "dim.h"

static void *dimdefault(int sel, Geom *g, va_list *args) {
    return
      (void *)(long)((g && (g->geomflags & VERT_4D)) ? g->pdim : g->pdim-1);
}

static void *dimSkel(int sel, Geom *g, va_list *args) {
    return (void *)(long)(((Skel *)g)->pdim-1);
}

static void *dimNDMesh(int sel, Geom *g, va_list *args) {
    return (void *)(long)(((NDMesh *)g)->pdim-1);
}

static void *dimNPolyList(int sel, Geom *g, va_list *args) {
    return (void *)(long)(((NPolyList *)g)->pdim-1);
}

static void *dimQuad(int sel, Geom *g, va_list *args) {
    return (void *)(long)((g && (g->geomflags & VERT_4D)) ? 4 : 3);
}

static void *dimMesh(int sel, Geom *g, va_list *args) {
    return (void *)(long)((g && (g->geomflags & VERT_4D)) ? 4 : 3);
}

static void *dimList(int sel, Geom *g, va_list *args) {
    int dim, maxdim = 3;
    List *l;
    for(l = (List *)g; l != NULL; l = l->cdr) {
	dim = GeomDimension(l->car);
	if(maxdim < dim) maxdim = dim;
    }
    return (void *)(long)maxdim;
}

static void *dimInst(int sel, Geom *g, va_list *args) {
    return (void *)(long)GeomDimension(((Inst *)g)->geom);
}

static int DimSel = 0;

int
GeomDimension(Geom *g)
{
    if(DimSel == 0) {
	DimSel = GeomNewMethod("dimension", dimdefault);
	GeomSpecifyMethod(DimSel, SkelMethods(), dimSkel);
	GeomSpecifyMethod(DimSel, NDMeshMethods(), dimNDMesh);
	GeomSpecifyMethod(DimSel, NPolyListMethods(), dimNPolyList);
	GeomSpecifyMethod(DimSel, ListMethods(), dimList);
	GeomSpecifyMethod(DimSel, InstMethods(), dimInst);
	GeomSpecifyMethod(DimSel, InstMethods(), dimInst);
	GeomSpecifyMethod(DimSel, QuadMethods(), dimQuad);
	GeomSpecifyMethod(DimSel, MeshMethods(), dimMesh);
    }
    return (int)(long)GeomCall( DimSel, g );
}
