/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
#include "origin.panel.h"

FL_FORM *MainForm;

FL_OBJECT
        *ShowButton,
        *DoneButton,
        *CancelButton,
        *Instruc1,
        *Instruc2,
        *Instruc3,
        *TargetInput,
        *Instruc4;

void create_form_MainForm(void)
{
  FL_OBJECT *obj;

  if (MainForm)
     return;

  MainForm = fl_bgn_form(FL_NO_BOX,220,330);
  obj = fl_add_box(FL_UP_BOX,0,0,220,330,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  ShowButton = obj = fl_add_button(FL_NORMAL_BUTTON,60,210,100,30,"Show Origin");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,ShowButtonProc,0);
  DoneButton = obj = fl_add_button(FL_NORMAL_BUTTON,10,250,100,30,"Done");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,DoneButtonProc,0);
  CancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,110,250,100,30,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,CancelButtonProc,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,60,290,100,30,"Quit");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,QuitButtonProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,0,10,220,40,"Move Origin");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lcolor(obj,FL_RED);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
  Instruc1 = obj = fl_add_text(FL_NORMAL_TEXT,0,60,220,20,"str1");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  Instruc2 = obj = fl_add_text(FL_NORMAL_TEXT,0,80,220,20,"str2");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  Instruc3 = obj = fl_add_text(FL_NORMAL_TEXT,0,100,220,20,"str3");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  TargetInput = obj = fl_add_input(FL_NORMAL_INPUT,10,170,200,30,"Target Object:");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  Instruc4 = obj = fl_add_text(FL_NORMAL_TEXT,0,120,220,20,"str3");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  fl_end_form();

}
/*---------------------------------------*/

FL_FORM *ErrorForm;

FL_OBJECT
        *Error1,
        *Error2,
        *Error3,
        *OkButton;

void create_form_ErrorForm(void)
{
  FL_OBJECT *obj;

  if (ErrorForm)
     return;

  ErrorForm = fl_bgn_form(FL_NO_BOX,220,150);
  obj = fl_add_box(FL_UP_BOX,0,0,220,150,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  obj = fl_add_text(FL_NORMAL_TEXT,0,10,220,30,"Oh No!!!");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lcolor(obj,FL_RED);
    fl_set_object_lsize(obj,20);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER);
    fl_set_object_lstyle(obj,FL_FIXED_STYLE);
  Error1 = obj = fl_add_text(FL_NORMAL_TEXT,0,40,220,20,"str3");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER);
  Error2 = obj = fl_add_text(FL_NORMAL_TEXT,0,60,220,20,"str3");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER);
  Error3 = obj = fl_add_text(FL_NORMAL_TEXT,0,80,220,20,"str3");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER);
  OkButton = obj = fl_add_button(FL_RETURN_BUTTON,70,110,90,30,"Ok");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fl_end_form();

}
/*---------------------------------------*/

void create_the_forms(void)
{
  create_form_MainForm();
  create_form_ErrorForm();
}

