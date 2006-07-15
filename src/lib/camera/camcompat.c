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
#include <stdio.h>
#include "ooglutil.h"
#include "handle.h"
#include "streampool.h"
#include "cameraP.h"
#include "transobj.h"

extern HandleOps CamOps;

Camera *
CamFLoad(Camera *proto, IOBFILE *inf, char *fname)
{
    Pool *p;
    Camera *cam = NULL;

    p = PoolStreamTemp(fname, inf, NULL, 0, &CamOps);
    if(p == NULL)
	return NULL;
    if(proto != NULL)
	OOGLError(1, "Note: CamFLoad(cam, ...) can't handle cam != NULL");
    (void) CamStreamIn(p, NULL, &cam);
    PoolDelete(p);
    return cam;
}

void
CamFSave(Camera *cam, FILE *outf, char *fname)
{
    Pool *p = PoolStreamTemp(fname, NULL, outf, 1, &CamOps);
    if(p == NULL)
	return;
    (void) CamStreamOut(p, NULL, cam);
    PoolDelete(p);
}

Camera *
CamLoad(Camera *cam, char *name)
{
  IOBFILE *f;
  
  if((f = iobfopen(name,"r")) == NULL) {
    perror(name);
    return NULL;
  }
  cam = CamFLoad(cam, f, name);
  iobfclose(f);
  return cam;
}


void
CamSave(Camera *cam, char *name)
{
    FILE *f;

    if((f = fopen(name, "w")) == NULL) {
	perror(name);
	return;
    }
    CamFSave(cam, f, name);
    fclose(f);
}


/* I'm not sure what to do with the following procedure.  On one hand
   I think it should come out and be replaced by get-only attributes??
   On the other hand, it's nice to be able to get both eyes in one call.
   I need to understand more about how stereo works.  Until then, I'm
   commenting this procedure out.  -- mbp Fri Aug  9 00:32:36 1991 */
#ifdef notdef
int
CamCurrentStereo( Camera *cam, Transform leye, Transform reye )
{
    if(leye != TMNULL) TmCopy(cam->stereyes[0], leye);
    if(reye != TMNULL) TmCopy(cam->stereyes[1], reye);
    return cam->whicheye;
}
#endif

/************************************************************************
 * The following procedures are on death row; they will be taken out    *
 * soon because they have been superceded by CamGet and CamSet          *
 ************************************************************************/

void
CamCurrentPosition( Camera *cam, Transform T )
{
    TmCopy( cam->camtoworld, T );
}

/*
 * Return world -> camera coordinate transform in T.
 * Note this is the inverse of the transform given by CamCurrentPosition().
 */
void
CamViewWorld( Camera *cam, Transform T )
{
    if (cam->flag & CAMF_NEWC2W ) {
	TmInvert( cam->camtoworld, cam->worldtocam );
	cam->flag &= ~CAMF_NEWC2W;
	}
    /* else we have the actual worldtocam transform already computed */
    TmCopy( cam->worldtocam, T );
}


void
CamFocus( Camera *cam, float focus )
{
    cam->focus = focus;
}

float
CamCurrentFocus( Camera *cam )
{
    return cam->focus;
}

float
CamCurrentAspect( Camera *cam )
{
    return cam->frameaspect;
}

/*
 * Set aspect ratio; preserve minimum field-of-view.
 */
void
CamFrameAspect( Camera *cam, float aspect )
{
    cam->frameaspect = aspect;
}

void
CamTransformTo( Camera *cam, Transform T )
{
    TmCopy( T, cam->camtoworld );
    cam->flag |= CAMF_NEWC2W;
}

/*
 * Select which eye to view through in stereo mode.
 */
/* incorporate into CamSet & remove: */
void
CamStereoEye( Camera *cam, int whicheye )
{
    if(whicheye > 1) whicheye = 1;
    cam->whicheye = whicheye;
}

void
CamClipping( Camera *cam, float cnear, float cfar )
{
    cam->cnear = cnear;
    cam->cfar = cfar;
}

void
CamCurrentClipping( Camera *cam, float *cnear, float *cfar )
{
    *cnear = cam->cnear;
    *cfar = cam->cfar;
}

void
CamHalfYField( Camera *cam, float halfyfov )
{
    cam->halfyfield = halfyfov;
}

float
CamCurrentHalfYField( Camera *cam )
{
    return cam->halfyfield;
}

float
CamCurrentHalfField( Camera *cam )
{
    return cam->frameaspect > 1 ?
		cam->halfyfield : cam->halfyfield * cam->frameaspect;
}

void
CamPerspective( Camera *cam, int persp )
{
    if (persp)  cam->flag |= CAMF_PERSP;
    else	cam->flag &= ~CAMF_PERSP;
}

int
CamIsPerspective( Camera *cam )
{
    return ((cam->flag & CAMF_PERSP) != 0);
}
