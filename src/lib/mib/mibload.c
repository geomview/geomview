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

mib_Widget	*mib_root_Widget;
Display		*dpy;
GC		 mib_gc;
static struct _mib_event_handle_funcs {
  void		(*mib_pick_mib_Widget)(/* Widget, XtPointer,
					  XButtonPressedEvent *, Boolean * */);
  void		(*mib_move_mib_Widget)(/* Widget, XtPointer,
					  XPointerMovedEvent *, Boolean * */);
  void		(*mib_unpick_mib_Widget)(/* Widget, XtPointer,
					    XButtonReleasedEvent *, Boolean * */);
} mib_events;

mib_widget_funcs mwfuncs[] =
  {
    { NULL, NULL, NULL, NULL, NULL },
    { "TextBox", mib_create_TextBox, mib_delete_TextBox,
      mib_save_TextBox, mib_load_TextBox},
    { "Button", mib_create_Button, mib_delete_Button,
      mib_save_Button, mib_load_Button},
    { "Toggle", mib_create_Toggle, mib_delete_Toggle,
      mib_save_Toggle, mib_load_Toggle},
    { "RadioBox", mib_create_RadioBox, mib_delete_RadioBox,
      mib_save_RadioBox, mib_load_RadioBox},
    { "DrawingArea", mib_create_DrawingArea, mib_delete_DrawingArea,
      mib_save_DrawingArea, mib_load_DrawingArea},
    { "Label", mib_create_Label, mib_delete_Label,
      mib_save_Label, mib_load_Label},
    { "Frame", mib_create_Frame, mib_delete_Frame,
      mib_save_Frame, mib_load_Frame},
    { "ScrollBar", mib_create_ScrollBar, mib_delete_ScrollBar,
      mib_save_ScrollBar, mib_load_ScrollBar},
    { "TextBig", mib_create_TextBig, mib_delete_TextBig,
      mib_save_TextBig, mib_load_TextBig},
    { "List", mib_create_List, mib_delete_List,
      mib_save_List, mib_load_List},
    { "Scale", mib_create_Scale, mib_delete_Scale,
      mib_save_Scale, mib_load_Scale},
    { "Menu", mib_create_Menu, mib_delete_Menu,
      mib_save_Menu, mib_load_Menu},
    { NULL, NULL, NULL, NULL, NULL },
  };

/*****************************************************************************/

void mib_add_mib_Widget(mib_Widget *this, mib_Widget *parent)
{
  mib_Widget *tmp;

  if (parent->child == NULL)
    {
      parent->child = this;
      this->prev = parent;
      this->parent = parent;
      this->sibling = NULL;
      this->child = NULL;
    }
  else
    {
      tmp = parent->child;
      while (tmp->sibling != NULL)
        tmp = tmp->sibling;
      tmp->sibling = this;
      this->prev = tmp;
      this->parent = parent;
      this->sibling = NULL;
      this->child = NULL;
    }
}

/*****************************************************************************/

void mib_add_backward(mib_Widget *this, mib_Widget *parent)
{
  mib_Widget *tmp;

  if (parent->child == NULL)
    {
      parent->child = this;
      this->prev = parent;
      this->parent = parent;
      this->sibling = NULL;
      this->child = NULL;
    }
  else
    {
      tmp = parent->child;
      parent->child = this;
      this->prev = parent;
      this->parent = parent;
      this->sibling = tmp;
      this->child = NULL;
      tmp->prev = this;
    }
}

/*****************************************************************************/

void mib_remove_mib_Widget(mib_Widget *this)
{

  XtVaSetValues(mib_root_Widget->me, XmNresizePolicy, XmRESIZE_NONE, NULL);
  XtDestroyWidget(this->me);

  while (this->child != NULL)
    mib_remove_mib_Widget(this->child);

  if (this->parent == this)
    {
      mib_clear_myres(this);
      return;
    }

  if (this->prev == this->parent)
    {
      this->parent->child = this->sibling;
      if (this->sibling != NULL)
	this->sibling->prev = this->parent;
    }
  else
    {
      this->prev->sibling = this->sibling;
      if (this->sibling != NULL)
	this->sibling->prev = this->prev;
    }

  mib_clear_myres(this);
}

