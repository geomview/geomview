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

/* routines for making sure matrix is orthogonal in Minkowski metric */
#include "3d.h"

/* row-orthogonalize an isometry of Minkowski space */
void
tuneup(Transform m1, int metric)
{

    HPt3SpaceNormalize((HPoint3 *)m1[0], metric);

    HPt3SpaceGramSchmidt((HPoint3 *)m1[0], (HPoint3 *)m1[1], metric);
    HPt3SpaceNormalize((HPoint3 *)m1[1], metric);

    HPt3SpaceGramSchmidt((HPoint3 *)m1[0], (HPoint3 *)m1[2], metric);
    HPt3SpaceGramSchmidt((HPoint3 *)m1[1], (HPoint3 *)m1[2], metric);
    HPt3SpaceNormalize((HPoint3 *)m1[2], metric);

    HPt3SpaceGramSchmidt((HPoint3 *)m1[0], (HPoint3 *)m1[3], metric);
    HPt3SpaceGramSchmidt((HPoint3 *)m1[1], (HPoint3 *)m1[3], metric);
    HPt3SpaceGramSchmidt((HPoint3 *)m1[2], (HPoint3 *)m1[3], metric);
    HPt3SpaceNormalize((HPoint3 *)m1[3], metric);
	
}

/*  following only works now with hyperbolic mode */
int
needstuneup(Transform m1)
{
    int i,j;
    float d;

    for (i=0; i<4; ++i)
	for (j=i; j<4; ++j)
	    {
    	    d =	m1[i][0] * m1[j][0] +
		m1[i][1] * m1[j][1] +
		m1[i][2] * m1[j][2] -
		m1[i][3] * m1[j][3]; 
	    if (i == 3) d *= -1;
	    if (fabs ( d - ( i == j ) ) > .01 ) return (1);
	    }
    return (0);
}
