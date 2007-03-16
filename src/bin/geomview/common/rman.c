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


/* Authors: Scott Wisdom, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "common.h"
#include "drawer.h"
#include "lang.h"
#include "ui.h"
#include "mg.h"
#include "rman.h"

#include "mgrib.h"
#include <string.h>

/* for mgrib */

static char defname[] = "geom";
struct _rman rman = {MG_RIBCYLINDER, MG_RIBASCII, 0, MG_RIBTIFF, MG_RIBDOBG, defname};

static int ribtype(char *s, int type);

static char RHelp[] = "\
Renderman:\n\
  RR send RIB output to <fileprefix>NNN.rib (default fileprefix == \"geom\")\n\
  RC Emulate lines using cylinders (default)\n\
  RP Emulate lines using polygons\n\
  Ra choose ASCII RIB format (default)\n\
  Rb choose BINARY RIB format\n\
  Rt choose Display token to specify .tiff file (default)\n\
  Rf choose Display token to specify framebuffer\n\
  Rs Simulate background color with Polygon (default)\n\
  Rx No background simulation - fully transparent (alpha) background\n\
";

/* 
 * Interpret R<suffix> keyboard commands.
 */
void rman_do(int suffix, int hasnumber, int number)
{
    switch(suffix) {
    case '?':	/* help message */
	printf("%s", RHelp);
	break;

    case 'R':	/* Write RenderMan snapshot */
	if(hasnumber) rman.seqno = number;
	gv_rib_snapshot(FOCUSID, NULL);
	break;
	
    case 'C':  /* (for photorman) chooses cylinder line emulation */
	rman.line = MG_RIBCYLINDER;
	break;
	
    case 'P':  /* (for photorman) chooses polygon line emulation */
	rman.line = MG_RIBPOLYGON;
	break;
	
    case 'a':  /* chooses ASCII format for RIB output */
	rman.format = MG_RIBASCII;
	break;
	
    case 'b':  /* chooses BINARY format for RIB output */
	rman.format = MG_RIBBINARY;
	break;
    case 't':  /* toggles Display to be tiff file or framebuffer */
        rman.display = MG_RIBTIFF;
	break;
    case 'f':  /* choose Display token to specify framebuffer */
        rman.display = MG_RIBFRAME;
	break;
    case 's':  /* simulate background color with polygon */
        rman.background = MG_RIBDOBG;
        break;
    case 'x':  /* no background color simulation - transparent background */
        rman.background = MG_RIBNOBG;
	break;
    }

}

LDEFINE(rib_display, LVOID,
"(rib-display    [frame|tiff] FILEPREFIX)\n\
	Set Renderman display to framebuffer (popup screen window) or a\n\
	TIFF format disk file. FILEPREFIX is used to construct\n\
	names of the form \"prefixNNNN.suffix\". (i.e. foo0000.rib)\n\
	The number is incremented on every call to \"rib-snapshot\" and\n\
	reset to 0000 when \"rib-display\" is called. TIFF files are given\n\
	the same prefix and number as the RIB file (i.e. foo0004.rib\n\
	generates foo0004.tiff). The default FILEPREFIX is \"geom\" and\n\
	the default format is TIFF. (Note that geomview just generates a\n\
	RIB file, which must then be rendered.)")
{
  char *fileprefix;
  int type;

  LDECLARE(("rib-display", LBEGIN,
	    LKEYWORD, &type,
	    LSTRING, &fileprefix,
	    LEND));
  type = ribtype("rib-display", type);
  if (rman.fileprefix && rman.fileprefix != defname) OOGLFree(rman.fileprefix);
  rman.fileprefix = strdup(fileprefix);
  rman.display = type;
  rman.seqno = 0;
  return Lt;
}

