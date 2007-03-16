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
#ifndef SPHERE_H
#define SPHERE_H

#include "geomclass.h"

typedef struct Sphere Sphere;

extern GeomClass *SphereClass;

void SphereMinMax(Sphere *sphere, HPoint3 *min, HPoint3 *max);
void SphereCenter(Sphere *sphere, HPoint3 *center);
float SphereRadius(Sphere *sphere);
void SphereSwitchSpace(Sphere *sphere, int space);

/* Makes sphere encompassing both other spheres - new sphere if dest == NULL,
 * just puts results in dest otherwise */
Sphere *SphereUnion3(Sphere *a, Sphere *b, Sphere *dest);

/* points is actually a (2*dim)-element array containing the points with
 * the minimum and maximum x, y, and z coordinates. */
void SphereEncompassBoundsN(Sphere *sphere, HPoint3 *points, int dim);
void SphereEncompassBounds(Sphere *sphere, HPoint3 *points);

/* Returns zero if didn't change sphere, non-zero if did */
int SphereAddHPt3(Sphere *sphere, HPoint3 *point, Transform T);
int SphereAddHPt3N(Sphere *sphere, HPoint3 *point, int n, Transform T);
int SphereAddHPtN(Sphere *sphere, HPointN *point,
		  Transform T, TransformN *TN, int *axes);
int SphereAddHPtNN(Sphere *sphere, HPointN **point, int n,
		   Transform T, TransformN *TN, int *axes);
int SphereAddPoint(Sphere *sphere, float *point, int dim, int pdim,
		   Transform T, TransformN *TN, int *axes);
int SphereAddPoints(Sphere *sphere,
		    float *point, int dim, int pdim, int n,
		    Transform T, TransformN *TN, int *axes);

/* Modifies sphere to surround all points */
void SphereEncompassHPt3N(Sphere *sphere, HPoint3 *point, int n, Transform T);
void SphereEncompassHPtNN(Sphere *sphere, HPointN **point, int n,
			  Transform T, TransformN *TN, int *axes);
void SphereEncompassPoints(Sphere *sphere,
			   float *points, int dim, int pdim, int n,
			   Transform T, TransformN *TN, int *axes);
/* Finds points with the maximum dimensional span - spanPts is an array
 * of 6 elements (2 points per axis) */
void MaxDimensionalSpan(HPoint3 *spanPts, HPoint3 *point);
void MaxDimensionalSpanN(HPoint3 *spanPts, HPoint3 *points, int n);
void MaxDimensionalSpanHPtN(HPointN **spanPts, HPointN *pt);
void MaxDimensionalSpanHPtNN(HPointN **spanPts, HPointN **points, int n);
void MaxNDimensionalSpanN(HPointN **spanPts,
			  float *points, int dim, int pdim, int n);

#endif

