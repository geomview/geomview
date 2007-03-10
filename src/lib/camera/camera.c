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

/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include <math.h>
#include "geomclass.h"
#include "cameraP.h"
#include "mg.h"
#include "reference.h"

Camera * _CamSet(Camera *cam, int attr, va_list *a_list);

#define SETFLAG(flag, bit, value)		\
  if (value) flag |= bit;			\
  else	 flag &= ~bit

#define GETFLAG(flag, bit)	( (flag & bit) != 0 )

static float GetHalfField( Camera *cam );
static void SetHalfField( Camera *cam, float halffield );
static void CamStereoCompute( Camera *cam );

Camera *
CamCreate(int a1, ...)
{
  Camera *thiscam;
  va_list a_list;

  thiscam = OOGLNewE(Camera, "CamCreate: unable to allocate camera\n");
  memset(thiscam, 0, sizeof(Camera));
  if (thiscam == NULL) return(NULL);

  RefInit((Ref *)thiscam, CAMMAGIC);

  CamDefault(thiscam);
  thiscam->changed = 0;
  
  va_start( a_list, a1 );
  _CamSet(thiscam, a1, &a_list);
  va_end(a_list);
  return thiscam;
}

void
CamDefault(Camera *cam)
{
  cam->flag = CAMF_PERSP;
  cam->frameaspect = 4.0/3.0;
  cam->focus = 3.0;
  cam->stereo_sep = 0.5;
  cam->stereo_angle = .08;
  cam->c2whandle = NULL;
  cam->w2chandle = NULL;
  cam->sterhandle[0] = NULL;
  cam->sterhandle[1] = NULL;
  CamStereoCompute(cam);
  cam->whicheye = 0;		/* only applies to stereo */
  cam->space = TM_EUCLIDEAN;
  cam->bgcolor.r = cam->bgcolor.g = cam->bgcolor.b = 1.0/3.0;
  cam->bgcolor.a = 1.0;
  cam->bgimage = NULL;
  cam->bgimghandle = NULL;
  CamReset( cam );
}

Camera *
CamSet(Camera *cam, int a1, ...)
{
  va_list a_list;
  
  va_start(a_list, a1);
  return ( _CamSet(cam, a1, &a_list) );
}

