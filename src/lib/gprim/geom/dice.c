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

#include "geomclass.h"

Geom *
GeomDice( Geom *g, int nu, int nv )
{
    if (g == NULL) {
	return NULL;
    }
    
    if(g->ap && g->ap->valid & APF_DICE) {
	nu = g->ap->dice[0];	/* Propagate "dice" appearance values */
	nv = g->ap->dice[1];	/* Note we ignore overrides, so this may not be
				 * correct in some cases.
				 */
    }
    if(g->Class->dice ) {
	(*g->Class->dice)(g, nu, nv);
    }
    
    return g;
}
/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
	    
