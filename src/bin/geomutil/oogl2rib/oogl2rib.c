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

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

/* Authors: Scott Wisdom, Tamara Munzner */

#include "ooglutil.h"
#include "mg.h"
#include "mgrib.h"
#include "mgribP.h"
#include "mgribtoken.h"
#include "camera.h"
#include "window.h"
#include "appearance.h"
#include "quad.h"
#include "geom.h"
#include "bbox.h"
#include "color.h"
#include <string.h>

ColorA white = {1, 1, 1, 1};
ColorA black = {0, 0, 0, 1};
Point lightposition = {0, 0, 1};
ColorA ambient = {.2, .2, .2, 1.};
ColorA background = {0, 0, 0, 1}; /* black */

#define xPosition 0
#define yPosition 0
#define windowWidth 300
#define windowHeight 300

/*
    Switch Descriptions
    	default: create a 'world' around the geometry. This will include
    	all tokens necessary to position camera, scale and position geometry,
	a provide a singal light illuminating the front of the geometry.
    -g  geometry: geometry only, no world, no world block.
    -b  block: encapsulate geometry-only into a world block. This is the
        configuration to define a RIB file 'clip object' via the Quick
	RenderMan specification.
    -w  define image width;
    -h  define image height;
    -B  background color. Default no background.
    -t  tiff file name. Default "geom.tiff". If "framebuffer", 
        draw to framebuffer, not as tiff file. 
*/

