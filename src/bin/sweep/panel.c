/* Form definition file generated with fdesign. */

#include "forms.h"
#include "panel.h"

FL_FORM *MainForm;

FL_OBJECT
        *RotGroup,
        *Rotlength,
        *DegButton,
        *RadButton,
        *Rotendz,
        *Rotendy,
        *Rotendx,
        *Rotdirz,
        *Rotdiry,
        *Rotdirx,
        *Rotdivisions,
        *TransGroup,
        *Translength,
        *Transxdir,
        *Transydir,
        *Transzdir,
        *TypeGroup,
        *RotSweep,
        *TransSweep;

void create_form_MainForm()
{
  FL_OBJECT *obj;
  MainForm = fl_bgn_form(FL_NO_BOX,360.0,370.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,360.0,370.0,"");
  obj = fl_add_button(FL_RETURN_BUTTON,20.0,20.0,160.0,40.0,"Make Sweep");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,GoButtonProc,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,180.0,20.0,160.0,40.0,"Quit");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,QuitButtonProc,0);
  RotGroup = fl_bgn_group();
  obj = fl_add_box(FL_FRAME_BOX,20.0,70.0,320.0,230.0,"");
  obj = fl_add_text(FL_NORMAL_TEXT,20.0,260.0,90.0,30.0,"Arc length:");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  Rotlength = obj = fl_add_input(FL_NORMAL_INPUT,120.0,260.0,50.0,30.0,"");
    fl_set_object_color(obj,15,15);
    fl_set_object_align(obj,FL_ALIGN_RIGHT);
  DegButton = obj = fl_add_button(FL_RADIO_BUTTON,190.0,260.0,70.0,30.0,"Degrees");
  RadButton = obj = fl_add_button(FL_RADIO_BUTTON,260.0,260.0,70.0,30.0,"Radians");
  obj = fl_add_text(FL_NORMAL_TEXT,20.0,220.0,90.0,30.0,"End of axis:");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  Rotendz = obj = fl_add_input(FL_NORMAL_INPUT,280.0,200.0,50.0,30.0,"z:");
    fl_set_object_color(obj,15,15);
  Rotendy = obj = fl_add_input(FL_NORMAL_INPUT,200.0,200.0,50.0,30.0,"y:");
    fl_set_object_color(obj,15,15);
  Rotendx = obj = fl_add_input(FL_NORMAL_INPUT,120.0,200.0,50.0,30.0,"x:");
    fl_set_object_color(obj,15,15);
  obj = fl_add_text(FL_NORMAL_TEXT,20.0,160.0,130.0,30.0,"Direction of axis:");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  Rotdirz = obj = fl_add_input(FL_NORMAL_INPUT,280.0,130.0,50.0,30.0,"z:");
    fl_set_object_color(obj,15,15);
  Rotdiry = obj = fl_add_input(FL_NORMAL_INPUT,200.0,130.0,50.0,30.0,"y:");
    fl_set_object_color(obj,15,15);
  Rotdirx = obj = fl_add_input(FL_NORMAL_INPUT,120.0,130.0,50.0,30.0,"x:");
    fl_set_object_color(obj,15,15);
  obj = fl_add_text(FL_NORMAL_TEXT,20.0,80.0,90.0,30.0,"Divisions:");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  Rotdivisions = obj = fl_add_input(FL_NORMAL_INPUT,120.0,80.0,50.0,30.0,"");
    fl_set_object_color(obj,15,15);
    fl_set_object_align(obj,FL_ALIGN_RIGHT);
  fl_end_group();
  TransGroup = fl_bgn_group();
  obj = fl_add_box(FL_FRAME_BOX,20.0,70.0,320.0,230.0,"");
  obj = fl_add_text(FL_NORMAL_TEXT,20.0,240.0,130.0,30.0,"Length of sweep:");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,20.0,140.0,160.0,30.0,"Direction of sweep:");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  Translength = obj = fl_add_input(FL_NORMAL_INPUT,120.0,210.0,50.0,30.0,"Units");
    fl_set_object_color(obj,15,15);
    fl_set_object_align(obj,FL_ALIGN_RIGHT);
  Transxdir = obj = fl_add_input(FL_NORMAL_INPUT,120.0,110.0,50.0,30.0,"x:");
    fl_set_object_color(obj,15,15);
  Transydir = obj = fl_add_input(FL_NORMAL_INPUT,200.0,110.0,50.0,30.0,"y:");
    fl_set_object_color(obj,15,15);
  Transzdir = obj = fl_add_input(FL_NORMAL_INPUT,280.0,110.0,50.0,30.0,"z:");
    fl_set_object_color(obj,15,15);
  fl_end_group();
  TypeGroup = fl_bgn_group();
  RotSweep = obj = fl_add_button(FL_RADIO_BUTTON,180.0,310.0,160.0,40.0,"Rotational Sweep");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,RotSweepProc,0);
  TransSweep = obj = fl_add_button(FL_RADIO_BUTTON,20.0,310.0,160.0,40.0,"Translational Sweep");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,TransSweepProc,0);
  fl_end_group();
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *FooForm;


void create_form_FooForm()
{
  FL_OBJECT *obj;
  FooForm = fl_bgn_form(FL_NO_BOX,500.0,610.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,500.0,610.0,"");
  fl_end_form();
}

/*---------------------------------------*/

void create_the_forms()
{
  create_form_MainForm();
  create_form_FooForm();
}

