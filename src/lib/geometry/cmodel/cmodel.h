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
/*
 * cmodel.h: declarations for functions dealing with
 *    conformal model of hyperbolic or spherical space
 */


#ifndef CMODEL_H
#define CMODEL_H

#include "polylistP.h"
#include "vectP.h"
#include "meshP.h"
#include "quadP.h"

void set_cm_refine(double cosmaxbend, int maxrefine, int show_subdivision);

void cmodel_clear(int space);
void cm_read_polylist(PolyList *polylist);
void cm_read_vect(Vect *v);
void cm_read_quad(Quad *q);
void cmodel_draw(int plflags);
void cm_draw_mesh(Mesh *m);

#endif /* CMODEL_H */