LDEFINE(rib_snapshot, LVOID,
"(rib-snapshot   CAM-ID  [filename])\n\
	Write Renderman snapshot (in RIB format) of CAM-ID to <filename>.\n\
	If no filename specified, see \"rib-display\" for explanation of\n\
	the filename used.")
{
  DView *view;
  Camera *cam = NULL;
  WnWindow *win;
  const Appearance *ap;
  mgcontext *ctx;
  char fname[1024];
  char displayname[1024];
  char *fileprefix = rman.fileprefix ? rman.fileprefix : defname;
  char *strend = NULL;
  FILE *f = NULL;
  int id, mgspace;
  char *filename = NULL;

  LDECLARE(("rib-snapshot", LBEGIN,
	    LID, &id,
	    LOPTIONAL, LSTRING, &filename,
	    LEND));
  

  if (filename) { /* explicit filename, don't use incremented fileprefix */
    if (filename[0] == '-') { /* treat '-' as stdout */
      sprintf(fname, "stdout");
      sprintf(displayname, "geom.tiff");
      f = stdout;
    } else {
      sprintf(fname, "%s", filename);
      if ((strend = strstr(filename, ".rib"))) { /* toss ".rib" */
	strncpy(displayname, filename, strend-filename);
	displayname[strend-filename] = '\0';
      } else {
	strcpy(displayname, filename);
      }
      strcat(displayname, ".tiff");
      f = fopen(fname, "w");
    }
  } else { /* autoincrement */
    sprintf(fname, "%s%04d.rib", fileprefix, rman.seqno);
    sprintf(displayname, "%s%04d.tiff", fileprefix, rman.seqno);
    f = fopen(fname, "w");
    rman.seqno++;
  }
  
  if(f == NULL) {
    OOGLError(1, "Can't create %s: %s", fname, sperror());
    return Lnil;
  }

  if (rman.display == MG_RIBFRAME) {
    strcpy(displayname, fname);
  }

  fprintf(stderr, "Writing %s ...", fname);
  
  if(!ISCAM(id) || (view = (DView *)drawer_get_object(id)) == NULL) {
    OOGLError(1, "rib-snapshot: bad view!");
    return Lnil;
  }
  mgctxselect(view->mgctx);
  mgctxget(MG_CAMERA, &cam);
  mgctxget(MG_SPACE, &mgspace);
#if 0
  /* Copy so that changed flags are set <- cH: what does that mean ???? */
  ap = ApCopy(mggetappearance(NULL), NULL);
#else
  ap = mggetappearance();
#endif
  mgctxget(MG_WINDOW, &win);
  if(cam == NULL || ap == NULL || win == NULL) {
    OOGLError(1, "rib-snapshot: trouble, %x %x %x", cam,ap,win);
    return Lnil;
  }
  
  mgdevice_RIB();
  ctx = mgctxcreate(MG_CAMERA, cam,
		    MG_APPEAR, ap,
		    MG_WINDOW, win,
		    MG_BACKGROUND, &view->backcolor,
		    MG_SPACE, spaceof(WORLDGEOM),
		    MG_RIBFORMAT, rman.format,
		    MG_RIBLINEMODE, rman.line,
		    MG_RIBFILE, f,
		    MG_RIBDISPLAY, rman.display,
		    MG_RIBDISPLAYNAME, displayname,
		    MG_RIBBACKING, rman.background,
		    MG_RIBSCENE, "geomview RIB snapshot",
		    MG_RIBCREATOR, "mgrib driver - geomview",
		    MG_SPACE, mgspace,
		    MG_END);
  {
    /* Try to be as realistic as possible when dumping a RenderMan snapshot.
     * Create a RenderMan drawing context, install (hopefully) all the
     * attributes that draw_view() doesn't reset on its own,
     * plug the RenderMan ctx into the camera, and force a redraw.
     * Then undo the subterfuge.
     */
    bool oldredraw = view->redraw;
    bool oldchanged = view->changed;
    mgcontext *oldctx = view->mgctx;
    view->mgctx = ctx;
    view->redraw = true;
    gv_draw(view->id);
    view->redraw = oldredraw, view->changed = oldchanged;
    view->mgctx = oldctx;
  }
  mgrib_flushbuffer(); /* now necessary to flush buffer to file */
  
  mgctxdelete(ctx);
#if 0
  ApDelete(ap);
#endif
  mgctxselect(view->mgctx);	/* Revert to previous device */
  
  if (f != stdout) 
    fclose(f);
  fprintf(stderr, " done.\n");
  
  return Lt;
}

static int ribtype(char *s, int type)
{
  switch (type) {
  case TIFF_KEYWORD: return MG_RIBTIFF;
  case FRAME_KEYWORD: return MG_RIBFRAME;
  default:
    fprintf(stderr, "%s: invalid rib type (assuming \"tiff\")\n",s);
    return MG_RIBTIFF;
  }
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
