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


/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef HG4DEF
#define HG4DEF

typedef float Hg4Coord;
typedef Hg4Coord Hg4Tensor1[4];
typedef Hg4Coord Hg4Tensor2[4][4];

#define TMX 0
#define TMY 1
#define TMZ 2
#define TMW 3

/* Tensor of Order 1 */
extern char *Hg4Create();
extern void Hg4Delete(/* Hg4Tensor1 p */);

extern void Hg4Print(/* Hg4Tensor1 p */);
extern void Hg4From(/* Hg4Tensor1 p, Hg4Coord x, y, z, w */);
extern void Hg4Copy(/* Hg4Tensor1 a, b */);

extern int Hg4Compare(/* Hg4Tensor1 a, b */);
extern int Hg4Coincident(/* Hg4Tensor1 a, b */);
extern int Hg4Undefined(/* Hg4Tensor1 a */);
extern int Hg4Infinity(/* Hg4Tensor1 a, int dual */);

extern void Hg4Normalize(/* Hg4Tensor1 a, b */);
extern void Hg4Pencil(
    /* Hg4Coord t1, Hg4Tensor1 p1, Hg4Coord t2, Hg4Tensor1 p2, p*/);

extern void Hg4Transform(/* Transform t, Hg4Tensor1 p1, p2 */);

/* Tensor of Order 2 */
extern void Hg4Print2(/* Hg4Tensor2 a */);
extern void Hg4Copy2(/* Hg4Tensor2 a, b */);

extern int Hg4Compare2(/* Hg4Tensor2 a, b */);
extern int Hg4Undefined2(/* Hg4Tensor2 a */);
extern int Hg4Infinity2(/* Hg4Tensor2 a, int dual */);

extern void Hg4Transform2(/* Transform3 T, Hg4Tensor2 a, b */);

extern void Hg4AntiProductPiQj( /* Hg4Tensor2 L, 
				   Hg4Tensor1 p1,
				   Hg4Tensor1 p2 */ );

extern void Hg4AntiProductPiQi(/* Hg4Tensor2 p12, Hg4Tensor1 p1, p2 */);
extern Hg4Coord Hg4ContractPiQi(/* Hg4Tensor1 p1, p2 */);
extern void Hg4AntiContractPijQj(/* Hg4Tensor2 a, Hg4Tensor1 p1, p2 */);
extern void Hg4ContractPijQjk(/* Hg4Tensor2 a, b, c */);
extern Hg4Coord Hg4ContractPii(/* Hg4Tensor2 a */);

extern int Hg4Intersect2(/* Hg4Tensor2 a, Hg4Tensor1 b, c */);
extern int Hg4Intersect3(/* Hg4Tensor1 a, b, c, p, int dual */);
extern int Hg4Intersect4(/* Hg4Tensor2 a, b, Hg4Tensor1 pl, pt */);

extern void Hg4Dual(/* Hg4Tensor2 L, K */);

extern void Hg4Add( /* Hg4Tensor1 p1, Hg4Tensor1 p2, Hg4Tensor1 p3 */ );


#endif
