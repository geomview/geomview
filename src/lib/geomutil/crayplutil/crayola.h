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
#ifndef CRAYOLA_H
#define CRAYOLA_H

#include "color.h"

/* 
 * Default color 
 */
#define CRAY_DEFR 1.0
#define CRAY_DEFG 1.0
#define CRAY_DEFB 1.0
#define CRAY_DEFA 1.00

extern ColorA crayDefColor;

/* 
 * Initialize the extended routines
 */
void crayolaInit();

int crayHasColor(Geom *geom, int *gpath);
int crayHasVColor(Geom *geom, int *gpath);
int crayHasFColor(Geom *geom, int *gpath);

int crayCanUseVColor(Geom *geom, int *gpath);
int crayCanUseFColor(Geom *geom, int *gpath);

int crayUseVColor(Geom *geom, ColorA *def, int *gpath);
int crayUseFColor(Geom *geom, ColorA *def, int *gpath);

int crayEliminateColor(Geom *geom, int *gpath);

/* 
 * The point itself is last because no one except beziers really cares
 * about it so we won't force them to read past it in the arguement list.
 */
int craySetColorAll(Geom *geom, ColorA *color, int *gpath);
int craySetColorAt(Geom *geom, ColorA *color, int vindex, int findex,
		   int *edge, int *gpath, HPoint3 *pt);
int craySetColorAtV(Geom *geom, ColorA *color, int index, int *gpath,
		    HPoint3 *pt);
int craySetColorAtF(Geom *geom, ColorA *color, int index, int *gpath);

int crayGetColorAt(Geom *geom, ColorA *color, int vindex, int findex,
		   int *edge, int *gpath, HPoint3 *pt);
int crayGetColorAtV(Geom *geom, ColorA *color, int index, int *gpath,
		    HPoint3 *pt);
int crayGetColorAtF(Geom *geom, ColorA *color, int index, int *gpath);

#endif
