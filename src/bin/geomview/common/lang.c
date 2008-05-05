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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef NeXT
#include <unistd.h>
#endif
/*#include <sys/fcntl.h>*/
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include "ooglutil.h"
#include "drawer.h"
#include "lisp.h"
#include "lispext.h"
#include "lang.h"
#include "event.h"
#include "ui.h"
#include "comm.h"
#include "streampool.h"
#include "handleP.h"
#include "transform.h"
#include "mgrib.h"
#include "version.h"

#include "clang.c"

/* do-lang, do-lang, do-lang... */

char geomview_version[] = GEOMVIEW_VERSION;
extern char *buildinfographics;	/* Possibly initialized elsewhere */

void lang_init()
{
  LInit();
  lispext_init();
  clang_init();

  /*
   * Functions that don't use LDECLARE need to be LDefun'ed
   * here  because they are not part of the "clang" interface.
   */
  {
    extern LObject *Lread(Lake *, LList *);
    extern char Hread[];
    extern LObject *Lmerge(Lake *, LList *);
    extern char Hmerge[];
    extern LObject *Lhdefine(Lake *, LList *);
    extern char Hhdefine[];
    extern LObject *LND_xform(Lake *, LList *);
    extern char HND_xform[];

    LDefun("read", Lread, Hread);
    LDefun("merge", Lmerge, Hmerge);
    LDefun("hdefine", Lhdefine, Hhdefine);
  }

  /*
   * Define the language keywords
   */
  define_keyword("yes",	YES_KEYWORD);
  define_keyword("no",	NO_KEYWORD);
  define_keyword("on",	ON_KEYWORD);
  define_keyword("off",	OFF_KEYWORD);
  define_keyword("0",	ZERO_KEYWORD);
  define_keyword("1",	ONE_KEYWORD);
  
  define_keyword("none", NONE_KEYWORD);
  define_keyword("each", EACH_KEYWORD);
  define_keyword("keep", KEEP_KEYWORD);
  define_keyword("all",	ALL_KEYWORD);
  
  define_keyword("euclidean", EUCLIDEAN_KEYWORD);
  define_keyword("hyperbolic", HYPERBOLIC_KEYWORD);
  define_keyword("spherical", SPHERICAL_KEYWORD);

  define_keyword("virtual", VIRTUAL_KEYWORD);
  define_keyword("projective", PROJECTIVE_KEYWORD);
  define_keyword("conformal-ball", CONFORMALBALL_KEYWORD);
  define_keyword("conformal", CONFORMALBALL_KEYWORD);
  
  define_keyword("tiff", TIFF_KEYWORD);
  define_keyword("frame", FRAME_KEYWORD);

  define_keyword("camera", CAMERA_KEYWORD);
  define_keyword("geometry", GEOM_KEYWORD);
  define_keyword("transform", TRANSFORM_KEYWORD);
  define_keyword("ntransform", NTRANSFORM_KEYWORD);
  define_keyword("command", COMMAND_KEYWORD);
  define_keyword("window", WINDOW_KEYWORD);
  define_keyword("image", IMAGE_KEYWORD);
  define_keyword("appearance", APPEARANCE_KEYWORD);
  
  define_keyword("translate", TRANSLATE_KEYWORD);
  define_keyword("e-translate", E_TRANSLATE_KEYWORD);
  define_keyword("h-translate", H_TRANSLATE_KEYWORD);
  define_keyword("s-translate", S_TRANSLATE_KEYWORD);
  
  define_keyword("translate-scaled", TRANSLATE_SCALED_KEYWORD);
  define_keyword("e-translate-scaled", E_TRANSLATE_SCALED_KEYWORD);
  define_keyword("h-translate-scaled", H_TRANSLATE_SCALED_KEYWORD);
  define_keyword("s-translate-scaled", S_TRANSLATE_SCALED_KEYWORD);
  
  define_keyword("rotate", ROTATE_KEYWORD);
  
  define_keyword("scale", SCALE_KEYWORD);
  define_keyword("zoom", ZOOM_KEYWORD);

  define_keyword("horizontal", HORIZONTAL_KEYWORD);
  define_keyword("vertical", VERTICAL_KEYWORD);
  define_keyword("colored", COLORED_KEYWORD);

  define_keyword("toggle", TOGGLE_KEYWORD);

  define_keyword("smooth", SMOOTH_KEYWORD);

  define_keyword("bbox-center", BBOX_CENTER_KEYWORD);
  define_keyword("origin", ORIGIN_KEYWORD);

  define_keyword("focus-change", FOCUS_CHANGE_KEYWORD);
  define_keyword("mouse-cross", MOUSE_CROSS_KEYWORD);

  define_keyword("inertia", MOTION_INERTIA_KEYWORD);
  define_keyword("constrain", MOTION_CONSTRAIN_KEYWORD);
  define_keyword("own-coordinates", MOTION_OWN_COORDS_KEYWORD);
  
  /*
   * Define various synonyms
   */
  LDefun("!", Lshell,
	 "(! COMMAND)\n`!' is a synonym for \"shell\"");
  LDefun("|", Lemodule_run,
	 "(| EMODULE)\n`|' is a synonym for \"emodule-run\"");
  LDefun("ui-emotion-program", Lui_emodule_define,
	 "(ui-emotion-program ...)\n"
	 "ui-emotion-program is an obsolete command."
	 "Use its new eqivalent \"emodule-define\" instead.");
  LDefun("ui-emotion-run", Lui_emodule_start,
	 "(ui-emotion-run ...)\n"
	 "ui-emotion-run is an obsolete command."
	 "Use its new eqivalent \"emodule_start\" instead.");
  LDefun("quit", Lexit,
	 "(quit)\n`quit' is a synonym for \"exit\"");
  LDefun("merge-base-ap", Lmerge_baseap,
	 "(merge-base-ap)\n`merge-base-ap' is a synonym for merge-baseap.");
  
  /*
   * And some extra help entries.
   */
  LHelpDef("ID",
	  "\nID is a string which names a geometry or camera. Besides\n\
	those you create, valid ones are:\n\
\n\
	World, world,\n\
	    worldgeom, g0:    the collection of all geom's\n\
	target:			selected target object (cam or geom)\n\
	center:			selected center-of-motion object\n\
	targetcam:		last selected target camera\n\
	targetgeom:		last selected target geom\n\
	focus:			camera where cursor is (or most\n\
				recently was)\n\
	allgeoms:		all geom objects\n\
	allcams:		all cameras\n\
	default, defaultcam:  prototype; future cameras inherit\n\
				default's settings\n\
\n\
	The following IDs are used to name coordinate systems,\n\
	e.g. in \"pick\" and \"write\" commands:\n\
\n\
	world, World, etc.:   the world, within which all other\n\
				geoms live.\n\
	universe, Universe:	the universe, in which the World, lights\n\
				and cameras live.  Cameras' world2cam\n\
				transforms might better be called\n\
				universe2cam, etc.\n\
	self:		      \"this geomview object\".	 Transform from\n\
				an object to \"self\" is the identity;\n\
				writing its geometry gives the object\n\
				itself with no enclosing transform;\n\
				picked points appear in the object's\n\
				coordinates.\n\
	primitive:	      (for \"pick\" only) Picked points appear in\n\
				the coordinate system of the\n\
				lowest-level OOGL primitive.\n\
\n\
	A name is also an acceptable id.  Given names are made unique\n\
	by appending numbers if necessary (i.e. \"foo<2>\"). Every geom\n\
	is also named g[n] and every camera is also named c[n] (\"g0\"\n\
	is always the worldgeom): this name is used as a prefix to\n\
	keyboard commands and can also be used as a command language\n\
	id.  Numbers are reused after an object is deleted. Both names\n\
	are shown in the Object browser.");
  
  LHelpDef("CAM-ID",
	   "\nCAM-ID is an ID that refers to a camera.");
  
  LHelpDef("GEOM-ID",
	   "\nGEOM-ID is an ID that refers to a geometry.");
  
  LHelpDef("GEOMETRY",
	   "\nGEOMETRY is an OOGL geometry specification.");
  
  LHelpDef("CAMERA",
	   "\nCAMERA is an OOGL camera specification.");
  
  LHelpDef("APPEARANCE",
	   "\nAPPEARANCE is an OOGL appearance specification.");
  
  LHelpDef("IMAGE",
	   "\nIMAGE is an OOGL image specification.");

  LHelpDef("TRANSFORM",
	   "\nTRANSFORM is an OOGL 4x4 transformation matrix.");

  LHelpDef("NTRANSFORM",
	   "\nNTRANSFORM is an OOGL NxM transformation matrix.");
}

