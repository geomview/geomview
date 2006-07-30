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

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Geometry Technologies, Inc.";
#endif

/*
 *
 * BasicFns.c
 * author: Brian T. Luense
 * date: August 18,1994
 *
 */

#include <stdlib.h>
#include <math.h>
#include "hpointn.h"
#include "hpoint3.h"

/*
 * BasicFns used in several routines I wrote this summer for NDview
 */

/*Takes two pointers to floats and an integer specifying the dimension of each.
Returns the standard euclidean inner product*/

float InnerProductN(float *a, float *b, int dim)
{	float sum=0.0;
	int i;
	for(i=0;i<dim;i++)
		sum+=(a[i]*b[i]);
	return sum;
}

/*Takes a pointer to a float and an integer dimension.  Returns
the Euclidean norm of an n-dimensional vector*/

float NormN(float *a, int dim)
{	float sum=0.0;
	int i;
	for(i=0;i<dim;i++)
		sum+=(a[i]*a[i]);
	return (sqrt(sum));
}

/*Takes a pointer to an HPoint3, a pointer to an HPointN, and an integer signifying
the inherent dimension of the HPoint3 (assumes either 3 or 4)
Extracts point data from the HPoint3 and puts it into the place data of the HPointN.
Assumes space has already been allocated in destination*/

HPointN *extract(HPoint3 *source, HPointN *destination, int dimsource)
{	float *temp;
	if(dimsource==3)
	{	temp=(float *)malloc((sizeof *temp)*4);
		temp[0]=source->x;
		temp[1]=source->y;
		temp[2]=source->z;
		temp[3]=source->w;
	}
	else
	{	temp=(float *)malloc((sizeof *temp)*5);
		temp[0]=source->x;
                temp[1]=source->y;
                temp[2]=source->z;
                temp[3]=source->w;
		temp[4]=1.0;
	}
	destination->v=temp;
	return destination;
}

/*Places the first four coordinate values of an HPointN into a given HPoint3*/

HPoint3 *place(HPointN *source, HPoint3 *destination)
{	float *ptr;
	ptr=source->v;
	destination->x=ptr[0];
	destination->y=ptr[1];
	destination->z=ptr[2];
	destination->w=ptr[3];
	return destination;
}
