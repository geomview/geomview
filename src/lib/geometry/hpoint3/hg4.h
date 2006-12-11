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


/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef _GV_HG4_H_
#define _GV_HG4_H_

# include <math.h>
# include <stdio.h>
#include "tolerance.h"
#include "ooglutil.h"

#include "geomtypes.h"

static inline char *Hg4Create(void);
static inline void Hg4Delete(Hg4Tensor1 p);
static inline void Hg4Print(Hg4Tensor1 p);
static inline void Hg4From(Hg4Tensor1 p,
			   Hg4Coord x, Hg4Coord y, Hg4Coord z, Hg4Coord w);
static inline void Hg4Copy(Hg4Tensor1 a, Hg4Tensor1 b);
static inline void Hg4Add(Hg4Tensor1 p1, Hg4Tensor1 p2, Hg4Tensor1 p3);
static inline int Hg4Compare(Hg4Tensor1 p1, Hg4Tensor1 p2);
static inline int Hg4Coincident(Hg4Tensor1 p1, Hg4Tensor1 p2);
static inline int Hg4Undefined(Hg4Tensor1 a);
static inline int Hg4Infinity(Hg4Tensor1 p, int dual);
static inline void Hg4Normalize(Hg4Tensor1 p, Hg4Tensor1 q);
static inline void Hg4Pencil(Hg4Coord t1, Hg4Tensor1 p1,
			     Hg4Coord t2, Hg4Tensor1 p2, Hg4Tensor1 p);
static inline void Hg4Transform(Transform3 T, Hg4Tensor1 p1,  Hg4Tensor1 p2);
static inline void Hg4Print2(Hg4Tensor2 L);
static inline void Hg4Copy2(Hg4Tensor2 L, Hg4Tensor2 K);
static inline int Hg4Compare2(Hg4Tensor2 L, Hg4Tensor2 K);
static inline int Hg4Undefined2(Hg4Tensor2 L);
static inline int Hg4Infinity2(Hg4Tensor2 L, int dual);
static inline void Hg4Transform2(Transform3 T, Hg4Tensor2 p1, Hg4Tensor2 p2);
static inline void Hg4AntiProductPiQj(Hg4Tensor2 L,
				      Hg4Tensor1 p1, Hg4Tensor1 p2);
static inline Hg4Coord Hg4ContractPiQi(Hg4Tensor1 pl, Hg4Tensor1  pt);
static inline void Hg4AntiContractPijQj(Hg4Tensor2 L, Hg4Tensor1 p1,
					Hg4Tensor1 p2);
static inline void Hg4ContractPijQjk(Hg4Tensor2 a, Hg4Tensor2 b, Hg4Tensor2 c);
static inline Hg4Coord Hg4ContractPii(Hg4Tensor2 L);
static inline int Hg4Intersect2(Hg4Tensor2 L, Hg4Tensor1 a, Hg4Tensor1 b);
static inline int Hg4Intersect3(Hg4Tensor1 a, Hg4Tensor1 b,
				Hg4Tensor1 c, Hg4Tensor1 p, int dual);
static inline int Hg4Intersect4(Hg4Tensor2 K, Hg4Tensor2 L,
				Hg4Tensor1 pl, Hg4Tensor1 pt);
static inline void Hg4Dual(Hg4Tensor2 L, Hg4Tensor2 K);

#include "transform3.h"

/* Tensor of Order 1 */
static inline char *
Hg4Create(void)
{
  return (char *) OOGLNewE(Hg4Tensor1, "Hg4Create");
}

static inline void
Hg4Delete(Hg4Tensor1 p)
{
  OOGLFree( (char *) p );
}

static inline void
Hg4Print(Hg4Tensor1 p)
{
  if( p ) 
    printf( "%g %g %g %g\n", p[TMX], p[TMY], p[TMZ], p[TMW] );
}

static inline void
Hg4From(Hg4Tensor1 p, Hg4Coord x, Hg4Coord y, Hg4Coord z, Hg4Coord w)
{
  p[TMX] = x;
  p[TMY] = y;
  p[TMZ] = z;
  p[TMW] = w;
}

static inline void
Hg4Copy(Hg4Tensor1 a, Hg4Tensor1 b)
{
  memcpy( (char *)b, (char *)a, sizeof(Hg4Tensor1) );
}

static inline void 
Hg4Add(Hg4Tensor1 p1, Hg4Tensor1 p2, Hg4Tensor1 p3)
{
  int i;
  for (i=0; i<4; ++i)
    p3[i] = p1[i] + p2[i];
} 

