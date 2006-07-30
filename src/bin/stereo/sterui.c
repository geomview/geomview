/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
#include "sterui.h"

FL_FORM *stereo;

FL_OBJECT
        *StereoBrowser,
        *SwapButton,
        *QuitButton,
        *BestViewText,
        *FocalButton,
        *HelpButton,
        *ConvSlider,
        *MoreButton;

void create_form_stereo(void)
{
  FL_OBJECT *obj;

  if (stereo)
     return;

  stereo = fl_bgn_form(FL_NO_BOX,225,205);
  obj = fl_add_box(FL_UP_BOX,0,0,225,205,"");
    fl_set_object_color(obj,FL_INDIANRED,FL_COL1);
    fl_set_object_lsize(obj,11);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  StereoBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,9,9,111,78,"");
    fl_set_object_lsize(obj,11);
    fl_set_object_callback(obj,StereoProc,0);
  SwapButton = obj = fl_add_lightbutton(FL_PUSH_BUTTON,123,36,93,24,"Eye Swap");
    fl_set_object_color(obj,39,FL_YELLOW);
    fl_set_object_lsize(obj,11);
    fl_set_object_callback(obj,SwapProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,0,175,170,26,"Stereo View 1.0");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_INDIANRED,FL_BLACK);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE+FL_ENGRAVED_STYLE);
  QuitButton = obj = fl_add_button(FL_NORMAL_BUTTON,180,175,38,24,"Quit");
    fl_set_object_lsize(obj,11);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,QuitProc,0);
  BestViewText = obj = fl_add_box(FL_BORDER_BOX,10,90,205,40,"");
    fl_set_object_color(obj,FL_INDIANRED,FL_COL1);
    fl_set_object_lsize(obj,11);
  FocalButton = obj = fl_add_button(FL_NORMAL_BUTTON,123,63,93,24,"Focal Plane");
    fl_set_object_lsize(obj,11);
    fl_set_object_callback(obj,FocalProc,0);
  HelpButton = obj = fl_add_button(FL_NORMAL_BUTTON,176,8,40,23,"Help");
    fl_set_object_lsize(obj,11);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,HelpProc,1);
  ConvSlider = obj = fl_add_valslider(FL_HOR_NICE_SLIDER,10,150,205,20,"Convergence angle");
    fl_set_object_color(obj,FL_INDIANRED,FL_PALEGREEN);
    fl_set_object_lsize(obj,11);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_callback(obj,ConvProc,0);
  MoreButton = obj = fl_add_button(FL_NORMAL_BUTTON,124,8,49,24,"More..");
    fl_set_object_lsize(obj,11);
    fl_set_object_callback(obj,MoreProc,1);
  fl_end_form();

}
/*---------------------------------------*/

FL_FORM *Help;

FL_OBJECT
        *HelpBrowser,
        *DoneButton;

void create_form_Help(void)
{
  FL_OBJECT *obj;

  if (Help)
     return;

  Help = fl_bgn_form(FL_NO_BOX,475,450);
  obj = fl_add_box(FL_UP_BOX,0,0,475,450,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  HelpBrowser = obj = fl_add_browser(FL_NORMAL_BROWSER,10,10,460,410,"");
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
  DoneButton = obj = fl_add_button(FL_NORMAL_BUTTON,425,420,45,24,"Done");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,DoneProc,(long)Help);
  fl_end_form();

}
/*---------------------------------------*/

FL_FORM *More;

FL_OBJECT
        *OcularInput,
        *ScreenWidthInput,
        *DoneButton,
        *FixedCamButton,
        *CamNameInput;

void create_form_More(void)
{
  FL_OBJECT *obj;

  if (More)
     return;

  More = fl_bgn_form(FL_NO_BOX,255,140);
  obj = fl_add_box(FL_UP_BOX,0,0,255,140,"");
    fl_set_object_color(obj,FL_INDIANRED,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  OcularInput = obj = fl_add_input(FL_NORMAL_INPUT,10,12,110,26,"Ocular Separation");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_CYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT);
    fl_set_object_callback(obj,OcularSepProc,0);
  ScreenWidthInput = obj = fl_add_input(FL_NORMAL_INPUT,10,44,110,24,"Screen Width");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_CYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT);
    fl_set_object_callback(obj,ScreenWidthProc,0);
  DoneButton = obj = fl_add_button(FL_NORMAL_BUTTON,204,105,45,24,"Done");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,DoneProc,(long)More);
  obj = fl_add_text(FL_NORMAL_TEXT,139,32,78,16,"(in inches)");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_INDIANRED,FL_BLACK);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  FixedCamButton = obj = fl_add_lightbutton(FL_PUSH_BUTTON,10,105,120,25,"Fixed camera");
    fl_set_object_color(obj,39,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  CamNameInput = obj = fl_add_input(FL_NORMAL_INPUT,10,75,110,24,"Camera Name");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_CYAN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT);
    fl_set_object_callback(obj,CamNameProc,0);
  fl_end_form();

}
/*---------------------------------------*/

void create_the_forms(void)
{
  create_form_stereo();
  create_form_Help();
  create_form_More();
}

