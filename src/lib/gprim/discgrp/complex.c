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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

#include <stdio.h>
#include "options.h"
#include "complex.h"

complex one		= {1.0, 0.0};
complex zero	= {0.0, 0.0};

complex cplx_plus(z0, z1)
complex z0, z1;
{
	complex sum;
	
	sum.real = z0.real + z1.real;
	sum.imag = z0.imag + z1.imag;
	return(sum);
}


complex cplx_minus(z0, z1)
complex z0, z1;
{
	complex diff;
	
	diff.real = z0.real - z1.real;
	diff.imag = z0.imag - z1.imag;
	return(diff);
}


complex cplx_div(z0, z1)
complex z0, z1;
{
	double	mod_sq;
	complex	quotient;

	mod_sq			=  z1.real * z1.real  +  z1.imag * z1.imag;
	quotient.real	= (z0.real * z1.real  +  z0.imag * z1.imag)/mod_sq;
	quotient.imag	= (z0.imag * z1.real  -  z0.real * z1.imag)/mod_sq;
	return(quotient);
}


complex cplx_mult(z0, z1)
complex z0, z1;
{
	complex product;

	product.real	= z0.real * z1.real  -  z0.imag * z1.imag;
	product.imag	= z0.real * z1.imag  +  z0.imag * z1.real;
	return(product);
}


double modulus(z0)
complex z0;
{
	return( sqrt(z0.real * z0.real  +  z0.imag * z0.imag) );
}


complex cplx_sqrt(z)
complex z;
{
	double	mod,
			arg;
	complex	result;

	mod = sqrt(modulus(z));
	if (mod == 0.0)
		return(zero);
	arg = 0.5 * atan2(z.imag, z.real);
	result.real = mod * cos(arg);
	result.imag = mod * sin(arg);
	return(result);
}



void sl2c_mult(a, b, product)
sl2c_matrix	a,
			b,
			product;
{
	sl2c_matrix	temp;

	temp[0][0] = cplx_plus(cplx_mult(a[0][0], b[0][0]),  cplx_mult(a[0][1], b[1][0]));
	temp[0][1] = cplx_plus(cplx_mult(a[0][0], b[0][1]),  cplx_mult(a[0][1], b[1][1]));
	temp[1][0] = cplx_plus(cplx_mult(a[1][0], b[0][0]),  cplx_mult(a[1][1], b[1][0]));
	temp[1][1] = cplx_plus(cplx_mult(a[1][0], b[0][1]),  cplx_mult(a[1][1], b[1][1]));
	product[0][0] = temp[0][0];
	product[0][1] = temp[0][1];
	product[1][0] = temp[1][0];
	product[1][1] = temp[1][1];
	return;
}


void sl2c_copy(a, b)
sl2c_matrix	a,
			b;
{
	a[0][0] = b[0][0];
	a[0][1] = b[0][1];
	a[1][0] = b[1][0];
	a[1][1] = b[1][1];
	return;
}


/* normalizes a matrix to have determinant one */
void sl2c_normalize(a)
sl2c_matrix a;
{
	complex det,
			factor;
	double	arg,
			mod;

	/* compute determinant */
	det = cplx_minus(cplx_mult(a[0][0], a[1][1]),  cplx_mult(a[0][1], a[1][0]));
	if (det.real == 0.0 && det.imag == 0.0) {
		printf("singular sl2c_matrix\n");
		exit(0);
	}

	/* convert to polar coordinates */
	arg = atan2(det.imag, det.real);
	mod = modulus(det);

	/* take square root */
	arg *= 0.5;
	mod = sqrt(mod);

	/* take reciprocal */
	arg = -arg;
	mod = 1.0/mod;

	/* return to rectangular coordinates */
	factor.real = mod * cos(arg);
	factor.imag = mod * sin(arg);

	/* normalize matrix */
	a[0][0] = cplx_mult(a[0][0], factor);
	a[0][1] = cplx_mult(a[0][1], factor);
	a[1][0] = cplx_mult(a[1][0], factor);
	a[1][1] = cplx_mult(a[1][1], factor);

	return;
}


/* inverts a matrix;  assumes determinant is already one */
void sl2c_invert(a, a_inv)
sl2c_matrix a,
			a_inv;
{
	complex	temp;

	temp = a[0][0];
	a_inv[0][0] = a[1][1];
	a_inv[1][1] = temp;

	a_inv[0][1].real = -a[0][1].real;
	a_inv[0][1].imag = -a[0][1].imag;

	a_inv[1][0].real = -a[1][0].real;
	a_inv[1][0].imag = -a[1][0].imag;

	return;
}


/* computes the square of the norm of a matrix */
/* relies on the assumption that the sl2c matrix is stored in memory as 8 consecutive doubles */
/* IS THIS RELIABLE? */
double sl2c_norm_squared(a)
sl2c_matrix a;
{
	int	i;
	double	*p;
	double	sum;

	p = (double *) a;
	sum = 0.0;
	for (i=8; --i>=0; ) {
		sum += *p * *p;
		p++;
	}
	return(sum);
}


void sl2c_adjoint(a, ad_a)
sl2c_matrix	a,
			ad_a;
{
	complex	temp;

	/* transpose */
	temp = a[0][1];
	ad_a[0][0] = a[0][0];
	ad_a[0][1] = a[1][0];
	ad_a[1][0] = temp;
	ad_a[1][1] = a[1][1];

	/* conjugate */
	ad_a[0][0].imag = -ad_a[0][0].imag;
	ad_a[0][1].imag = -ad_a[0][1].imag;
	ad_a[1][0].imag = -ad_a[1][0].imag;
	ad_a[1][1].imag = -ad_a[1][1].imag;

	return;
}
 
