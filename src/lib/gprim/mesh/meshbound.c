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

#include "bboxP.h"
#include "meshP.h"

BBox *
MeshBound(mesh, T)
	Mesh	*mesh;
	Transform T;
{
	register int	n;
	register HPoint3 *p;
	HPoint3	min, max;
	HPoint3	p0;

	n = mesh->nu * mesh->nv;
	p = mesh->p;

#ifdef BBOX_ND_HACK
	if(mesh->flag & MESH_4D)
	    return BBox_ND_hack(NULL, (float *)p, 4*n);
#endif

	while(--n >= 0 && !finite(p->x + p->y + p->z))
		p++;
	if(n <= 0)
		return NULL;	/* No finite elements! */
	min = *p;
	HPt3TransPt3(T, p, (Point3 *)(void *)&min);
	min.w = 1.;
	max = min;
	while(--n >= 0) {
		p++;
            	if (T != TM_IDENTITY)
			HPt3TransPt3(T, p, (Point3 *)(void *)&p0);
		else if(p->w == 1.)
			p0 = *p;
		else
			HPt3Normalize(p, &p0);
		if(min.x > p0.x) min.x = p0.x;
		else if(max.x < p0.x) max.x = p0.x;
		if(min.y > p0.y) min.y = p0.y;
		else if(max.y < p0.y) max.y = p0.y;
		if(min.z > p0.z) min.z = p0.z;
		else if(max.z < p0.z) max.z = p0.z;
	}
	return (BBox *) GeomCCreate(NULL,BBoxMethods(), CR_MIN, &min, CR_MAX, &max, NULL);
}

