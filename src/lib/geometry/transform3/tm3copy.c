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
#include <math.h>
#include <string.h>

/*-----------------------------------------------------------------------
 * Function:	Tm3Copy
 * Description:	copy a transform
 * Args:	Tsrc: source transform  (INPUT)
 *		Tdst: destination transform (OUTPUT)
 * Returns:	nothing
 * Author:	hanrahan, mbp
 * Date:	Thu Aug  8 15:40:09 1991
 * Notes:	
 */
void
Tm3Copy( Tsrc, Tdst )
    Transform3 Tsrc, Tdst;
{
    memcpy( (char *)Tdst, (char *)Tsrc, sizeof(Transform3) );
}

/*-----------------------------------------------------------------------
 * Function:	Tm3Compare
 * Description:	compare 2 transforms
 * Args:	T1, T2: the two transforms
 *		tol: tolerance
 * Returns:	1 if equal, 0 if not
 * Author:	hanrahan, mbp
 * Date:	Thu Aug  8 15:40:09 1991
 * Notes:	"equal" means corresponding elements are within tol of
 *		each other.
 */
int
Tm3Compare( Transform3 T1, Transform3 T2, float tol )
{
    int i,j;
    double d;
    for (i=0; i<4; ++i)
      for (j=0; j<4; ++j) {
	d = T1[i][j] - T2[i][j];
	if (fabs(d) > tol) {
	  return 0;
	}
      }
    return 1;
}

