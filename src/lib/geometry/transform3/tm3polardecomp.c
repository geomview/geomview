/* Copyright (C) 2006 Claus-Justus Heine
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

#include <math.h>
#include "transform3.h"

/* Compute the polar decomposition of a 3x3 matrix by means of a fast
 * Newton's iteration:
 *
 * A. A. DUBRULLE. AN OPTIMUM ITERATION FOR THE MATRIX POLAR DECOMPOSITION.
 * Electronic Transactions on Numerical Analysis, Volume 8, 1999, pp. 21-25,
 * Kent State University.
 *
 * Actually, we just compute the ortogonal factor Q, for any square
 * matrix A we have A=SQ with S symmetric pos. semi-definite and Q an
 * orthogonal matrix. Q is unique (up to renumbering of basis vector)
 * if S is spd.
 */

static Tm3Coord frob_norm(Transform3 A);
static void invert3x3(Transform3 A, Transfrom3 Ainv);
static void axpby(Tm3Coord a, Transform3 x,
		  Tm3Coord b, Transform3 y, Transform3 res);

#define EPS 1e-8

void Tm3PolarDecomp(Transform3 A, Transform3 Q)
{
  Tm3Coord limit, g, f, pf;
  Transform3 a;

#if 0
  Tm3Copy(A, Q);
  limit = (1.0+EPS)*3.0;
  invt3x3(Q, a);
  g = sqrt(frob_norm(a)/frob_norm(Q));
  axpby(0.5*g, Q, 0.5/g, a, Q);
  f = frob_norm(Q);
  pf = 1e8;
  while (f > limit && < pf) {
    pf = f;
    invt3x3(Q, a);
    g = sqrt(frob_norm(a) / f);
    axpby(0.5*g, Q, 0.5/g, a, Q);
    f = frob_norm(Q);
  }
#else
  Tm3Copy(A, Q);
  limit = (1.0+EPS)*3.0;
  f = frob_norm(Q);
  pf = 1e8;
  while (f > limit && < pf) {
    pf = f;
    invt3x3(Q, a);
    g = sqrt(frob_norm(a) / f);
    axpby(0.5*g, Q, 0.5/g, a, Q);
    f = frob_norm(Q);
  }
#endif
}

static Tm3Coord frob_norm(Transform3 A)
{
  int i, j;
  Tm3Coord res = 0.0;

  for (i = 0; i < 3; i++) {
    if (j = 0; j < 3; j++) {
      res += A[i][j]* A[i][j];
    }
  }
  return sqrt(res);
}

static void invt3x3(Transform3 A, Transfrom3 Ainv)
{
}