/*****************************************************************************/

void mib_clear_myres(mib_Widget *this)
{
  free(this->mib_class);
  free(this->name);

  if ((this->mib_class_num < 1) || (this->mib_class_num > MI_NUMCLASSES))
    return;

  mwfuncs[this->mib_class_num].mib_delete(this);
  free(this);
}

/*****************************************************************************/

mib_Widget *mib_new_mib_Widget()
{
  mib_Widget *this;
  this = (mib_Widget *)malloc(sizeof(mib_Widget));
  this->me = NULL;
  this->mib_class_num = MIB_NULL;
  this->mib_selected = 0;
  this->mib_resizing = 0;
  this->myres = NULL;
  this->parent = NULL;
  this->sibling = NULL;
  this->prev = NULL;
  this->child = NULL;
  this->width = 0;
  this->height = 0;
  this->topAttachment = 0;
  this->bottomAttachment = 0;
  this->leftAttachment = 0;
  this->rightAttachment = 0;
  this->topOffset = 0;
  this->bottomOffset = 0;
  this->leftOffset = 0;
  this->rightOffset = 0;

  return this;
}

/*****************************************************************************/

mib_Widget *mib_find_name(mib_Widget *temp, char *name)
{
  mib_Widget *child = temp->child;
  mib_Widget *ret = NULL;

  if (!strcmp(temp->name, name))
    return temp;

  if (child != NULL)
    if ((ret = mib_find_name(child, name)))
      return ret;

  child = temp->sibling;
  if (child != NULL)
    if ((ret = mib_find_name(child, name)))
      return ret;

  return NULL;
}

/*****************************************************************************/

Widget
BuildMenu(Widget parent, int menu_type, char *menu_title, char menu_mnemonic,
	  MenuItem *items)
{
  Widget menu, cascade = NULL, widget;
  int i;
  XmString str;

  if (menu_type == XmMENU_PULLDOWN || menu_type == XmMENU_OPTION)
    menu = XmCreatePulldownMenu(parent, "_pulldown", NULL, 0);
  else if (menu_type == XmMENU_POPUP)
    menu = XmCreatePopupMenu(parent, "_popup", NULL, 0);
  else {
    XtWarning("Invalid menu type passed to BuildMenu()");
    return NULL;
  }

  /* Pulldown menus require a cascade button to be made */
  if (menu_type == XmMENU_PULLDOWN) {
    int i;
    char bname[24];

    XtVaGetValues(parent, XmNnumChildren, &i, NULL);
    sprintf(bname, "button_%d", i);
    str = XmStringCreateSimple(menu_title);
    cascade = XtVaCreateManagedWidget(bname,
				      xmCascadeButtonGadgetClass, parent,
				      XmNsubMenuId,   menu,
				      XmNlabelString, str,
				      XmNmnemonic,    (XID)menu_mnemonic,
				      NULL);
    XmStringFree(str);
  } else if (menu_type == XmMENU_OPTION) {
    /* Option menus are a special case, but not hard to handle */
    Arg args[2];
    str = XmStringCreateSimple(menu_title);
    XtSetArg(args[0], XmNsubMenuId, menu);
    XtSetArg(args[1], XmNlabelString, str);
    /* This really isn't a cascade, but this is the widget handle
     * we're going to return at the end of the function.
     */
    cascade = XmCreateOptionMenu(parent, menu_title, args, 2);
    XmStringFree(str);
  }

  /* Now add the menu items */
  for (i = 0; items[i].label != NULL; i++) {
    /* If subitems exist, create the pull-right menu by calling this
     * function recursively.  Since the function returns a cascade
     * button, the widget returned is used..
     */
    if (items[i].subitems)
      if (menu_type == XmMENU_OPTION) {
	widget = XtVaCreateManagedWidget(items[i].label,
					 *items[i].class, menu, NULL);
	items[i].subitems = (struct _menu_item *) widget;
	/* daeron mod (tm) :-) ... we now use this to pass back each
	   widget we create to the mibMenu functions so that it can
	   be stored as part of the mibMenu structure */

	/* XtWarning("You can't have submenus from option menu items.");
	   continue;*/
      } else
	widget = BuildMenu(menu, XmMENU_PULLDOWN,
			   items[i].label, items[i].mnemonic, items[i].subitems);
    else
      {
	widget = XtVaCreateManagedWidget(items[i].label,
					 *items[i].class, menu,
					 NULL);
	/* ditto here from above ... - Daeron mod (tm) */
	items[i].subitems = (struct _menu_item *) widget;
      }

    /* Whether the item is a real item or a cascade button with a
     * menu, it can still have a mnemonic.
     */
    if (items[i].mnemonic)
      XtVaSetValues(widget, XmNmnemonic, (XID)items[i].mnemonic, NULL);

    /* any item can have an accelerator, except cascade menus. But,
     * we don't worry about that; we know better in our declarations.
     */
    if (items[i].accelerator) {
      str = XmStringCreateSimple(items[i].accel_text);
      XtVaSetValues(widget,
		    XmNaccelerator, items[i].accelerator,
		    XmNacceleratorText, str,
		    NULL);
      XmStringFree(str);
    }

    if (items[i].callback)
      XtAddCallback(widget,
		    (items[i].class == &xmToggleButtonWidgetClass ||
		     items[i].class == &xmToggleButtonGadgetClass)?
                    XmNvalueChangedCallback : /* ToggleButton class */
                    XmNactivateCallback,      /* PushButton class */
		    items[i].callback, items[i].callback_data);
  }

  /* for popup menus, just return the menu; pulldown menus, return
   * the cascade button; option menus, return the thing returned
   * from XmCreateOptionMenu().  This isn't a menu, or a cascade button!
   */
  return menu_type == XmMENU_POPUP? menu : cascade;
}