Camera *
_CamSet(Camera *cam, int attr, va_list *alist)
{
  TransformPtr tt;
  int sethalffield = 0, setaspect = 0, setstereogeom = 0;
  double halffield = 0, v;
  Handle *h;
  int bit, unbit;
  char **ablock = NULL;

#define NEXT(type) OOGL_VA_ARG(type,alist,ablock)

  while (attr != CAM_END) {
    bit = unbit = 0;
    switch(attr) {
    case CAM_ABLOCK:
      ablock = NEXT(char**);
      break;
    case CAM_C2W:
      tt = NEXT(TransformPtr);
      bit = CAMF_NEWC2W, unbit = CAMF_W2C;
      TmCopy(tt, cam->camtoworld);
      TmInvert(cam->camtoworld, cam->worldtocam);
      break;
    case CAM_W2C:
      tt = NEXT(TransformPtr);
      bit = CAMF_W2C, unbit = CAMF_NEWC2W;
      TmCopy(tt, cam->worldtocam);
      TmInvert(cam->worldtocam, cam->camtoworld);
      break;
    case  CAM_FOV:
      v = NEXT(double) / 2;
      bit = CAMF_FOV;
      if(cam->flag & CAMF_PERSP) {
	if(v >= 180/2) v = 120/2;
	v = tan( RADIANS(v) );
      }
      halffield = v;
      sethalffield = 1;
      break;
    case  CAM_HALFYFIELD:
      cam->halfyfield = NEXT(double);
      if(cam->flag & CAMF_PERSP)
	cam->halfyfield *= cam->focus;
      bit = CAMF_FOV;
      break;
    case  CAM_HALFFIELD:
      halffield = NEXT(double);
      sethalffield = 1;
      bit = CAMF_FOV;
      break;
    case  CAM_ASPECT:
      if((v = NEXT(double)) > 0.) {
	if (!sethalffield)
	  halffield = GetHalfField(cam);
	cam->frameaspect = v;
	bit = CAMF_ASPECT;
	setaspect = 1;
      }
      break;
    case  CAM_FOCUS:
      if((v = NEXT(double)) > 0) {
	if(cam->flag & CAMF_PERSP)
	  cam->halfyfield *= v / cam->focus;
	cam->focus = v;
	bit = CAMF_FOCUS;
      }
      break;
    case  CAM_NEAR:
      cam->cnear = NEXT(double);
      bit = CAMF_NEAR;
      break;
    case  CAM_FAR:
      cam->cfar = NEXT(double);
      bit = CAMF_FAR;
      break;
    case  CAM_STEREOSEP:
      cam->stereo_sep = NEXT(double);
      bit = CAMF_STEREOGEOM, unbit = CAMF_STEREOXFORM;
      setstereogeom = 1;
      break;
    case  CAM_STEREOANGLE:
      cam->stereo_angle = NEXT(double);
      bit = CAMF_STEREOGEOM, unbit = CAMF_STEREOXFORM;
      setstereogeom = 1;
      break;
    case  CAM_STEREOEYE:
      cam->whicheye = NEXT(int);
      bit = CAMF_EYE;
      break;
    case CAM_PERSPECTIVE:  bit = CAMF_PERSP; goto flagbit;
    case CAM_STEREO: 	   bit = CAMF_STEREO; goto flagbit;
    flagbit:
      SETFLAG(cam->flag, bit, NEXT(int));
      break;
    case CAM_STEREYES:
      memcpy(cam->stereyes, NEXT(TransformPtr), 2*sizeof(Transform));
      bit = CAMF_STEREOXFORM, unbit = CAMF_STEREOGEOM;
      break;
    case CAM_STERHANDLES:
      memcpy(cam->sterhandle, NEXT(Handle **), 2*sizeof(Handle *));
      bit = CAMF_STEREOXFORM, unbit = CAMF_STEREOGEOM;
      break;
    case CAM_C2WHANDLE:
      h = NEXT(Handle *);
      if (cam->c2whandle) {
	HandlePDelete(&cam->c2whandle);
      }
      cam->c2whandle = REFGET(Handle, h);
      if (h) {
	HandleRegister(&cam->c2whandle,
		       (Ref *)cam, cam->camtoworld, CamTransUpdate);
      }
      bit = CAMF_NEWC2W, unbit = CAMF_W2C;
      break;

    case CAM_W2CHANDLE:
      h = NEXT(Handle *);
      if (cam->w2chandle) {
	HandlePDelete(&cam->w2chandle);
      }
      cam->w2chandle = REFGET(Handle, h);
      if (h) {
	HandleRegister(&cam->w2chandle,
		       (Ref *)cam, cam->worldtocam, CamTransUpdate);
      }
      bit = CAMF_W2C, unbit = CAMF_NEWC2W;
      break;
    case CAM_SPACE:
      {
	int space = NEXT(int);
	if (   space != TM_EUCLIDEAN
	       && space != TM_HYPERBOLIC
	       && space != TM_SPHERICAL) {
	  OOGLError(0,"illegal space value %1d\n", space);
	} else {
	  cam->space = space;
	  bit = CAMF_SPACE;
	}
      }
      break;
    case CAM_BGCOLOR:
      cam->bgcolor = *NEXT(ColorA *);
      break;
    case CAM_BGIMAGE:
      if (cam->bgimghandle) {
	HandlePDelete(&cam->bgimghandle);
      }
      if (cam->bgimage) {
	ImgDelete(cam->bgimage);
      }
      cam->bgimage = REFGET(Image, NEXT(Image *));
      break;
    case CAM_BGIMGHANDLE:
      if (cam->bgimghandle) {
	HandlePDelete(&cam->bgimghandle);
      }
      cam->bgimghandle = REFGET(Handle, NEXT(Handle *));
      if (cam->bgimghandle) {
	HandleRegister(&cam->bgimghandle,
		       (Ref *)cam, &cam->bgimage, HandleUpdRef);
      }
      break;      
    default:
      OOGLError (0, "CamSet: Undefined attribute: %d", attr);
      return NULL;
    }
    cam->changed &= ~unbit;
    cam->changed |= bit;
    attr = NEXT(int);
  }

  /*
    (sethalffield) means we have a new halffield value, stored in local
    var "halffield". (setaspect) means we have a new aspect ratio, and the
    halffield must be updated in accordance with this.  In this case,
    "halffield" holds either the original halffield value, if a new one
    hasn't been explicitly set with CAM_HALFFIELD or CAM_FOV, or the
    new value, if it was explicitly set.  All of these cases are dealt with
    by the following call to SetHalfField.
  */
  if (setaspect || sethalffield)
    SetHalfField(cam, halffield);

  /* following works since the only way to change stereo parameters is
     by using this routine */
  if (setstereogeom)
    CamStereoCompute(cam);
  
  return cam;

#undef NEXT

}


