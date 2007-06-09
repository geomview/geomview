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

/* static char *copyright = "Copyright (C) 1992 The Geometry Center"; currently unused*/

/* Authors: Stuart Levy, Tamara Munzner, Mark Phillips */

#include "drawer.h"
#include "event.h"
#include "main.h"
#include "ui.h"
#include "comm.h"
#include "geom.h"
#include "camera.h"
#include "pick.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>

#include <sys/stat.h>
#include <sys/file.h>	
#ifdef NeXT
#include <libc.h>
#else
#include <unistd.h>
#endif
#if HAVE_LOCALE_H
# include <locale.h>
#endif

#define STREQ(s1,s2) (strcmp(s1,s2)==0)

static char *initfilename = ".geomview";

char *geomview_system_initfile = NULL;
int do_init = 1;

Color initial_defaultbackcolor = {1.0/3.0, 1.0/3.0, 1.0/3.0 };
static char *pipesubdir = "geomview";

int gv_nwins = 1;
int gv_no_opengl = 0;

vvec vv_load_path;
vvec vv_emodule_path;

/* Default backstop appearance */
static char apdefault[] = "\
appearance { \
	+face \
	-edge \
	+vect \
	normscale 1.0 \
	linewidth 1 \
	patchdice 10 10 \
	-normal \
	+evert \
	shading flat \
	-transparent \
	material { \
		kd 1.0 diffuse 1 1 1 \
		ka 0.3 ambient 1 1 1 \
		ks 0.3 specular 1 1 1 \
		shininess 15 \
		edgecolor 0 0 0 \
		normalcolor 1 1 1 \
		alpha 1 \
	} \
	lighting { \
		ambient .2 .2 .2 \
		replacelights \
		light { color .75 .75 .75 position 0 0 10 0 } \
		light { color .6 .6 .6 position 0 1 -1 0 } \
		light { color .4 .4 .4 position 1 -2 -1 0 } \
	} \
}";

/*
 * Default geometry to use when drawing cameras.
 */
static char defaultcam[] = "\
appearance {linewidth 4}\n\
VECT\n\
7 17 4\n\
2 2  2 2 2 2  5\n\
1 1  1 0 0 0  1\n\
-.5 -.5 -.5  0 -.5 -.5\n\
-.5 -.5 -.5  -.5 0 -.5\n\
0 0 0   .5 .5 -.5\n\
0 0 0   -.5 -.5 -.5\n\
0 0 0   -.5 .5 -.5\n\
0 0 0   .5 -.5 -.5\n\
-.5 0 -.5  -.5 .5 -.5  .5 .5 -.5  .5 -.5 -.5  0 -.5 -.5\n\
1 0 0 1\n\
0 1 0 1\n\
0 0 1 1\n\
0 0 0 1";

static char windefault[] = "{ size 450 450  resize }";

void usage(char *me, char *morestuff)
{
  fprintf(stderr, "\
Usage: %s [-b r g b] [-c commandfile] [-wins #windows]\n\
	[-nopanels] [-noinit] [-noopengl]\n\
        [-wpos width,height[@xmin,ymin]] [-wpos -] [-M[gc][sp] pipename]\n\
        [-start external-module-string args ... --]\n\
        [-run external-module-pathname args ... --]\n\
%s	[ file ... ]\n", me, morestuff);
  exit(1);
}

static void
parse_emodule(int room, int *acp, char ***avp, char *first, char *firstname, char *indir)
{
  int i;
  char *cmd;
  char *next;
  emodule new;
  int baselen;

  for (i = 1; i <= *acp && !STREQ((next = (*avp)[i]), "--"); i++)
    room += strlen(next)+1+1;
  if (room) {
    cmd = OOGLNewNE(char, room, "emod cmd");
    if (first) {
      strcpy(cmd, first);
      strcat(cmd, " ");
    } else 
      cmd[0] = '\0';
    baselen = strlen(cmd);
    while (*acp > 0 && !STREQ((next = *++(*avp)), "--")) {
      strcat(cmd, next);
      strcat(cmd, " ");
      (*acp)--;
    }
    new.text = strdup(cmd);
    if(firstname) {
      new.name = OOGLNewNE(char, room + strlen(firstname) + 1, "emod name");
      sprintf(new.name, "%s %s", firstname, cmd+baselen);
    } else {
      new.name = strdup(cmd);
    }
    new.data = NULL;
    new.dir = indir;
    emodule_run(&new);
  }
}