HandleOps *keyword2ops(Keyword keyword)
{
  switch (keyword) {
  case CAMERA_KEYWORD:     return &CamOps;
  case GEOM_KEYWORD:       return &GeomOps;
  case TRANSFORM_KEYWORD:  return &TransOps;
  case NTRANSFORM_KEYWORD: return &NTransOps;
  case COMMAND_KEYWORD:    return &CommandOps;
  case WINDOW_KEYWORD:     return &WindowOps;
  case APPEARANCE_KEYWORD: return &AppearanceOps;
  case IMAGE_KEYWORD:      return &ImageOps;
  default: return NULL;
  }
}

/**********************************************************************/

LDEFINE(shell, LVOID,
	"(shell SHELL-COMMAND)\n"
	"Execute the given UNIX SHELL-COMMAND using /bin/sh. Geomview"
	"waits for it to complete and will be unresponsive until it does.")
{
  char *cmd;
  int status;
  void (*oldsigchld)();
  LDECLARE(("shell", LBEGIN,
	    LSTRINGS, &cmd,
	    LEND));
  oldsigchld = signal(SIGCHLD, SIG_DFL);
  status = system(cmd);
  signal(SIGCHLD, oldsigchld);
  return LNew(LINT, &status);
}


LDEFINE(write_sexpr, LVOID,
       "(write-sexpr     FILENAME LISPOBJECT)\n\
	Writes the given LISPOBJECT to FILENAME. This function is intended\n\
	for internal debugging use only.")
{
  LObject *obj;
  char *filename;
  LDECLARE(("write-sexpr", LBEGIN,
	    LSTRING, &filename,
	    LHOLD, LLOBJECT, &obj,
	    LEND));
  LWriteFile(filename, obj);
  return Lt;
}

