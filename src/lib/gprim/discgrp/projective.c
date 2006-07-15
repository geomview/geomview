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

#include "options.h"
#include "complex.h"
#include "projective.h"

/*	sl2c_to_proj() converts sl2c_matrices to proj_matrices.  It is based on
an explanation provided by Craig Hodgson of a program written by
Diane Hoffoss which in turn was based on an algorithm explained to her
by Bill Thurston.  */

void sl2c_to_proj(s, p)
sl2c_matrix	s;
proj_matrix	p;
{
	int			j;		/* which column of p	*/
	sl2c_matrix	ad_s,	/* s* = adjoint of s	*/
				fs,		/* f(s) = s m s*		*/
				temp;
	static sl2c_matrix	m[4] = {{{{ 0.0, 0.0},{ 0.0, 1.0}},
								 {{ 0.0,-1.0},{ 0.0, 0.0}}},

								{{{ 0.0, 0.0},{ 1.0, 0.0}},
								 {{ 1.0, 0.0},{ 0.0, 0.0}}},

								{{{-1.0, 0.0},{ 0.0, 0.0}},
								 {{ 0.0, 0.0},{ 1.0, 0.0}}},

								{{{ 1.0, 0.0},{ 0.0, 0.0}},
								 {{ 0.0, 0.0},{ 1.0, 0.0}}}};

	for (j=0; j<4; j++) {
		sl2c_adjoint(s, ad_s);
		sl2c_mult(s, m[j], temp);
		sl2c_mult(temp, ad_s, fs);
		p[0][j] = fs[0][1].imag;
		p[1][j] = fs[0][1].real;
		p[2][j] = 0.5 * (fs[1][1].real - fs[0][0].real);
		p[3][j] = 0.5 * (fs[1][1].real + fs[0][0].real);
	}

	return;
}


void proj_to_sl2c(p, s)
proj_matrix	p;
sl2c_matrix	s;
{
	double	t2,
			t3,
			aa,
			bb;

	/* Notation:  The entries of s are a, b, c, d (as is standard).	*/
	/* The complex conjugate of a is written as a' (read "a-bar").	*/

	/* Outline of algorithm:  Write down the four matrices {sM0s*,	*/
	/* sM1s*, sM2s*, sM3s*} in terms of {a, b, c, d}, and find		*/
	/* expressions for {2a'a, 2a'b, 2a'c, 2a'd} as differences of	*/
	/* matrix entries.  Express these differences as functions of	*/
	/* the entries of p.  The matrix (2a'a, 2a'b; 2a'c, 2a'd) is a	*/
	/* multiple of the desired matrix (a, b; c, d), so when we		*/
	/* normalize the former we get the latter (we'd have to			*/
	/* normalize in any case, since this scheme can find			*/
	/* (a, b; c, d) only up to multiplication by a complex number	*/
	/* of modulus one).												*/

	/* a may be zero, but a and b can't both be zero.  We'll use	*/
	/* the one with the bigger norm.								*/

	t2 = p[3][2] - p[2][2]; /* 00 entry of F(M2)	*/
	t3 = p[3][3] - p[2][3]; /* 00 entry of F(M3)	*/
	aa = t3 - t2;			/* aa = 2 * |a|^2		*/
	bb = t3 + t2;			/* bb = 2 * |b|^2		*/
	if (aa > bb) {
		s[0][0].real =   aa;
		s[0][0].imag =   0.0;
		s[0][1].real =   p[3][1] - p[2][1];
		s[0][1].imag =   p[3][0] - p[2][0];
		s[1][0].real =   p[1][3] - p[1][2];
		s[1][0].imag =   p[0][2] - p[0][3];
		s[1][1].real =   p[0][0] + p[1][1];
		s[1][1].imag =   p[1][0] - p[0][1];
	}
	else {
		s[0][0].real =   p[3][1] - p[2][1];
		s[0][0].imag =   p[2][0] - p[3][0];
		s[0][1].real =   bb;
		s[0][1].imag =   0.0;
		s[1][0].real =   p[1][1] - p[0][0];
		s[1][0].imag = - p[0][1] - p[1][0];
		s[1][1].real =   p[1][3] + p[1][2];
		s[1][1].imag = - p[0][2] - p[0][3];
	}

	sl2c_normalize(s);

	return;
}


void proj_mult(a, b, product)
proj_matrix	a,
			b,
			product;
{
	int	i,
					j,
					k;
	double	sum;
	proj_matrix		temp;

	for (i=0; i<4; i++)
		for (j=0; j<4; j++) {
			sum = 0.0;
			for (k=0; k<4; k++)
				sum += a[i][k] * b[k][j];
			temp[i][j] = sum;
		}

	for (i=0; i<4; i++)
		for (j=0; j<4; j++)
			product[i][j] = temp[i][j];

	return;
}


void proj_copy(a, b)
proj_matrix	a,
			b;
{
	int	i,
					j;

	for (i=0; i<4; i++)
		for (j=0; j<4; j++)
			a[i][j] = b[i][j];

	return;
}


/* proj_invert() assumes the matrix a is nonsingular, as will always	*/
/* be the case for matrices representing isometries of H^3.				*/

void proj_invert(m, m_inv)
proj_matrix	m,
			m_inv;
{
	int			i, j, k;
	double		scratch[4][8],
				*a[4],
				*temp;

	/* set up */
	for (i=4; --i>=0; ) {
		for (j=4; --j>=0; ) {
			scratch[i][j]	= m[i][j];
			scratch[i][j+4]	= (i == j) ? 1.0 : 0.0;
		}
		a[i] = scratch[i];
	}

	/* do the forward part of Gaussian elimination */
	for (j=0; j<4; j++) {
		/* find the best pivot */
		for (i=j+1; i<4; i++)
			if (fabs(a[i][j]) > fabs(a[j][j])) {
				temp = a[i];
				a[i] = a[j];
				a[j] = temp;
			}

		/* normalize row j */
		for (i=j+1; i<8; i++)
			a[j][i] /= a[j][j];

		/* clear the lower part of column j */
		for (i=j+1; i<4; i++)
			for (k=j+1; k<8; k++)
				a[i][k] -= a[i][j] * a[j][k];
	}

	/* do the back substitution */
	for (j=4; --j>=0; )
		for (i=j; --i>=0; )
			for (k=4; k<8; k++)
				a[i][k] -= a[i][j] * a[j][k];
			
	/* copy the answer into m_inv */
	for (i=4; --i>=0; )
		for (j=4; --j>=0; )
			m_inv[i][j] = a[i][j+4];

	return;
}

