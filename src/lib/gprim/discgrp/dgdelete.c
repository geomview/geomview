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

#include "discgrpP.h"

DiscGrpElList *
DiscGrpElListDelete (DiscGrpElList *exist)
{
    if (exist)	{
	if (exist->el_list)	OOGLFree(exist->el_list);
	OOGLFree(exist);
	}
    return NULL;
}

DiscGrp *
DiscGrpDelete(dg)
DiscGrp           *dg;
{
	if (dg)
	{
		if (dg->name)
			OOGLFree(dg->name);
		if (dg->comment)
			OOGLFree(dg->comment);
		if (dg->fsa)
			OOGLFree(dg->fsa);
		if (dg->gens) {
			if (dg->gens->el_list)
				OOGLFree(dg->gens->el_list);
			OOGLFree(dg->gens);
			}

		if (dg->nhbr_list) {
			if (dg->nhbr_list->el_list)
				OOGLFree(dg->nhbr_list->el_list);
			OOGLFree(dg->nhbr_list);
			}

		if (dg->big_list) {
			if (dg->big_list->el_list)
				OOGLFree(dg->big_list->el_list);
			OOGLFree(dg->big_list);
			}

		if (dg->geom && dg->geom != dg->ddgeom)
			GeomDelete(dg->geom);

		if (dg->ddgeom) GeomDelete(dg->ddgeom);
	}
	return NULL;
}