static inline int
Hg4Compare(Hg4Tensor1 p1, Hg4Tensor1 p2)
{  
  Hg4Coord test;

  test = p1[TMX]*p2[TMY] - p1[TMY]*p2[TMX];
  if( fneg(test) ) return -1;
  if( fpos(test) ) return  1;

  test = p1[TMX]*p2[TMZ] - p1[TMZ]*p2[TMX];
  if( fneg(test) ) return -1;
  if( fpos(test) ) return  1;

  test = p1[TMY]*p2[TMZ] - p1[TMZ]*p2[TMY];
  if( fneg(test) ) return -1;
  if( fpos(test) ) return  1;

  test = p1[TMX]*p2[TMW] - p1[TMW]*p2[TMX];
  if( fneg(test) ) return -1;
  if( fpos(test) ) return  1;

  test = p1[TMY]*p2[TMW] - p1[TMW]*p2[TMY];
  if( fneg(test) ) return -1;
  if( fpos(test) ) return  1;

  test = p1[TMZ]*p2[TMW] - p1[TMW]*p2[TMZ];
  if( fneg(test) ) return -1;
  if( fpos(test) ) return  1;

  return 0;
}

static inline int
Hg4Coincident(Hg4Tensor1 p1, Hg4Tensor1 p2)
{
  return Hg4Compare( p1, p2 ) == 0;
}

static inline int
Hg4Undefined(Hg4Tensor1 a)
{
  if( !fzero(a[TMX]) ) return 0;
  if( !fzero(a[TMY]) ) return 0;
  if( !fzero(a[TMZ]) ) return 0;
  if( !fzero(a[TMW]) ) return 0;
  return 1;
}

static inline int
Hg4Infinity(Hg4Tensor1 p, int dual)
{
  /* Assume not undefined */
  if( dual ) { /* plane */
    if( !fzero(p[TMX]) ) return 0;
    if( !fzero(p[TMY]) ) return 0;
    if( !fzero(p[TMZ]) ) return 0;
    return 1;
  }
  else { /* point */
    if( !fzero(p[TMW]) ) return 0;
    return 1;
  }
}

static inline void
Hg4Normalize(Hg4Tensor1 p, Hg4Tensor1 q)
{
  Hg4Copy( p, q );
  if( q[TMW] != 1. && q[TMW] != 0. ) {
    q[TMX] /= q[TMW];
    q[TMY] /= q[TMW];
    q[TMZ] /= q[TMW];
    q[TMW]  = 1.;
  }
}

static inline void
Hg4Pencil(Hg4Coord t1, Hg4Tensor1 p1, Hg4Coord t2, Hg4Tensor1 p2, Hg4Tensor1 p)
{  
  p[TMW] = t1 * p1[TMW] + t2 * p2[TMW];
  /* Keep W positive */
  if( p[TMW] < 0. ) {
    p[TMW] = -p[TMW];
    t1 = -t1;
    t2 = -t2;
  }
  p[TMX] = t1 * p1[TMX] + t2 * p2[TMX];
  p[TMY] = t1 * p1[TMY] + t2 * p2[TMY];
  p[TMZ] = t1 * p1[TMZ] + t2 * p2[TMZ];
}

/*
 * transform a 3d point
 *
 * pt2 = pt1 * [a]
 *
 */
static inline void
Hg4Transform(Transform3 T, Hg4Tensor1 p1,  Hg4Tensor1 p2)
{
  Tm3Coord *aptr;
  Hg4Coord *pptr;
  Hg4Coord x, y, z, w;
  int cnt;

  x = p1[TMX];
  y = p1[TMY];
  z = p1[TMZ];
  w = p1[TMW];
  aptr= T[0];
  pptr= p2;
  cnt=4;
  do{
    *pptr++ = aptr[0]*x + aptr[4]*y + aptr[8]*z + aptr[12]*w;
    ++aptr;
  } while(--cnt);
}

/* Tensor of Order 2 */
static inline void
Hg4Print2(Hg4Tensor2 L)
{
  printf( "[%g %g %g %g\n",  L[TMX][TMX], L[TMX][TMY], L[TMX][TMZ], L[TMX][TMW] );
  printf( " %g %g %g %g\n",  L[TMY][TMX], L[TMY][TMY], L[TMY][TMZ], L[TMY][TMW] );
  printf( " %g %g %g %g\n",  L[TMZ][TMX], L[TMZ][TMY], L[TMZ][TMZ], L[TMZ][TMW] );
  printf( " %g %g %g %g]\n", L[TMW][TMX], L[TMW][TMY], L[TMW][TMZ], L[TMW][TMW] );
}

