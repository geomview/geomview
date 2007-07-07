/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2007 Claus-Justus Heine
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
#include "meshP.h"

/* actually u, the texture parameter */
static bool
getmeshvert(IOBFILE *file, int flag, int u, int v, HPoint3 *p,
	    Point3 *n, ColorA *c, TxST *st)
{
  int binary = flag&MESH_BINARY;

  if (flag & MESH_Z) {
    p->x = (float) u;
    p->y = (float) v;
    p->w = 1.0;
    if (iobfgetnf(file, 1, &p->z, binary) <= 0) {
      return false;
    }
  } else if (flag & MESH_4D) {
    if (iobfgetnf(file, 4, (float *)p, binary) < 4) {
      return false;
    }    
  } else {
    if (iobfgetnf(file, 3, (float *)p, binary) < 3) {
      return false;
    }
    p->w = 1.0;
  }

  if ((flag & MESH_N) && iobfgetnf(file, 3, (float *)n, binary) < 3) {
    return false;
  }
  if ((flag & MESH_C) && iobfgetnf(file, 4, (float *)c, binary) < 4) {
    return false;
  }
  if (flag & MESH_U) {
    if (iobfgetnf(file, 2, (float *)st, binary) < 2) {
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
  }

  return true;
}

/* static char oldbinary; *//* Old binary format -- has 3-component colors */

static int
getheader(IOBFILE *file)
{
  int i, flag;
  char *token;
  static char keys[] = "UCNZ4Uuv";
  static int bit[] =
    { MESH_U, MESH_C, MESH_N, MESH_Z, MESH_4D, MESH_U, MESH_UWRAP, MESH_VWRAP };

  /* Parse [ST][C][N][Z][4][U][u][v]MESH[ BINARY]\n */
  flag = 0;
  token = GeomToken(file);
  for (i = 0; keys[i] != '\0'; i++) {
    if (*token == keys[i]) {
      flag |= bit[i];
      token++;
    }
  }
  if (strcmp(token, "MESH") != 0) {
    return(-1);
  }

  if (iobfnextc(file, 1) == 'B') {
    if (iobfexpectstr(file, "BINARY")) {
      return(-1);
    }
    flag |= MESH_BINARY;
    if (iobfnextc(file, 1) == '\n') {
      (void) iobfgetc(file);	/* Toss \n */
    }
  }
  return(flag);
}


Mesh *
MeshFLoad(IOBFILE *file, char *fname)
{
  Mesh	m;
  int	n;
  int i, u, v;
  int binary;

  if (!file)
    return NULL;

  if ((m.geomflags = getheader(file)) == -1)
    return NULL;

  binary = m.geomflags & MESH_BINARY;

  if (iobfgetni(file, 1, &m.nu, binary) <= 0 ||
      iobfgetni(file, 1, &m.nv, binary) <= 0) {
    OOGLSyntax(file,"Reading MESH from \"%s\": expected mesh grid size", fname);
    return NULL;
  }
  if (m.nu <= 0 || m.nv <= 0 || m.nu > 9999999 || m.nv > 9999999) {
    OOGLSyntax(file,"Reading MESH from \"%s\": invalid mesh size %d %d",
	       fname,m.nu,m.nv);
    return NULL;
  }

  n = m.nu * m.nv;

  m.p = OOGLNewNE(HPoint3, n, "MeshFLoad: vertices");
  m.n = NULL;
  m.u = NULL;
  m.c = NULL;

  if (m.geomflags & MESH_N) {
    m.n = OOGLNewNE(Point3, n, "MeshFLoad: normals");
  }
  if (m.geomflags & MESH_C) {
    m.c = OOGLNewNE(ColorA, n, "MeshFLoad: colors");
  }
  if (m.geomflags & MESH_U) {
    m.u = OOGLNewNE(TxST, n, "MeshFLoad: texture coords");
  }

  for (i = 0, v = 0; v < m.nv; v++) {
    for (u = 0; u < m.nu; u++, i++) {
      if (getmeshvert(file, m.geomflags, u, v,
		      &m.p[i], &m.n[i], &m.c[i], &m.u[i]) == 0) {
	OOGLSyntax(file,
		   "Reading MESH from \"%s\": bad element (%d,%d) of (%d,%d)",
		   fname, u,v, m.nu, m.nv);
	return NULL;
      }
    }
  }
  return 
    (Mesh *)GeomCCreate (NULL, MeshMethods(), CR_NOCOPY,
			 CR_4D, (m.geomflags & MESH_4D),
			 CR_FLAG, m.geomflags, CR_NU, m.nu,
			 CR_NV, m.nv, CR_POINT4, m.p, CR_COLOR, m.c,
			 CR_NORMAL, m.n, CR_U, m.u, NULL);
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
