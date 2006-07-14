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
static Widget    real_list;

/* Code for Lists */
/*****************************************************************************/

mib_Widget *mib_create_List(mib_Widget *parent, char *name, char *label,
		int posx, int posy, int width, int height, int mib_fill)
{
  mib_Widget	*temp;
  mib_List	*myres;
  Widget	 wtemp;
  /*unsigned char *label_text;*/
  XmString	 label_text;
  char		 ttext[30];
  Arg		 args[20];
  int		 n;

  /* create the new widget and add it to the tree */

  temp = mib_new_mib_Widget();
  if (mib_fill == WDEFAULT)
    mib_add_backward(temp, parent);
  else
    mib_add_mib_Widget(temp, parent);

  myres = (mib_List *)malloc(sizeof(mib_List));

  /* initialize public resources */

  if (mib_fill == WDEFAULT)
  {
    temp->name = (char *)malloc(strlen(name)+1);
    strcpy(temp->name,name);
  }
  temp->mib_class = (char *)malloc(5);
  sprintf(temp->mib_class,"List");
  temp->mib_class_num = MIB_LIST;
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

  /* create Xt widget */

  n = 0;

  if (mib_fill == WDEFAULT)
  {
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNleftOffset, posx); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNtopOffset, posy); n++;
  }
  XtSetArg (args[n], XmNrubberPositioning, False); n++;
  XtSetArg (args[n], XmNshadowType, XmSHADOW_ETCHED_IN); n++;

  temp->me = XtCreateManagedWidget(name, xmFrameWidgetClass,
                temp->parent->me, args, n);

  n = 0;

  if (mib_fill == WDEFAULT)
  {
    XtSetArg (args[n], XmNwidth, width); n++;
    XtSetArg (args[n], XmNheight, height); n++;
  }
  XtSetArg (args[n], XmNhighlightThickness, 0); n++;
  XtSetArg (args[n], XmNrubberPositioning, False); n++;
  XtSetArg (args[n], XmNorientation, XmVERTICAL); n++;

  if (mib_fill == WEMPTY)
  {
    XtSetArg (args[n], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE); n++;
  }

  wtemp = XmCreateScrolledList(temp->me, name, args, n);
  myres->real_list = wtemp;
  real_list = wtemp;

  XtManageChild(wtemp);

  if (mib_fill != WEMPTY)
    real_list = NULL;

  if (mib_fill == WEDIT || mib_fill == WDEFAULT)
  {
    label_text = XmStringCreateLtoR("Simple", XmSTRING_DEFAULT_CHARSET);
    XmListAddItem(wtemp, label_text, 0);
    XmStringFree(label_text);
    label_text = XmStringCreateLtoR("List", XmSTRING_DEFAULT_CHARSET);
    XmListAddItem(wtemp, label_text, 0);
    XmStringFree(label_text);
    for (n=0;n<200;n++)
    {
      sprintf(ttext,"Item %d   ",n);
      label_text = XmStringCreateLtoR(ttext, XmSTRING_DEFAULT_CHARSET);
      XmListAddItem(wtemp, label_text, 0);
      XmStringFree(label_text);
    }

    mib_apply_eventhandlers(temp->me, temp);
    mib_apply_eventhandlers(wtemp, temp);
  }

  return temp;
}

void mib_delete_List(mib_Widget *this)
{
  mib_List *temp = (mib_List *)this->myres;

  free(temp);
}

void mib_save_List(mib_Widget *this, FILE *fout)
{
}

int mib_load_List(mib_Widget *this, mib_Buffer *fin)
{
  char		res[MI_MAXSTRLEN];
  char		val[MI_MAXSTRLEN];


  if (real_list)
   this->me = real_list;

  if (!mib_read_line(fin, res, val))
    return 0;

  if (strcmp(res,"EndWidget"))
    return 0;

  return 1;
}
