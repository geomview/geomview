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
#ifdef notdef
static int       scaleflag;
#endif

/* Code for Scale */
/*****************************************************************************/

mib_Widget *mib_create_Scale(mib_Widget *parent, char *name, char *orient,
		int posx, int posy, int width, int height, int mib_fill)
{
  mib_Widget	*temp;
  mib_Scale	*myres;
  Arg		 args[20];
  int		 n;


#ifdef notdef
  scaleflag = 0;
#endif
  /* create the new widget and add it to the tree */

  temp = mib_new_mib_Widget();
  if (mib_fill == WDEFAULT)
    mib_add_backward(temp, parent);
  else
    mib_add_mib_Widget(temp, parent);

  myres = (mib_Scale *)malloc(sizeof(mib_Scale));

  /* initialize public resources */

  if (mib_fill == WDEFAULT)
  {
    temp->name = (char *)malloc(strlen(name)+1);
    strcpy(temp->name,name);
  }
  temp->mib_class = (char *)malloc(10);
  sprintf(temp->mib_class,"Scale");
  temp->mib_class_num = MIB_SCALE;
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
  myres->orientation = 0;

  /* create Xt widget */

  n = 0;

  if (mib_fill == WDEFAULT)
  {
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNleftOffset, posx);n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNtopOffset, posy);n++;
    XtSetArg (args[n], XmNwidth, width); n++;
    XtSetArg (args[n], XmNheight, height); n++;

  }

  XtSetArg (args[n], XmNrubberPositioning, False); n++;
  XtSetArg (args[n], XmNhighlightThickness, 0); n++;

  if (mib_fill == WDEFAULT) {
    if (!strcmp("VertScale",orient))
    {
      XtSetArg (args[n], XmNorientation, XmVERTICAL); n++;
      XtSetArg (args[n], XmNprocessingDirection, XmMAX_ON_TOP); n++;
    }
    else
    if (!strcmp("HorzScale",orient))
    {
      XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
      XtSetArg (args[n], XmNprocessingDirection, XmMAX_ON_RIGHT); n++;
      myres->orientation = 1;
    }
  }

  if (mib_fill == WEDIT || mib_fill == WDEFAULT)
  {
    XtSetArg (args[n], XmNshowArrows, False); n++;
    XtSetArg (args[n], XmNsliderSize, 30); n++;
  }

  if (mib_fill == WEDIT || mib_fill == WDEFAULT)
    temp->me = XtCreateManagedWidget(name, xmScrollBarWidgetClass,
                temp->parent->me, args, n);
  else
    temp->me = XtCreateManagedWidget(name, xmScaleWidgetClass,
                temp->parent->me, args, n);

#ifdef notdef
  if (mib_fill == WEMPTY)
    scaleflag = 1;
#endif

  if (mib_fill == WEDIT || mib_fill == WDEFAULT)
  {
    mib_apply_eventhandlers(temp->me, temp);
  }

  return temp;
}

void mib_delete_Scale(mib_Widget *this)
{
  mib_Scale *temp = (mib_Scale *)this->myres;

  free(temp);
}

void mib_save_Scale(mib_Widget *this, FILE *fout)
{
  mib_Scale *temp = (mib_Scale *)this->myres;

  fprintf(fout,"orientation: %d\\n\\\n", temp->orientation);
}

int mib_load_Scale(mib_Widget *this, mib_Buffer *fin)
{
  mib_Scale *myres;
  char          res[MI_MAXSTRLEN];
  char          val[MI_MAXSTRLEN];
  Arg           args[5];
  int		n;

  myres = (mib_Scale *)this->myres;

  if (!mib_read_line(fin, res, val))
    return 0;

  if (!strcmp(res,"orientation"))
  {
    sscanf(val,"%d",&(myres->orientation));


#ifdef notdef
	/* What's this doing here?  Setting XmNscaleWidth/XmNscaleHeight
	 * seems to break lesstif's sizing of the widget.  -slevy 97.07.31
	 */
    n = 0;
    if (scaleflag)
    {
      if (!myres->orientation)
      {
        XtSetArg (args[n], XmNscaleWidth, this->width); n++;
      }
      else
      {
        XtSetArg (args[n], XmNscaleWidth, this->height); n++;
      }

      XtSetValues(this->me, args, n);

    }
#endif	/* end what's this */

    n = 0;
    switch (myres->orientation) {
        case 0:
        XtSetArg (args[n], XmNorientation, XmVERTICAL); n++;
	XtSetArg (args[n], XmNprocessingDirection, XmMAX_ON_TOP); n++;
        break;
        case 1:
        XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
	XtSetArg (args[n], XmNprocessingDirection, XmMAX_ON_RIGHT); n++;
        break;
        default:
        break;
    }
    XtSetValues(this->me, args, n);

  }
  else
    return 0;

  if (!mib_read_line(fin, res, val))
    return 0;

  if (strcmp(res,"EndWidget"))
    return 0;

  return 1;
}
