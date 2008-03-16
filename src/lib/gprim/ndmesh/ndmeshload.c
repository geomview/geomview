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

#include <ctype.h>
#include "ndmeshP.h"

static bool
getmeshvert(IOBFILE *file, int flag, int pdim, int u, int v, HPointN **p,
	    ColorA *c, TxST *st)
{
  float inputs[128]; /* really plenty */
  float *readv = flag&MESH_4D ? inputs : inputs+1;
  int readdim = flag&MESH_4D ? pdim : pdim - 1;
  int binary = flag&MESH_BINARY;

  (void)u;
  (void)v;

  inputs[0] = 1.0;
  if (iobfgetnf(file, readdim, readv, binary) < readdim) {
    return false;
  }

  *p = HPtNCreate(pdim, inputs);
  
  if ((flag & MESH_C) && iobfgetnf(file, 4, (float *)c, binary) < 4) {
    return false;
  }

  if (flag & MESH_U && iobfgetnf(file, 2, (float *)st, binary) < 2) {
    return false;
  } else {
    /* consume unused "r" component for compatibility */
    float dummy;
    int c;

    if ((c = iobfnextc(file, 1)) != '\n' && c != '}' && c != EOF) {
      if (iobfgetnf(file, 1, &dummy, 0) < 1) {
	return false;
      }
    }
  }
  
  return true;
}

static int
getheader(IOBFILE *file, const char *fname, int *dimp)
{
  int i, flag;
  char *token;
  static char keys[] = "UCN4HUuv";
  static short bit[] = {
    MESH_U, MESH_C, MESH_N, MESH_4D, MESH_4D, MESH_U, MESH_UWRAP, MESH_VWRAP
  };

  /* Parse [U][C][N][Z][4][u][v]MESH[ BINARY]\n */

  flag = 0;
  token = GeomToken(file);
  for(i = 0; keys[i] != '\0'; i++) {
    if (*token == keys[i]) {
      flag |= bit[i];
      token++;
    }
  }
  if (strcmp(token, "nMESH"))
    return -1;

  if (iobfgetni(file, 1, dimp, 0) <= 0)
    return -1;

  if (*dimp < 4) {
    OOGLSyntax(file,
	       "Reading nMESH from \"%s\": dimension %d < 4",
	       fname, *dimp);
  }
  (*dimp)++;

  if (iobfnextc(file, 1) == 'B') {
    if (iobfexpectstr(file, "BINARY"))
      return(-1);
    flag |= MESH_BINARY;
    if (iobfnextc(file, 1) == '\n')
      (void) iobfgetc(file);	/* Toss \n */
  }
  return flag;
}


NDMesh *
NDMeshFLoad(IOBFILE *file, char *fname)
{
  NDMesh	m;
  int	n;
  int i, u, v;
  int size[2];
  int binary;

  if (!file)
    return NULL;

  if ((m.geomflags = getheader(file, fname, &m.pdim)) == -1)
    return NULL;

  m.meshd = 2;	/* Hack.  Should allow general meshes */

  binary = m.geomflags & MESH_BINARY;

  if (iobfgetni(file, m.meshd, size, binary) < 2) {
    OOGLSyntax(file,"Reading nMESH from \"%s\": expected mesh grid size", fname);
    return NULL;
  }
  if (size[0] <= 0 || size[1] <= 0 || size[0] > 9999999 || size[1] > 9999999) {
    OOGLSyntax(file,"Reading nMESH from \"%s\": invalid mesh size %d %d",
	       fname,size[0],size[1]);
    return NULL;
  }

  n = size[0] * size[1];

  m.p = OOGLNewNE(HPointN *, n, "NDMeshFLoad: vertices");
  m.u = NULL;
  m.c = NULL;

  if (m.geomflags & MESH_C) {
    m.c = OOGLNewNE(ColorA, n, "NDMeshFLoad: colors");
  }
  if (m.geomflags & MESH_U) {
    m.u = OOGLNewNE(TxST, n, "NDMeshFLoad: texture coords");
  }

  for (i = 0, v = 0; v < size[1]; v++) {
    for (u = 0; u < size[0]; u++, i++) {
      if (getmeshvert(file, m.geomflags, m.pdim, u, v,
		      &m.p[i], &m.c[i], &m.u[i]) == 0) {
	OOGLSyntax(file,
		   "Reading nMESH from \"%s\": bad element (%d,%d) of (%d,%d)",
		   fname, u, v, size[0], size[1]);
	return NULL;
      }
    }
  }
  return (NDMesh *)GeomCCreate(NULL, NDMeshMethods(), CR_NOCOPY,
			       CR_MESHDIM, 2, CR_MESHSIZE, size,
			       CR_DIM, m.pdim-1,
			       CR_4D, (m.geomflags & MESH_4D),
			       CR_FLAG, m.geomflags,
			       CR_POINT4, m.p, CR_COLOR, m.c,
			       CR_U, m.u, CR_END);
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
