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

#include "meshP.h"

static int tossmesh(Mesh *);
static int meshfield(int copy, int amount,
		     void **fieldp, void *value, char *name);

Mesh *
MeshCreate (Mesh *exist, GeomClass *classp, va_list *a_list)
{
  Mesh *mesh;
  int		attr, copy = 1;
  int		npts;
  Point3 	*p3;
  ColorA	*c;

  c = NULL; p3 = NULL;

  if (exist == NULL) {
    mesh = OOGLNewE(Mesh, "MeshCreate mesh");
    memset(mesh, 0, sizeof(Mesh));
    GGeomInit (mesh, classp, MESHMAGIC, NULL);
    mesh->bsptree = NULL;
    mesh->nu = 1;
    mesh->nv = 1;
    mesh->umin = -1;
    mesh->umax = -1;
    mesh->vmin = -1;
    mesh->vmax = -1;
  } else {
    /* Check that exist is a Mesh... */
    mesh = exist;
  }

  npts = mesh->nu * mesh->nv;

  mesh->pdim = 4; /* hard-wired */

  while ((attr = va_arg (*a_list, int))) switch (attr) {
  case CR_FLAG:
    mesh->geomflags = va_arg (*a_list, int);
    break;
  case CR_NU:
    mesh->nu = va_arg (*a_list, int);
    tossmesh(mesh);
    npts = mesh->nu * mesh->nv;
    break;
  case CR_NV:
    mesh->nv = va_arg (*a_list, int);
    tossmesh(mesh);
    npts = mesh->nu * mesh->nv;
    break;
  case CR_UMIN:
    mesh->umin = va_arg (*a_list, int);
    break;
  case CR_UMAX:
    mesh->umax = va_arg (*a_list, int);
    break;
  case CR_VMIN:
    mesh->vmin = va_arg (*a_list, int);
    break;
  case CR_VMAX:
    mesh->vmax = va_arg (*a_list, int);
    break;
  case CR_POINT:
    mesh->geomflags &= ~MESH_NQ;
    if(mesh->p) OOGLFree(mesh->p);
    mesh->p = OOGLNewNE(HPoint3, npts, "mesh points");
    p3 = va_arg(*a_list, Point3 *);
    Pt3ToHPt3(p3, mesh->p, npts);
    if(!copy) OOGLFree(p3);
    break;

  case CR_POINT4:
    mesh->geomflags &= ~MESH_NQ;
    meshfield(copy, npts*sizeof(HPoint3), (void **)(void *)&mesh->p,
	      (void *)va_arg (*a_list, HPoint3 *), "mesh points");
    break;

  case CR_NORMAL:
    mesh->geomflags = (mesh->geomflags & ~MESH_N) |
      (MESH_N & meshfield(copy, npts*sizeof(Point3),
			  (void **)(void *)&mesh->n,
			  (void *)va_arg (*a_list, Point3 *),
			  "mesh normals"));
    break;

  case CR_U:
    mesh->geomflags = (mesh->geomflags & ~MESH_U) |
      (MESH_U & meshfield(copy, npts*sizeof(TxST),
			  (void **)(void *)&mesh->u,
			  (void *)va_arg (*a_list, TxST *),
			  "mesh texture coords"));
    break;

  case CR_COLOR:
    mesh->geomflags &= ~COLOR_ALPHA;
    mesh->geomflags = (mesh->geomflags & ~MESH_C) |
      (MESH_C & meshfield(copy, npts*sizeof(ColorA),
			  (void **)(void *)&mesh->c,
			  (void *)(c = va_arg (*a_list, ColorA *)),
			  "mesh colors"));
    break;

  default:
    if (GeomDecorate (mesh, &copy, attr, a_list)) {
      GeomError (0, "MeshCreate: Undefined option: %d", attr);
      OOGLFree (mesh);
      return NULL;
    }
  }
  /* set submesh dimensions if not otherwise set */
  if (mesh->umin == -1)	mesh->umin = 0;
  if (mesh->umax == -1)	mesh->umax = mesh->nu-1;
  if (mesh->vmin == -1)	mesh->vmin = 0;
  if (mesh->vmax == -1)	mesh->vmax = mesh->nv-1;

  if (c) {
    int i;
    
    for (i = 0; i < mesh->nu*mesh->nv; i++) {
      if (mesh->c[i].a < 1.0) {
	mesh->geomflags |= COLOR_ALPHA;
      }
    }
  }

  return mesh;
}

static int
tossmesh(Mesh *m)
{
  if(m->p) OOGLFree(m->p);
  if(m->n) OOGLFree(m->n);
  if(m->nq) OOGLFree(m->nq);
  if(m->c) OOGLFree(m->c);
  if(m->u) OOGLFree(m->u);
  m->p  = NULL;
  m->n  = NULL;
  m->nq = NULL;
  m->c  = NULL;
  m->u  = NULL;
  m->umin = m->umax = m->vmin = m->vmax = -1;
  m->geomflags &= ~MESH_NQ;
  return 0;
}

static int meshfield(int copy, int amount, void **fieldp, void *value, char *name)
{
  (void)name;
  
  if(value) {
    if(copy) {
      if(*fieldp == NULL)
	*fieldp = OOGLNewNE(char, amount, name);
      memcpy(*fieldp, value, amount);
    } else {
      if(*fieldp)
	OOGLFree(*fieldp);
      *fieldp = value;
    }
    return ~0;
  } else {
    if(*fieldp)
      OOGLFree(*fieldp);
    *fieldp = NULL;
    return 0;
  }
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
