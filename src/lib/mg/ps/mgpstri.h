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
#ifndef PSTRI_H
#define PSTRI_H

void MGPS_startPS(FILE *outf, ColorA *col, double aspect, int width, int height);
void MGPS_finishPS(void);
void MGPS_polyline(CPoint3 *pts, int num, double width, int *col);
void MGPS_spolyline(CPoint3 *pts, int num, double width);
void MGPS_poly(CPoint3 *pts, int num, int *col);
void MGPS_epoly(CPoint3 *pts, int num, int *col,
		double ewidth, int *ecol);
void MGPS_spoly(CPoint3 *pts, int num);
void MGPS_sepoly(CPoint3 *pts, int num, double ewidth, int *ecol);

#endif