/*****************************************************************************/

mib_Widget *mib_load_interface(Widget parent, char *from, int file_type)
{
  mib_Buffer  thisfile;
  mib_Widget *this;
  FILE	     *infile;
  char	     *instring;
  char	      ch;

  thisfile.buf_type = file_type;

  dpy = XtDisplay(parent);

  if ((file_type == MI_FROMFILE) || (file_type == MI_EDITFROMFILE))
    {
      if (!(infile = fopen(from,"r")))
	return NULL;

      ch = '\0';
      while ((ch != '\n')&&(!feof(infile)))  /* throw away first line */
	ch = (char)fgetc(infile);

      thisfile.buffer = (void *)infile;
      thisfile.point = 0;

      if (!mib_load_Root(parent, &this, &thisfile))
	{
	  /* delete this */
	  return NULL;
	}
      else
	{
	  fclose(infile);
	  return this;
	}
    }
  else
    if ((file_type == MI_FROMSTRING) || (file_type == MI_EDITFROMSTRING))
      {
	instring = from;
	if (instring == NULL)
	  return NULL;

	thisfile.buffer = (void *)instring;
	thisfile.buflen = strlen(instring);
	thisfile.point = 0;

	if (!mib_load_Root(parent, &this, &thisfile))
	  {
	    /* delete this */
	    return NULL;
	  }
	else
	  return this;
      }
    else
      return NULL;

}

/*****************************************************************************/

int mib_load_mib_class(mib_Widget **this, mib_Widget *parent, char *name,
		       char *iname, mib_Buffer *fin )
{
  int namelen, editstate, count;

  if ((fin->buf_type == MI_EDITFROMFILE) ||
      (fin->buf_type == MI_EDITFROMSTRING))
    editstate = WEDIT;
  else
    editstate = WEMPTY;

  namelen = strlen(name);
  if (namelen < 2)
    return 0;

  name[namelen-1] = '\0';
  name[0] = ' ';

  count = 1;

  while (mwfuncs[count].name)
    {
      if (!strcmp(&(name[1]), mwfuncs[count].name))
	{
	  *this = mwfuncs[count].mib_create(parent, iname, NULL, 0, 0, 0, 0,
					    editstate);
	  return 1;
	}
      count++;
    }

  (*this)->parent = (*this);
  return 1;
}

