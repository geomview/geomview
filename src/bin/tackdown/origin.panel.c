/* Form definition file generated with fdesign. */

#include "forms.h"
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

void create_form_MainForm()
{
  FL_OBJECT *obj;
  MainForm = fl_bgn_form(FL_NO_BOX,220.0,330.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,220.0,330.0,"");
  ShowButton = obj = fl_add_button(FL_NORMAL_BUTTON,60.0,90.0,100.0,30.0,"Show Origin");
    fl_set_call_back(obj,ShowButtonProc,0);
  DoneButton = obj = fl_add_button(FL_NORMAL_BUTTON,10.0,50.0,100.0,30.0,"Done");
    fl_set_call_back(obj,DoneButtonProc,0);
  CancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,110.0,50.0,100.0,30.0,"Cancel");
    fl_set_call_back(obj,CancelButtonProc,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,60.0,10.0,100.0,30.0,"Quit");
    fl_set_call_back(obj,QuitButtonProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,0.0,280.0,220.0,40.0,"Move Origin");
    fl_set_object_lcol(obj,1);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
    fl_set_object_lstyle(obj,FL_ENGRAVED_STYLE);
  Instruc1 = obj = fl_add_text(FL_NORMAL_TEXT,0.0,250.0,220.0,20.0,"str1");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  Instruc2 = obj = fl_add_text(FL_NORMAL_TEXT,0.0,230.0,220.0,20.0,"str2");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  Instruc3 = obj = fl_add_text(FL_NORMAL_TEXT,0.0,210.0,220.0,20.0,"str3");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  TargetInput = obj = fl_add_input(FL_NORMAL_INPUT,10.0,130.0,200.0,30.0,"Target Object:");
    fl_set_object_color(obj,47,47);
    fl_set_object_align(obj,FL_ALIGN_TOP);
  Instruc4 = obj = fl_add_text(FL_NORMAL_TEXT,0.0,190.0,220.0,20.0,"str3");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *ErrorForm;

FL_OBJECT
        *Error1,
        *Error2,
        *Error3,
        *OkButton;

void create_form_ErrorForm()
{
  FL_OBJECT *obj;
  ErrorForm = fl_bgn_form(FL_NO_BOX,220.0,150.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,220.0,150.0,"");
  obj = fl_add_text(FL_NORMAL_TEXT,0.0,110.0,220.0,30.0,"Oh No!!!");
    fl_set_object_lcol(obj,1);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
    fl_set_object_lstyle(obj,FL_ENGRAVED_STYLE);
  Error1 = obj = fl_add_text(FL_NORMAL_TEXT,0.0,90.0,220.0,20.0,"str3");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  Error2 = obj = fl_add_text(FL_NORMAL_TEXT,0.0,70.0,220.0,20.0,"str3");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  Error3 = obj = fl_add_text(FL_NORMAL_TEXT,0.0,50.0,220.0,20.0,"str3");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  OkButton = obj = fl_add_button(FL_RETURN_BUTTON,70.0,10.0,90.0,30.0,"Ok");
  fl_end_form();
}

/*---------------------------------------*/

void create_the_forms()
{
  create_form_MainForm();
  create_form_ErrorForm();
}