static inline void
Hg4Copy2(Hg4Tensor2 L, Hg4Tensor2 K)
{
  memcpy( (char *)K, (char *)L, sizeof(Hg4Tensor2) );
}

static inline int
Hg4Compare2(Hg4Tensor2 L, Hg4Tensor2 K)
{
  Hg4Coord t;
  Hg4Tensor2 N;

  Hg4ContractPijQjk( K, L, N );
  t = Hg4ContractPii( N );

  if( fzero(t) ) return 0;
  if( t < 0.   ) return -1;
  return  1;
}

static inline int
Hg4Undefined2(Hg4Tensor2 L)
{
  if( !fzero(L[TMX][TMY]) ) return 0;
  if( !fzero(L[TMX][TMZ]) ) return 0;
  if( !fzero(L[TMX][TMW]) ) return 0;
  if( !fzero(L[TMY][TMZ]) ) return 0;
  if( !fzero(L[TMY][TMW]) ) return 0;
  if( !fzero(L[TMZ][TMW]) ) return 0;
  return 1;
}

static inline int
Hg4Infinity2(Hg4Tensor2 L, int dual)
{
  /* plane form */
  if( dual ) {
    if( !fzero(L[TMX][TMY]) ) return 0;
    if( !fzero(L[TMX][TMZ]) ) return 0;
    if( !fzero(L[TMY][TMZ]) ) return 0;
    return 1;
  }
  else {
    if( !fzero(L[TMX][TMW]) ) return 0;
    if( !fzero(L[TMY][TMW]) ) return 0;
    if( !fzero(L[TMZ][TMW]) ) return 0;
    return 1;
  }
}

static inline void
Hg4Transform2(Transform3 T, Hg4Tensor2 p1, Hg4Tensor2 p2)
{
  Transform3 Tt;

  fprintf(stderr,"\nWARNING: dubious procedure Hg4Transform2 being called.\n\
This procedure may not have been correctly updated for new transform\n\
library.  Ask me about this. --- mbp Mon Aug 19 10:38:19 1991.\n\n");

  /*
    In fact, this procedure has not been updated at all.  This is the old
    version.   I don't know whether this depends on a notion of col vs row
    vectors, or right vs left mult.  I think it does but I'm not sure how,
    so I'll deal with it later.  -- mbp
  */

  /* Assume p1 is the plane-form */
  Tm3Transpose(T, Tt);
  Hg4ContractPijQjk( T,  p1, p2 );
  Hg4ContractPijQjk( p2, Tt, p2 ); 
}

static inline void
Hg4AntiProductPiQj(Hg4Tensor2 L, Hg4Tensor1 p1, Hg4Tensor1 p2)
{
  L[TMX][TMX] = L[TMY][TMY] = L[TMZ][TMZ] = L[TMW][TMW] = 0.;

  L[TMX][TMY] = p1[TMX]*p2[TMY] - p1[TMY]*p2[TMX];
  L[TMX][TMZ] = p1[TMX]*p2[TMZ] - p1[TMZ]*p2[TMX];
  L[TMX][TMW] = p1[TMX]*p2[TMW] - p1[TMW]*p2[TMX];
  L[TMY][TMZ] = p1[TMY]*p2[TMZ] - p1[TMZ]*p2[TMY];
  L[TMY][TMW] = p1[TMY]*p2[TMW] - p1[TMW]*p2[TMY];
  L[TMZ][TMW] = p1[TMZ]*p2[TMW] - p1[TMW]*p2[TMZ];

  L[TMY][TMX] = -L[TMX][TMY];
  L[TMZ][TMX] = -L[TMX][TMZ];
  L[TMW][TMX] = -L[TMX][TMW];
  L[TMZ][TMY] = -L[TMY][TMZ];
  L[TMW][TMY] = -L[TMY][TMW];
  L[TMW][TMZ] = -L[TMZ][TMW];
}

/* contraction of various tensors */
static inline Hg4Coord
Hg4ContractPiQi(Hg4Tensor1 pl, Hg4Tensor1  pt)
{
  Hg4Coord sum;

  sum = 0.;
  sum += pl[TMX] * pt[TMX];
  sum += pl[TMY] * pt[TMY];
  sum += pl[TMZ] * pt[TMZ];
  sum += pl[TMW] * pt[TMW];

  return sum;
}

