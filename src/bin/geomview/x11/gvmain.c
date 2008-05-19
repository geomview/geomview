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

#include "../common/main.h"
#include "../common/drawer.h"
#include "../common/ui.h"
#include "../common/comm.h"
#include "../common/lang.h"
#include "../common/worldio.h"
#include "mibload.h"
#include "gvui.h"

#include "string.h"

/* Floating-point handling strategy lifted from GSL - the GNU
 * Scientific Library. */
#if HAVE_FPU_CONTROL_H
#  include <fpu_control.h>
/* Apparently _FPU_SETCW was not available in libc5, but __setfpucw() was */
#  if !defined(_FPU_SETCW) && HAVE___SETFPUCW
#    if HAVE_i386_FPU_CONTROL_H
#      include <i386/fpu_control.h>
#    endif
#    define _FPU_SETCW(cw) __setfpucw(cw)
#  endif
#endif

#include <signal.h>

/* xgv main - global variables */
/*****************************************************************************/

				/* set initial camera background color */
extern	  HandleOps CommandOps, GeomOps;
extern	  void  ui_windowWillOpen(DView *dv);

/* private methods */
/*****************************************************************************/

static    void  parse_args(int argc, char *argv[]);
static    void  env_usage();

/*****************************************************************************/
	
int main(int argc, char **argv)
{
  int i;
  CameraStruct  cs;

  unsetenv("LD_PRELOAD");

#if defined(_FPU_SETCW)
  fpu_control_t hctrlword = _FPU_IEEE;
  _FPU_SETCW(hctrlword);
#endif

  signal(SIGFPE, SIG_IGN);   /* Ignore e.g. divide-by-zero traps */
			     /* (Need this at least for Alpha/OSF & FreeBSD) */

  load_interface(argc,argv);

  init_geomview(argc, argv);

  cam_mgdevice();		/* in gvcamui.c */

  parse_args(argc, argv);	/* load command & data file(s) */


  cs.h = NULL;
  cs.cam = NULL;

  for (i=0; i < gv_nwins; i++)
    if (!dview[i])
    {/*
      ui_windowWillOpen((DView *)NULL);
      */
       gv_new_camera(NULL, &cs);
    }


  ui_final_init();

  main_loop();

  return 0;
 
}

/*****************************************************************************/

void env_init()
{
  extern char *getenv();
  extern void set_load_path(char **);
  extern void set_emodule_path(char **);
#define MAXDIRS 100
  char *dirs[MAXDIRS];
  char *geomview_load_path = getenv("GEOMVIEW_LOAD_PATH");
  char *p, *geomview_emodule_path = getenv("GEOMVIEW_EMODULE_PATH");
  int i;

  if (geomview_load_path == NULL) {
    env_usage();
    exit(1);
  }

  geomview_system_initfile = getenv("GEOMVIEW_SYSTEM_INITFILE");

  if (geomview_load_path) {
    char *path = strdup(geomview_load_path);
    for(i=0, p = strtok(path, ":"); p; p = strtok(NULL, ":"), i++)
        dirs[i] = p;
    dirs[i] = NULL;
    set_load_path(dirs);
    free(path);
  }

  if (geomview_emodule_path) {
    char *path = strdup(geomview_emodule_path);
    for(i=0, p = strtok(path, ":"); p; p = strtok(NULL, ":"), i++)
        dirs[i] = p;
    dirs[i] = NULL;
    set_emodule_path(dirs);
    gv_emodule_sort();
    free(path);
  }

#undef MAXDIRS
}


/*****************************************************************************/

static void env_usage()
{
  fprintf(stderr,
"The shell environment variable GEOMVIEW_LOAD_PATH is not set;\n\
are you trying to run the geomview executable, gvx, directly?\n\
If so, try running the main geomview shell script, called\n\
\"geomview\" instead.  It takes care of setting this and other\n\
relevant environment variables for you.\n\
\n\
If for some reason you do not have the \"geomview\" shell\n\
script you should obtain a copy of geomview from www.geomview.org.\n\
\n\
Geomview expects the following environment variables to be\n\
set\n\
\n\
GEOMVIEW_LOAD_PATH\n\
    colon-separated list of directories that geomview should\n\
    look in for files you load\n\
\n\
GEOMVIEW_EMODULE_PATH\n\
    colon-separated list of directories that geomview should\n\
    look in for external modules (these appear in the\n\
    \"Applications\" browser on the main panel).\n\
\n\
GEOMVIEW_SYSTEM_INITFILE\n\
    pathname of a system-wide initialization file (.geomview)\n\
    to read upon startup\n");
}

/*****************************************************************************/

#define STREQ(s1,s2) (strcmp(s1,s2)==0)

/*****************************************************************************/

static void parse_args(int argc, char *argv[])
{

  char **av;
  int	 ac;

  for (ac = argc, av = argv+1; --ac > 0; av++)
  {
    if(!parse_common_arg(&ac, &av)) {
	usage(argv[0], "");
    }
  }
}

/*****************************************************************************/

/* there should be some way to avoid putting this here, but the call to ntsc
   is in the COMMON code.... (why?) :P */

int isntsc()
{  return 0; }

int ntsc(int on)
{
  return 0;
}

/*****************************************************************************/
/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
 
