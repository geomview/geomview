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

#ifndef TRANSFORMDEFS
#define TRANSFORMDEFS

#include "transform3.h"

typedef Transform3 Transform;
typedef Tm3Coord TmCoord;

#define TmAdjoint Tm3Adjoint
#define TmInvert Tm3Invert
#define TmDeterminant Tm3Determinant
#define TmDual Tm3Dual
#define TmTranspose Tm3Transpose
#define TmPostConcat Tm3PostConcat
#define TmPreConcat Tm3PreConcat
#define TmConcat Tm3Concat
#define TmConjugate Tm3Conjugate
#define TmPolarDecomp Tm3PolarDecomp

#define TmPrint Tm3Print
#define TmCopy Tm3Copy
#define TmCompare Tm3Compare
#define TmPush Tm3Push
#define TmPop Tm3Pop

#define TmIdentity Tm3Identity
#define TmScale Tm3Scale
#define TmRotateX Tm3RotateX
#define TmRotateY Tm3RotateY
#define TmRotateZ Tm3RotateZ
#define TmRotate Tm3Rotate
#define TmRotateBetween Tm3RotateBetween
#define TmRotateTowardZ Tm3RotateTowardZ
#define TmCarefulRotateTowardZ Tm3CarefulRotateTowardZ
#define TmAlignX Tm3AlignX
#define TmAlignY Tm3AlignY
#define TmAlignZ Tm3AlignZ
#define TmAlign Tm3Align

#define CtmTranslate Ctm3Translate
#define CtmScale Ctm3Scale
#define CtmRotateX Ctm3RotateX
#define CtmRotateY Ctm3RotateY
#define CtmRotateZ Ctm3RotateZ
#define CtmRotate Ctm3Rotate
#define CtmAlignX Ctm3AlignX
#define CtmAlignY Ctm3AlignY
#define CtmAlignZ Ctm3AlignZ
#define CtmAlign Ctm3Align

#define TM_XAXIS TM3_XAXIS
#define TM_YAXIS TM3_YAXIS
#define TM_ZAXIS TM3_ZAXIS
#define TM_IDENTITY TM3_IDENTITY

#define TmTranslate Tm3Translate
#define TmTranslateOrigin Tm3TranslateOrigin
#define TmHypTranslate Tm3HypTranslate
#define TmHypTranslateOrigin Tm3HypTranslateOrigin
#define TmSphTranslate Tm3SphTranslate
#define TmSphTranslateOrigin Tm3SphTranslateOrigin
#define TmSpaceTranslate Tm3SpaceTranslate
#define TmSpaceTranslateOrigin Tm3SpaceTranslateOrigin

#define TmProject Tm3Project
#define CtmProject Ctm3Project
#define TmReflect Tm3Reflect
#define CtmReflect Ctm3Reflect
#define CtmRefract  Ctm3Refract
#define CtmRefract2  Ctm3Refract2
#define TmRefract  Tm3Refract
#define TmRefract2  Tm3Refract2
#define TmSkew Tm3Skew
#define TmShear Tm3Shear
#define TmPerspective Tm3Perspective
#define TmOrthographic Tm3Orthographic
#define TmWindow Tm3Window
#define CtmSkew Ctm3Skew
#define CtmShear Ctm3Shear
#define CtmWindow Ctm3Window
#define CtmPerspective Ctm3Perspective
#define CtmOrthographic Ctm3Orthographic


#endif
