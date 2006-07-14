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

#include "discgrpP.h"

static int constraint_depth;
static float constraint_stored, constraint_printd;

void
DiscGrpInitStandardConstraint(int depth, float ind0, float ind1)
{
    constraint_depth = depth;
    constraint_stored = ind0;
    constraint_printd = ind1;
}

	static HPoint3 origin = {0.0, 0.0, 0.0, 1.0};
int
DiscGrpStandardConstraint(DiscGrpEl *dgel)
{
 	int big=0, l;
	float d;
	HPoint3 image;
	int metric = dgel->attributes & DG_METRIC_BITS;
	extern double getnorm();
	
	if ((l = strlen(dgel->word)) > constraint_depth) 
		return(DG_CONSTRAINT_LONG);
	if (l == constraint_depth) big |= DG_CONSTRAINT_MAXLEN;

	/* find hyperbolic distance of image from origin: take short cut */
	HPt3Transform(dgel->tform, &origin, &image);
	d = HPt3SpaceDistance(&origin, &image, metric);

	if ( d < constraint_stored)	{
	    big |= DG_CONSTRAINT_STORE;
	    if ( d < constraint_printd)	{
	        big |= DG_CONSTRAINT_PRINT;
		}
	    }
	else		{	
		big |= DG_CONSTRAINT_TOOFAR;
		}
		
	return(big);
}

