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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef CAMERA_H
#define CAMERA_H

#include "3d.h"
#include "handle.h"
#include "color.h"

#include <stdarg.h>

typedef struct Camera Camera;

/* Public Camera methods (more below): */

Camera *CamCreate( int attr1, ... );			
Camera *CamSet( Camera *, int attr1, ... );
Camera * _CamSet(Camera *cam, int attr, va_list *a_list);
int	CamGet( Camera *, int attr, void *value);
void	CamDelete( Camera * );				
Camera *CamCopy( Camera *src, Camera *dst );		
Camera *CamMerge( Camera *src, Camera *dst );

/* Camera attributes: */
				/* Set/Create type	Get type	*/
#define CAM_END		800	/* ---------------	--------	*/
#define CAM_PERSPECTIVE 801	/* int			int *		*/
#define CAM_C2W		802	/* Transform		Transform	*/
#define CAM_W2C		803	/* Transform		Transform	*/
#define CAM_FOV		804	/* float		float *		*/
#define CAM_HALFYFIELD	805	/* float		float *		*/
#define CAM_HALFFIELD	806	/* float		float *		*/
#define CAM_ASPECT	807	/* float		float *		*/
#define CAM_FOCUS	808	/* float		float *		*/
#define CAM_NEAR	809	/* float		float *		*/
#define CAM_FAR		810	/* float		float *		*/
#define CAM_STEREO	811	/* int			int *		*/
#define CAM_STEREOSEP	812	/* float		float *		*/
#define CAM_STEREOANGLE	813	/* float		float *		*/
#define CAM_STEREOEYE	814	/* int			int *		*/
#define	CAM_C2WHANDLE	815	/* Handle *		Handle **	*/
#define	CAM_W2CHANDLE	816	/* Handle *		Handle **	*/
#define CAM_STEREYES	817	/* Transform [2]	Transform [2]	*/
#define	CAM_STERHANDLES	818	/* Handle *[2]		Handle *[2]	*/
#define CAM_SPACE	819	/* int			int *		*/
#define CAM_BGCOLOR     820	/* ColorA *		ColorA *	*/
#define CAM_BGIMAGE     821	/* Image *		Image **	*/
#define CAM_BGIMGHANDLE 822	/* Handle *		Handle **	*/

#define CAM_ABLOCK	899	/* void **ablock */

/*
  CAM_END:		end of attribute list
  CAM_PERSPECTIVE:	perspective (1) or ortho (0) projection (default:1)
  CAM_C2W:		camera-to-world transform: <world> * C2W = <cam>
  CAM_W2C:		world-to-camera transform (inverse of CAM_C2W)
  CAM_C2WHANDLE:	Handle onto CAM_C2W
  CAM_W2CHANDLE:	Handle onto CAM_W2C
  CAM_FOV:		field of view (in degrees)
  CAM_HALFYFIELD:	FOV half-width at z=1 (Y direction)
  CAM_HALFFIELD:	min FOV half-width
  CAM_ASPECT:		aspect ratio, X/Y
  CAM_FOCUS:		nominal focal distance for perspec<->ortho
  CAM_NEAR:		near clipping plane
  CAM_FAR:		far clipping plane
  CAM_STEREO:		stereo (1) or mono (0) (default:0)
  CAM_STEREOSEP:	separation distance between stereo eyes
  CAM_STEREOANGLE:	angle between stereo eyes (degrees)
  CAM_STEREYES:		array of two transforms resp. for left & right eye,
			  applied as <world> * C2W * STEREYE * projection
  CAM_STERHANDLES:	Handles for left and right eye transforms
  CAM_STEREOEYE:	Which stereo eye selected: CAM_LEFT or CAM_RIGHT
  CAM_SPACE:		TM_EUCLIDEAN, TM_HYPERBOLIC, or TM_SPHERICAL
  */

#define	CAM_LEFT	0
#define	CAM_RIGHT	1

/*
 * Additional public Camera methods:
 */

/* Reset to defaults */
void	CamReset( Camera * );			

/*  return Camera's projection xform; doesn't change camera at all */
void	CamViewProjection( Camera *, Transform ); 

/*  Complete cam world->proj xform; doesn't change camera at all */
void	CamView( Camera *, Transform );	

/* Apply T to camtoworld xform */
void	CamTransform( Camera *, Transform T );	

/* Rotate camera about X */
void	CamRotateX( Camera *, float angle );	

/* Rotate about Y (radians) */
void	CamRotateY( Camera *, float );		

/* Rotate about Z */
void	CamRotateZ( Camera *, float );		

/* Translate X,Y,Z */
void	CamTranslate( Camera *, float,float,float );	

/* Hyperbolic xlate */
void	CamHypTranslate( Camera *, float,float,float, float );	

/* Zoom in X,Y,Z */
void	CamScale( Camera *, float,float,float );		

/* ??? */
void	CamAlignZ( Camera *, float,float,float );		

/* Save to file */
void    CamSave(Camera *, char *);
void    CamFSave(Camera *, FILE *, char *);

/* Load from file */
Camera  *CamLoad(Camera *, char *);
Camera  *CamFLoad(Camera *, IOBFILE *, char *);

/************************************************************************
 * The following procedures are on death row; they will be taken out    *
 * soon because they have been superceded by CamGet and CamSet          *
 ************************************************************************/

/* Get object xform */
void	CamCurrentPosition( Camera *, Transform );		

/*  Camera's world->camera xform (inverse of CamCurrentPosition) */
void	CamViewWorld( Camera *, Transform ); 

/* Set "focal" length */
void	CamFocus( Camera *, float focus );			

/* Get focal length */
float	CamCurrentFocus( Camera * );			

/* Set X/Y aspect */
void	CamFrameAspect( Camera *, float aspectratio );	

/* Get X/Y aspect */
float	CamCurrentAspect( Camera * );			

/* camtoworld xform = T */
void	CamTransformTo( Camera *, Transform T );	

/* Left/Right/Mono */
void	CamStereoEye( Camera *cam, int whicheye );		

/* Set clip planes */
void	CamClipping( Camera *, float near, float far );	

/* Get clipping */
void	CamCurrentClipping( Camera *, float *near, float *far ); 

/* Set field: minfov/2 */
void	CamHalfField( Camera *, float halffield );		

/* Set field: Yfov/2 */
void	CamHalfYField( Camera *, float halfyfield );	

/* perspective/ortho */
void	CamPerspective( Camera *, int perspective );	

/* or orthographic */
int	CamIsPerspective( Camera * );			

/* Get fov/2 (max way) */
float	CamCurrentHalfField( Camera * );			

/* Get fov/2 (Y dir) */
float	CamCurrentHalfYField( Camera * );			

/* Get xfms, cur eye */
int	CamCurrentStereo( Camera *, Transform leye, Transform reye );

void CamDefault(Camera *cam);

int CamStreamIn(Pool *p, Handle **hp, Camera **camp);
int CamStreamOut(Pool *p, Handle *hp, Camera *cam);

void CamHandleScan( Camera *cam, int (*func)(), void *arg );

#endif /* !CAMERA_H */