LDEFINE(write_handle, LVOID,
	"(write-handle HANDLEOPS FILENAME HANDLE)\n"
	"Writes the object underlying the given handle to FILENAME."
	"This function is intended for internal debugging use only.")
{
  char *prefix, *filename, *handle;
  Handle *h;
  HandleOps *ops;
  Pool *p;
  
  LDECLARE(("write-handle", LBEGIN,
	    LSTRING, &prefix,
	    LSTRING, &filename,
	    LSTRING, &handle,
	    LEND));

  if ((ops = HandleOpsByName(prefix)) != NULL &&
      (h = HandleByName(handle, ops)) != NULL) {
    REFPUT(h);
    if (h->ops->strmout) {
      bool old_objsaved = h->obj_saved;
      FILE *outf = NULL;
      if (strcmp(filename, "-") == 0) {
	outf = stdout;
      }
      p = PoolStreamTemp(filename, NULL, outf, 1, &CommandOps);
      h->obj_saved = false;
      h->ops->strmout(p, h, HandleObject(h));
      h->obj_saved = old_objsaved;
      PoolClose(p);
      PoolDelete(p);
    }
  } else {
    OOGLWarn("Cannot find handle \"%s@%s\"", handle, prefix);
  }
  
  return Lt;
}

LDEFINE(dump_handles, LVOID,
	"(dump-handles)\n"
	"Dump the list of currently active handles to stdout."
	"This function is intended for internal debugging use only.")
{
  extern void handle_dump(void);

  LDECLARE(("dump-handles", LBEGIN,
	    LEND));

  handle_dump();

  return Lt;
}

LDEFINE(dump_pools, LVOID,
	"(dump-pools)\n"
	"Dump the list of currently active input-\"pools\" to stdout."
	"This function is intended for internal debugging use only.")
{
  extern void pool_dump(void);

  LDECLARE(("dump-pools", LBEGIN,
	    LEND));

  pool_dump();

  return Lt;
}

LDEFINE(geomview_version, LSTRING,
	"(geomview-version)\n\
	Returns a string representing the version of geomview that is\n\
	running.")
{
  char buf[128];
  char *s;
  LDECLARE(("geomview-version", LBEGIN,
	    LEND));
  sprintf(buf, buildinfographics ? "%s-%s" : "%s",
		geomview_version, buildinfographics);
  s = strdup(buf);
  return LNew( LSTRING, &s );
}

bool boolval(char *s, Keyword keyword)
{
  switch (keyword) {
  case YES_KEYWORD:
  case ON_KEYWORD:
  case ONE_KEYWORD:
    return true;
  case NO_KEYWORD:
  case OFF_KEYWORD:
  case ZERO_KEYWORD:
    return false;
  default:
    fprintf(stderr, "%s: %s is not a boolean keyword; assuming \"no\"\n",
	    s, keywordname(keyword));
    return false;
  }
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
