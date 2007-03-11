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
#include "handleP.h"
#include "cameraP.h"
#include "transobj.h"

HandleOps CamOps = {
  "cam",
  (int ((*)()))CamStreamIn,
  (int ((*)()))CamStreamOut,
  (void ((*)()))CamDelete,
  NULL,		/* Resync */
  NULL		/* close pool */
};


/*
 * Load Camera from file.
 * Syntax:
 *  [camera] < "filename_containing_camera"	[or]
 *  [camera] {   keyword  value   keyword  value   ...  }
 *
 */
int
CamStreamIn(Pool *p, Handle **hp, Camera **camp)
{
  char *w, *raww;
  IOBFILE *f;
  Handle *h = NULL;
  Handle *hname = NULL;
  Camera *cam = NULL;
  int credible = 0;
  int i;
  int got;
  float v;
  int brack = 0;
  int empty = 1, braces = 0;
  static struct kw {
    char *name;
    char args;
    int kbit;
  } kw[] = {
    { "camtoworld", 0, CAMF_NEWC2W },
    { "worldtocam", 0, CAMF_W2C },
    { "flag",	    1, CAMF_PERSP|CAMF_STEREO },
    { "halfyfield", 1, CAMF_FOV },
    { "frameaspect",1, CAMF_ASPECT },
    { "focus",	    1, CAMF_FOCUS },       /*  5 */
    { "near",	    1, CAMF_NEAR },
    { "far",	    1, CAMF_FAR },
    { "stereo_sep", 1, CAMF_STEREOGEOM },
    { "stereo_angle",1, CAMF_STEREOGEOM },
    { "stereyes",   0, CAMF_STEREOXFORM }, /* 10 */
    { "whicheye",   1, CAMF_EYE },
    { "define",	    0, 0 },
    { "camera",     0, 0 },
    { "perspective",1, CAMF_PERSP },
    { "stereo",     1, CAMF_STEREO },      /* 15 */
    { "hyperbolic", 1, 0 /* CAMF_HYPERBOLIC is now obsolete */ },
    { "fov",	    1, CAMF_FOV },
    { "bgcolor",    0, 0 },
    { "bgimage",    0, 0 },
  };

  if ((f = PoolInputFile(p)) == NULL)
    return 0;

  for(;;) {
    switch(i = iobfnextc(f, 0)) {
    case '<':
    case ':':
    case '@':
      iobfgetc(f);
      w = iobfdelimtok("(){}", f, 0);
      if (i == '<' && (h = HandleByName(w, &CamOps)) == NULL && w[0] != '\0') {
	w = findfile(PoolName(p), raww = w);
	if (w == NULL) {
	  OOGLSyntax(f, "Reading camera from \"%s\": can't find file \"%s\"",
		     PoolName(p), raww);
	}
      } else if (h) {
	/* HandleByName() increases the ref. count s.t. the
	 * caller of HandleByName() owns the returned handle.
	 */
	HandleDelete(h);
      }

      if ((h = HandleReferringTo(i, w, &CamOps, hp)) != NULL) {
	cam = (Camera *)h->object;
	/* Increment the ref. count. This way we can call
	 * HandleDelete() and CamDelete() independently.
	 */
	RefIncr((Ref*)cam);
      }
      if (!brack) goto done;
      break;

    case '{': brack++; iobfgetc(f); break;
    case '}':
      if (brack > 0) { iobfgetc(f); braces = 1; }
      if (--brack <= 0) goto done;
      /* Otherwise, fall through into... */
    default:
      empty = 0;
      w = iobfdelimtok("(){}", f, 0);
      if (w == NULL)
	goto done;
      
      for(i = sizeof(kw)/sizeof(kw[0]); --i >= 0; )
	if (!strcmp(w, kw[i].name))
	  break;

      if (i < 0) {
	if (credible)
	  OOGLSyntax(f, "Reading camera from \"%s\": unknown camera keyword \"%s\"",
		     PoolName(p), w);
	if (cam) CamDelete(cam);
	return 0;
      } else if ( (got= iobfgetnf(f, kw[i].args, &v, 0)) != kw[i].args ) {
	OOGLSyntax(f, "Reading camera from \"%s\": \"%s\" expects %d values, got %d",
		   PoolName(p), w, kw[i].args, got);
	CamDelete(cam);
	return false;
      }
      if (i != 13 && cam == NULL) {
	cam = CamCreate(CAM_END);
	credible = 1;
      }
      if (cam) {
	cam->changed |= kw[i].kbit;
      }
      switch (i) {
      case 0: {
	TransObj *tobj = NULL;
	Handle *thandle = NULL;
	if (TransObjStreamIn(p, &thandle, &tobj)) {
	  CamSet(cam, CAM_C2W, tobj->T, CAM_C2WHANDLE, thandle, CAM_END);
	  HandleDelete(thandle);
	  TransDelete(tobj);
	}
	break;
      }
      case 1: {
	TransObj *tobj = NULL;
	Handle *thandle = NULL;
	if (TransObjStreamIn(p, &thandle, &tobj)) {
	  CamSet(cam, CAM_W2C, tobj, CAM_W2CHANDLE, thandle, CAM_END);
	  HandleDelete(thandle);
	  TransDelete(tobj);
	}
	break;
      }
      case 2: cam->flag = (int)v; break;
      case 3: CamSet(cam, CAM_HALFYFIELD, v, CAM_END); break;
      case 4: CamSet(cam, CAM_ASPECT, v, CAM_END); break;
      case 5: CamSet(cam, CAM_FOCUS, v, CAM_END); break;
      case 6: cam->cnear = v; break;
      case 7: cam->cfar = v; break;
      case 8: CamSet(cam, CAM_STEREOSEP, v, CAM_END); break;
      case 9: CamSet(cam, CAM_STEREOANGLE, v, CAM_END); break;
      case 10: {
	TransObj *tobj[2] = { NULL, NULL };
	Handle *thandle[2] = { NULL, NULL };
	int i;
	if (TransObjStreamIn(p, &thandle[0], &tobj[0]) &&
	    TransObjStreamIn(p, &thandle[1], &tobj[1])) {
	  TmCopy(tobj[0]->T, cam->stereyes[0]);
	  TmCopy(tobj[1]->T, cam->stereyes[1]);
	  CamSet(cam, CAM_STEREYES, cam->stereyes,
		 CAM_STERHANDLES, thandle,
		 CAM_END);
	}
	for (i = 0; i < 2; i++) {
	  HandleDelete(thandle[i]);
	  TransDelete(tobj[i]);
	}
      }
	
	break;
      case 11: cam->whicheye = (int)v; break;
      case 12: /* "define" */
	hname = HandleCreateGlobal(iobfdelimtok("(){}", f, 0), &CamOps);
	break;
      case 13: /* "camera" */ break;

      case 14: /* "perspective" */
      case 15: /* "stereo" */
	cam->flag &= ~kw[i].kbit;
	if (v != 0) cam->flag |= kw[i].kbit;
	break;

      case 16: /* "hyperbolic" */
	/* The "hyperbolic" field is obsolete (replaced by the
	   "space" field.  Ignore it for now [ Mon Dec 7 14:05:50
	   1992 ].  After a few months [ say in March 1993 ] add a
	   warning message to be printed out at this point in the
	   code, and after a few more months [ say June 1993 ],
	   remove it completely. -- mbp */
	break;

      case 17: /* "fov" */ CamSet(cam, CAM_FOV, v, CAM_END); break;
      case 18: /* bgcolor */
	/* we always expect RGB values in the range 0..1 as floating
	 * point values.
	 */
	if ((got = iobfgetnf(f, 3, (float *)&cam->bgcolor, 0)) != 3) {
	  OOGLSyntax(f, "Reading camera from \"%s\": \"%s\" expects "
		     "an RGB(A) color specification "
		     "(got only %d values, not 3 or 4)",
		     PoolName(p), "bgcolor", got);
	  CamDelete(cam);
	  return false;
	}
	if (got == 3 && iobfgetnf(f, 1, (float *)&cam->bgcolor+3, 0) != 1) {
	  cam->bgcolor.a = 1.0;
	}
	break;
      case 19: /* bgimage */
	if (!ImgStreamIn(p, &cam->bgimghandle, &cam->bgimage)) {
	  OOGLSyntax(f, "Reading camera from \"%s\": "
		     "unable to read background image",
		     PoolName(p));
	  CamDelete(cam);
	  return false;
	}
	break;
      }
    }
  }

 done:

  if (hname) {
    if (cam) {
      HandleSetObject(hname, (Ref *)cam);
    }
    if (h) {
      /* HandleReferringTo() has passed the ownership to use, so
       * delete h because we do not need it anymore.
       */
      HandleDelete(h);
    }
    h = hname;
  }

  /* Pass the ownership of h and cam to the caller if requested */

  if (hp != NULL) {
    /* pass on ownership of the handle h to the caller of this function */
    if (*hp != NULL) {
      if (*hp != h) {
	HandlePDelete(hp);
      } else {
	HandleDelete(*hp);
      }
    }
    *hp = h;
  } else if (h) {
    /* Otherwise delete h because we are its owner. Note that
     * HandleReferringTo() has passed the ownership of h to us;
     * explicitly defined handles (hdefine and define constructs)
     * will not be deleted by this call.
     */
    HandleDelete(h);
  }

  /* same logic as for hp */
  if (camp != NULL) {
    if (*camp) {
      CamDelete(*camp);
    }
    *camp = cam;
  } else if(cam) {
    CamDelete(cam);
  }

  return (cam != NULL || h != NULL || (empty && braces));
}


