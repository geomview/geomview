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
#ifndef _complex_
#define _complex_

#include <math.h>

typedef struct {
	double real, imag;
} complex;

typedef complex sl2c_matrix[2][2];

#ifndef PROTOTYPES
Please #include options.h BEFORE complex.h.
#endif

#if PROTOTYPES
extern complex	cplx_minus(complex, complex),
				cplx_plus(complex, complex),
				cplx_mult(complex, complex),
				cplx_div(complex, complex);
extern double	modulus(complex);
extern complex	cplx_sqrt(complex);
extern void		sl2c_mult(sl2c_matrix, sl2c_matrix, sl2c_matrix);
extern void		sl2c_copy(sl2c_matrix, sl2c_matrix);
extern void		sl2c_normalize(sl2c_matrix);
extern void		sl2c_invert(sl2c_matrix, sl2c_matrix);
extern double	sl2c_norm_squared(sl2c_matrix);
extern void		sl2c_adjoint(sl2c_matrix, sl2c_matrix);
extern double	acosh(double);
#else
extern complex	cplx_minus(),
				cplx_plus(),
				cplx_mult(),
				cplx_div();
extern double	modulus();
extern complex	cplx_sqrt();
extern double	sl2c_norm_squared();
/*extern double	acosh();*/
#endif

extern complex zero;
extern complex one;

#endif