int main(int argc, char **argv)
{
    float cx,cy,cz,cs,dx,dy,dz;
    HPoint3 min,max;
    Geom *bbox = NULL;
    Transform ObjectTransform;
    LmLighting *lighting;
    LtLight *li[4];
    Camera *cam = NULL;
    WnWindow *win;
    WnPosition position;
    Appearance *ap;
    Geom *ageom;
    float aspect;
    int wwidth;
    int wheight;
    enum _flag {
        geometry,
        minimal,
	world,
	block
    } flag;
    int errflag=0;
    int c, display;
    char *name = "geom.tiff";
    int doback = 0;
    char *infile = "-";
    char *outfile = "-";
    FILE *outfileFD = stdout;
    FILE *infileFD = stdin;
    int framebuffer = 0;

    /* defaults */
    flag = world;
    wwidth = windowWidth;
    wheight = windowHeight;
    
    /* process switches */
    while ((c = getopt(argc, argv, "n:w:h:B:gbf")) != EOF)
        switch(c) {
	case 'w':
	    wwidth = atoi(optarg);
	    break;
	case 'h':
	    wheight = atoi(optarg);
	    break;
	case 'b':
	    flag = block;
	    break;
	case 'g':
	    flag = geometry;
	    break;
	case 'B':
	    sscanf(optarg, "%f%*c%f%*c%f", &(background.r), 
		   &(background.g), &(background.b));
	    doback = 1;
	    break;
	case 'n':
	    name = optarg;
	    break;
	case 'f':
	    framebuffer = 1;
	    /* don't change the name if they've set it: 
	       check if it's the default */
	    if (!strcmp(name, "geom.tiff"))
	      name = "geom.rib";
	    break;
	default:
	    errflag = 1;
	    break;
	}
    
        if(optind<argc) {
	  infile = argv[optind];
	  if (!(infile[0] == '-')) 
	    infileFD = fopen(infile, "rb");
	}

	if((optind+1)<argc) {
	    outfile = argv[optind+1];
	    if (!(outfile[0] == '-'))
	      outfileFD=fopen(outfile, "w+");
	    if(!outfileFD) {
	        fprintf(stderr,"Unable to open file %s for output.\n",outfile);
	    	exit(1);
	    }
	}
	
    	if(errflag) {
	    fprintf(stderr,"Usage:\n\
oogl2rib [-n name] [-B r,g,b] [-w width] [-h height] [-fgb] [infile] [outfile]\n\
Convert OOGL file to RenderMan rib format.\n\
Default: read from stdin, write to stdout. Accepts \"-\" as infile/outfile.\n\
-n <name> => name for rendered TIFF file (default \"geom.tiff\")\n\
             or framebuffer window (default \"geom.rib\").\n\
-B r,g,b => background color, each component ranges from 0 to 1. Default none.\n\
-w <width> -h <height> => width/height of rendered frame, in pixels.\n\
-f => .rib file renders to on-screen framebuffer instead of TIFF file.\n\
Default: create complete rib file containing default camera, lights, etc.\n\
-g => only geometry, -b => only Quick Renderman clip object. Ignores -nBwhf.\n");

	    exit(1);
	}
	
  	TmIdentity(ObjectTransform);

	mgdevice_RIB();

	li[0] = LtCreate(LT_COLOR, &white,
		LT_POSITION, &lightposition,
		LT_INTENSITY, 1.0,
		LT_END);
    
	lighting = LmCreate(LM_AMBIENT, &ambient,
		LM_REPLACELIGHTS, 1,
		LM_END);
	    
	LmAddLight(lighting, li[0]);
  	
	if(flag==world) {
	    cam = CamCreate( CAM_FOV, 60.0, CAM_END );
	    
	    ap = ApCreate(AP_DO, APF_FACEDRAW, AP_DO, APF_VECTDRAW,
			  AP_LINEWIDTH, 1, 
			  AP_MtSet, MT_Kd, 1.0, MT_DIFFUSE, &white, MT_END,
			  AP_LGT, lighting,
			  AP_SHADING, APF_SMOOTH,
			  AP_END);
		    
	    /* create the virtual window with an arbitrary size */
	    position.xmin = 0;
	    position.xmax = wwidth-1;
	    position.ymin = 0;
	    position.ymax = wheight-1;
	    win = WnCreate(WN_NAME, "noname", WN_CURPOS, &position, WN_END);
	    
	    /* update the camera postion and set the proper aspect ratio */
	    CamReset(cam);
	    WnGet(win, WN_ASPECT, &aspect);
	    CamSet(cam, CAM_ASPECT, aspect, CAM_FOV, 60.0, CAM_END);

	    /* At the moment, mgctxset does not actually change 
	       MG_RIBDISPLAY and MG_RIBDISPLAYNAME, so do this during 
	       mgctxcreate - TMM
	     */
	    if(framebuffer) display = MG_RIBFRAME;
	    else display = MG_RIBTIFF;

	    mgctxcreate(MG_RIBFILE, outfileFD,
			MG_RIBFORMAT, MG_RIBASCII,
			MG_RIBBACKING, MG_RIBNOBG,
			MG_WINDOW, win,	
			MG_CAMERA, cam,
			MG_APPEAR, ap,
			MG_RIBDISPLAY, display,
			MG_RIBDISPLAYNAME, name,
			MG_END );
    
	    if(doback) {
		mgctxset(
		    MG_RIBBACKING, MG_RIBDOBG,
		    MG_BACKGROUND, &background,
		    MG_END );
	    }
	} else {
	    ap = ApCreate(AP_DO, APF_FACEDRAW, AP_DO, APF_VECTDRAW, 
			  AP_LINEWIDTH,1,
			  AP_SHADING, APF_SMOOTH,  /* Emit normals - slevy */
			  AP_MtSet, MT_Kd, 1.0, MT_Ks, 0.3, MT_Ka, 0.1,
				    MT_DIFFUSE, &white,
				    MT_EDGECOLOR, &black,
				    MT_ALPHA, 1.0,
				MT_END,
			  AP_LGT, lighting,
			  AP_END);
	    mgctxcreate(MG_RIBFILE, outfileFD,
			MG_SHOW, 0,		/* overides Format & Display */
		 	MG_RIBFORMAT, MG_RIBASCII,
			MG_APPEAR, ap,
			MG_END);
	}

	ageom = GeomFLoad(iobfileopen(infileFD), infile);

	if(flag==world) { 
	    mgworldbegin();

	    /* we use the bbox to determain how to place the geometry */
	    bbox = GeomBound(ageom, NULL, NULL);
	    if (bbox != NULL) {
		BBoxMinMax((BBox*)bbox, &min, &max);
		cx = (max.x + min.x) * .5;
		cy = (max.y + min.y) * .5;
		cz = (max.z + min.z) * .5;
		dx = max.x - min.x;
		dy = max.y - min.y;
		dz = max.z - min.z;
		cs = 2.0 / sqrt(dx*dx + dy*dy + dz*dz);
		TmScale(ObjectTransform,cs,cs,cs);
		CtmTranslate(ObjectTransform,-cx,-cy,-cz);
		mgtransform(ObjectTransform);
	    }
	}
	
	if(flag==block) {
	    mrti(mr_worldbegin, mr_NULL);
	}
	
	GeomDraw(ageom);
	
	if(flag==world) {
	   mgworldend();
           mgrib_flushbuffer();
        }
	else {
	    if(flag==block) mrti(mr_worldend, mr_NULL);
	    mgrib_flushbuffer();
	}
        /* end up with a newline so that rib file can easily be appended to */
        fprintf(outfileFD, "\n");
        fflush(outfileFD);
	return 0;
}
