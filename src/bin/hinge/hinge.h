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
#ifndef HINGE_H
#define HINGE_H

#include "geomclass.h"
#include "polylistP.h"
#include "forms.h"

extern PolyList *pl;

#define MAXTHINGS	50
extern TransformPtr TT[];
extern float epsilon;

extern FILE *togv;
extern IOBFILE *fromgv;
#define HYPERBOLIC	1
#define EUCLIDEAN	2
#define SPHERICAL	3
extern int space;

extern int haveaxis;
extern float angle;
extern float hingeincr;
extern int killgv;

extern Point base, tip, axis, currentedge[2];
extern Transform BaseT;

extern int mainpos[2];
extern int helppos[2];
extern int infopos[2];
extern int filepos[2];

#define SHOWMAT(f, name, T) \
  { fprintf(f, "%s =\n", name); \
fprintf(f, "\t{{%6f, %6f, %6f, %6f},\n", T[0][0], T[0][1], T[0][2], T[0][3]);\
fprintf(f, "\t {%6f, %6f, %6f, %6f},\n", T[1][0], T[1][1], T[1][2], T[1][3]);\
fprintf(f, "\t {%6f, %6f, %6f, %6f},\n", T[2][0], T[2][1], T[2][2], T[2][3]);\
fprintf(f, "\t {%6f, %6f, %6f, %6f}}\n", T[3][0], T[3][1], T[3][2], T[3][3]);\
    fprintf(f, "\n"); }

int WhichFace(HPoint3 *p, Transform T, PolyList *pl);
int CoPlanar(HPoint3 *p, Transform T, Poly *poly);
int pt4equal(HPoint3 *a, HPoint3 *b);
float space_distance(HPoint3 *a, HPoint3 *b);
float HPt3EucDistance( HPoint3 *a, HPoint3 *b );
float HPt3HypDistance( HPoint3 *a, HPoint3 *b );
float MinkDot( HPoint3 *a, HPoint3 *b );
int PolyContainsEdge(Point e[], Transform T, Poly *poly);
void WritePolyListInfo(PolyList *pl);
int HingeLoad(char *file);
int NewInst(float ang);
void ComputeTransform(Transform T, Transform BaseT, float ang);
int NewTTindex();
void DefineAxis(Point *a, Point *b);
void Rotation(Transform T, Point *base, Point *axis, float angle);
void DefineThing(Geom *g);
void DefinePick(Geom *g);
void Inst(int n, float ang);
void Undo();
void Reset();
void ShowAxis();
void space_translate_origin(Point *pt, Transform T);
void HingeSpace(int s);


#define SHOWPT3(f, name, pt) \
  fprintf(f, "%s = (%6f, %6f, %6f)\n", name, (pt).x, (pt).y, (pt).z)

#define SHOWHPT3(f, name, pt) \
  fprintf(f, "%s = [%6f, %6f, %6f, %6f]\n", name, (pt).x, (pt).y, (pt).z, (pt).w)


#endif /* HINGE_H */
