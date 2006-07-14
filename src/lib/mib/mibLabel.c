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

#include "mibload.h"
#include "mibwidgets.h"

extern Display	*dpy;
extern GC	 mib_gc;

/* Code for Label */
/*****************************************************************************/

mib_Widget *mib_create_Label(mib_Widget *parent, char *name, char *label,
		int posx, int posy, int width, int height, int mib_fill)
{
  mib_Widget *temp;
  mib_Label *myres;
  /*  unsigned char *label_text; */
  XmString label_text;
  Arg     args[20];
  int     n;

  /* create the new widget and add it to the tree */

  temp = mib_new_mib_Widget();
  if (mib_fill == WDEFAULT)
    mib_add_backward(temp, parent);
  else
    mib_add_mib_Widget(temp, parent);

  myres = (mib_Label *)malloc(sizeof(mib_Label));

  /* initialize public resources */

  if (mib_fill == WDEFAULT)
  {
    temp->name = (char *)malloc(strlen(name)+1);
    strcpy(temp->name,name);
  }
  temp->mib_class = (char *)malloc(6);
  sprintf(temp->mib_class,"Label");
  temp->mib_class_num = MIB_LABEL;
  temp->width = width;
  temp->height = height;
  temp->topOffset = posy;
  temp->leftOffset = posx;
  temp->bottomOffset = 0;
  temp->rightOffset = 0;
  temp->topAttachment = 1;
  temp->leftAttachment = 1;
  temp->bottomAttachment = 0;
  temp->rightAttachment = 0;

  temp->mib_allowresize = 1;

  /* initialize private resources */

  temp->myres = (void *)myres;

  if (mib_fill == WDEFAULT)
  {
    myres->label = (char *)malloc(strlen(label)+1);
    strcpy(myres->label,label);
  }

  /* create Xt widget */

  n = 0;

  if (mib_fill == WDEFAULT)
  {
    label_text = XmStringCreateLtoR(label, XmSTRING_DEFAULT_CHARSET);

    XtSetArg (args[n], XmNlabelString, label_text); n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNleftOffset, posx);n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNtopOffset, posy);n++;
    XtSetArg (args[n], XmNwidth, width); n++;
    XtSetArg (args[n], XmNheight, height); n++;
  }

  XtSetArg (args[n], XmNrubberPositioning, False); n++;
  XtSetArg (args[n], XmNhighlightThickness, 0); n++;

  temp->me = XtCreateManagedWidget(name, xmLabelWidgetClass,
                temp->parent->me, args, n);

  if (mib_fill == WDEFAULT)
    XmStringFree(label_text);
  

  if (mib_fill == WEDIT || mib_fill == WDEFAULT)
  {
    mib_apply_eventhandlers(temp->me, temp);
  }

  return temp;
}

void mib_delete_Label(mib_Widget *this)
{
  mib_Label *temp = (mib_Label *)this->myres;

  free(temp->label);
  free(temp);
}

void mib_save_Label(mib_Widget *this, FILE *fout)
{
  mib_Label *temp = (mib_Label *)this->myres;

  fprintf(fout,"label: \\\"%s\\\"\\n\\\n", temp->label);
}

int mib_load_Label(mib_Widget *this, mib_Buffer *fin)
{
  mib_Label    *myres;
  /*unsigned char *label_text;*/
  XmString label_text;
  char          res[MI_MAXSTRLEN];
  char          val[MI_MAXSTRLEN];
  Arg           args[5];
  int           n, vallen;

  myres = (mib_Label *)this->myres;

  while (mib_read_line(fin, res, val)) {
    if (!strcmp(res,"label")) {
	vallen = strlen(val);
	if (vallen < 2)
	  return 0;
	val[vallen-1] = '\0';
	myres->label = (char *)malloc(vallen-1);
	sprintf(myres->label,"%s",&(val[1]));
	label_text = XmStringCreateLtoR(myres->label, XmSTRING_DEFAULT_CHARSET);

	n = 0;
	XtSetArg (args[n], XmNlabelString, label_text); n++;
	XtSetValues(this->me, args, n);

	XmStringFree(label_text);
    }
    else if (!strcmp(res, "alignment")) {
	int align = atoi(val);
	XtVaSetValues(this->me, XmNalignment, align, NULL);
    }
    else if (!strcmp(res, "EndWidget")) {
	return 1;
    }
    else {
	return 0;
    }
  }
  return 0;
}
