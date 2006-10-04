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

#include "ndmeshP.h"

/* Hack.  Use BBox_ND_hack() to compute a bounding box that conservatively
 * covers everything -- but ignoring the transform.
 */
NDMesh *NDMeshTransform(NDMesh *m, TransformN *T)
{
  if (T != NULL && T != (void *)TM_IDENTITY) {
    HPointN **p;
    int i, n = m->mdim[0] * m->mdim[1];
      
    for (p = m->p, i = 0; i < n; i++, p++) {
      HPtNTransform(T, *p, *p);
    }
  }
  
  return m;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
