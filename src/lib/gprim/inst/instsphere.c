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

#include "geom.h"
#include "create.h"
#include "instP.h"
#include "sphere.h"

Geom *InstBoundSphere(Inst *inst,
		      Transform T, TransformN *TN, int *axes, int space)
{
  Sphere *geomsphere, *sphere;
  Transform Tnew;
  GeomIter *it;

  if (inst == NULL || inst->geom == NULL)
    return NULL;

  if (inst->location > L_LOCAL || inst->origin > L_LOCAL)
    return NULL;

  if (T == NULL)
    T = TM_IDENTITY;

  if (inst->NDaxis == NULL) {
    if (TN == NULL) {
      it = GeomIterate((Geom *)inst, DEEP);
      geomsphere = NULL;
      while (NextTransform(it, Tnew) > 0) {
	TmConcat(Tnew, T, Tnew);
	sphere = (Sphere *)GeomBoundSphere(inst->geom, Tnew, NULL, NULL, space);
	if (sphere != NULL) {
	  if (geomsphere != NULL) {
	    SphereUnion3(geomsphere, sphere, geomsphere);
	    GeomDelete((Geom *)sphere);
	  } else geomsphere = sphere;
	}
      }
    } else {
      TransformN *TnewN = TmNCopy(TN, NULL);
      static int dflt_axes[] = { 1, 2, 3, 0 };

      it = GeomIterate((Geom *)inst, DEEP);
      geomsphere = NULL;
      while (NextTransform(it, Tnew) > 0) {

	/* If TN != NULL, then Tnew is first applied to the x,y,z
	 * sub-space of the object, then folded into TN, and finally
	 * the product is projected down to the sub-space defined by
	 * the "axes" argument.
	 */
	TmNCopy(TN, TnewN);
	TmNApplyT3TN(Tnew, dflt_axes, TnewN);
	sphere =
	  (Sphere *)GeomBoundSphere(inst->geom, NULL, TnewN,  axes, space);
	if (sphere != NULL) {
	  if (geomsphere != NULL) {
	    SphereUnion3(geomsphere, sphere, geomsphere);
	    GeomDelete((Geom *)sphere);
	  } else geomsphere = sphere;
	}
      }

      TmNDelete(TnewN);
    }
  } else {
    if (TN) {
      TransformN *TnewN;
      
      TnewN = TmNConcat(inst->NDaxis, TN, NULL);
      geomsphere =
	(Sphere *)GeomBoundSphere(inst->geom, NULL, TnewN, axes, space);
      TmNDelete(TnewN);
    } else {
      TransformN *TnewN = TmNCopy(inst->NDaxis, NULL);
      static int dflt_axes[] = { 1, 2, 3, 0 };

      TmNApplyDN(TnewN, dflt_axes, T);
      geomsphere =
	(Sphere *)GeomBoundSphere(inst->geom, NULL, TnewN, dflt_axes, space);
      TmNDelete(TnewN);
    }
  }

  return (Geom *)geomsphere;
}
