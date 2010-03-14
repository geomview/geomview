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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef	POLYLISTPDEFS
#define	POLYLISTPDEFS
/*
 * Private definitions for PolyList's
 */

#include "bboxP.h"
#include "polylist.h"
#include "bsptree.h"

#define PLMAGIC GeomMagic ('p', 1)

typedef struct Vertex
{
  HPoint3 pt;
  ColorA  vcol;
  Point3  vn;
  TxST    st;
} Vertex;

typedef struct Poly
{
  int    n_vertices;
  Vertex **v;
  ColorA pcol;
  Point3 pn;    /* average normal */
  int    flags; /* the flags below are valid if PL_HASPFL is set */
}  Poly;

struct PolyList
{
  GEOMFIELDS;
  int	   n_polys;
  int	   n_verts;
  Poly	   *p;
  Vertex   *vl;
  PolyList *plproj; /* Projected list during ND-drawing. The idea is
		     * to get rid of alloca() and not to have to
		     * regenerated the connectivity structure all the
		     * time.
		     */
# define PL_HASVN   VERT_N         /* Per-vertex normals (vn) valid */
# define PL_HASVCOL VERT_C         /* Per-vertex colors (vcol) valid */
# define PL_HASST   VERT_ST        /* Has s,t texture coords */
# define PL_HASPCOL FACET_C        /* Per-polygon colors (pcol) valid */
# define PL_HASPN   GEOMFLAG(0x01) /* Per-polygon normals (pn) valid */
# define PL_EVNORM  GEOMFLAG(0x04) /* Normals are everted */
# define PL_HASPFL  GEOMFLAG(0x08) /* Has per-poly flags */
  /* For 4-D points, see geomflags & VERT_4D */
};

static inline Ref *PolyListRef(PolyList *pl)
{
  union castit {
    Ref      ref;
    PolyList pl;
  };

  return &((union castit *)pl)->ref;
}

static inline Geom *PolyListGeom(PolyList *pl)
{
  union castit {
    Geom     geom;
    PolyList pl;
  };

  return &((union castit *)pl)->geom;
}

extern PolyList *PolyListCreate(PolyList *exist, GeomClass *Class, va_list *a_list);
extern PolyList *PolyListCopy( PolyList *poly );
extern PolyList *PolyListDelete( PolyList *poly );
extern PolyList *PolyListPick( PolyList *, Pick *, Appearance *,
			       Transform T, TransformN *TN, int *axes );
extern GeomClass *PolyListMethods();
extern PolyList *PolyListComputeNormals(PolyList *polyList, int flags_needed);
extern void PolyNormal(Poly *p, Point3 *nu_av, int fourd, int evert,
		       int *flagsp, int *first_concave);
extern PolyList *PolyListBSPTree(PolyList *poly, BSPTree *bsptree, int action);

#endif /*POLYLISTPDEFS*/

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
