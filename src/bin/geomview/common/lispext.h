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
#ifndef LISPEXT_H
#define LISPEXT_H

#include "lisp.h"
#include "streampool.h"
#include "3d.h"
#include "camera.h"
#include "geom.h"
#include "appearance.h"
#include "window.h"
#include "transformn.h"
#include "lang.h"

/* All extension objects except the transform objects have the
 * following structure.
 */
typedef struct HandleRefStruct
{
  Handle *h;
  Ref *ref;
} HandleRefStruct;

typedef struct {
  Handle *h;
  Camera *cam;
} CameraStruct;

typedef struct {
  Handle *h;
  Geom *geom;
} GeomStruct;

typedef struct {
  Handle *h;
  Transform tm;
} TransformStruct;

typedef struct {
  Handle *h;
  WnWindow *wn;
} WindowStruct;

typedef struct {
  Handle *h;
  Appearance *ap;
} ApStruct;

typedef struct {
  Handle *h;
  TransformN *tm;
} TmNStruct;

typedef struct
{
  Handle *h;
  Image *img;
} ImgStruct;

#define LAPVAL(obj)		((ApStruct*)       (obj->cell.p))
#define LCAMERAVAL(obj)		((CameraStruct*)   (obj->cell.p))
#define LGEOMVAL(obj)		((GeomStruct*)     (obj->cell.p))
#define LWINDOWVAL(obj)		((WindowStruct*)   (obj->cell.p))
#define LTRANSFORMVAL(obj)	((TransformStruct*)(obj->cell.p))
#define LTRANSFORMNVAL(obj)	((TmNStruct*)	   (obj->cell.p))
#define LIMAGEVAL(obj)          ((ImgStruct*)      (obj->cell.p))

#define LSTRINGSVAL(obj)	LSTRINGVAL(obj)
#define LIDVAL(obj)		(obj->cell.i)
#define LKEYWORDVAL(obj)	(obj->cell.i)

extern LType LApp;
extern LType LCamerap;
extern LType LGeomp;
extern LType LIdp;
extern LType LKeywordp;
extern LType LStringsp;
extern LType LTransformp;
extern LType LTransformNp;
extern LType LWindowp;
extern LType LImagep;
extern LType LApp;

#define LAP		(&LApp)
#define LCAMERA 	(&LCamerap)
#define LGEOM		(&LGeomp)
#define LID		(&LIdp)
#define LKEYWORD	(&LKeywordp)
#define LSTRINGS	(&LStringsp)
#define LTRANSFORM	(&LTransformp)
#define LTRANSFORMN	(&LTransformNp)
#define LWINDOW		(&LWindowp)
#define LIMAGE          (&LImagep)

extern LObject *L0, *L1;

extern Keyword parse_keyword(char *word);
extern void lispext_init();
extern void define_keyword(char *word, Keyword value);
char *keywordname(Keyword keyword);

#endif /* ! LISPEXT_H */

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