/*-----------------------------------------------------------------------
 * Function:	CamGet
 * Description:	query a camera
 * Args:	*cam: the camera to query
 *		attr: the attribute to query
 *		value: attr's value is written here
 * Returns:	1:  attr is valid and value has been written
 *		0:  attr is valid but currently does not
 *		    have a value
 *		-1: invalid attr
 * Author:	mbp
 * Date:	Thu Aug  8 10:09:10 1991
 * Notes:	At present, there are no camera attr's that might
 *		not be set, so 0 is never returned.  This might change
 *		in the future.
 */
int
CamGet(Camera *cam, int attr, void *value)
{
#define VALUE(type) ((type*)value)

  switch (attr) {

  case CAM_PERSPECTIVE:
    *VALUE(int) = GETFLAG(cam->flag, CAMF_PERSP);
    break;

  case CAM_STEREO:
    *VALUE(int) = GETFLAG(cam->flag, CAMF_STEREO);
    break;

  case CAM_C2W:
    /* camtoworld is always up to date, so just copy */
    TmCopy(cam->camtoworld, (TransformPtr)value);
    break;

  case CAM_W2C:
    /* worldtocam is not always up to date, so update if necessary ... */
    if (cam->flag & CAMF_NEWC2W ) {
      TmInvert( cam->camtoworld, cam->worldtocam );
      cam->flag &= ~CAMF_NEWC2W;
    }
    /* ... then copy */
    TmCopy( cam->worldtocam, (TransformPtr)value );
    break;

  case CAM_FOV:
    *VALUE(float) = 2 * ( (cam->flag & CAMF_PERSP)
			  ? DEGREES( atan( (double)(GetHalfField(cam)) ) )
			  : GetHalfField(cam));
    break;

  case CAM_HALFYFIELD:
    *VALUE(float) = (cam->flag & CAMF_PERSP) ? cam->halfyfield / cam->focus
      : cam->halfyfield;
    break;

  case CAM_HALFFIELD:
    *VALUE(float) = GetHalfField(cam);
    break;

  case CAM_ASPECT:
    *VALUE(float) = cam->frameaspect;
    break;

  case CAM_FOCUS:
    *VALUE(float) = cam->focus;
    break;

  case CAM_NEAR:
    *VALUE(float) = cam->cnear;
    break;

  case CAM_FAR:
    *VALUE(float) = cam->cfar;
    break;

  case CAM_STEREOSEP:
    *VALUE(float) = cam->stereo_sep;
    break;

  case CAM_STEREOANGLE:
    *VALUE(float) = cam->stereo_angle;
    break;

  case CAM_STEREOEYE:
    *VALUE(int) = cam->whicheye;
    break;

  case CAM_C2WHANDLE:
    *VALUE(Handle *) = cam->c2whandle;
    break;

  case CAM_W2CHANDLE:
    *VALUE(Handle *) = cam->w2chandle;
    break;

  case CAM_STEREYES:
    memcpy(value, cam->stereyes, 2*sizeof(Transform));
    break;

  case CAM_STERHANDLES:
    memcpy(value, cam->sterhandle, 2*sizeof(Handle *));
    break;

  case CAM_SPACE:
    *VALUE(int) = cam->space;
    break;

  case CAM_BGCOLOR:
    *VALUE(ColorA) = cam->bgcolor;
    break;
    
  case CAM_BGIMAGE:
    *VALUE(Image *) = cam->bgimage;
    break;
    
  case CAM_BGIMGHANDLE:
    *VALUE(Handle *) = cam->bgimghandle;
    break;

  default:
    return -1;
    break;
  }
  return 1;

#undef VALUE
}

void
CamDelete( Camera *cam )
{
  if(cam == NULL)
    return;
  if(cam->magic != CAMMAGIC) {
    OOGLWarn("Internal warning: trying to CamDelete non-Camera %x (%x != %x)",
	     cam, cam->magic, CAMMAGIC);
    return;
  }
  if(RefDecr((Ref *)cam) <= 0) {
    cam->magic ^= 0x80000000;	/* Invalidate */
    if(cam->c2whandle) HandlePDelete( &cam->c2whandle );
    if(cam->w2chandle) HandlePDelete( &cam->w2chandle );
    if(cam->sterhandle[0]) HandlePDelete( &cam->sterhandle[0] );
    if(cam->sterhandle[1]) HandlePDelete( &cam->sterhandle[1] );
    if (cam->bgimghandle) HandlePDelete(&cam->bgimghandle);
    if (cam->bgimage) ImgDelete(cam->bgimage);
    OOGLFree(cam);
  }
}

