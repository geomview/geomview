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

#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
#include "config.h"
#endif

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include <ctype.h>
#include "ndmeshP.h"

static int
getmeshvert(FILE *file, int flag, int pdim, int u, int v, HPointN **p,
		ColorA *c, Point3 *t)
{
	float	inputs[128];
	int readdim = flag&MESH_4D ? pdim+1 : pdim;
	int binary = flag&MESH_BINARY;

	inputs[pdim] = 1.0;
	if (fgetnf(file, readdim, inputs, binary) < readdim)
	    return 0;

	*p = HPtNCreate(pdim+1, inputs);

	if (flag & MESH_C && fgetnf(file, 4, (float *)c, binary) < 4)
		return 0;

	if (flag & MESH_U && fgetnf(file, 3, (float *)t, binary) < 3)
		return 0;

	return 1;
}

static int
getheader(FILE *file, int *dimp)
{
	int c, i, flag, dim;
	char *token;
	static char keys[] = "UCN4HUuv";
	static short bit[] =
	 { MESH_U, MESH_C, MESH_N, MESH_4D, MESH_4D, MESH_U, MESH_UWRAP, MESH_VWRAP };

	    /* Parse [U][C][N][Z][4][u][v]MESH[ BINARY]\n */

	flag = 0;
	token = GeomToken(file);
	for(i = 0; keys[i] != '\0'; i++) {
	    if(*token == keys[i]) {
		flag |= bit[i];
		token++;
	    }
	}
	if(strcmp(token, "nMESH"))
	    return(-1);

	if(fgetni(file, 1, dimp, 0) <= 0)
	    return -1;

	if(fnextc(file, 1) == 'B') {
	    if(fexpectstr(file, "BINARY"))
		return(-1);
	    flag |= MESH_BINARY;
	    if(fnextc(file, 1) == '\n')
		(void) fgetc(file);	/* Toss \n */
	}
	return(flag);
}


NDMesh *
NDMeshFLoad(file, fname)
	FILE *file;
	char *fname;
{
	NDMesh	m;
	int	n;
	Point3	*p3;
	int i, u, v;
	int size[2];
	int binary;

	if (!file)
		return NULL;

	if((m.flag = getheader(file, &m.pdim)) == -1)
		return NULL;

	m.meshd = 2;	/* Hack.  Should allow general meshes */

	binary = m.flag & MESH_BINARY;

	if (fgetni(file, m.meshd, size, binary) < 2) {
	    OOGLSyntax(file,"Reading nMESH from \"%s\": expected mesh grid size", fname);
	    return NULL;
	}
	if(size[0] <= 0 || size[1] <= 0 || size[0] > 9999999 || size[1] > 9999999) {
	    OOGLSyntax(file,"Reading nMESH from \"%s\": invalid mesh size %d %d",
		fname,size[0],size[1]);
	    return NULL;
	}

	n = size[0] * size[1];

	m.p = OOGLNewNE(HPointN *, n, "NDMeshFLoad: vertices");
	m.u = NULL;
	m.c = NULL;

	if (m.flag & MESH_C)
	    m.c = OOGLNewNE(ColorA, n, "NDMeshFLoad: colors");
	if (m.flag & MESH_U)
	    m.u = OOGLNewNE(Point3, n, "NDMeshFLoad: texture coords");

	for (i = 0, v = 0; v < size[1]; v++) {
	    for (u = 0; u < size[0]; u++, i++) {
		if(getmeshvert(file, m.flag, m.pdim, u, v,
			&m.p[i], &m.c[i], &m.u[i]) == 0) {
		    OOGLSyntax(file,
		"Reading nMESH from \"%s\": bad element (%d,%d) of (%d,%d)",
			    fname, u,v, size[0],size[1]);
		    return NULL;
		}
	    }
	}
	return (NDMesh *) GeomCCreate (NULL, NDMeshMethods(), CR_NOCOPY,
		CR_MESHDIM, 2, CR_MESHSIZE, size, CR_DIM, m.pdim,
		CR_4D, (m.flag & MESH_4D), CR_FLAG, m.flag,
		CR_POINT4, m.p, CR_COLOR, m.c, CR_U, m.u, CR_END);
}
