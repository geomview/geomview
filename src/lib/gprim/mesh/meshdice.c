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

#include "meshP.h"

Mesh *
MeshDice(m, proc)
    Mesh           *m;
    int             (*proc) ();
{
	int             u, v;
	int             nu, nv;
	float           umin, umax, vmin, vmax;
	float           fu, fv;
	Point3          *n;
	HPoint3		*p;

	if (m && proc)
	{
		umin = m->umin;
		umax = m->umax;
		vmin = m->vmin;
		vmax = m->vmax;
		nu = m->nu;
		nv = m->nv;
		p = m->p;
		n = m->n;
		for (v = 0; v < nv; v++)
		{
			for (u = 0; u < nu; u++)
			{
				fu = umin + ((umax - umin) * u) / (nu - 1);
				fv = vmin + ((vmax - vmin) * v) / (nv - 1);
				(*proc) (fu, fv, p, n);
				if (n)
					n++;
				p++;
			}
		}
	}
	return m;
}
