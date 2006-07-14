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

/* Code for RadioBox */
/*****************************************************************************/

mib_Widget *mib_create_RadioBox(mib_Widget *parent, char *name, char *label,
		int posx, int posy, int width, int height, int mib_fill)
{
  mib_Widget    *temp;
  mib_RadioBox  *myres;
  Widget	wtemp;
  char		label_temp[50];
  Arg		args[20];
  int		n;

  /* create the new widget and add it to the tree */

  temp = mib_new_mib_Widget();
  if (mib_fill == WDEFAULT)
    mib_add_backward(temp, parent);
  else
    mib_add_mib_Widget(temp, parent);

  myres = (mib_RadioBox *)malloc(sizeof(mib_RadioBox));

  /* initialize public resources */

  if (mib_fill == WDEFAULT)
  {
    temp->name = (char *)malloc(strlen(name)+1);
    strcpy(temp->name,name);
  }
  temp->mib_class = (char *)malloc(9);
  sprintf(temp->mib_class,"RadioBox");
  temp->mib_class_num = MIB_RADIOBOX;

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

  temp->mib_allowresize = 0;

  /* initialize private resources */

  temp->myres = (void *)myres;

  myres->numlabel = 0;

  if (mib_fill == WDEFAULT)
  {
    myres->numlabel = 2;
    myres->labels = (char **)malloc(10);
    myres->buttons = (Widget *)malloc(sizeof(Widget) * myres->numlabel);
    sprintf(label_temp, "Radio 1");
    myres->labels[0] = (char *)malloc(strlen(label_temp)+1);
    strcpy(myres->labels[0],label_temp);
    sprintf(label_temp, "Radio 2");
    myres->labels[1] = (char *)malloc(strlen(label_temp)+1);
    strcpy(myres->labels[1],label_temp);
  }


  /* create Xt widget */

  n = 0;

  XtSetArg (args[n], XmNrubberPositioning, False); n++;

  if (mib_fill == WDEFAULT)
  {
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNleftOffset, posx);n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg (args[n], XmNtopOffset, posy);n++;
    XtSetArg (args[n], XmNwidth, width); n++;
    XtSetArg (args[n], XmNheight, height); n++;
  }

  XtSetArg (args[n], XmNhighlightThickness, 0); n++;

  temp->me = XmCreateRadioBox(temp->parent->me, "Radio", args, n);
  wtemp = temp->me;


  if (mib_fill == WDEFAULT)
  {
    n = 0;
    XtSetArg (args[n], XmNhighlightThickness, 0); n++;

    XtCreateManagedWidget("Radio 1", xmToggleButtonGadgetClass,
		wtemp, args, n);

    n = 0;
    XtSetArg (args[n], XmNhighlightThickness, 0); n++;
    XtCreateManagedWidget("Radio 2", xmToggleButtonGadgetClass,
		wtemp, args, n);
  }


  XtManageChild(wtemp);

  if (mib_fill == WEDIT || mib_fill == WDEFAULT)
  {
    mib_apply_eventhandlers(temp->me, temp);
  }

  return temp;
}

void mib_delete_RadioBox(mib_Widget *this)
{
  mib_RadioBox *temp = (mib_RadioBox *)this->myres;
  int		count;

  for (count=0; count < temp->numlabel; count++)
    free(temp->labels[count]);

  free(temp);
}

void mib_save_RadioBox(mib_Widget *this, FILE *fout)
{
  mib_RadioBox *temp = (mib_RadioBox *)this->myres;
  int count;

  fprintf(fout,"numlabel: %d\\n\\\n", temp->numlabel);

  for (count=0; count < temp->numlabel; count++)
  {
    fprintf(fout,"label: \\\"%s\\\"\\n\\\n", temp->labels[count]);
  }
}

int mib_load_RadioBox(mib_Widget *this, mib_Buffer *fin)
{
  char		res[MI_MAXSTRLEN];
  char		val[MI_MAXSTRLEN];
  int		count, vallen, n;
  Arg		args[5];
  mib_RadioBox *myres;

  myres = (mib_RadioBox *)this->myres;

  if (!mib_read_line(fin, res, val))
    return 0;

  if (!strcmp(res,"numlabel"))
  {
    myres->numlabel = 0;
    sscanf(val,"%d",&(myres->numlabel));
    if (!(myres->numlabel))
      return 0;

    myres->labels = (char **)malloc((myres->numlabel+1)*4);
    myres->buttons = (Widget *)malloc(sizeof(Widget)*(myres->numlabel));
    for (count=0; count < myres->numlabel; count++)
    {
      if (!mib_read_line(fin, res, val))
	return 0;
      vallen = strlen(val);
      if (vallen < 2)
	return 0;
      val[vallen-1] = '\0';
      myres->labels[count] = (char *)malloc(vallen-1);
      sprintf(myres->labels[count],"%s",&(val[1]));

      n = 0;
      XtSetArg (args[n], XmNhighlightThickness, 0); n++;

      myres->buttons[count] = XtCreateManagedWidget(myres->labels[count],
		xmToggleButtonGadgetClass, this->me, args, n);

    }
  }
  else
    return 0;

  if (!mib_read_line(fin, res, val))
    return 0;
  if (strcmp(res,"EndWidget"))
    return 0;

  return 1;
}
