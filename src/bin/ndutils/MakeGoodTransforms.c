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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "hpointn.h"

#define EPSILON .000001

static int gram_schmidt( TransformN *T, float **order, int dimension);

int return_orthonormal_matrix(TransformN *T, int xdim, int ydim, int zdim, int dimension)
{
    float **rearranged;
    int j = 0, k;
    srand(time(NULL));
    rearranged = calloc(dimension, sizeof(float *));
    rearranged[j++] = &(T->a[zdim*(dimension+1)]);
    rearranged[j++] = &(T->a[xdim*(dimension+1)]);
    rearranged[j++] = &(T->a[ydim*(dimension+1)]);
    for ( k = 0; k < dimension; k++)
        if ( k != xdim && k != ydim && k != zdim )
            rearranged[j++] = &(T->a[k*(dimension+1)]);
    while ( gram_schmidt(T,rearranged,dimension) == 0) {
        /* got an almost zero matrix, jiggle and try again! */
        for(j=0; j< dimension; j++ )
           for(k=0; k< dimension; k++ )

                rearranged[j][k] +=  EPSILON*((float)rand())/(2147483648.0);
  /* RAND_MAX +1 */
    }
    free(rearranged);
    return 1;
}

static int gram_schmidt( TransformN *T, float **order, int dimension)
{
	float *a, r, *point;
	int i, j, k, currentrow = 0;
	a = order[currentrow];
	for ( k = 0; k< dimension; k++) {
		for( r = 0.0, i = dimension; i; i--, a++)
			r += *a * *a;
		if ( r < EPSILON ) return 0;
		r = sqrt(r);
		a--;
		for (  i = dimension; i; i--)
			*a-- /= r;
		point = order[currentrow];
		for ( j = k + 1; j < dimension; j++) {
			r = 0.0;
			a = order[currentrow + j - k];
			for ( i = dimension; i; i--)
				r += *point++ * *a++;
			for ( i = dimension; i; i--)
				*(--a) -= *(--point) * r;
		}
		a = order[++currentrow];
	}
	return 1;
}
