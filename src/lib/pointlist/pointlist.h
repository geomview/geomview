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
#ifndef POINTLIST_H
#define POINTLIST_H

/* 
 * Call this to pointlist routines.
 */
void pointlist_init();


HPoint3 *PointList_get(Geom *geom, Transform T, int CoordSystem);
HPoint3 *PointList_fillin(Geom *geom, Transform T, 
                          int CoordSystem, HPoint3 *plist);
void PointList_set(Geom *geom, int CoordSystem, HPoint3 *plist);
int PointList_length(Geom *geom);


/* 
 * Flags for describing coordinates.  
 * POINTLIST_SELF	This geomview object
 * POINTLIST_PRIMITIVE	Lowest-level OOGL primitive
 */
#define POINTLIST_SELF 0
#define POINTLIST_PRIMITIVE 1

#endif
