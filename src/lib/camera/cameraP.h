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

#ifndef CAMERAP_H
#define CAMERAP_H

#include "camera.h"
#include "appearance.h"

#define	CAMMAGIC	0x9c816301

struct Camera {
    REFERENCEFIELDS;
    Handle	*c2whandle;	/* Handle for external camtoworld's */
    Transform	camtoworld;	/* this moves the camera to its spot */
    Handle	*w2chandle;	/* Handle for external worldtocam's */
    Transform 	worldtocam;	/* this puts the camera at the origin, etc*/
    int 	flag;		/* perspective?  stereo? */
    float	halfyfield,	/* FOV half-width at z=1 (Y direction) */
    		frameaspect,	/* Aspect ratio, X/Y */
    		focus,		/* Nominal focal distance for perspec<->ortho*/
    		cnear, cfar,	/* Near & far clipping distances */
    		stereo_sep,
		stereo_angle;
    Handle	*sterhandle[2];
    Transform	stereyes[2];	/* For stereo separation */
    int		whicheye;	/* Current stereo eye selection */
    int		changed;	/* Mask of "changed" fields, for CamMerge */
    int		space;		/* TM_EUCLIDEAN, TM_HYPERBOLIC, or TM_SPHERICAL */
#if 1
    /* cH: Background information. Does this belong here? I think
     * so. The background layout is clearly no property of the drawing
     * routines (mg-layer), and it has nothing to do with the window
     * system. Arguably it also has nothing to do with the camera, but
     * would be a global property of the entire scene. OTH, it is nice
     * to have cameras with different backgrounds, at least with
     * different background colours.
     *
     * Note that this image is not scaled, it just sits at the center
     * of the window (if the underlying mg-routines support a
     * background image at all). The mg-layer may or may not use alpha
     * blending with the background color if this is a four channel
     * image.
     */
    ColorA      bgcolor;       /* background color */
    Image       *bgimage;      /* background image, potentionally with alpha */
    Handle      *bgimghandle;  /* handle for background image */
#endif
};

/* bits for 'flag' and 'changed' fields */
#define	CAMF_PERSP		0x01
#define	CAMF_STEREO		0x02
#define	CAMF_NEWC2W		0x04

/* bits for 'changed' only */

#define	CAMF_EYE		0x20
#define	CAMF_STEREOXFORM	0x40
#define	CAMF_STEREOGEOM		0x80
#define	CAMF_W2C		0x100
#define	CAMF_FOV		0x200
#define	CAMF_ASPECT		0x400
#define	CAMF_FOCUS		0x800
#define	CAMF_NEAR		0x1000
#define	CAMF_FAR		0x2000
#define CAMF_SPACE		0x4000

/*
 * routine for updating transforms
 */
void CamTransUpdate(Handle **, Camera *, Transform);

#endif /* !CAMERAP_H */

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
