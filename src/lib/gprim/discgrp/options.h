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
#ifndef _options_
#define _options_

#define MAC		0
#define SUN		1
#define VAX		2
#define CRAY	3
#define GENERIC	4

#define HOST	GENERIC

/*  LightspeedC has a few idiosyncracies which vary from one version
to the next.  If you are using LightspeedC (a.k.a. THINK C) define the
appropriate version number here (e.g. #define LSC 4 for version 4.0).
If you aren't using LightspeedC (and perhaps not even using a Mac at
all) you should #define LSC 0.  */

#if (HOST == MAC)
#define LSC 4
#else
#define LSC 0
#endif

#define PROTOTYPES	1

#if (LSC == 3)
#define _MC68881_
#endif

#if (HOST == MAC)
typedef long int	MyInt32;
#else
#include <stdlib.h>	/* to get size_t definition */
typedef size_t		MyInt32;
#endif

/* The constant HARDWARE_PRECISION gives the accuracy of a double	*/
/* precision floating point number on the host hardware.  I.e. if	*/
/* the mantissa of a double has b bits, HARDWARE_PRECISION is 2^-b.	*/
/* Note:  in the unlikely event that HARDWARE_PRECISION < 1e-21		*/
/* you'll have to provide more coefficients in the series for the	*/
/* Lobachevsky function in volume.c.								*/
#if (HOST == MAC)
#define HARDWARE_PRECISION	6e-20
#else
#define HARDWARE_PRECISION	4e-15
#endif

#endif
