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

#include "discgrpP.h"
#include "mgP.h"
#include "math.h"

/* this method is called before drawing, to set up a standard data
 * structure containing a description of the camera; the user can
 * replace it with his/her own routine that gets this information
 * from somewhere else.
 */
void
DiscGrpStandardPreDraw(DiscGrp *discgrp)
{
	float halfy, aspect, halfx;
	static float magic_scale = 1.2;

    /* we're about to mess around with the camera matrix; before doing so,
    get the current one (it's already been set for this frame) to use in
    culling later on */
    if (discgrp->flag  & (DG_DRAWCAM | DG_ZCULL | DG_CENTERCAM))	{
        /* get the pieces of the model to viewing transform */
	CamGet(_mgc->cam, CAM_W2C, discgrp->viewinfo.w2c);
	TmInvert(discgrp->viewinfo.w2c, discgrp->viewinfo.c2w);
        /* concatenate the model to world coordinate transform */
        mg_gettransform(discgrp->viewinfo.m2w);
	TmInvert(discgrp->viewinfo.m2w, discgrp->viewinfo.w2m);
        TmConcat(discgrp->viewinfo.m2w, discgrp->viewinfo.w2c, discgrp->viewinfo.m2c);
	TmInvert(discgrp->viewinfo.m2c, discgrp->viewinfo.c2m);
	CamGet(_mgc->cam, CAM_HALFYFIELD, &halfy);
	CamGet(_mgc->cam, CAM_ASPECT, &aspect);
	halfy = halfy * magic_scale;
	halfx = aspect * halfy;

	/* Hack to get culling to work -Celeste */
	halfx = (halfx > halfy) ? halfx : halfy;
	halfy = halfx;
	/* the frustrum planes evaluate positive on cull-able points */
#define INITPT(ptr,xx,yy,zz,ww) { ((ptr)->x) = (xx);	\
			((ptr)->y) = (yy);		\
			((ptr)->z) = (zz);		\
			((ptr)->w) = (ww); }		
	INITPT(&discgrp->viewinfo.frustrum[0], -1, 0, halfx, 0)
	INITPT(&discgrp->viewinfo.frustrum[1], 1, 0, halfx, 0)
	INITPT(&discgrp->viewinfo.frustrum[2], 0, -1, halfy, 0)
	INITPT(&discgrp->viewinfo.frustrum[3], 0, 1, halfy, 0)
	}

}

    static float visd1 = 2.0;	/* for debugging */
