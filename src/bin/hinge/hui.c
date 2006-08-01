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

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>
#include "ooglutil.h"
#include "3d.h"
#include "hinge.h"
#include "hui.h"
#include "version.h"

#include "panel.h"

int mainplacement = FL_PLACE_MOUSE;
int helpplacement = FL_PLACE_MOUSE;
int infoplacement = FL_PLACE_MOUSE;
int fileplacement = FL_PLACE_MOUSE;

char hingehelpstr[] =
#include "hingehelp.h"
;

static char *gv_getline(char *s);

void hui_init(void)
{
  char buf[120];

#ifdef XFORMS
  FL_INITIALIZE("Hinge");
#else
  fl_init();
  foreground();
#endif

  create_the_forms();
  
  sprintf(buf, "%1g", angle);
  fl_set_input(AngleInput, buf);
  
  fl_add_browser_line(SpaceBrowser, "Euclidean");
  fl_add_browser_line(SpaceBrowser, "Hyperbolic");
  fl_add_browser_line(SpaceBrowser, "Spherical");

  switch (space) {
  case EUCLIDEAN:
    fl_select_browser_line( SpaceBrowser, 1 );
    break;
  case HYPERBOLIC:
    fl_select_browser_line( SpaceBrowser, 2 );
    break;
  case SPHERICAL:
    fl_select_browser_line( SpaceBrowser, 3 );
    break;
  }
  
  {
    FILE *hf = fopen("hingehelp", "r");

    if (hf == NULL) {
      char *line;
      line = gv_getline(hingehelpstr);
      while (line) {
	fl_add_browser_line( HelpBrowser, line );
	line = gv_getline(NULL);
      }
      fl_set_browser_topline( HelpBrowser, 1 );
    } else {
      if (hf != NULL) {
	while (fgets(buf, 120, hf) != NULL)
	  fl_add_browser_line( HelpBrowser, buf );
	fl_set_browser_topline( HelpBrowser, 1 );
	fclose(hf);
      }
    }
  }
}

void hui_main_loop(IOBFILE *inf)
{
  fd_set fdmask;
  static struct timeval timeout0 = {0, 200000};
  struct timeval timeout;

  if (mainplacement == FL_PLACE_POSITION) {
    fl_set_form_position(MainForm, mainpos[0], mainpos[1]);
  }
  if (fileplacement == FL_PLACE_POSITION) {
    fl_set_form_position(FileForm, filepos[0], filepos[1]);
  }
  if (helpplacement == FL_PLACE_POSITION) {
    fl_set_form_position(HelpForm, helppos[0], helppos[1]);
  }
  if (infoplacement == FL_PLACE_POSITION) {
    fl_set_form_position(InfoForm, infopos[0], infopos[1]);
  }

  {
    char buf[20];
    sprintf(buf, "Hinge %s", HINGE_VERSION);
    fl_set_object_label(MainFormLabel, buf);
    fl_set_object_label(InfoFormLabel, buf);
  }

  fl_show_form(MainForm, mainplacement, TRUE, "Hinge");

  while (1) {

    FD_ZERO(&fdmask);
    FD_SET(iobfileno(inf), &fdmask);
    timeout = timeout0;
    select(iobfileno(inf)+1, &fdmask, NULL, NULL, &timeout);

    if (async_iobfnextc(inf, 0) != NODATA) {
      Input();
    }
    fl_check_forms();
  }
}

void SpaceBrowserProc(FL_OBJECT *obj, long val)
{
  int line = fl_get_browser(obj);
  switch (line) {
  case 1:
    HingeSpace(EUCLIDEAN);
    break;
  case 2:
    HingeSpace(HYPERBOLIC);
    break;
  case 3:
    HingeSpace(SPHERICAL);
    break;
  }
}


void QuitProc(FL_OBJECT *obj, long val)
{
  if (killgv) {
    fprintf(togv, "(quit)\n");
    fflush(togv);
  }
  exit(0);
}


void FileButtonProc(FL_OBJECT *obj, long val)
{
  int winid;
  winid = fl_show_form(FileForm,fileplacement,TRUE,"Hinge File");
}


void FileCancelButtonProc(FL_OBJECT *obj, long val)
{
  fl_hide_form(FileForm);
}


void FileOKButtonProc(FL_OBJECT *obj, long val)
{
  char *str = (char *)fl_get_input(FileInput);

  if (HingeLoad(str))
    fl_hide_form(FileForm);
}

void FileInputProc(FL_OBJECT *obj, long val)
{
  /*noop*/
}

void UndoButtonProc(FL_OBJECT *obj, long val)
{
  Undo();
}

void
hui_message(char *s)
{
#if 0
  printf("message: %s\n",s);
#endif
}

extern void AngleInputProc(FL_OBJECT *obj, long val)
{}

void InfoButtonProc(FL_OBJECT *obj, long val)
{
  int winid;
  winid = fl_show_form(InfoForm,infoplacement,TRUE,"Hinge Info Window");
}



void HelpButtonProc(FL_OBJECT *obj, long val)
{
  int winid;
  winid = fl_show_form(HelpForm,helpplacement,TRUE,"Hinge Help Window");
}


void InfoOKButtonProc(FL_OBJECT *obj, long val)
{
  fl_hide_form(InfoForm);
}



void HelpOKButtonProc(FL_OBJECT *obj, long val)
{
  fl_hide_form(HelpForm);
}


void ResetButtonProc(FL_OBJECT *obj, long val)
{
  Reset();
}


void HingeIt(int dir)
{
  float ang, initang, limit;
  int instno;

  initang = 0;
  angle = atof(fl_get_input(AngleInput));
  limit = initang + angle;
  
  /* create a new inst */
  instno = NewInst(initang);

  /* hinge the inst by steps of hingeincr up to limit */
  for (ang=initang+hingeincr; ang<limit; ang += hingeincr) {
    Inst(instno, (float)(dir * ang));
  }
  Inst(instno, (float)(dir * limit));

}

static char *gv_getline(char *s)
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