int parse_common_arg(int *acp, char ***avp)
{
  int ac = *acp;
  char **av = *avp;
  char *arg = *av;
  int ok = 1;
  int i;

  if (STREQ(arg,"-b") && (ac-=3) > 0) {
    initial_defaultbackcolor.r = atof(*++av);
    initial_defaultbackcolor.g = atof(*++av);
    initial_defaultbackcolor.b = atof(*++av);
    drawer_color(DEFAULTCAMID, DRAWER_BACKCOLOR, &initial_defaultbackcolor);
  }
  else if(STREQ(arg, "-wins") && --ac > 0) {
    gv_nwins = atoi(*++av);
  }
  else if((STREQ(arg, "-wpos") || STREQ(arg, "-w")) && --ac > 0) {
    WindowStruct ws;
    WnPosition wp;
    int a1,a2,a3,a4;
    char ch;
    static char rats[] = "geomview: -wpos: expected width,height[@xmin,ymin]";

    ws.wn = WnCreate(WN_ENLARGE, 1, WN_SHRINK, 1, WN_END);
    ws.h = NULL;

    switch(sscanf(*++av, "%d%*c%d%c%d%*c%d", &a1,&a2,&ch,&a3,&a4)) {
    case 2:
      WnSet(ws.wn, WN_XSIZE, a1, WN_YSIZE, a2, WN_END);
      break;
    case 1:
      wp.xmin = a1;
      if((ac -= 3) < 0) {
	OOGLError(0, rats);
	exit(1);
      }
      wp.ymin = atoi(*++av);
      wp.xmax = wp.xmin + atoi(*++av)-1;
      wp.ymax = wp.ymin + atoi(*++av)-1;
      break;
    case 5:
      if(ch == '@') {	/* xsize,ysize@xmin,ymin */
	wp.xmin = a3; wp.xmax = a3 + a1-1;
	wp.ymin = a4; wp.ymax = a4 + a2-1;
      } else {		/* xmin,ymin,xsize,ysize */
	wp.xmin = a1; wp.ymin = a2; wp.xmax = a1+a3-1; wp.ymax = a2+a4-1;
      }
      WnSet(ws.wn, WN_PREFPOS, &wp, WN_END);
      break;
    default:
      if(!STREQ(arg, "-")) {
	OOGLError(0, rats);
	exit(1);
      }
      /* "-" => interactive positioning -- fall through */
    }
    gv_window(DEFAULTCAMID, &ws);
  } else if(STREQ(arg, "-debug")) {
    /* -debug -- don't catch SIGINT, SIGSEGV, etc. signals */
    /* -debug and -noopengl are handled in init_geomview()
    gv_debug = 1;
    */
  } else if (STREQ(arg, "-noopengl")) {
    /* -debug and -noopengl are handled in init_geomview()
    gv_no_opengl = 1;
    */
  } else if(STREQ(arg, "-c") && --ac > 0) {
    /* -c -- Process command file (or string) */
    gv_load(*++av, "command");
  } else if(!strncmp(arg, "-M", 2) && --ac > 0) {
    /* -M[cg][ps[un|in|in6]] pipename|port  -- external connection */
    char pipedir[PATH_MAX];
    char *tmpdir = getenv("TMPDIR");
    
    if (tmpdir == NULL) {
      tmpdir = "/tmp";
    }
    snprintf(pipedir, PATH_MAX, "%s/%s", tmpdir, pipesubdir);
    usepipe(pipedir, *++av, &arg[2]);
  } else if(strncmp(arg, "-nop", 4) == 0) {	/* "-nopanels" */
    /* -nopanels -- Suppress all possible panels */
    for(i=0; i<30; i++)
      ui_showpanel(i, 0);
  } else if(STREQ(arg, "-e") && --ac > 0) {
    /* -e -- start an e-module with no arguments */
    gv_ui_emodule_start(*++av);
  } else if(STREQ(arg, "-start") && --ac > 0) {
    /* -start emodule-name args ... */
    emodule *old;
    int room;
    char *name;
    ui_emodule_index((name = *++av), &old);
    if (old) {
      room = strlen(old->text) +1 +1;
      ac--;
      parse_emodule(room, &ac, &av, old->text, old->name, old->dir);
    } else {
      OOGLError(0, "No external module named %s.", name);
    }
  } else if(STREQ(arg, "-run") && --ac > 0) {
    /* -run program-name args ... -- run program as e-module */
    parse_emodule(0, &ac, &av, NULL, NULL, NULL);
  } else if(STREQ(arg, "-time")) {
    /* Enable timing as with ctrl-T */
    timing(999999);
  } else if(STREQ(arg, "-noinit")) {
    /* Don't read any initialization files */
    do_init = 0;
  } else if(arg[0] == '-' && arg[1] != '\0') {
    ok = 0;		/* Unrecognized */
  } else {
    gv_load(arg, NULL);
  }
  *acp = ac;
  *avp = av;
  return ok;
}

