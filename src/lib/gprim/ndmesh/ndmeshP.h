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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef NDMESHPDEF
#define NDMESHPDEF

#include "geomclass.h"
#include "hpointn.h"
#include "ndmesh.h"

struct NDMesh {
	GEOMFIELDS
	int	flag;
	int 	seq;
	int	meshd;		/* mesh dimension */
	int	*mdim;		/* mesh size, indexed 0..meshd-1 */
	int	pdim;		/* dimension of space where mesh points live */
	HPointN	**p;		/* N-D vertices; dim[0] axis varies fastest */
	ColorA	*c;		/* vertex colors, indexed likewise */
	Point3	*u;		/* texture coordinates, should we need any */
};

/* NOTE that these meshes may have missing vertices; some of the p[]
 * and c[] entries may be NULL.
 */


/*
 * mesh indexing:
 *  given a mesh index vector (of length 'meshd') i[0] i[1] ... i[meshd-1]
 * the corresponding index into the p[] and c[] arrays is given by
 *  index = 0;  size = 1;
 *  for(k = 0; k < meshd; k++) { index += size*i[k];  size *= dim[k]; }
 */

#endif /* ! NDMESHPDEF */
