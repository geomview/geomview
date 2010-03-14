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

/* geometric types, the include structure of the various
 * [h]point[3n].h etc. files is a little bit complicated, so I have
 * separated the typedefs from the function definitions. This way we
 * can use inline functions easily. This file may be included multiple
 * times.
 */

#ifndef HG4TENSOR_TYPEDEF
# define HG4TENSOR_TYPEDEF 1
typedef float Hg4Coord;
typedef Hg4Coord Hg4Tensor1[4];
typedef Hg4Coord *Hg4Tensor1Ptr;
typedef Hg4Coord Hg4Tensor2[4][4];
typedef Hg4Tensor1 *Hg4Tensor2Ptr;
#endif

#ifndef POINT3_TYPEDEF
# define POINT3_TYPEDEF
typedef float Pt3Coord;
typedef struct { Pt3Coord x, y, z; } Point3;
#endif

#ifndef HPOINT3_TYPEDEF
# define HPOINT3_TYPEDEF 1
typedef Pt3Coord HPt3Coord;
typedef struct { HPt3Coord x, y, z, w; } HPoint3;

static inline Point3 *HPoint3Point3(HPoint3 *hpt3)
{
  typedef union
  {
    HPt3Coord array[4];
    HPoint3   hpt3;
    Point3    pt3;
  } HPoint3Cast;  

  return &((HPoint3Cast *)hpt3)->pt3;
}

static inline HPt3Coord *HPoint3Data(HPoint3 *hpt3)
{
  typedef union
  {
    HPt3Coord array[4];
    HPoint3   hpt3;
    Point3    pt3;
  } HPoint3Cast;  

  return ((HPoint3Cast *)hpt3)->array;
}

#endif

#ifndef TRANSFORM3_TYPEDEF
# define TRANSFORM3_TYPEDEF 1
typedef float Tm3Coord;
typedef Tm3Coord Transform3[4][4];
typedef	Tm3Coord (*TransformPtr)[4];
#endif

#ifndef HPLANE3_TYPEDEF
# define HPLANE3_TYPEDEF 1
typedef float HPl3Coord;
typedef struct { HPl3Coord a, b, c, d; } HPlane3;
#endif

#ifndef HLINE3_TYPEDEF
# define HLINE3_TYPEDEF 1
typedef float HLn3Coord;
typedef struct { 
  HLn3Coord L[4][4];
  int type;
} HLine3;
#endif

#ifndef HPOINTN_TYPEDEF
# define HPOINTN_TYPEDEF 1
typedef float HPtNCoord;
typedef struct HPtN {
	int dim;	/* Dimension, including homogeneous divisor */
	int flags;	/* Space tag */
	int size;       /* allocated size */
	HPtNCoord *v;	/* Array of coordinates; v[0] is the homogenous divisor */
} HPointN;
#endif

#ifndef TRANSFORMN_TYPEDEF
# define TRANSFORMN_TYPEDEF 1
/* N-dimensional transformation matrix.
 * Transforms row vectors (multiplied on the left) of dimension 'idim',
 * yielding row vectors of dimension 'odim'.
 */
#include "reference.h"

typedef struct TmN {
	REFERENCEFIELDS;
	int idim, odim;
	int flags;
	HPtNCoord *a;	/* Array of idim rows, odim columns */
} TransformN;
#endif