void
init_geomview(int argc, char **argv)
{
  char *userhome, homefile[512], dotfile[512];
  struct stat homebuf, dotbuf;
  int i, homestat, dotstat;

#if HAVE_SETLOCALE && HAVE_DECL_LC_ALL
  setlocale(LC_ALL, "POSIX");
#endif

  if (getenv("GEOMVIEW_DEBUG"))
    gv_debug = 1;

  if (getenv("GEOMVIEW_NO_OPENGL"))
    gv_no_opengl = 1;
  
  for(i = 1; i < argc; i++)
    if(STREQ(argv[i], "-noinit"))
      do_init = 0;
    else if(STREQ(argv[i], "-debug"))
      gv_debug = 1;
    else if(STREQ(argv[i], "-noopengl"))
      gv_no_opengl = 1;

  drawer_init(apdefault, defaultcam, windefault);

  comm_init();

  event_init();
  ui_init();

  VVINIT(vv_load_path, char *, 3);
  VVINIT(vv_emodule_path, char *, 3);

  /*
    env_init() should arrange for vv_load_path, vv_emodule_path, and
    geomview_system_initfile to be set to whatever default values
    are appropriate for the environment.  On the IRIS this is done
    through environment variables set in a driving shell script
    (the IRIS version of env_init() is in gl/glmain.c).
    The NeXT uses a CONFIG.gv script in the Geomview.app wrapper.
  */

  env_init();

  if(do_init) {
    /* load system init file, if present */
    if (geomview_system_initfile) {
      if (access(geomview_system_initfile, R_OK) == 0) {
	gv_load(geomview_system_initfile, "command");
      }
    }


    /* now load ~/.geomview and ./.geomview, in that order, if present,
       unless they are the same, in which case just load once */

    userhome = getenv("HOME");
    sprintf(homefile,"%s/%s", userhome, initfilename);
    sprintf(dotfile,"./%s", initfilename);
      
    homestat =  stat(homefile, &homebuf);
    dotstat =  stat(dotfile, &dotbuf);
      
      
    if (homestat == 0) {
      if (access(homefile, R_OK) == 0)
	gv_load(homefile, "command");
      else
	OOGLError(0, "Warning: file %s present but unreadable; ignoring it.",
		  homefile);
    }
      
    if ( (dotstat == 0) && (   (dotbuf.st_dev != homebuf.st_dev)
			       || (dotbuf.st_ino != homebuf.st_ino))) {
      if (access(dotfile, R_OK) == 0)
	gv_load(dotfile, "command");
      else
	OOGLError(0, "Warning: file %s present but unreadable; ignoring it.",
		  dotfile);
    }
  }
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
