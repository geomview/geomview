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
#include <math.h>
typedef	double  point4[4];
typedef	double  plane4[4];
typedef	double  matrix4[4][4];

#define	ABS(aaa)	(( (aaa) > 0) ? (aaa) : -(aaa))
#define	VDOT3(v0,v1)	(v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2])
#define	INPRO31(v0,v1)	(v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2] - v0[3]*v1[3])
#define	INPRO4(v0,v1)	(v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2] + v0[3]*v1[3])

#define MAGNITUDE3(v0)	(sqrt (VDOT3(v0, v0)))
#define MAGNITUDE31(v0)	(sqrt (ABS (INPRO31(v0, v0))))
#define MAGNITUDE4(v0)	(sqrt (INPRO4(v0, v0)))

#define PROJECT4(v0)	\
	{float d = v0[3];	\
	if (d)	{	\
	d = 1.0/d;	\
	v0[0] = v0[0] * d;	\
	v0[1] = v0[1] * d;	\
	v0[2] = v0[2] * d;	\
	v0[3] = 1.0; }}	

#define NORMALIZE3(v0)	\
	{double xxx;		\
	if (xxx = MAGNITUDE3(v0))	\
	{v0[0] /= xxx;  v0[1] /= xxx;  v0[2] /= xxx; }  }

#define 	NORMALIZE31(v)	\
 	{double tmp = INPRO31(v,v);		\
	if (tmp) { tmp = 1.0/sqrt(ABS(tmp)); v[0] *= tmp; v[1] *= tmp; v[2] *= tmp; v[3] *= tmp; }}

#define 	NORMALIZE4(v)	\
 	{double tmp = INPRO4(v,v);		\
	if (tmp) { tmp = 1.0/sqrt(ABS(tmp)); v[0] *= tmp; v[1] *= tmp; v[2] *= tmp; v[3] *= tmp; }}

#define VPRINT3(name,v)		printf("name\t%f %f %f\n",v[0],v[1],v[2]);
#define VPRINT4(name,v)		printf("name\t%f %f %f %f %f\n",v[0],v[1],v[2],v[3]);
#define VADD3(v0,v1,v2)	\
	{v2[0] = v0[0] + v1[0]; \
	v2[1] = v0[1] + v1[1]; \
	v2[2] = v0[2] + v1[2]; \
	v2[3] = 1.0; }
	
#define VADD4(v0,v1,v2)	\
	{v2[0] = v0[0] + v1[0]; \
	v2[1] = v0[1] + v1[1]; \
	v2[2] = v0[2] + v1[2]; \
	v2[3] = v0[3] + v1[3]; }

#define VSUB3(v0,v1,v2)	\
	{v2[0] = v0[0] - v1[0]; \
	v2[1] = v0[1] - v1[1]; \
	v2[2] = v0[2] - v1[2]; \
	v2[3] = 1.0;}
	
#define VSUB4(v0,v1,v2)	\
	{v2[0] = v0[0] - v1[0]; \
	v2[1] = v0[1] - v1[1]; \
	v2[2] = v0[2] - v1[2]; \
	v2[3] = v0[3] - v1[3];}

#define MIDPOINT4(v0,v1,v2)	\
	{v2[0] = (v0[0] + v1[0])/2.0; \
	v2[1] = (v0[1] + v1[1])/2.0; \
	v2[2] = (v0[2] + v1[2])/2.0; \
	v2[3] = (v0[3] + v1[3])/2.0; }

#define VISZERO4(v)	(v[0] == 0 && v[1] == 0 && v[2] == 0 && v[3] == 0)

#define MIDPOINT31(v0,v1,v2)	\
	{float m00, m11, a,v3[4];	\
	VSUB4(v0, v1, v3);	\
	if (VISZERO4(v3))	\
		VCOPY4(v0,v2)	\
	else	{		\
	m00 =  INPRO31(v0,v3);	\
	m11 = INPRO31(v1,v3);	\
	v2[0] = m11*v0[0] - m00*v1[0]; \
	v2[1] = m11*v0[1] - m00*v1[1]; \
	v2[2] = m11*v0[2] - m00*v1[2]; \
	v2[3] = m11*v0[3] - m00*v1[3]; }}

#define XPRO3(v0,v1,v2)	\
	{v2[0] = v0[1]*v1[2] - v0[2]*v1[1];	\
	v2[1] = v0[2]*v1[0] - v0[0]*v1[2];	\
	v2[2] = v0[0]*v1[1] - v0[1]*v1[0];  }

#define VSCALE3(v0, s, v1)	\
	{v1[0] = s * v0[0];	\
	v1[1] = s * v0[1];	\
	v1[2] = s * v0[2];	}

#define VSCALE4(v0, s, v1)	\
	{v1[0] = s * v0[0];	\
	v1[1] = s * v0[1];	\
	v1[2] = s * v0[2];	\
	v1[3] = s * v0[3];	}

#define VCOPY3(v0, v1)	\
	{v1[0] = v0[0];	\
	v1[1] = v0[1];	\
	v1[2] = v0[2];	}

#define VCOPY4(v0, v1)	\
	{v1[0] = v0[0];	\
	v1[1] = v0[1];	\
	v1[2] = v0[2];	\
	v1[3] = v0[3];}

#define VZERO3(v0)	\
	{v0[0] = 0;	\
	v0[1] = 0;	\
	v0[2] = 0;	\
	v0[3] = 1.0; }

#define COPYMAT4(m1, m2)	\
	memcpy(m2, m1, (sizeof(m1[0][0]) * 16));