Camera *
CamCopy( Camera *src, Camera *dst )
{
  if (src == NULL) {
    return NULL;
  }
  if (dst == NULL) {
    dst = OOGLNewE(Camera, "CamCopy Camera");
  }
#if 0  
  else
    HandleDelete(dst->handle);
#endif
  *dst = *src;
  dst->ref_count = 1;
  /*dst->handle = NULL;*/
  return dst;
}

void
CamReset( Camera *cam )
{
  Transform T;
  int persp;

  CamGet(cam, CAM_PERSPECTIVE, &persp);

  switch (cam->space) {

  case TM_EUCLIDEAN:
    CamSet( cam,
	    CAM_NEAR,		.07,
	    CAM_FAR,		100.0,
	    CAM_FOCUS,		3.0,
	    CAM_FOV,		persp ? 40.0 : 2.2,
	    CAM_END);
    break;

  case TM_HYPERBOLIC:
    CamSet( cam,
	    CAM_NEAR,		.07,
	    CAM_FAR,		100.0,
	    CAM_FOCUS,		2.5, 
	    CAM_FOV,		persp ? 40.0 : 2.2,
	    CAM_END);
    break;

  case TM_SPHERICAL:
    CamSet( cam,
	    CAM_NEAR,		.05,
	    CAM_FAR,		-.05,
	    CAM_FOCUS,		0.5,
	    CAM_FOV,		persp ? 90.0 : 2.2,
	    CAM_END);
    break;
  }

  TmSpaceTranslate( T, 0.0, 0.0, cam->focus, cam->space );
  CamSet(cam, CAM_C2W, T, CAM_END);
}

/*
 * Return camera's projection transform in proj.
 * See CamView below for the range of the projection.
 */
void
CamViewProjection( Camera *cam, Transform proj )
{
  float y;
  float x;

  y = cam->halfyfield;
  if(cam->flag & CAMF_PERSP)
    y *= cam->cnear / cam->focus;
  x = cam->frameaspect * y;

  if(cam->flag & CAMF_PERSP) {
    TmPerspective( proj, -x, x, -y, y, cam->cnear, cam->cfar );
  } else {
    TmOrthographic( proj, -x, x, -y, y, cam->cnear, cam->cfar );
  }
  if (cam->flag & CAMF_STEREO)
    TmConcat( cam->stereyes[cam->whicheye], proj, proj );
}

/*
 * Computes complete transformation from world -> projected coordinates
 * and leaves it in T.
 * Projected coordinates map the visible world into -1 <= {X,Y,Z} <= 1,
 * with Z = -1 at the near plane and Z = +1 at the far plane.
 */
void
CamView( Camera *cam, Transform T )
{
  Transform t;

  CamViewProjection( cam, t );
  if(cam->flag & CAMF_NEWC2W) {
    TmInvert( cam->camtoworld, cam->worldtocam );
    cam->flag &= ~CAMF_NEWC2W;
  }
  TmConcat( cam->worldtocam, t, T );
}

void
CamRotateX( Camera *cam, float angle )
{
  CtmRotateX( cam->camtoworld, angle );
  cam->flag |= CAMF_NEWC2W;
}

void
CamRotateY( Camera *cam, float angle )
{
  CtmRotateY( cam->camtoworld, angle );
  cam->flag |= CAMF_NEWC2W;
}

void
CamRotateZ( Camera *cam, float angle )
{
  CtmRotateZ( cam->camtoworld, angle );
  cam->flag |= CAMF_NEWC2W;
}

/* translate the camera, using the camera's notion of what space it
   is in */
void
CamTranslate( Camera *cam, float tx, float ty, float tz )
{
  Transform T;

  TmSpaceTranslate( T, tx, ty, tz, cam->space );
  TmConcat(T, cam->camtoworld, cam->camtoworld);
  cam->flag |= CAMF_NEWC2W;
}

/* CamScale is a noop if the camera is not in Euclidean space */
void
CamScale( Camera *cam, float sx, float sy, float sz )
{
  if (cam->space == TM_EUCLIDEAN) {
    CtmScale( cam->camtoworld, sx, sy, sz );
    cam->flag |= CAMF_NEWC2W;
  }
}

void
CamAlignZ( Camera *cam, float x, float y, float z )
{
  Point3 axis;

  axis.x = x;
  axis.y = y;
  axis.z = z;
  CtmAlignZ( cam->camtoworld, &axis );
  cam->flag |= CAMF_NEWC2W;
}

