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

#include <stdio.h>
#include <string.h>
#include "transform3.h"

/************************************************************************
 * NOTE: we're taking out PostConcat and PreConcat.  They're being left	*
 * here for a short while longer as a memorial.				*
 ************************************************************************/

/*-----------------------------------------------------------------------
 * Function:	Tm3PostConcat
 * Description:	concatenate one transform to another on the right
 * Args:	Ta: left transform	(INPUT & OUTPUT)
 *		Tb: right transform	(INPUT)
 * Returns:	nothing
 * Author:	hanrahan, mbp
 * Date:	Thu Aug  8 13:31:35 1991
 * Notes:	Sets Ta to Ta * Tb.
 */
void
Tm3PostConcat( Ta, Tb )
     Transform3 Ta, Tb;
{
  fprintf(stderr, "WARNING: obsolete function Tm3PostConcat called.  Use\n\
Tm3Concat instead.\n");
  Tm3Concat( Ta, Tb, Ta );
}

/*-----------------------------------------------------------------------
 * Function:	Tm3PreConcat
 * Description:	concatenate one transform to another on the left
 * Args:	Ta: left transform	(INPUT)
 *		Tb: right transform	(INPUT & OUTPUT)
 * Returns:	nothing
 * Author:	hanrahan, mbp
 * Date:	Thu Aug  8 13:31:35 1991
 * Notes:	Sets Tb to Ta * Tb.
 */
void
Tm3PreConcat( Ta, Tb )
    Transform3 Ta, Tb;
{
  fprintf(stderr, "WARNING: obsolete function Tm3PreConcat called.  Use\n\
Tm3Concat instead.\n");
    Tm3Concat( Ta, Tb, Tb );
}




/*-----------------------------------------------------------------------
 * Function:	Tm3Concat
 * Description:	concatenate two transforms
 * Args:	Ta: left factor  (INPUT)
 *		Tb: right factor (INPUT)
 *		Tprod: product (OUTPUT)
 * Returns:	nothing
 * Author:	hanrahan, mbp
 * Date:	Thu Aug  8 13:15:08 1991
 * Notes:	Passing the same transform for either factor and the
 *		product is allowed.
 */
void
Tm3Concat( Ta, Tb, Tprod )
  Transform3 Ta, Tb, Tprod;
{
  int i;
  
#define MAKEPRODUCT(T)				\
    for( i=0; i<4; i++ ) {			\
	T[i][0] = Ta[i][0]*Tb[0][0] +		\
		  Ta[i][1]*Tb[1][0] +		\
		  Ta[i][2]*Tb[2][0] +		\
		  Ta[i][3]*Tb[3][0];		\
	T[i][1] = Ta[i][0]*Tb[0][1] +		\
		  Ta[i][1]*Tb[1][1] +		\
		  Ta[i][2]*Tb[2][1] +		\
		  Ta[i][3]*Tb[3][1];		\
	T[i][2] = Ta[i][0]*Tb[0][2] +		\
		  Ta[i][1]*Tb[1][2] +		\
		  Ta[i][2]*Tb[2][2] +		\
		  Ta[i][3]*Tb[3][2];		\
	T[i][3] = Ta[i][0]*Tb[0][3] +		\
		  Ta[i][1]*Tb[1][3] +		\
		  Ta[i][2]*Tb[2][3] +		\
		  Ta[i][3]*Tb[3][3];		\
      }

  if( Ta == Tprod || Tb == Tprod ) {
    Transform3 T;
    MAKEPRODUCT(T);
    memcpy( Tprod, T, sizeof(Transform3) );
  }
  else {
    MAKEPRODUCT(Tprod);
  }

#undef MAKEPRODUCT
}
