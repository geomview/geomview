/* Copyright (C) 2006 Claus-Justus Heine
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

/* N-transform of an NPolylist */

#include "npolylistP.h"

NPolyList *NPolyListTransform(NPolyList *np, Transform dummy, TransformN *T)
{
  (void)dummy;
  
  if (T != NULL && T != (void *)TM_IDENTITY) {
    HPointN *tmp = HPtNCreate(np->pdim, NULL);
    float *tmp_data = tmp->v;
    int i;
      
    for (tmp->v = np->v, i = 0; i < np->n_verts; i++, tmp->v += np->pdim) {
      HPtNTransform(T, tmp, tmp);
    }
    
    tmp->v = tmp_data;
    HPtNDelete(tmp);
  }
  
  return np;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
