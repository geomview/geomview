/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
#include "panel.h"

FL_FORM *TransForm;

FL_OBJECT
        *t00,
        *t01,
        *t02,
        *t03,
        *t10,
        *t11,
        *t12,
        *t13,
        *t20,
        *t21,
        *t22,
        *t30,
        *t31,
        *t32,
        *t33,
        *ApplyButton,
        *SetButton,
        *ReadButton,
        *DoneButton,
        *IdentityButton,
        *targetGroup,
        *actGeom,
        *actCam,
        *TranslateGroup,
        *TranslationAxisX,
        *TranslationAxisY,
        *TranslationAxisZ,
        *TranslationMeasure,
        *TranslateButton,
        *ModeGroup,
        *RotateModeButton,
        *TranslateModeButton,
        *ScaleModeButton,
        *RotateGroup,
        *RotationAxisY,
        *RotationAxisZ,
        *RotateButton,
        *RotationMeasure,
        *RotationAxisX,
        *t23,
        *ScaleGroup,
        *ScaleMeasure,
        *ScaleButton,
        *ScaleX,
        *ScaleY,
        *ScaleZ,
        *CoordSysGroup,
        *CameraButton,
        *SelfButton,
        *WorldButton;

void create_form_TransForm(void)
{
  FL_OBJECT *obj;

  if (TransForm)
     return;

  TransForm = fl_bgn_form(FL_NO_BOX,340,470);
  obj = fl_add_box(FL_UP_BOX,0,0,340,470,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  obj = fl_add_box(FL_DOWN_BOX,10,145,320,200,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  t00 = obj = fl_add_input(FL_NORMAL_INPUT,30,15,70,30,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  t01 = obj = fl_add_input(FL_NORMAL_INPUT,100,15,70,30,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  t02 = obj = fl_add_input(FL_NORMAL_INPUT,170,15,70,30,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  t03 = obj = fl_add_input(FL_NORMAL_INPUT,240,15,70,30,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  t10 = obj = fl_add_input(FL_NORMAL_INPUT,30,45,70,30,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  t11 = obj = fl_add_input(FL_NORMAL_INPUT,100,45,70,30,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  t12 = obj = fl_add_input(FL_NORMAL_INPUT,170,45,70,30,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  t13 = obj = fl_add_input(FL_NORMAL_INPUT,240,45,70,30,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  t20 = obj = fl_add_input(FL_NORMAL_INPUT,30,75,70,30,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  t21 = obj = fl_add_input(FL_NORMAL_INPUT,100,75,70,30,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  t22 = obj = fl_add_input(FL_NORMAL_INPUT,170,75,70,30,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  t30 = obj = fl_add_input(FL_NORMAL_INPUT,30,105,70,30,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  t31 = obj = fl_add_input(FL_NORMAL_INPUT,100,105,70,30,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  t32 = obj = fl_add_input(FL_NORMAL_INPUT,170,105,70,30,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  t33 = obj = fl_add_input(FL_NORMAL_INPUT,240,105,70,30,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  ApplyButton = obj = fl_add_button(FL_NORMAL_BUTTON,10,355,80,30,"Apply [a ]");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,ApplyProc,0);
  SetButton = obj = fl_add_button(FL_NORMAL_BUTTON,90,355,80,30,"Set [s]");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,SetProc,0);
  ReadButton = obj = fl_add_button(FL_NORMAL_BUTTON,170,355,80,30,"Read [r]");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,ReadProc,0);
  DoneButton = obj = fl_add_button(FL_NORMAL_BUTTON,115,435,100,30,"Quit [q]");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,DoneProc,0);
  IdentityButton = obj = fl_add_button(FL_NORMAL_BUTTON,250,355,80,30,"Identity [i]");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,IdentityProc,0);

  targetGroup = fl_bgn_group();
  actGeom = obj = fl_add_lightbutton(FL_RADIO_BUTTON,30,395,140,30,"Act On Geom");
    fl_set_object_color(obj,39,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  actCam = obj = fl_add_lightbutton(FL_RADIO_BUTTON,170,395,140,30,"Act On Camera");
    fl_set_object_color(obj,39,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fl_end_group();


  TranslateGroup = fl_bgn_group();
  TranslationAxisX = obj = fl_add_lightbutton(FL_RADIO_BUTTON,20,265,100,30,"x");
    fl_set_object_color(obj,39,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  TranslationAxisY = obj = fl_add_lightbutton(FL_RADIO_BUTTON,120,265,100,30,"y");
    fl_set_object_color(obj,39,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  TranslationAxisZ = obj = fl_add_lightbutton(FL_RADIO_BUTTON,220,265,100,30,"z");
    fl_set_object_color(obj,39,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  TranslationMeasure = obj = fl_add_input(FL_NORMAL_INPUT,40,305,100,30,"units");
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT);
  TranslateButton = obj = fl_add_button(FL_NORMAL_BUTTON,200,305,100,30,"Translate [T]");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,TranslateButtonProc,0);
  fl_end_group();


  ModeGroup = fl_bgn_group();
  RotateModeButton = obj = fl_add_button(FL_RADIO_BUTTON,20,155,100,30,"Rotate");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,RotateMode,0);
  TranslateModeButton = obj = fl_add_button(FL_RADIO_BUTTON,120,155,100,30,"Translate");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,TranslateMode,0);
  ScaleModeButton = obj = fl_add_button(FL_RADIO_BUTTON,220,155,100,30,"Scale");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,ScaleMode,0);
  fl_end_group();


  RotateGroup = fl_bgn_group();
  RotationAxisY = obj = fl_add_lightbutton(FL_RADIO_BUTTON,120,265,100,30,"About y");
    fl_set_object_color(obj,39,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  RotationAxisZ = obj = fl_add_lightbutton(FL_RADIO_BUTTON,220,265,100,30,"About z");
    fl_set_object_color(obj,39,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  RotateButton = obj = fl_add_button(FL_NORMAL_BUTTON,210,305,100,30,"Rotate [R]");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,RotateButtonProc,0);
  RotationMeasure = obj = fl_add_input(FL_NORMAL_INPUT,30,305,100,30,"degrees");
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT);
  RotationAxisX = obj = fl_add_lightbutton(FL_RADIO_BUTTON,20,265,100,30,"About x");
    fl_set_object_color(obj,39,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fl_end_group();

  t23 = obj = fl_add_input(FL_NORMAL_INPUT,240,75,70,30,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);

  ScaleGroup = fl_bgn_group();
  ScaleMeasure = obj = fl_add_input(FL_NORMAL_INPUT,40,305,100,30,"units");
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT);
  ScaleButton = obj = fl_add_button(FL_NORMAL_BUTTON,200,305,100,30,"Scale [S]");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,ScaleButtonProc,0);
  ScaleX = obj = fl_add_lightbutton(FL_PUSH_BUTTON,20,265,100,30,"x");
    fl_set_object_color(obj,39,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  ScaleY = obj = fl_add_lightbutton(FL_PUSH_BUTTON,120,265,100,30,"y");
    fl_set_object_color(obj,39,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  ScaleZ = obj = fl_add_lightbutton(FL_PUSH_BUTTON,220,265,100,30,"z");
    fl_set_object_color(obj,39,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fl_end_group();


  CoordSysGroup = fl_bgn_group();
  CameraButton = obj = fl_add_lightbutton(FL_RADIO_BUTTON,20,215,110,30,"Camera [C]");
    fl_set_object_color(obj,39,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,CoordSysProc,0);
  SelfButton = obj = fl_add_lightbutton(FL_RADIO_BUTTON,130,215,90,30,"Self [E]");
    fl_set_object_color(obj,39,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,CoordSysProc,0);
  WorldButton = obj = fl_add_lightbutton(FL_RADIO_BUTTON,220,215,100,30,"World [W]");
    fl_set_object_color(obj,39,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,CoordSysProc,0);
  fl_end_group();

  obj = fl_add_text(FL_NORMAL_TEXT,10,185,315,30,"In This Coordinate System:");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  fl_end_form();

}
/*---------------------------------------*/

FL_FORM *fooform;


void create_form_fooform(void)
{
  FL_OBJECT *obj;

  if (fooform)
     return;

  fooform = fl_bgn_form(FL_NO_BOX,430,690);
  obj = fl_add_box(FL_UP_BOX,0,0,430,690,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fl_end_form();

}
/*---------------------------------------*/

void create_the_forms(void)
{
  create_form_TransForm();
  create_form_fooform();
}