/*****************************************************************************/

mib_Widget *mib_load_public(mib_Widget *root, mib_Widget *this, mib_Buffer *fin)
{
  int	got_line, done;
  char	res[MI_MAXSTRLEN];
  char	val[MI_MAXSTRLEN];
  char	valcp[MI_MAXSTRLEN];
  Arg	args[20];
  int	mynum, n;

  got_line = 1;
  done = 0;

  /* this loop reads basic info about Widget */
  while (got_line && (!done))
    {
      got_line = mib_read_line(fin, res, val);
      if (!strcmp(res,"Ref"))
	sscanf(val, "%d", &mynum);
      else
	if (!strcmp(res,"Widget"))
	  {
	    strcpy(valcp,val);
	    done = 1;
	  }
    }

  done = 0;

  while (got_line && (!done))
    {
      got_line = mib_read_line(fin, res, val);
      if (!strcmp(res, "Children"))
	n = 0;
      else
	if (!strcmp(res, "Parent")) /* don't support complete widget tree yet */
	  n = 0;
	else
	  if (!strcmp(res,"Public-"))
	    n = 0;
	  else
	    if (!strcmp(res,"Name"))
	      {
		val[strlen(val)-1] = '\0';
		mib_load_mib_class(&this, root, valcp, &(val[1]), fin);
		this->name = (char *)malloc(strlen(val));
		sprintf(this->name,"%s",&(val[1]));
		this->mib_mynum = mynum;
		done = 1;
	      }
	    else
	      return 0;
    }

  if (!got_line)
    return NULL;

  done = 0;

  /* second loop reads public info */
  while (got_line && (!done))
    {
      got_line = mib_read_line(fin, res, val);
      if (!strcmp(res,"Xmwidth"))
	sscanf(val,"%d",&(this->width));
      else
	if (!strcmp(res,"Xmheight"))
	  sscanf(val,"%d",&(this->height));
	else
	  if (!strcmp(res,"XmtopAttachment"))
	    sscanf(val,"%d",&(this->topAttachment));
	  else
	    if (!strcmp(res,"XmbottomAttachment"))
	      sscanf(val,"%d",&(this->bottomAttachment));
	    else
	      if (!strcmp(res,"XmleftAttachment"))
		sscanf(val,"%d",&(this->leftAttachment));
	      else
		if (!strcmp(res,"XmrightAttachment"))
		  sscanf(val,"%d",&(this->rightAttachment));
		else
		  if (!strcmp(res,"XmtopOffset"))
		    sscanf(val,"%d",&(this->topOffset));
		  else
		    if (!strcmp(res,"XmbottomOffset"))
		      sscanf(val,"%d",&(this->bottomOffset));
		    else
		      if (!strcmp(res,"XmleftOffset"))
			sscanf(val,"%d",&(this->leftOffset));
		      else
			if (!strcmp(res,"XmrightOffset"))
			  sscanf(val,"%d",&(this->rightOffset));
			else
			  if (!strcmp(res,"Private-"))
			    done = 1;
    }

  n = 0;
  if ((fin->buf_type == MI_EDITFROMFILE) ||
      (fin->buf_type == MI_EDITFROMSTRING))
    {
      XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
      XtSetArg (args[n], XmNleftOffset, this->leftOffset); n++;
      XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
      XtSetArg (args[n], XmNtopOffset, this->topOffset); n++;
      if (this == root)
	{
	  XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	  XtSetArg (args[n], XmNrightOffset, this->rightOffset); n++;
	  XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	  XtSetArg (args[n], XmNbottomOffset, this->bottomOffset); n++;
	}
    }
  else
    {
      if (this->leftAttachment)
	{
	  XtSetArg (args[n], XmNleftAttachment, this->leftAttachment); n++;
	  if(this->leftAttachment == XmATTACH_FORM)
	    XtSetArg (args[n], XmNleftOffset, this->leftOffset);
	  else
	    XtSetArg (args[n], XmNleftPosition, this->leftOffset*100/root->width);
	  n++;
	}
      if (this->topAttachment)
	{
	  XtSetArg (args[n], XmNtopAttachment, this->topAttachment); n++;
	  if(this->topAttachment == XmATTACH_FORM)
	    XtSetArg (args[n], XmNtopOffset, this->topOffset);
	  else
	    XtSetArg (args[n], XmNtopPosition, this->topOffset*100/root->height);
	  n++;
	}
      if (this->bottomAttachment)
	{
	  XtSetArg (args[n], XmNbottomAttachment, this->bottomAttachment); n++;
	  if(this->bottomAttachment == XmATTACH_FORM)
	    XtSetArg (args[n], XmNbottomOffset, this->bottomOffset);
	  else
	    XtSetArg (args[n], XmNbottomPosition, 100-this->bottomOffset*100/root->height);
	  n++;
	}
      if (this->rightAttachment)
	{
	  XtSetArg (args[n], XmNrightAttachment, this->rightAttachment); n++;
	  if(this->rightAttachment == XmATTACH_FORM)
	    XtSetArg (args[n], XmNrightOffset, this->rightOffset);
	  else
	    XtSetArg (args[n], XmNrightPosition, 100-this->rightOffset*100/root->width);
	  n++;
	}
    }

  /* Do not set the width or height, if zero.
     The core widget will compute appropriately. */
  if ( this->width ) {
    XtSetArg (args[n], XmNwidth, this->width); n++;
  }
  if ( this->height ) {
    XtSetArg (args[n], XmNheight, this->height); n++;
  }

  XtSetValues(this->me, args, n);

  return this;
}

