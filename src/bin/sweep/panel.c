/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
#include "panel.h"

FL_FORM *MainForm;

FL_OBJECT
        *TransGroup,
        *Translength,
        *Transxdir,
        *Transydir,
        *Transzdir,
        *TypeGroup,
        *RotSweep,
        *TransSweep,
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
        *Rotdivisions;

void create_form_MainForm(void)
{
  FL_OBJECT *obj;

  if (MainForm)
     return;

  MainForm = fl_bgn_form(FL_NO_BOX,360,370);
  obj = fl_add_box(FL_UP_BOX,0,0,360,370,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  obj = fl_add_button(FL_RETURN_BUTTON,20,310,160,40,"Make Sweep");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,GoButtonProc,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,180,310,160,40,"Quit");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,QuitButtonProc,0);

  TransGroup = fl_bgn_group();
  obj = fl_add_box(FL_ROUNDED_BOX,20,70,320,230,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,100,130,30,"Length of sweep:");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,200,160,30,"Direction of sweep:");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  Translength = obj = fl_add_input(FL_NORMAL_INPUT,120,130,50,30,"Units");
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT);
  Transxdir = obj = fl_add_input(FL_NORMAL_INPUT,120,230,50,30,"x:");
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  Transydir = obj = fl_add_input(FL_NORMAL_INPUT,200,230,50,30,"y:");
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  Transzdir = obj = fl_add_input(FL_NORMAL_INPUT,280,230,50,30,"z:");
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fl_end_group();


  TypeGroup = fl_bgn_group();
  RotSweep = obj = fl_add_button(FL_RADIO_BUTTON,180,20,160,40,"Rotational Sweep");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,RotSweepProc,0);
  TransSweep = obj = fl_add_button(FL_RADIO_BUTTON,20,20,160,40,"Translational Sweep");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,TransSweepProc,0);
  fl_end_group();


  RotGroup = fl_bgn_group();
  obj = fl_add_box(FL_ROUNDED_BOX,20,70,320,230,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,80,90,30,"Arc length:");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  Rotlength = obj = fl_add_input(FL_NORMAL_INPUT,120,80,50,30,"");
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT);
  DegButton = obj = fl_add_button(FL_RADIO_BUTTON,190,80,70,30,"Degrees");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  RadButton = obj = fl_add_button(FL_RADIO_BUTTON,260,80,70,30,"Radians");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,120,90,30,"End of axis:");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  Rotendz = obj = fl_add_input(FL_NORMAL_INPUT,280,140,50,30,"z:");
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  Rotendy = obj = fl_add_input(FL_NORMAL_INPUT,200,140,50,30,"y:");
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  Rotendx = obj = fl_add_input(FL_NORMAL_INPUT,120,140,50,30,"x:");
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,180,130,30,"Direction of axis:");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  Rotdirz = obj = fl_add_input(FL_NORMAL_INPUT,280,210,50,30,"z:");
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  Rotdiry = obj = fl_add_input(FL_NORMAL_INPUT,200,210,50,30,"y:");
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  Rotdirx = obj = fl_add_input(FL_NORMAL_INPUT,120,210,50,30,"x:");
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,260,90,30,"Divisions:");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  Rotdivisions = obj = fl_add_input(FL_NORMAL_INPUT,120,260,50,30,"");
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT);
  fl_end_group();

  fl_end_form();

}
/*---------------------------------------*/

FL_FORM *FooForm;


void create_form_FooForm(void)
{
  FL_OBJECT *obj;

  if (FooForm)
     return;

  FooForm = fl_bgn_form(FL_NO_BOX,500,610);
  obj = fl_add_box(FL_UP_BOX,0,0,500,610,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fl_end_form();

}
/*---------------------------------------*/

void create_the_forms(void)
{
  create_form_MainForm();
  create_form_FooForm();
}

