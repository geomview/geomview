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

#include "mgP.h"
#include "mgx11P.h"
#include "mgx11shade.h"

extern void mgx11_linewidth(short);

void
mgx11_appearance( struct mgastk *ma, int mask )
{
  Appearance *ap = &(ma->ap);

  if (mask & APF_LINEWIDTH) {
    mgx11_linewidth(ap->linewidth);
    _mgc->has &= ~HAS_POINT;
  }

  if (mask & APF_SHADING) {
/*
    if (ap->shading == APF_SMOOTH)
      ap->shading = APF_FLAT;
  bye bye to kludge in non-gouraud shading mgx11
*/
    if (ma->shader != NULL && IS_SHADED(ap->shading)) {
      /* Use software shader if one exists and user wants lighting */
      ma->flags |= MGASTK_SHADER;
    } else {
	/* No software shading, just use raw colors */
      ma->flags &= ~MGASTK_SHADER;
    }
  }

}

void
mgx11_setshader(mgshadefunc shader)
{
    struct mgastk *ma = _mgc->astk;
    unsigned short int wasusing = ma->flags & MGASTK_SHADER;

    ma->shader = shader;
    if (shader != NULL && IS_SHADED(ma->ap.shading)) {
      ma->flags |= MGASTK_SHADER;
    } else {
      ma->flags &= ~MGASTK_SHADER;
    }
    if((ma->flags & MGASTK_SHADER) != wasusing)
	mgx11_appearance(_mgc->astk, APF_SHADING);
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