static inline void
Hg4AntiContractPijQj(Hg4Tensor2 L, Hg4Tensor1 p1, Hg4Tensor1 p2)
{
  Hg4Coord x, y, z, w;
  Hg4Coord xy, xz, xw, yz, yw, zw;

  x = p1[TMX];
  y = p1[TMY];
  z = p1[TMZ];
  w = p1[TMW];

  xy = L[TMX][TMY];
  xz = L[TMX][TMZ];
  xw = L[TMX][TMW];
  yz = L[TMY][TMZ];
  yw = L[TMY][TMW];
  zw = L[TMZ][TMW];

  p2[TMX] =  xy * y + xz * z + xw * w;
  p2[TMY] = -xy * x + yz * z + yw * w;
  p2[TMZ] = -xz * x - yz * y + zw * w;
  p2[TMW] = -xw * x - yw * y - zw * z;
}

static inline void
Hg4ContractPijQjk(Hg4Tensor2 a, Hg4Tensor2 b, Hg4Tensor2 c)
{
  Hg4Tensor2 d;
  int i, j, k;

  /* This can be made more efficient */
  for( i=0; i<4; i++ )
    for( j=0; j<4; j++ ) {
      d[i][j] = 0.;
      for( k=0; k<4; k++ )
	d[i][j] += a[i][k] * b[k][j];
    }
  Hg4Copy2( d, c );
}

static inline Hg4Coord
Hg4ContractPii(Hg4Tensor2 L)
{
  return L[TMX][TMX] + L[TMY][TMY] + L[TMZ][TMZ] + L[TMW][TMW];
}

static inline int
Hg4Intersect2(Hg4Tensor2 L, Hg4Tensor1 a, Hg4Tensor1 b)
{
  Hg4AntiContractPijQj( L, a, b );
  return Hg4Undefined( b );
}

static inline int
Hg4Intersect3(Hg4Tensor1 a, Hg4Tensor1 b, Hg4Tensor1 c, Hg4Tensor1 p, int dual)
{
  Hg4Tensor2 L;

  Hg4AntiProductPiQj( L, a, b );
  if( dual )
    Hg4Dual( L, L );

  Hg4AntiContractPijQj( L, c, p );

  return Hg4Undefined( p );
}

/*
**	Hg4Intersect4 - predicate which tests for 3d line intersection and
**	if an intersection is found returns the point at which th
**	two lines cross and the plane in which the two lines lie.
**	
**	Note: One of the lines should be in the "plane-form" and the
**	other in the "point-form."
**
**	Assume K is plane-form, L is point-form
*/
static inline int
Hg4Intersect4(Hg4Tensor2 K, Hg4Tensor2 L, Hg4Tensor1 pl, Hg4Tensor1 pt)
{
  int flag;
  int i, j;
  Hg4Tensor2 N;
  Hg4Coord t;

  Hg4ContractPijQjk( K, L, N );
  Hg4From( pl, 0., 0., 0., 0. );
  Hg4From( pt, 0., 0., 0., 0. );

  t = Hg4ContractPii( N );
  if( fzero(t) ) {
    /* Look for a non-zero row */
    flag = 0;
    for( i=0; i<4; i++ ) {
      for( j=0; j<4; j++ ) {
	pt[j] = N[i][j];
	if( !fzero( pt[j] ) ) flag++;
      }
      if( flag ) break;
    }

    /* Look for a non-zero col */
    flag = 0;
    for( i=0; i<4; i++ ) {
      for( j=0; j<4; j++ ) {
	pl[j] = N[j][i];
	if( !fzero( pl[j] ) ) flag++;
      }
      if( flag ) break;
    }
  }

  return fzero(t);
}

static inline void
Hg4Dual(Hg4Tensor2 L, Hg4Tensor2 K)
{
  Hg4Coord p, q, r, u, t, s;

  /*
    t = xy; xy = -zw; zw = -t;
    t = xz; xz =  yw; yw =  t;
    t = yz; yz = -xw; xw = -t;
  */

  if( L != K )
    Hg4Copy2( L, K );

  p = K[TMX][TMY]; u = K[TMZ][TMW];
  K[TMZ][TMW] = -p; K[TMW][TMZ] = p;
  K[TMX][TMY] = -u; K[TMY][TMX] = u;

  q = K[TMZ][TMX]; t = K[TMW][TMY];
  K[TMY][TMW] = -q; K[TMW][TMY] = q;
  K[TMX][TMZ] = -t; K[TMZ][TMX] = t;

  s = K[TMY][TMZ]; r = K[TMX][TMW];
  K[TMX][TMW] = -s; K[TMW][TMX] = s;
  K[TMY][TMZ] = -r; K[TMZ][TMY] = r;
}

#endif /* _GV_HG4_H_ */

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
