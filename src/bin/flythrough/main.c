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

#include <stdlib.h>
#include <stdio.h>
#include "forms.h"
#include "xforms-compat.h"
#include "flythrough.h"
#include "panel.h"

float dodecscale;
FILE *f;
int go;
int speed;
int whichpath;
int helpwinid = 0;
int mainwinid = 0;


char flyhelpstr[] =
#include "flyhelp.h"
;

static char *getline(char *s);

void init(void)
{

#ifdef XFORMS
  FL_INITIALIZE("Flythrough");
#else
  fl_init();
  foreground();
#endif

  create_the_forms();

  printf("(progn \n");
  printf("(geometry notknot.vect { INST transforms : tile geom {");
  printf("    INST geom < dodec.vect transform : scale}})\n");
  printf(" (space hyperbolic) (bbox-draw allgeoms off)\n");
  printf(" (backcolor c0 0 0 0)\n");
  printf(" (merge camera c0 {fov 100})\n");
  printf(" (merge-ap notknot.vect {linewidth 2})\n");
  printf(" (load-path (. $GEOMDATA $GEOMDATA/geom $GEOMDATA/groups))\n");
  printf(" (echo 'caughtup\n')\n");
  printf(")\n");
  fflush(stdout);

  go = 1;
  fl_set_button(Go, 1);
  dodecscale = .99;  		/* scale dodecahedron */
  fl_set_slider_bounds(DodecScale, 0.01, 1.0);
  fl_set_slider_value(DodecScale, dodecscale); 
  ScaleProc(DodecScale, dodecscale);
  speed = 2;			/* speed */
  fl_set_button(Speed2, 1); 
  SpeedProc(Speed2,2);
  fl_set_button(Loop, 1); 	/* path */
  PathProc(Loop, LOOP);
  fl_set_button(Level2, 1); 	/* tile depth */
  TilingProc(Level2, 2);

  fl_set_form_position(MainForm,8,8);
  mainwinid = fl_show_form(MainForm, FL_PLACE_POSITION, TRUE, "Flythrough");
}

void TilingProc(FL_OBJECT *obj, long val) 
{
  printf("(read geometry {define tile { < br4.%1d.tlist}})\n", (int)val);
  fflush(stdout);
}

void SpeedProc(FL_OBJECT *obj, long val) 
{
  speed = val;
  PathProc(NULL, whichpath);
}

void GoProc(FL_OBJECT *obj, long val) 
{
  go = val;
}

void ScaleProc(FL_OBJECT *obj, long val)
{
  float scale = fl_get_slider_value(obj);
  printf("(read transform {define scale {1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 %f}})\n", 1.0/scale);
  fflush(stdout);
}


void PathProc(FL_OBJECT *obj, long val)
{
  char *first = NULL;
  char *name;
  char path[512];

  whichpath = val;

  switch(whichpath) {
  case LOOP: 
    first = "loop"; break; 
  case QUARTER:
    first = "quart"; break;
  case DIRECT:
    first = "dir"; break;
  case EQUI:
    first = "equi"; break;
  }
  
  fclose(f);  
  /* GEOMDATA environment variable is supposed to be set by geomview. 
     We'll assume it's correct and just whine if we can't find anything.
     Note the setenv hint for the user.
   */
  if ((name = getenv("GEOMDATA")) == NULL) 
    sprintf(path,"%s.%1d.gv", first, speed);
  else 
    sprintf(path,"%s/groups/%s.%1d.gv", name, first, speed);
  f = fopen(path, "r");
  if (!f) {
    fprintf(stderr, "Can't find path file %s.\nTry setting environment variable GEOMDATA.\n", path);
    exit(0);
  } else 
    rewind(f);

  /* Just in case someone recentered the camera and the fov got reset to 40 */
  printf(" (merge camera c0 {fov 100})\n");
  fflush(stdout);
}

static char todemogv[] = "togeomview -c flythrough  geomview -nopanels -wpos 200x200@559,535 flythrough_diagram.gv";

void InfoProc(FL_OBJECT *obj, long val) 
{
  FILE *hf = fopen("flyhelp", "r");
  char gvstr[256];

  if (hf == NULL) {
    char *line;
    line = getline(flyhelpstr);
    while (line) {
      fl_add_browser_line( HelpBrowser, line );
      line = getline(NULL);
    }
    fl_set_browser_topline( HelpBrowser, 1 );
  } else {
    fl_load_browser(HelpBrowser, "flyhelp");
  }
  fclose(hf);
  fl_set_button(Info, 1);
  fl_set_button(EucDiag, 1);
  fl_set_button(HypDiag, 0);
  fl_set_form_position(HelpForm,8, 393);
  helpwinid = fl_show_form(HelpForm, FL_PLACE_POSITION, TRUE, "Flythrough Help");
  if (helpwinid) winset(helpwinid);
  (void)winpop();
  winset(mainwinid);
  /* OK, so this is probably not the most elegant way to do this.
     It does work though...
   */
  sprintf(gvstr,"%s < /dev/null&", todemogv);
  system(gvstr);
}

void DiagProc(FL_OBJECT *obj, long val) 
{
  char str[256];
  sprintf(str, "echo '(space %s)' | %s &", 
	  (val == EUC) ? "euclidean" : "hyperbolic", todemogv);
  system(str);
}

void DoneProc(FL_OBJECT *obj, long val) 
{
  char str[256];
  if (helpwinid) fl_hide_form(HelpForm);
  helpwinid = 0;
  sprintf(str, "echo '(exit)' | %s &", todemogv);
  system(str);
  fl_set_button(Info, 0);
}

void QuitProc(FL_OBJECT *obj, long val) 
{
  DoneProc(NULL, 0);
  exit(0);
}

int main(int argc, char *argv[])
{
  char line[80];
  char *more, *caughtup;
  int i, turbo;

  init();
  while (--argc > 0) {
    if (argv[1][0] == '-' && argv[1][1] == 't')
      turbo = 1;
    else if (argv[1][0] == '-' && argv[1][1] == 'h')
      InfoProc(NULL, 0);
    argv++;
  }
  while (1) {
    fl_check_forms();
    if (go)
      if (turbo || (caughtup = fgets(line, 10, stdin))) {
      /* a command is exactly 18 lines long due to the vagaries of
	 Mathematica... */
      for (i = 0; i < 18; i++) {
	if (! (more = fgets(line, 80, f))) {
	  rewind(f);
	  break;
	} else {
	  fputs(line, stdout);
	}
      }
      if (!turbo) printf("(echo 'caughtup\n')\n");
      fflush(stdout);
    }
  }
  return 0;
}

/* Stolen from Mark Phillips' Hinge module */
static char *getline(char *s)
{
  static char *p;
  char *first;

  if (s != NULL) {
    p = s;
  } else {
    *p = '\n';
  }
  ++p;
  first = p;
  while (*p != '\n' && *p != '\0') ++p;
  if (*p == '\n') {
    *p = '\0';
    return first;
  }
  return NULL;
}
