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

#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
#include "config.h"
#endif

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Geometry Technologies, Inc.";


/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips */

#
/*
**	hg4.h - procedural interface to homogeneous geometry
**
**	pat hanrahan
**
*/

# include <math.h>
# include <stdio.h>
# include "tolerance.h"
# include "transform3.h"
# include "hg4.h"
# include "ooglutil.h"

char *
Hg4Create()
{
    return (char *) OOGLNewE(Hg4Tensor1, "Hg4Create");
}

void
Hg4Delete( p )
    Hg4Tensor1 p;
{
    OOGLFree( (char *) p );
}

void
Hg4Print( p )
  Hg4Tensor1 p;
{
    if( p ) 
	printf( "%g %g %g %g\n", p[TMX], p[TMY], p[TMZ], p[TMW] );
}

void
Hg4From( p, x, y, z, w )
    Hg4Tensor1 p;
    Hg4Coord x, y, z, w;
{
    p[TMX] = x;
    p[TMY] = y;
    p[TMZ] = z;
    p[TMW] = w;
}

void
Hg4Copy( a, b )
    Hg4Tensor1 a, b;
{
    memcpy( (char *)b, (char *)a, sizeof(Hg4Tensor1) );
}

void 
Hg4Add( p1, p2, p3)
    Hg4Tensor1 p1, p2, p3;
{
    register int i;
    for (i=0; i<4; ++i)
	p3[i] = p1[i] + p2[i];
} 

int
Hg4Compare( p1, p2 )
    Hg4Tensor1 p1, p2;
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


int
Hg4Coincident( p1, p2 )
    Hg4Tensor1 p1;
    Hg4Tensor1 p2;
{
    return Hg4Compare( p1, p2 ) == 0;
}

int
Hg4Undefined( a )
    Hg4Tensor1 a;
{
    if( !fzero(a[TMX]) ) return 0;
    if( !fzero(a[TMY]) ) return 0;
    if( !fzero(a[TMZ]) ) return 0;
    if( !fzero(a[TMW]) ) return 0;
    return 1;
}


int
Hg4Infinity( p, dual )
    Hg4Tensor1 p;
    int dual;
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

void
Hg4Normalize( p, q )
    Hg4Tensor1 p, q;
{
    Hg4Copy( p, q );
    if( q[TMW] != 1. && q[TMW] != 0. ) {
	q[TMX] /= q[TMW];
	q[TMY] /= q[TMW];
	q[TMZ] /= q[TMW];
	q[TMW]  = 1.;
    }
}

void
Hg4Pencil( t1, p1, t2, p2, p )
    Hg4Coord t1, t2;
    Hg4Tensor1 p1, p2, p;
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
void
Hg4Transform( T, p1, p2)
    Transform3 T;
    Hg4Tensor1 p1, p2;
{
    register Tm3Coord *aptr;
    register Hg4Coord *pptr;
    Hg4Coord x, y, z, w;
    int register cnt;

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

void
Hg4Print2( L )
    Hg4Tensor2 L;
{
    printf( "[%g %g %g %g\n",  L[TMX][TMX], L[TMX][TMY], L[TMX][TMZ], L[TMX][TMW] );
    printf( " %g %g %g %g\n",  L[TMY][TMX], L[TMY][TMY], L[TMY][TMZ], L[TMY][TMW] );
    printf( " %g %g %g %g\n",  L[TMZ][TMX], L[TMZ][TMY], L[TMZ][TMZ], L[TMZ][TMW] );
    printf( " %g %g %g %g]\n", L[TMW][TMX], L[TMW][TMY], L[TMW][TMZ], L[TMW][TMW] );
}

void
Hg4Copy2( L, K )
    Hg4Tensor2 L, K;
{
    memcpy( (char *)K, (char *)L, sizeof(Hg4Tensor2) );
}

int
Hg4Compare2( L, K )
    Hg4Tensor2 L, K;
{
    Hg4Coord t;
    Hg4Tensor2 N;

    Hg4ContractPijQjk( K, L, N );
    t = Hg4ContractPii( N );

    if( fzero(t) ) return 0;
    if( t < 0.   ) return -1;
    return  1;
}

int
Hg4Undefined2( L )
    Hg4Tensor2 L;
{
    if( !fzero(L[TMX][TMY]) ) return 0;
    if( !fzero(L[TMX][TMZ]) ) return 0;
    if( !fzero(L[TMX][TMW]) ) return 0;
    if( !fzero(L[TMY][TMZ]) ) return 0;
    if( !fzero(L[TMY][TMW]) ) return 0;
    if( !fzero(L[TMZ][TMW]) ) return 0;
    return 1;
}

int
Hg4Infinity2( L, dual )
    Hg4Tensor2 L;
    int dual;
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


void
Hg4Transform2( T, p1, p2 )
    Transform3 T;
    Hg4Tensor2 p1, p2;
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
    Tm3Transpose( T, Tt );
    Hg4ContractPijQjk( T,  p1, p2 );
    Hg4ContractPijQjk( p2, Tt, p2 ); 
}

void
Hg4AntiProductPiQj( L, p1, p2 )
    Hg4Tensor2 L;
    Hg4Tensor1 p1, p2;
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

Hg4Coord
Hg4ContractPiQi( pl, pt )
    Hg4Tensor1 pl, pt;
{
    Hg4Coord sum;

    sum = 0.;
    sum += pl[TMX] * pt[TMX];
    sum += pl[TMY] * pt[TMY];
    sum += pl[TMZ] * pt[TMZ];
    sum += pl[TMW] * pt[TMW];

    return sum;
}

void
Hg4AntiContractPijQj( L, p1, p2 )
    Hg4Tensor2 L;
    Hg4Tensor1 p1, p2;
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

void
Hg4ContractPijQjk( a, b, c )
    Hg4Tensor2 a, b, c;
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

Hg4Coord
Hg4ContractPii( L )
    Hg4Tensor2 L;
{
    return L[TMX][TMX] + L[TMY][TMY] + L[TMZ][TMZ] + L[TMW][TMW];
}

int
Hg4Intersect2( L, a, b )
    Hg4Tensor2 L;
    Hg4Tensor1 a, b;
{
    Hg4AntiContractPijQj( L, a, b );
    return Hg4Undefined( b );
}

int
Hg4Intersect3( a, b, c, p, dual )
    Hg4Tensor1 a, b, c, p;
    int dual;
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
int
Hg4Intersect4( K, L, pl, pt )
    Hg4Tensor2 K, L;
    Hg4Tensor1 pl;
    Hg4Tensor1 pt;
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

void
Hg4Dual( L, K )
    Hg4Tensor2 L, K;
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

