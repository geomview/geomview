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

#include "bboxP.h"

static void _BBoxCenter(BBox *bbox, HPtNCoord *center)
{
  int i;
  
  HPtNDehomogenize(bbox->min, bbox->min);
  HPtNDehomogenize(bbox->max, bbox->max);
  center[0] = 1.0;
  
  for (i = 1; i < bbox->pdim; i++) {
    center[i] = 0.5 * (bbox->min->v[i] + bbox->max->v[i]);
  }
}

HPointN *BBoxCenterND(BBox *bbox, HPointN *center)
{
  if (!center) {
    center = HPtNCreate(bbox->pdim, NULL);
  } else if (center->dim != bbox->pdim) {
    center->v= OOGLRenewNE(HPtNCoord, center->v, bbox->pdim, "renew HPointN");
    center->dim = bbox->pdim;
  }
      
  _BBoxCenter(bbox, (HPtNCoord *)center->v);

  return center;
}

void BBoxCenter(BBox *bbox, HPoint3 *center)
{
  HPointN *Ncenter;
  
  Ncenter = BBoxCenterND(bbox, NULL);

  HPtNToHPt3(Ncenter, NULL, center);
  
  HPtNDelete(Ncenter);
}


/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
