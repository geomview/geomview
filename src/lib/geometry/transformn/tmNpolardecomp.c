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
#include "transformn.h"

/* Compute the polar decomposition of a NxN matrix by means of a fast
 * Newton's iteration:
 *
 * A. A. DUBRULLE. AN OPTIMUM ITERATION FOR THE MATRIX POLAR DECOMPOSITION.
 * Electronic Transactions on Numerical Analysis, Volume 8, 1999, pp. 21-25,
 * Kent State University.
 *
 * Actually, we just compute the ortogonal factor Q, for any square
 * matrix A we have A=SQ with S symmetric pos. semi-definite and Q an
 * orthogonal matrix. Q is unique (up to renumbering of basis vectors)
 * if S is spd.
 *
 * We leave the translation and homogeneous part as is, that is
 * axpbyNxN() only copies the lower NxN square.
 */

#define EPS 1e-8

static inline HPtNCoord frob_norm(TransformN *A)
{
  int i, j;
  HPtNCoord res = 0.0;
  int idim = A->idim, odim = A->odim;

  for (i = 1; i < idim; i++) {
    for (j = 1; j < odim; j++) {
	    res += A->a[i*odim+j]* A->a[i*odim+j];
    }
  }
  return sqrt(res);
}

/* a X + b Y^t */
static inline void axpbytNxN(HPtNCoord a, TransformN *x,
			     HPtNCoord b, TransformN *y, TransformN *res)
{
  int i, j;
  int dim = x->idim;
  
  for (i = 1; i < dim; i++) {
    for (j = 1; j < dim; j++) {
      res->a[i*dim+j] = a * x->a[i*dim+j] + b * y->a[j*dim+i];
    }
  }
}

TransformN *TmNPolarDecomp(const TransformN *A, TransformN *Q)
{
  HPtNCoord limit, g, f, pf;
  TransformN *a;

  Q = TmNCopy(A, Q);
  limit = (1.0+EPS)*sqrt((float)(A->odim-1));
  a = TmNInvert(Q, NULL);
  g = sqrt(frob_norm(a)/frob_norm(Q));
  axpbytNxN(0.5*g, Q, 0.5/g, a, Q);
  f = frob_norm(Q);
  pf = 1e8;
  while (f > limit && f < pf) {
    pf = f;
    TmNInvert(Q, a);
    g = sqrt(frob_norm(a) / f);
    axpbytNxN(0.5*g, Q, 0.5/g, a, Q);
    f = frob_norm(Q);
  }
  TmNDelete(a);
  return Q;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
