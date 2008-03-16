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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips,
   Celeste Fowler */

#include "skelP.h"

Skel *SkelTransform( Skel *s, Transform T, TransformN *TN )
{
#if 1
  (void)T;
  (void)TN;
#else
  if (T && T != TM_IDENTITY) {

    if(s->pdim == 4) {
      HPt3TransformN(T, (HPoint3 *)s->p, (HPoint3 *)s->p, s->nvert);
    } else if (s->pdim > 4) {
      HPointN *tmp = HPtNCreate(s->dim, NULL);
      float *tmp_data = tmp->v;
      int i;
      
      for (tmp->v = s->p, i = 0; i < s->nvert; i++, tmp->v += s->dim) {
	HPtNTransform(T, tmp, tmp);
      }

      tmp->v = tmp_data;
      HPtNDelete(tmp);      
    }
  } else {   
    OOGLError(1, "Can't SkelTransform() %d-D Skel", s->dim);
  }
#endif
  return s;
}
