/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2007 Claus-Justus Heine
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

#include <stdio.h>
#include "mg.h"
#include "mgP.h"
#include "mgrib.h"
#include "mgribtoken.h"
#include "transform.h"
#include "bezierP.h"

#define DEFAULT_RIB_FILE "geom.rib"

typedef struct mgribcontext {
  struct mgcontext mgctx;	/* The mgcontext */
  bool born;			/* Has window been displayed on the screen? */
  int win;			/* window ID, or 0 */
  int world;                    /* inc/decremented by worldbegin()/end() */
  int persp;                    /* camera projection */
  float focallen;               /* focallen */

  enum { RMD_ASCII, RMD_BINARY } render_device;	/* Device Options */

/* Which line drawing technique?
 * MG_RIBPOLYGON for creating RIB tokens true to OOGL/geomview, but may appear
 *    incorrect when viewed dynamically
 * MG_RIBCYLINDER for creating RIB tokens which may be dynamically viewed, but
 *    may not be true to OOGL/geomview
 * MG_RIBPRMANLINE for NeXT Renderer - May Cause Incompatibilities!
 * Note: Future versions of renderman should support line drawing, NeXT does
 * now.
 */
  enum line_mode {
    RM_POLYGON = MG_RIBPOLYGON,
    RM_CYLINDER = MG_RIBCYLINDER,
    RM_PRMANLINE = MG_RIBPRMANLINE
  } line_mode;

  /* MG_RIBFRAME: render to framebuffer (screen window)
   * MG_RIBTIFF: render to file
   */
  enum display { RM_FRAME = MG_RIBFRAME, RM_RIBTIFF = MG_RIBTIFF } display;

  FILE *rib;
  bool rib_close; /* set to true if we have opened rib ourselves */
  char displayname[PATH_MAX]; /* if display == MG_RIFILE, name of the file */
  char displaypath[PATH_MAX]; /* dirname(display), if MG_RIFILE */
  char displaybase[PATH_MAX]; /* basename(display), if MG_RIFILE */
  const char *tmppath;

  /* MG_RIBDOBG: simulate colored background w/ polygon
   * MG_RIBNOBG: no background simulation (defualt)
   */
  enum backing { RB_DOBG = MG_RIBDOBG, RB_NOBG = MG_RIBNOBG } backing;

  /* MG_RIBSTDSHADE: uses standard shader
   * MG_RIBEXTSHADE: uses extended shaders (eplastic,
   * heplastic, hplastic)
   *
   * + the shaders needed to support Geomview's texture model
   * (modulate/decal/blend), including alpha channel support.
   */
  enum shader {
    RM_STDSHADE = MG_RIBSTDSHADE,
    RM_EXTSHADE = MG_RIBEXTSHADE
  } shader;

  char *shadepath;	/* path to extended shaders or user shaders */
  char ribscene[128];	/* scene name for RIB 1.0 file comments */
  char ribcreator[128]; /* creator field for RIB 1.0 file comments */
  			/* defaults to "mgrib driver" */
  char ribfor[128];	/* for(user) field for RIB 1.0 file comments */
  			/* defaults to user account name */
  char ribdate[128];	/* creation date, defualts to today's date */
  Texture **tx ;        /* array of textures used; texture images are
			 * dumped to disk with
			 * displayname.#seq.tiff. Textures are
			 * considered equal (for this purpose) if they
			 * refer to the same image and have the same
			 * clamping settings (we leave the clamping to
			 * MakeTexture, that is the reason).
			 */
  int n_tximg;          /* How many of them */
  int n_txdumped;       /* How many already got their MakeTexture line */

  TokenBuffer worldbuf; /* buffer for everything except MakeTexture */
  TokenBuffer txbuf;    /* buffer for MakeTexture */
} mgribcontext;

/* Make some convenient defines */
#define RiDEV  _mgribc->render_device
#define NotImplemented(name) fprintf(stderr,"%s is not yet implemented.\n",name);

#define _mgribc		((mgribcontext*)_mgc)
#define RI_INFINITY 1.0e38

void mgrib_printmatrix(Transform T);
void mgrib_drawline(HPoint3 *p1, HPoint3 *p2);
void mgrib_drawpoint(HPoint3 *p);
void mgrib_drawnormal(HPoint3 *p, Point3 *n);


static inline void
mgrib_mktexname(char *txname, int seq, const char *path, const char *suffix)
{
  if (snprintf(txname, PATH_MAX, "%s%s%s-tx%d.%s",
	       path ? path : "", path ? "/" : "",
	       _mgribc->displaybase, seq, suffix) >= PATH_MAX) {
    OOGLError(1, "path to texture-file exceedsd maximum length %d", PATH_MAX);
  }
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