/*****************************************************************************/

int mib_load_private(mib_Widget *this, mib_Buffer *fin)
{

  if (this->mib_class_num == MIB_NULL)
    return 1;

  if ((this->mib_class_num < 1) || (this->mib_class_num > MI_NUMCLASSES))
    return 0;

  mwfuncs[this->mib_class_num].mib_load(this, fin);

  return 1;
}

/*****************************************************************************/

int mib_load_Root(Widget parent, mib_Widget **this, mib_Buffer *fin)
{

  char		res[MI_MAXSTRLEN];
  char		val[MI_MAXSTRLEN];
  int		num_widgets, count;
  Arg		args[20];
  XGCValues     gcvals;
  XtGCMask      val_mask;
  mib_Widget   *temp = NULL;
  int		rubberPositioning = False;

  mib_read_line(fin, res, val);
  if (!strcmp(res,"TotalWidgets"))
    sscanf(val, "%d", &num_widgets);
  else
    return 0;

  (*this) = mib_new_mib_Widget();
  (*this)->mib_class = (char*)malloc(9);
  sprintf((*this)->mib_class,"RootForm");

  XtVaGetValues(parent, XmNrubberPositioning, &rubberPositioning, NULL);

  (*this)->me = XmCreateForm(parent, "MainForm", args, 0);
  if (!mib_load_public((*this), (*this), fin))
    return 0;

  /* we don't expect any private resources for the root widget */

  mib_read_line(fin, res, val);
  if (strcmp(res,"EndWidget."))
    return 0;

  XtManageChild((*this)->me);
  XtVaSetValues((*this)->me, XmNresizePolicy, XmRESIZE_NONE,
		XmNrubberPositioning, rubberPositioning,
		XmNresizable, rubberPositioning, NULL);

  count = num_widgets - 1;
  while (count > 0)
    {

      if (!(temp = mib_load_public((*this), temp, fin)))
	{
	  /* delete temp */
	  return 0;
	}

      if (!mib_load_private(temp,fin))
	{
	  /* delete temp */
	  return 0;
	}
      count--;

    }

  mib_reset_size((*this));

  XtVaSetValues((*this)->me, XmNresizePolicy, XmRESIZE_ANY, NULL);

  val_mask = (long)0;
  mib_gc = XtGetGC((*this)->me, val_mask, &gcvals);

  return 1;
}

/*****************************************************************************/

