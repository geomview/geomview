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


/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "transform3.h"


/*-----------------------------------------------------------------------
 * Function:	Tm3Invert
 * Description:	compute the inverse of a Transform
 * Args:	m: the transform (INPUT)
 *		mi: the inverse transform (OUTPUT)
 * Returns:	nothing
 * Author:	njt
 * Date:	Sat Jul 18 12:04:19 CDT 1992
 */
float
Tm3Invert(Transform3 m, Transform3 mi)
{
	int i, j;
	int k;
	float x;
	Transform3 t;
	float f;
	Tm3Copy(m, t);
	Tm3Copy(TM3_IDENTITY, mi);

	/* Components of unrolled inner loops: */
#define	SUB(v,k)  v[j][k] -= f*v[i][k]
#define	SWAP(v,k) x = v[i][k], v[i][k] = v[largest][k], v[largest][k] = x


	for (i = 0; i < 4; i++) {
		int largest = i;
		float largesq = t[i][i]*t[i][i];
		for (j = i+1; j < 4; j++)
			if ((x = t[j][i]*t[j][i]) > largesq)
				largest = j,  largesq = x;

		/* swap t[i][] with t[largest][] */
		SWAP(t,0);  SWAP(t,1);  SWAP(t,2);  SWAP(t,3);
		SWAP(mi,0); SWAP(mi,1); SWAP(mi,2); SWAP(mi,3);

		for (j = i+1; j < 4; j++) {
			f = t[j][i] / t[i][i];
			/* subtract f*t[i][] from t[j][] */
			SUB(t,0); SUB(t,1); SUB(t,2); SUB(t,3);
			SUB(mi,0); SUB(mi,1); SUB(mi,2); SUB(mi,3);
		}
	}
	for (i = 0; i < 4; i++) {
		f = t[i][i];
		for (k = 0; k < 4; k++) {
			t[i][k] /= f;
			mi[i][k] /= f;
		}
	}
	for (i = 3; i >= 0; i--)
		for (j = i-1; j >= 0; j--) {
			f = t[j][i];
			SUB(t,0); SUB(t,1); SUB(t,2); SUB(t,3);
			SUB(mi,0); SUB(mi,1); SUB(mi,2); SUB(mi,3);
		}
				
	return 1;
}
