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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

/*
 * Compute bounding box of an Inst
 */
#include "instP.h"

BBox *
InstBound(Inst *inst, Transform T, TransformN *TN)
{
  BBox *geombbox;
  Transform Tnew;
  GeomIter *it;

  if( inst == NULL || inst->geom == NULL)
    return NULL;

  if (T == NULL)
    T = TM_IDENTITY;

  /* Insts which tie themselves to particular locations have no
   * bounding box, either.
   */
  if( inst->location > L_LOCAL || inst->origin > L_LOCAL )
    return NULL;

  /* FIXME: if inst->ndaxis != NULL and/or TN != NULL we have to do
   * something here.
   *
   * If TN != NULL, then Tnew should be prepended to TN w.r.t. to
   * the default x,y,z space (because this is the way we interprete
   * 3d objects in Nd).
   *
   * GeomIterate() should not be necessary in this case; MMmh.
   */

  if (inst->ndaxis == NULL) {
    if (TN == NULL) {
      it = GeomIterate( (Geom *)inst, DEEP );
      geombbox = NULL;
      while(NextTransform(it, Tnew) > 0) {
	BBox *box;
		
	TmConcat( Tnew, T, Tnew );
	if((box = (BBox *)GeomBound( inst->geom, Tnew, TN)) != NULL) {
	  if(geombbox) {
	    BBoxUnion3(geombbox, box, geombbox);
	    GeomDelete((Geom *)box);
	  } else
	    geombbox = box;
	}
      }
    } else {
      TransformN *TnewN = TmNCopy(TN, NULL);
      static int dflt_axes[] = { 0, 1, 2, -1 };

      it = GeomIterate( (Geom *)inst, DEEP );
      geombbox = NULL;
      while(NextTransform(it, Tnew) > 0) {
	BBox *box;

	TmNCopy(TN, TnewN);
	TmNApplyT3TN(Tnew, dflt_axes, TnewN);
	if((box = (BBox *)GeomBound( inst->geom, NULL, TnewN)) != NULL) {
	  if(geombbox) {
	    BBoxUnion3(geombbox, box, geombbox);
	    GeomDelete((Geom *)box);
	  } else
	    geombbox = box;
	}
      }
      TmNDelete(TnewN);
    }
  } else {
    /* FIXME, TODO */
  }
    
  return geombbox;
}