/*
 * Apply T to camera as seen by world (== T^-1 to world, as seen by camera)
 */
void
CamTransform( Camera *cam, Transform T )
{
  TmConcat(T, cam->camtoworld, cam->camtoworld);
  cam->flag |= CAMF_NEWC2W;
}

static void
CamStereoCompute( Camera *cam )
{
  float tanconv = tan(cam->stereo_angle);
  TmTranslate( cam->stereyes[CAM_RIGHT], cam->stereo_sep, 0., 0. );
  TmTranslate( cam->stereyes[CAM_LEFT], -cam->stereo_sep, 0., 0. );
  cam->stereyes[CAM_RIGHT][TMZ][TMX] = -tanconv;
  cam->stereyes[CAM_LEFT][TMZ][TMX] = tanconv;
}

/*-----------------------------------------------------------------------
 * Function:	SetHalfField
 * Description:	set camera's "halffield" value
 * Args:	*cam: the camera
 *		halffield: the halffied value to set to
 * Returns:	nothing
 * Author:	mbp
 * Date:	Wed Aug 21 14:26:43 1991
 * Notes:	This procedure modifies the halfyfield member of cam
 *		in such a way as to guarantee that the min half-width
 *		of the view window is halffield.  This depends on the
 *		camera's current aspect ratio.
 */
static void
SetHalfField( Camera *cam, float halffield )
{
  cam->halfyfield =
    (cam->frameaspect < 1 && cam->frameaspect > 0)
    ? halffield / cam->frameaspect
    : halffield;
  if(cam->flag & CAMF_PERSP)
    cam->halfyfield *= cam->focus;
}

/*-----------------------------------------------------------------------
 * Function:	GetHalfField
 * Description:	return camera's "halffield" value
 * Args:	*cam: the camera
 * Returns:	the halffield value
 * Author:	mbp
 * Date:	Wed Aug 21 14:29:31 1991
 * Notes:	the "halffield" is the min half-width of the view
 *		window.  If the aspect ratio is >= 1, this is
 *		the vertical half-width (halfyfield).  If the aspect
 *		ratio is < 1, this is the horizontal half-width.
 */
static float
GetHalfField( Camera *cam )
{
  float v = cam->halfyfield;
  if(cam->frameaspect < 1) v *= cam->frameaspect;
  if(cam->flag & CAMF_PERSP) v /= cam->focus;
  return v;
}

/*
 * Merge one Camera's changed values into another Camera
 */
Camera *
CamMerge(Camera *src, Camera *dst)
{
  int chg;
  float fov;

  if(src == NULL) return dst;
  if(dst == NULL) return NULL;

  chg = src->changed;

  if(chg & CAMF_NEWC2W)
    CamSet(dst, CAM_C2WHANDLE, src->c2whandle, CAM_C2W, src->camtoworld, CAM_END);
  if(chg & CAMF_STEREOGEOM)
    CamSet(dst, CAM_STEREOSEP, src->stereo_sep,
	   CAM_STEREOANGLE, src->stereo_angle, CAM_END);
  if(chg & CAMF_STEREOXFORM)
    CamSet(dst, CAM_STEREYES, src->stereyes,
	   CAM_STERHANDLES, src->sterhandle, CAM_END);
  if(chg & CAMF_W2C)
    CamSet(dst, CAM_W2CHANDLE, src->w2chandle,
	   CAM_W2C, src->worldtocam, CAM_END);
  CamGet(src, CAM_FOV, &fov);
  if(chg & CAMF_FOCUS) CamSet(dst, CAM_FOCUS, src->focus, CAM_END);
  if(chg & CAMF_PERSP) CamSet(dst,CAM_PERSPECTIVE,src->flag&CAMF_PERSP,CAM_END);
  if(chg & CAMF_FOV) CamSet(dst, CAM_FOV, fov, CAM_END);
  if(chg & CAMF_ASPECT) CamSet(dst, CAM_ASPECT, src->frameaspect, CAM_END);
  if(chg & CAMF_NEAR) dst->cnear = src->cnear;
  if(chg & CAMF_FAR) dst->cfar = src->cfar;
  if(chg & CAMF_EYE) dst->whicheye = src->whicheye;
  if(chg & CAMF_STEREO) CamSet(dst,CAM_STEREO,src->flag&CAMF_STEREO,CAM_END);
  if(chg & CAMF_SPACE) dst->space = src->space;
  return dst;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
 