DiscGrp *
DiscGrpDraw(DiscGrp *discgrp)
{
	static HPoint3 origin = {0,0,0,1}, cpos;
	Transform c2wprime;
	DiscGrpEl *nhbr;
	int metric;

    /* set up the viewing system dependent matrices, etc */
    if (discgrp->predraw)    (*discgrp->predraw)(discgrp);
    else		DiscGrpStandardPreDraw(discgrp);
	
    metric = discgrp->attributes & DG_METRIC_BITS;
    /*  make sure we have some geometry to display; the control code
     *	is a bit suspiciously topheavy  */
    if (discgrp->geom == NULL || discgrp->flag & DG_NEWDIRDOM ||
	(discgrp->flag & DG_DRAWDIRDOM && discgrp->ddgeom == NULL)) {

      	discgrp->ddgeom = DiscGrpDirDom(discgrp);
	if (discgrp->geom == NULL ) discgrp->geom = discgrp->ddgeom;
        /* turn off the alarm */
	discgrp->flag &= ~DG_NEWDIRDOM;
	if (!discgrp->ddgeom)	{
	    OOGLError(1,"DiscGrpDraw: Unable to create dirichlet domain\n");
	}
    }

    /* be sure we have some group elements */
    if (discgrp->big_list == NULL ) {
      if (discgrp->nhbr_list != NULL) {
	discgrp->big_list = discgrp->nhbr_list;
      } else { return(discgrp); }
    }

    /* 
     * the main idea here is to find out the position of the current camera
     * and use it to make sure the group elements used are centered at the 
     * camera.  We do this by looking at the orbit of the origin and seeing
     * if the camera is closer to 
     */
    if (discgrp->flag & DG_CENTERCAM )	{
	Transform cinv, hprime, h;
	/* use it to derive the model coordinates of the camera */
	HPt3Transform(discgrp->viewinfo.c2m, &origin, &cpos);	

	/* compute the group element which is 'closest' to the camera */
	nhbr = DiscGrpClosestGroupEl(discgrp, &cpos);	

	/* apply the inverse of this transform to the camera */
  	TmInvert(nhbr->tform, h);
	/* this requires conjugating cinv by the w2m transform: 
	 *
         *  C  --- c2w --->   W   ---- w2m ---->   M
	 *   \                |                    |
	 *    \  	      h'                 h = nhbr->tform inverse
	 *     c2w'	      |                    |
	 *	\--------->   W   <--- m2w -----   M
	 *
	 * In the above diagram we want c2w' = c2w h'
  	 */
	TmConcat(h, discgrp->viewinfo.m2w, cinv);
	TmConcat(discgrp->viewinfo.w2m, cinv, hprime);
	TmConcat(discgrp->viewinfo.c2w, hprime, c2wprime);
	if (discgrp->attributes & DG_HYPERBOLIC && needstuneup(c2wprime)) {
	    tuneup(c2wprime, metric);
	    if (needstuneup(c2wprime))
	        OOGLError(1,"DiscGrpDraw: tuneup failed\n");
	    }
	CamSet(_mgc->cam, CAM_C2W, c2wprime, CAM_END);
	}

    {
    int viscnt = 0; 
    /*float ratio = 1.0;*/
    HPoint3 image;
    int vis;
    float d;
    Transform tile2c;
#ifdef UNNECESSARY
    extern Geom *large_dd, *small_dd;	/*very ugly but appearances
		don't work correctly when pushed down into a list */
#endif
    Transform Tnew;
    GeomIter *it;

    it = GeomIterate( (Geom *)discgrp, DEEP );
    /* loop through them */
    while(NextTransform(it, Tnew) > 0) {
	vis = 1;
	if (discgrp->flag  & DG_ZCULL)	{
            TmConcat(Tnew, discgrp->viewinfo.m2c, tile2c);
            HPt3Transform(tile2c, &discgrp->cpoint, &image);
	    d = HPt3SpaceDistance(&image, &discgrp->cpoint, metric);
	    /* discard images that are too far away */
	    if (d > discgrp->drawdist) vis = 0;
	    /* some close copies are guaranteed to be drawn... */
	    else if (d > visd1 ) {	/* only discard far-away tiles */
	      /* first discard the ones behind the eye, if that makes sense */
	      if (metric != DG_SPHERICAL && image.z*image.w > 0.0) vis = 0;
	      else {	/* then test outside the camera frustrum */
		  { int i;
		  for (i=0; i<4; ++i)		{
		    d = HPt3R40Dot(&image, &discgrp->viewinfo.frustrum[i]); 
		    if ( d > 0)	{
			vis = 0;
			break;
		 	}
		    }
		  }
		}
	      }
	    }

	/* if the tile has passed the visibility tests... */
	if (vis)	{
	  viscnt++;
          mgpushtransform();
    	  mgtransform( Tnew );

	  /* while the appearance stuff doesn't work correctly, we have to
	   * commit some pretty terrible crimes...we'd like to be able to
	   * just say GeomDraw(discgrp->geom)...but we can't as long as
	   * appearances don't work as advertised */

    	  if (discgrp->ddgeom && discgrp->flag & DG_DRAWDIRDOM)    {
	   if (discgrp->flag & DG_DDBEAM)	GeomDraw(discgrp->ddgeom);
	   else {
	    GeomDraw(discgrp->ddgeom);
#ifdef UNNECESSARY
            mgpushappearance();
            mgctxset(MG_ApSet, AP_DONT, APF_FACEDRAW, AP_DO, APF_EDGEDRAW, AP_END, MG_END);
            GeomDraw(large_dd);
            mgpopappearance();

    	    mgpushappearance();
            mgctxset(MG_ApSet, AP_DO, APF_FACEDRAW, AP_END, MG_END);
            GeomDraw(small_dd);
            mgpopappearance();
#endif /*UNNECESSARY*/
            }
	   }
          if (discgrp->flag & DG_DRAWGEOM && discgrp->geom && discgrp->geom != discgrp->ddgeom) 
		GeomDraw( discgrp->geom );

	  if ((discgrp->flag & DG_DRAWCAM) && (discgrp->camgeom)) {
	    mgpushtransform();
	    mgtransform( discgrp->viewinfo.c2m );
	    GeomDraw(discgrp->camgeom);
	    mgpoptransform();
	    }

          mgpoptransform();
	  }
        }
    /* for debugging to see how culling works */
    /*ratio = viscnt / ((double) discgrp->big_list->num_el);*/
    }

    return(discgrp);
}

    		
#ifdef REASONABLE
 	TmScale( scaler, discgrp->scale, discgrp->scale, discgrp->scale);
	large = DiscGrpDirDom(discgrp, &discgrp->cpoint);
	large->ap = ApCreate(AP_DO, APF_EDGEDRAW, AP_DONT, APF_FACEDRAW, AP_END);
	ap = ApCreate(AP_DONT, APF_EDGEDRAW, AP_DO, APF_FACEDRAW, AP_END);
	small = GeomCreate("inst", CR_GEOM, large, CR_AXIS, scaler, CR_APPEAR, ap, CR_END);
	smlist = GeomCreate("list", CR_GEOM, small, CR_END);
	mylist = GeomCreate("list", CR_GEOM, large, CR_CDR, smlist, CR_END);
	discgrp->ddgeom = mylist;
 	TmScale( scaler, discgrp->scale, discgrp->scale, discgrp->scale);
	Tm3SpaceTranslateOrigin(tlate, &discgrp->cpoint, metric);
	/* perform translation so cpoint is at origin */
	TmInvert(tlate, invtlate);
	/* apply scaling transform centered at cpoint */
	TmConcat(invtlate, scaler, tmp);
	/* and move cpoint back to where it started */
	TmConcat(tmp, tlate, tmp);

	    d = HPt3SpaceDistance( &discgrp->cpoint, &image, metric);
#else
#endif
