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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef COLORDEFS
#define COLORDEFS

typedef struct { float r, g, b;   } Color;
typedef struct { float r, g, b, a; } ColorA;

extern void CoPrint(Color *c);
extern void CoCopy(Color *c0, Color *c1);
extern void CoACopy(ColorA *c0, ColorA *c1);
extern void CoFrom(Color *c, float r, float g, float b);
extern void CoAdd(Color *c0, Color *c1, Color *c2);
extern void CoSub(Color *c0, Color *c1, Color *c2);
extern void CoFilter(Color *c0, Color *c1, Color *c2);

extern void hsv2rgb(Color *hsv, Color *rgb);
extern void rgb2hsv(Color *rgb, Color *hsv);

#endif
