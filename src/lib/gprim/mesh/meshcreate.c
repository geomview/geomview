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

#include "meshP.h"

static int tossmesh(Mesh *);
static int meshfield(int copy, int amount,
		void **fieldp, void *value, char *name);

Mesh *
MeshCreate (exist, classp, a_list)
Mesh *exist;
GeomClass *classp;
va_list *a_list;
{
    register Mesh *mesh;
    int		attr, copy = 1, fourd = 0;
    int		i;
    int		npts;
    HPoint3	*p;
    Point3 	*n, *u, *p3;
    ColorA	*c;

    p = NULL; n = NULL; u = NULL; c = NULL; p3 = NULL;

    if (exist == NULL) {
	mesh = OOGLNewE(Mesh, "MeshCreate mesh");
	memset(mesh, 0, sizeof(Mesh));
	GGeomInit (mesh, classp, MESHMAGIC, NULL);
	mesh->flag = 0;
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

    while ((attr = va_arg (*a_list, int))) switch (attr) {
	case CR_FLAG:
	    mesh->flag = va_arg (*a_list, int);
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
	    if(mesh->p) OOGLFree(mesh->p);
	    mesh->p = OOGLNewNE(HPoint3, npts, "mesh points");
	    p3 = va_arg(*a_list, Point3 *);
	    Pt3ToPt4(p3, mesh->p, npts);
	    if(!copy) OOGLFree(p3);
	    break;

	case CR_POINT4:
	    meshfield(copy, npts*sizeof(HPoint3), (void **)&mesh->p,
		(void *)va_arg (*a_list, HPoint3 *), "mesh points");
	    break;

	case CR_NORMAL:
	    mesh->flag = (mesh->flag & ~MESH_N) |
		(MESH_N & meshfield(copy, npts*sizeof(Point3),
				(void **)&mesh->n,
				(void *)va_arg (*a_list, Point3 *),
				"mesh normals"));
	    break;

	case CR_U:
	    mesh->flag = (mesh->flag & ~MESH_U) |
		(MESH_U & meshfield(copy, npts*sizeof(Point3),
				(void **)&mesh->u,
				(void *)va_arg (*a_list, Point3 *),
				"mesh texture coords"));
	    break;

	case CR_COLOR:
	    mesh->flag = (mesh->flag & ~MESH_C) |
		(MESH_C & meshfield(copy, npts*sizeof(ColorA),
				(void **)&mesh->c,
				(void *)va_arg (*a_list, ColorA *),
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

    return mesh;
}

static int
tossmesh(register Mesh *m)
{
    if(m->p) OOGLFree(m->p);
    if(m->n) OOGLFree(m->n);
    if(m->c) OOGLFree(m->c);
    if(m->u) OOGLFree(m->u);
    if(m->d) OOGLFree(m->d);
    if(m->nd) OOGLFree(m->nd);
    m->p = NULL;
    m->n = NULL;
    m->c = NULL;
    m->u = NULL;
    m->d = NULL;
    m->nd = NULL;
    m->umin = m->umax = m->vmin = m->vmax = -1;
    return 0;
}

static int
meshfield(int copy, int amount, void **fieldp, void *value, char *name)
{
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
