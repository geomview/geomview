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
#ifndef _GV_BSPTREE_H_
#define _GV_BSPTREE_H_

#include "geomclass.h"

typedef struct BSPTree BSPTree;

extern void BSPTreeCreate(Geom *object);
extern void BSPTreeAddObject(BSPTree *bsp, Geom *object);
extern void BSPTreeFinalize(BSPTree *bsp);
extern void BSPTreeFree(Geom *object);

#endif

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
