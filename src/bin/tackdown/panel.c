/* Form definition file generated with fdesign. */

#include "forms.h"
#include "panel.h"

FL_FORM *MainForm;

FL_OBJECT
        *TransButton,
        *worldButton;

void create_form_MainForm()
{
  FL_OBJECT *obj;
  MainForm = fl_bgn_form(FL_NO_BOX,300.0,240.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,300.0,240.0,"");
  obj = fl_add_text(FL_NORMAL_TEXT,10.0,190.0,280.0,40.0,"Tack Down");
    fl_set_object_lcol(obj,1);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
    fl_set_object_lstyle(obj,FL_ENGRAVED_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,10.0,170.0,280.0,30.0,"Click button to fix target object in its current");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  TransButton = obj = fl_add_lightbutton(FL_NORMAL_BUTTON,10.0,50.0,130.0,40.0,"Tack Down");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,TransButtonProc,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,160.0,50.0,130.0,40.0,"Quit");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,QuitButtonProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,10.0,150.0,280.0,30.0,"location in the selected coordinate system");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  worldButton = obj = fl_add_button(FL_RADIO_BUTTON,0.0,0.0,150.0,40.0,"World Coordinates");
  obj = fl_add_button(FL_RADIO_BUTTON,150.0,0.0,150.0,40.0,"Universe Coordinates");
  obj = fl_add_text(FL_NORMAL_TEXT,10.0,120.0,280.0,30.0,"For best results, make sure normalization");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  obj = fl_add_text(FL_NORMAL_TEXT,10.0,100.0,280.0,30.0,"is off and the target object is not the world.");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  fl_end_form();
}

/*---------------------------------------*/

void create_the_forms()
{
  create_form_MainForm();
}

