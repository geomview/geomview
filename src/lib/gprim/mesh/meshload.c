/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
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
Copyright (C) 1998-2000 Geometry Technologies, Inc.";


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include <ctype.h>
#include "meshP.h"

static int
getmeshvert(file, flag, u, v, p, n, c, t)
	FILE	*file;
	register int	flag;
	int	u;
	int	v;
	register HPoint3	*p;
	Point3	*n;
	ColorA	*c;
	Point3	*t;		/* actually u, the texture parameter */
{
	float	inputs[4];
	Point3  p3;
	register int binary = flag&MESH_BINARY;

	if (flag & MESH_Z) {
		p->x = (float) u;
		p->y = (float) v;
		p->w = 1.0;
		if (fgetnf(file, 1, &p->z, binary) <= 0)
			return 0;
	} else if (flag & MESH_4D) {
		if (fgetnf(file, 4, (float *)p, binary) < 4)
			return 0;
	} else 			{
		if (fgetnf(file, 3, (float *)p, binary) < 3)
			return 0;
		p->w = 1.0;
	}

	if (flag & MESH_N && fgetnf(file, 3, (float *)n, binary) < 3)
		return 0;

	if (flag & MESH_C && fgetnf(file, 4, (float *)c, binary) < 4)
		return 0;

	if (flag & MESH_U && fgetnf(file, 3, (float *)t, binary) < 3)
		return 0;

	return 1;
}

static char oldbinary;	/* Old binary format -- has 3-component colors */

static int
getheader(file)
	FILE	*file;
{
	register int c, i, flag;
	char *token;
	static char keys[] = "UCNZ4Uuv";
	static short bit[] =
		{ MESH_U, MESH_C, MESH_N, MESH_Z, MESH_4D, MESH_U, MESH_UWRAP, MESH_VWRAP };

	    /* Parse [ST][C][N][Z][4][U][u][v]MESH[ BINARY]\n */
	flag = 0;
	token = GeomToken(file);
	for(i = 0; keys[i] != '\0'; i++) {
	    if(*token == keys[i]) {
		flag |= bit[i];
		token++;
	    }
	}
	if(strcmp(token, "MESH") != 0)
	    return(-1);

	if(fnextc(file, 1) == 'B') {
	    if(fexpectstr(file, "BINARY"))
		return(-1);
	    flag |= MESH_BINARY;
	    if(fnextc(file, 1) == '\n')
		(void) fgetc(file);	/* Toss \n */
	}
	return(flag);
}


Mesh *
MeshFLoad(file, fname)
	FILE *file;
	char *fname;
{
	Mesh	m;
	int	n;
	Point3	*p3;
	register int i, u, v;
	int binary;

	if (!file)
		return NULL;

	if((m.flag = getheader(file)) == -1)
		return NULL;

	binary = m.flag & MESH_BINARY;

	if (fgetni(file, 1, &m.nu, binary) <= 0 ||
	    fgetni(file, 1, &m.nv, binary) <= 0) {
	    OOGLSyntax(file,"Reading MESH from \"%s\": expected mesh grid size", fname);
	    return NULL;
	}
	if(m.nu <= 0 || m.nv <= 0 || m.nu > 9999999 || m.nv > 9999999) {
	    OOGLSyntax(file,"Reading MESH from \"%s\": invalid mesh size %d %d",
		fname,m.nu,m.nv);
	    return NULL;
	}

	n = m.nu * m.nv;

	m.p = OOGLNewNE(HPoint3, n, "MeshFLoad: vertices");
	m.n = NULL;
	m.u = NULL;
	m.c = NULL;
	m.d = NULL;
	m.nd = NULL;

	if (m.flag & MESH_N)
	    m.n = OOGLNewNE(Point3, n, "MeshFLoad: normals");
	if (m.flag & MESH_C)
	    m.c = OOGLNewNE(ColorA, n, "MeshFLoad: colors");
	if (m.flag & MESH_U)
	    m.u = OOGLNewNE(Point3, n, "MeshFLoad: texture coords");

	for (i = 0, v = 0; v < m.nv; v++) {
	    for (u = 0; u < m.nu; u++, i++) {
		if(getmeshvert(file, m.flag, u, v,
			&m.p[i], &m.n[i], &m.c[i], &m.u[i]) == 0) {
		    OOGLSyntax(file,
		"Reading MESH from \"%s\": bad element (%d,%d) of (%d,%d)",
			    fname, u,v, m.nu,m.nv);
		    return NULL;
		}
	    }
	}
	return (Mesh *) GeomCCreate (NULL, MeshMethods(), CR_NOCOPY,
		CR_4D, (m.flag & MESH_4D), CR_FLAG, m.flag, CR_NU, m.nu,
		CR_NV, m.nv, CR_POINT4, m.p, CR_COLOR, m.c, CR_NORMAL, m.n,
		CR_U, m.u, NULL);
}
