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

typedef	double  point4[4];
typedef	double  plane4[4];
typedef	double  matrix4[4][4];

void
matvecmul4(m,v1,v2)
matrix4 m;
point4 v1,v2;
{
    int i,j;
    double tmp[4], *tv1, *tv2;
 
    tv1 = (double *) v1; 
    tv2 = (double *) v2;
    for (i=0; i<4; ++i)
	for (tmp[i]=0, j = 0; j<4; ++j)
	    tmp[i] += m[i][j] * tv1[j];
    for (i=0; i<4; ++i)
	tv2[i] = tmp[i];
}

void
vecmatmul4(v1,m,v2)
matrix4 m;
point4 v1,v2;
{
    int i,j;
    double tmp[4], *tv1, *tv2;
 
    tv1 = (double *) v1; 
    tv2 = (double *) v2;
    for (i=0; i<4; ++i)
	for (tmp[i]=0, j = 0; j<4; ++j)
	    tmp[i] += m[j][i] * tv1[j];
    for (i=0; i<4; ++i)
	tv2[i] = tmp[i];
}

void
matmatmul4 (mat1, mat2, mat3)
matrix4 mat1, mat2, mat3;

{
    int i, j, k;
    double tmp[4];

    for (i = 0; i < 4; i++) {
	for (j = 0; j < 4; j++) {
	    tmp[j] = 0.0;
	    for (k = 0; k < 4; k++) {
		tmp[j] += mat1[i][k] * mat2[k][j];
	    }
	    mat3[i][j] = tmp[j];
	}
    }
}
