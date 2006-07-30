/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
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
#ifndef BASICFNS_H
#define BASICFNS_H

#include "hpointn.h"
#include "hpoint3.h"

/*Find the standard inner product of two n-dimensional vectors*/
float InnerProductN(float *a, float *b, int dim);

/*Find the Euclidean norm of an n-dimensional vector*/
float NormN(float *a, int dim);

/*Turn an HPoint3 into an HPointN.  assumes space has already been allocated in
destination*/
HPointN *extract(HPoint3 *source, HPointN *destination, int dimsource);

HPoint3 *place(HPointN *source, HPoint3 *destination);

#endif
