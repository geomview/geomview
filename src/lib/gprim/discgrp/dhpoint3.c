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

#include <stdio.h>
#include "vec4.h" 
#include "dgflag.h"
#include "math.h"

double 
DHPt3Dot(point4 v0, point4 v1, int metric)
{
    switch (metric)	{
	case DG_EUCLIDEAN:
	    return(v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2] + v0[3]*v1[3]);
	    break;
	case DG_HYPERBOLIC:
	    return(v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2] - v0[3]*v1[3]);
	    break;
	case DG_SPHERICAL:
	    return(v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2] + v0[3]*v1[3]);
	    break;
	}
    return (double)0;
}

double 
DHPt3Dot3(point4 v0, point4 v1, int metric)
{
    switch (metric)	{
	case DG_EUCLIDEAN:
	    return(v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2]);
	    break;
	case DG_HYPERBOLIC:
	    return(v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2] - v0[3]*v1[3]);
	    break;
	case DG_SPHERICAL:
	    return(v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2] + v0[3]*v1[3]);
	    break;
    }
    return (double)0;

}

double
DHPt3Distance(point4 p0, point4 p1, int metric)
{
	double d0, d1;
	point4 diff;
    switch (metric)	{
	case DG_EUCLIDEAN:
	    VSUB3(p0,p1, diff);
	    return(MAGNITUDE3(diff));
	    break;
	case DG_HYPERBOLIC:
	    d0 =INPRO31(p0, p0);
	    d1 =INPRO31(p1, p1);
	    if (d0 >= 0.0 || d1 >= 0.0)	{
		fprintf(stderr,"Invalid points in dist_proj3\n");
		return(0.0);
		}
	    return(acosh( ABS( INPRO31(p0, p1) / sqrt(d0 * d1))));
	    break;
	case DG_SPHERICAL:
	    d0 =INPRO31(p0, p0);
	    d1 =INPRO31(p1, p1);
	    return(acos( ABS( INPRO4(p0, p1) / sqrt(d0 * d1))));
	    break;

	}
    return (double)0;
}

void
DHPt3PerpBisect(point4 p0, point4 p1, point4 result, int metric)
{
    switch (metric)	{
      case  DG_EUCLIDEAN:
	{
	point4 tmp;
	VSUB3(p1, p0, result)
	VADD3(p0, p1, tmp)
	VSCALE3(tmp, .5, tmp)
	result[3] = -(VDOT3(tmp, result));
	}
	break;

      case DG_HYPERBOLIC:
	NORMALIZE31(p0);
	NORMALIZE31(p1);
	VSUB4(p0, p1, result)
	if (INPRO31(p0, result) > 0.0)	VSCALE4(result, -1, result)
	break;

      case DG_SPHERICAL:
	NORMALIZE4(p0);
	NORMALIZE4(p1);
	VSUB4(p0, p1, result)
	if (INPRO4(p0, result) > 0.0)	VSCALE4(result, -1, result)
	break;

    }
}
	
	
    