int mib_read_line(mib_Buffer *bufin, char *res, char *val)
{
  FILE *fin;
  char *strin;
  char  ch;
  int   count, mark;
  char  inbuf[MI_MAXSTRLEN];

  if ((bufin->buf_type == MI_FROMFILE) || (bufin->buf_type == MI_EDITFROMFILE))
    {
      fin = (FILE *)bufin->buffer;
      ch = '\0';
      count = 0;
      mark = 0;
      while ((ch != '\n')&&(!feof(fin))&&(count<MI_MAXSTRLEN))
	{
	  ch = (char)fgetc(fin);
	  if ((mark == 0) && (ch == ':'))
	    mark = count;
	  if ((ch != '\\')&&(ch != '\n'))
	    {
	      inbuf[count] = ch;
	      count++;
	    }
	}
      if (feof(fin))
	return 0;
      inbuf[count] = '\0';
      if (count > 0)
	{
	  if (inbuf[count-1] == 'n')
	    inbuf[count-1] = '\0';
	}
      else
	return 0;

    }
  else
    if ((bufin->buf_type == MI_FROMSTRING) ||
	(bufin->buf_type == MI_EDITFROMSTRING))
      {
	strin = (char *)bufin->buffer;
	count = bufin->point;
	mark = 0;

	if (count >= bufin->buflen)
	  return 0;

	while ((strin[count] != '\n') && (count < bufin->buflen))
	  {
	    if ((mark == 0) && (strin[count] == ':'))
	      mark = count;
	    count++;
	  }

	strin[count] = '\0';
	if (count >= bufin->buflen)
	  return 0;
	sprintf(inbuf,"%s",&(strin[bufin->point]));
	strin[count] = '\n';
	if (mark != 0)
	  mark -= bufin->point;
	bufin->point = count+1;
      }
    else
      return 0;

  if (mark == 0)
    {
      sprintf(res,"%s",inbuf);
#if 0
      sprintf(val,"\0"); /* ???? */
#else
      val[0] = '\0';
      val[1] = '\0';
#endif
    }
  else
    {
      inbuf[mark] = '\0';
      sprintf(res,"%s",inbuf);
      inbuf[mark] = ' ';
      if (strlen(inbuf)-mark > 1)
	sprintf(val,"%s",&(inbuf[mark+2]));
      else
#if 0
	sprintf(val,"\0");
#else
      {
	val[0] = '\0';
	val[1] = '\0';
      }
#endif
    }

  return 1;
}

/*****************************************************************************/

void mib_reset_size(mib_Widget *temp)
{
  Arg	args[5];
  int	n;

  mib_Widget *child = temp->child;

  if (temp->mib_class_num != MIB_NULL)
    {
      n = 0;

      /* Do not set the width or height, if zero.
	 The core widget will compute appropriately. */
      if (temp->width) {
	XtSetArg (args[n], XmNwidth, temp->width); n++;
      }
      if (temp->height) {
	XtSetArg (args[n], XmNheight, temp->height); n++;
      }

      XtSetValues(temp->me, args, n);
    }

  if (child != NULL)
    mib_reset_size(child);

  child = temp->sibling;
  if (child != NULL)
    mib_reset_size(child);
}

/*****************************************************************************/

void mib_set_eventhandlers(void (*a)(), void (*b)(), void (*c)())
{
  mib_events.mib_pick_mib_Widget = (void (*)())a;
  mib_events.mib_move_mib_Widget = (void (*)())b;
  mib_events.mib_unpick_mib_Widget = (void (*)())c;
}

/*****************************************************************************/

void mib_apply_eventhandlers(Widget this, mib_Widget *actual)
{
  XtAddEventHandler(this, ButtonPressMask, FALSE,
		    mib_events.mib_pick_mib_Widget, (XtPointer)actual);
  XtAddEventHandler(this, Button3MotionMask, FALSE,
		    mib_events.mib_move_mib_Widget, (XtPointer)actual);
  XtAddEventHandler(this, ButtonReleaseMask, FALSE,
		    mib_events.mib_unpick_mib_Widget, (XtPointer)actual);

}

/*****************************************************************************/

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
