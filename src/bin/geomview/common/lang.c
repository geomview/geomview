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
char *buildinfographics;	/* Possibly initialized elsewhere */

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
  define_keyword("command", COMMAND_KEYWORD);
  define_keyword("window", WINDOW_KEYWORD);
  
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

  
  /*
   * Define various synonyms
   */
  LDefun("!", Lshell,   "! is a synonym for \"shell\"");
  LDefun("|", Lemodule_run, "| is a synonym for \"emodule-run\"");
  LDefun("ui-emotion-program", Lui_emodule_define,
	  "ui-emotion-program is an obsolete command.\n\
	Use its new eqivalent \"emodule-define\" instead.");
  LDefun("ui-emotion-run", Lui_emodule_start,
	  "ui-emotion-run is an obsolete command.\n\
	Use its new eqivalent \"emodule_start\" instead.");
  LDefun("quit", Lexit, "quit is a synonym for \"exit\"");
  LDefun("merge-base-ap",	Lmerge_baseap,
	  "merge-base-ap is a synonym for merge-baseap.");
  
  /*
   * And some extra help entries.
   */
  LHelpDef("ID",
	  "ID is a string which names a geometry or camera.  Besides\n\
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
	universe:	      the universe, in which the World, lights\n\
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
	  "CAM-ID is an ID that refers to a camera.");
  
  LHelpDef("GEOM-ID",
	  "GEOM-ID is an ID that refers to a geometry.");
  
  LHelpDef("GEOMETRY",
	  "GEOMETRY is an OOGL geometry specification.");
  
  LHelpDef("CAMERA",
	  "CAMERA is an OOGL camera specification.");
  
  LHelpDef("APPEARANCE",
	  "APPEARANCE is an OOGL appearance specification.");
  
  LHelpDef("TRANSFORM",
	  "TRANSFORM is an OOGL 4x4 transformation matrix.");
}

HandleOps *keyword2ops(int keyword)
{
  switch (keyword) {
  case CAMERA_KEYWORD:		return &CamOps;
  case GEOM_KEYWORD: 		return &GeomOps;
  case TRANSFORM_KEYWORD: 	return &TransOps;
  case COMMAND_KEYWORD: 	return &CommandOps;
  case WINDOW_KEYWORD: 		return &WindowOps;
  default: return NULL;
  }
}

char *keywordname(int keyword)
{
  switch (keyword) {
  case NO_KEYWORD:		return "no";
  case YES_KEYWORD:		return "yes";
  case ON_KEYWORD:		return "on";
  case OFF_KEYWORD:		return "off";
  case ZERO_KEYWORD:		return "no";
  case ONE_KEYWORD:		return "yes";
  case NONE_KEYWORD:		return "none";
  case EACH_KEYWORD:		return "each";
  case KEEP_KEYWORD:		return "keep";
  case ALL_KEYWORD:		return "all";
  case EUCLIDEAN_KEYWORD:	return "euclidean";
  case HYPERBOLIC_KEYWORD:	return "hyperbolic";
  case SPHERICAL_KEYWORD:	return "spherical";
  case VIRTUAL_KEYWORD:		return "virtual";
  case PROJECTIVE_KEYWORD:	return "projective";
  case CONFORMALBALL_KEYWORD:	return "conformal";
  case TIFF_KEYWORD:		return "tiff";
  case FRAME_KEYWORD:		return "frame";
  case CAMERA_KEYWORD:		return "camera";
  case GEOM_KEYWORD: 		return "geometry";
  case TRANSFORM_KEYWORD: 	return "transform";
  case COMMAND_KEYWORD: 	return "command";
  case WINDOW_KEYWORD: 		return "window";

  case TRANSLATE_KEYWORD:	return "translate";
  case E_TRANSLATE_KEYWORD:	return "e-translate";
  case H_TRANSLATE_KEYWORD:	return "h-translate";
  case S_TRANSLATE_KEYWORD:	return "s-translate";

  case TRANSLATE_SCALED_KEYWORD:	return "translate-scaled";
  case E_TRANSLATE_SCALED_KEYWORD:	return "e-translate-scaled";
  case H_TRANSLATE_SCALED_KEYWORD:	return "h-translate-scaled";
  case S_TRANSLATE_SCALED_KEYWORD:	return "s-translate-scaled";

  case ROTATE_KEYWORD:		return "rotate";

  case SCALE_KEYWORD:		return "scale";
  case ZOOM_KEYWORD:		return "zoom";

  case HORIZONTAL_KEYWORD:	return "horizontal";
  case VERTICAL_KEYWORD:	return "vertical";
  case COLORED_KEYWORD:		return "colored";

  default: return "???";
  }
}

/**********************************************************************/

LDEFINE(shell, LVOID,
"(shell         SHELL-COMMAND)\n\
	Execute the given UNIX SHELL-COMMAND using /bin/sh.  Geomview\n\
	waits for it to complete and will be unresponsive until it does.")
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

int boolval(char *s, int keyword)
{
  switch (keyword) {
  case YES_KEYWORD:
  case ON_KEYWORD:
  case ONE_KEYWORD:
    return 1;
  case NO_KEYWORD:
  case OFF_KEYWORD:
  case ZERO_KEYWORD:
    return 0;
  default:
    fprintf(stderr, "%s: %s is not a boolean keyword; assuming \"no\"\n",
	    s, keywordname(keyword));
    return 0;
  }
}

