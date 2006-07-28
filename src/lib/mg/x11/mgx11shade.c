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
    if(!IS_SHADED(ap->shading) || ma->shader != NULL) {
	/* Use software shader if one exists and user wants lighting */
	ma->useshader = (ma->shader != NULL) && IS_SHADED(ap->shading);
    }
    else {
	/* No software shading, just use raw colors */
	ma->useshader = 0;
    }
  }

}

void
mgx11_setshader(mgshadefunc shader)
{
    struct mgastk *ma = _mgc->astk;
    int wasusing = ma->useshader;

    ma->shader = shader;
    ma->useshader = (shader != NULL && IS_SHADED(ma->ap.shading)) ? 1 : 0;
    if(ma->useshader != wasusing)
	mgx11_appearance(_mgc->astk, APF_SHADING);
}