int
CamStreamOut(Pool *p, Handle *h, Camera *cam)
{
  float fov;
  FILE *outf;

  if ((outf = PoolOutputFile(p)) == NULL) {
    return 0;
  }

  fprintf(outf, "camera {\n");
  PoolIncLevel(p, 1);  

  if (cam == NULL && h != NULL && h->object != NULL) {
    cam = (Camera *)h->object;
  }

  if (PoolStreamOutHandle(p, h, cam != NULL)) {
    if (cam->w2chandle) {
      PoolFPrint(p, outf, "worldtocam ");
      TransStreamOut(p, cam->w2chandle, cam->worldtocam);
    } else {
      PoolFPrint(p, outf, "camtoworld ");
      TransStreamOut(p, cam->c2whandle, cam->camtoworld);
    }
    PoolFPrint(p, outf, "perspective %d  stereo %d\n",
	    cam->flag & CAMF_PERSP ? 1 : 0,
	    cam->flag & CAMF_STEREO ? 1 : 0);
    CamGet(cam, CAM_FOV, &fov);
    PoolFPrint(p, outf, "fov %g\n", fov);
    PoolFPrint(p, outf, "frameaspect %g\n", cam->frameaspect);
    PoolFPrint(p, outf, "focus %g\n", cam->focus);
    PoolFPrint(p, outf, "near %g\n", cam->cnear);
    PoolFPrint(p, outf, "far %g\n", cam->cfar);
    if (cam->flag & CAMF_STEREOGEOM) {
      PoolFPrint(p, outf, "stereo_sep %g\n", cam->stereo_sep);
      PoolFPrint(p, outf, "stereo_angle %g\n", cam->stereo_angle);
    }
    if (cam->flag & CAMF_EYE)
      PoolFPrint(p, outf, "whicheye %d\n", cam->whicheye);
    if (cam->flag & CAMF_STEREOXFORM) {
      PoolFPrint(p, outf, "stereyes\n");
      TransStreamOut(p, cam->sterhandle[0], cam->stereyes[0]);
      fputc('\n', outf);
      TransStreamOut(p, cam->sterhandle[1], cam->stereyes[1]);
    }
    PoolFPrint(p, outf, "bgcolor %g %g %g %g\n",
	    cam->bgcolor.r, cam->bgcolor.g, cam->bgcolor.b, cam->bgcolor.a);
    if (cam->bgimage) {
      PoolFPrint(p, outf, "bg");
      ImgStreamOut(p, cam->bgimghandle, cam->bgimage);
    }
  }
  PoolIncLevel(p, -1);  
  PoolFPrint(p, outf, "}\n");
  return !ferror(outf);
}
 
void
CamHandleScan( Camera *cam, int (*func)(), void *arg )
{
  if (cam) {
    if (cam->c2whandle)
      (*func)(&cam->c2whandle, cam, arg);
    if (cam->w2chandle)
      (*func)(&cam->w2chandle, cam, arg);
  }
}

void CamTransUpdate(Handle **hp, Camera *cam, Transform T)
{
  TransUpdate(hp, (Ref *)cam, T);

  if (hp == &cam->c2whandle) {
    TmInvert(cam->camtoworld, cam->worldtocam);
  } else if (hp == &cam->w2chandle) {
    TmInvert(cam->worldtocam, cam->camtoworld);
  }
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
