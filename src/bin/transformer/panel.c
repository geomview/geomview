/* Form definition file generated with fdesign. */

#include "forms.h"
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

void create_form_TransForm()
{
  FL_OBJECT *obj;
  TransForm = fl_bgn_form(FL_NO_BOX,340.0,475.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,340.0,475.0,"");
  obj = fl_add_box(FL_DOWN_BOX,10.0,130.0,320.0,200.0,"");
  t00 = obj = fl_add_input(FL_NORMAL_INPUT,30.0,430.0,70.0,30.0,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,15,15);
  t01 = obj = fl_add_input(FL_NORMAL_INPUT,100.0,430.0,70.0,30.0,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,15,15);
  t02 = obj = fl_add_input(FL_NORMAL_INPUT,170.0,430.0,70.0,30.0,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,15,15);
  t03 = obj = fl_add_input(FL_NORMAL_INPUT,240.0,430.0,70.0,30.0,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,15,15);
  t10 = obj = fl_add_input(FL_NORMAL_INPUT,30.0,400.0,70.0,30.0,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,15,15);
  t11 = obj = fl_add_input(FL_NORMAL_INPUT,100.0,400.0,70.0,30.0,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,15,15);
  t12 = obj = fl_add_input(FL_NORMAL_INPUT,170.0,400.0,70.0,30.0,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,15,15);
  t13 = obj = fl_add_input(FL_NORMAL_INPUT,240.0,400.0,70.0,30.0,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,15,15);
  t20 = obj = fl_add_input(FL_NORMAL_INPUT,30.0,370.0,70.0,30.0,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,15,15);
  t21 = obj = fl_add_input(FL_NORMAL_INPUT,100.0,370.0,70.0,30.0,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,15,15);
  t22 = obj = fl_add_input(FL_NORMAL_INPUT,170.0,370.0,70.0,30.0,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,15,15);
  t30 = obj = fl_add_input(FL_NORMAL_INPUT,30.0,340.0,70.0,30.0,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,15,15);
  t31 = obj = fl_add_input(FL_NORMAL_INPUT,100.0,340.0,70.0,30.0,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,15,15);
  t32 = obj = fl_add_input(FL_NORMAL_INPUT,170.0,340.0,70.0,30.0,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,15,15);
  t33 = obj = fl_add_input(FL_NORMAL_INPUT,240.0,340.0,70.0,30.0,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,15,15);
  ApplyButton = obj = fl_add_button(FL_NORMAL_BUTTON,10.0,90.0,80.0,30.0,"Apply [a ]");
    fl_set_call_back(obj,ApplyProc,0);
  SetButton = obj = fl_add_button(FL_NORMAL_BUTTON,90.0,90.0,80.0,30.0,"Set [s]");
    fl_set_call_back(obj,SetProc,0);
  ReadButton = obj = fl_add_button(FL_NORMAL_BUTTON,170.0,90.0,80.0,30.0,"Read [r]");
    fl_set_call_back(obj,ReadProc,0);
  DoneButton = obj = fl_add_button(FL_NORMAL_BUTTON,115.0,10.0,100.0,30.0,"Quit [q]");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,DoneProc,0);
  IdentityButton = obj = fl_add_button(FL_NORMAL_BUTTON,250.0,90.0,80.0,30.0,"Identity [i]");
    fl_set_call_back(obj,IdentityProc,0);
  targetGroup = fl_bgn_group();
  actGeom = obj = fl_add_lightbutton(FL_RADIO_BUTTON,30.0,50.0,140.0,30.0,"Act On Geom");
  actCam = obj = fl_add_lightbutton(FL_RADIO_BUTTON,170.0,50.0,140.0,30.0,"Act On Camera");
  fl_end_group();
  TranslateGroup = fl_bgn_group();
  TranslationAxisX = obj = fl_add_lightbutton(FL_RADIO_BUTTON,20.0,180.0,100.0,30.0,"x");
  TranslationAxisY = obj = fl_add_lightbutton(FL_RADIO_BUTTON,120.0,180.0,100.0,30.0,"y");
  TranslationAxisZ = obj = fl_add_lightbutton(FL_RADIO_BUTTON,220.0,180.0,100.0,30.0,"z");
  TranslationMeasure = obj = fl_add_input(FL_NORMAL_INPUT,40.0,140.0,100.0,30.0,"units");
    fl_set_object_color(obj,15,15);
    fl_set_object_align(obj,FL_ALIGN_RIGHT);
  TranslateButton = obj = fl_add_button(FL_NORMAL_BUTTON,200.0,140.0,100.0,30.0,"Translate [T]");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,TranslateButtonProc,0);
  fl_end_group();
  ModeGroup = fl_bgn_group();
  RotateModeButton = obj = fl_add_button(FL_RADIO_BUTTON,20.0,290.0,100.0,30.0,"Rotate");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,RotateMode,0);
  TranslateModeButton = obj = fl_add_button(FL_RADIO_BUTTON,120.0,290.0,100.0,30.0,"Translate");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,TranslateMode,0);
  ScaleModeButton = obj = fl_add_button(FL_RADIO_BUTTON,220.0,290.0,100.0,30.0,"Scale");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,ScaleMode,0);
  fl_end_group();
  RotateGroup = fl_bgn_group();
  RotationAxisY = obj = fl_add_lightbutton(FL_RADIO_BUTTON,120.0,180.0,100.0,30.0,"About y");
  RotationAxisZ = obj = fl_add_lightbutton(FL_RADIO_BUTTON,220.0,180.0,100.0,30.0,"About z");
  RotateButton = obj = fl_add_button(FL_NORMAL_BUTTON,210.0,140.0,100.0,30.0,"Rotate [R]");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,RotateButtonProc,0);
  RotationMeasure = obj = fl_add_input(FL_NORMAL_INPUT,30.0,140.0,100.0,30.0,"degrees");
    fl_set_object_color(obj,15,15);
    fl_set_object_align(obj,FL_ALIGN_RIGHT);
  RotationAxisX = obj = fl_add_lightbutton(FL_RADIO_BUTTON,20.0,180.0,100.0,30.0,"About x");
  fl_end_group();
  t23 = obj = fl_add_input(FL_NORMAL_INPUT,240.0,370.0,70.0,30.0,"");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,15,15);
  ScaleGroup = fl_bgn_group();
  ScaleMeasure = obj = fl_add_input(FL_NORMAL_INPUT,40.0,140.0,100.0,30.0,"units");
    fl_set_object_color(obj,15,15);
    fl_set_object_align(obj,FL_ALIGN_RIGHT);
  ScaleButton = obj = fl_add_button(FL_NORMAL_BUTTON,200.0,140.0,100.0,30.0,"Scale [S]");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,ScaleButtonProc,0);
  ScaleX = obj = fl_add_lightbutton(FL_PUSH_BUTTON,20.0,180.0,100.0,30.0,"x");
  ScaleY = obj = fl_add_lightbutton(FL_PUSH_BUTTON,120.0,180.0,100.0,30.0,"y");
  ScaleZ = obj = fl_add_lightbutton(FL_PUSH_BUTTON,220.0,180.0,100.0,30.0,"z");
  fl_end_group();
  CoordSysGroup = fl_bgn_group();
  CameraButton = obj = fl_add_lightbutton(FL_RADIO_BUTTON,20.0,230.0,110.0,30.0,"Camera [C]");
    fl_set_call_back(obj,CoordSysProc,0);
  SelfButton = obj = fl_add_lightbutton(FL_RADIO_BUTTON,130.0,230.0,90.0,30.0,"Self [E]");
    fl_set_call_back(obj,CoordSysProc,0);
  WorldButton = obj = fl_add_lightbutton(FL_RADIO_BUTTON,220.0,230.0,100.0,30.0,"World [W]");
    fl_set_call_back(obj,CoordSysProc,0);
  fl_end_group();
  obj = fl_add_text(FL_NORMAL_TEXT,10.0,260.0,315.0,30.0,"In This Coordinate System:");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *fooform;


void create_form_fooform()
{
  FL_OBJECT *obj;
  fooform = fl_bgn_form(FL_NO_BOX,430.0,690.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,430.0,690.0,"");
  fl_end_form();
}

/*---------------------------------------*/

void create_the_forms()
{
  create_form_TransForm();
  create_form_fooform();
}

