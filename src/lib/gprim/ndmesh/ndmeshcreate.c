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

#include "ndmeshP.h"

static int ndmeshfield(int copy, int amount,
		       void **fieldp, void *value, char *name);

static int ndmeshsize(NDMesh *m)
{
    int i, s;
    for(i = m->meshd, s = 1; --i >= 0; ) s *= m->mdim[i];
    return s;
}

static void tosspoints(NDMesh *m)
{
    int n;
    HPointN **p;
    if(m->mdim && m->p) {
	for(n = ndmeshsize(m), p = m->p; --n >= 0; p++)
	    if(*p) HPtNDelete( *p );
    }
}

static void tossmesh(NDMesh *m)
{

    tosspoints(m);
    if(m->p) OOGLFree(m->p);
    if(m->c) OOGLFree(m->c);
    if(m->u) OOGLFree(m->u);

    m->p = NULL;
    m->c = NULL;
    m->u = NULL;
}


NDMesh *
NDMeshCreate (NDMesh *exist, GeomClass *classp, va_list *a_list)
{
    NDMesh *m;
    int		attr, copy = 1;
    int		i;
    int		npts;
    ColorA	*c;

    c = NULL;

    if (exist == NULL) {
	m = OOGLNewE(NDMesh, "NDMeshCreate mesh");
	memset(m, 0, sizeof(NDMesh));
	GGeomInit (m, classp, NDMESHMAGIC, NULL);
	m->geomflags = 0;
	m->meshd = 2;
	m->mdim = OOGLNewNE(int, m->meshd, "NDMesh dim");
	memset(m->mdim, 0, m->meshd * sizeof(int));
    } else {
	/* Check that exist is a NDMesh... */
	m = exist;
    }

    npts = ndmeshsize( m );

    while ((attr = va_arg (*a_list, int))) switch (attr) {
	case CR_FLAG:
	    m->geomflags = va_arg (*a_list, int);
	    break;

	case CR_MESHDIM:
	    i = va_arg (*a_list, int);
	    if(i <= 0 || i >= 1000) {
		OOGLError(1, "Incredible NDMesh dimension %d", i);
		return NULL;
	    }
	    m->meshd = i;
	    OOGLFree(m->mdim);
	    m->mdim = OOGLNewNE(int, m->meshd, "NDMesh dim");
	    memset(m->mdim, 0, m->meshd * sizeof(int));
	    tossmesh(m);
	    npts = 0;
	    break;

	case CR_MESHSIZE:
	    memcpy(m->mdim, va_arg (*a_list, int *), m->meshd*sizeof(int));
	    tossmesh(m);
	    npts = ndmeshsize(m);
	    break;

	case CR_DIM:
	    m->pdim = va_arg(*a_list, int);
	    if (m->pdim < 4) {
		    OOGLError(1, "Dimension %d < 4", m->pdim);
		    return NULL;
	    }
	    m->pdim++; /* should be the projective dimension */
	    break;

	case CR_POINT:
	case CR_POINT4:
	    tosspoints(m);
	    ndmeshfield(copy, npts*sizeof(HPointN *), (void **)(void *)&m->p,
		(void *)va_arg (*a_list, HPointN **), "ND mesh points");
	    break;

	case CR_U:
	    m->geomflags = (m->geomflags & ~MESH_U) |
		(MESH_U & ndmeshfield(copy, npts*sizeof(TxST),
				      (void **)(void *)&m->u,
				(void *)va_arg (*a_list, TxST *),
				"ndmesh texture coords"));
	    break;

	case CR_COLOR:
	    m->geomflags &= ~COLOR_ALPHA;
	    m->geomflags = (m->geomflags & ~MESH_C) |
		(MESH_C & ndmeshfield(copy, npts*sizeof(ColorA),
				      (void **)(void *)&m->c,
				      (void *)(c = va_arg (*a_list, ColorA *)),
				"ndmesh colors"));
	    break;

	default:
	    if (GeomDecorate (m, &copy, attr, a_list)) {
		GeomError (0, "NDMeshCreate: Undefined option: %d", attr);
		OOGLFree (m);
		return NULL;
	    }
    }

    if (c) {
	int i;
    
	for (i = 0; i < m->mdim[0]*m->mdim[1]; i++) {
	    if (m->c[i].a < 1.0) {
		m->geomflags |= COLOR_ALPHA;
	    }
	}
    }

    return m;
}

static int
ndmeshfield(int copy, int amount, void **fieldp, void *value, char *name)
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

NDMesh *
NDMeshDelete(NDMesh *m)
{
    if (m)
    {
	tossmesh(m);
	OOGLFree(m->mdim);
    }
    return NULL;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
