/* Form definition file generated with fdesign. */

#include "forms.h"
#include "panel.h"

FL_FORM *Example;

FL_OBJECT
        *VelocitySlider;

void create_form_Example()
{
  FL_OBJECT *obj;
  Example = fl_bgn_form(FL_NO_BOX,340.0,130.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,340.0,130.0,"");
  VelocitySlider = obj = fl_add_slider(FL_HOR_SLIDER,10.0,40.0,320.0,30.0,"Velocity");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,VelocityProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,10.0,90.0,320.0,30.0,"EXAMPLE");
    fl_set_object_lcol(obj,4);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
    fl_set_object_lstyle(obj,FL_ENGRAVED_STYLE);
  fl_end_form();
}

/*---------------------------------------*/

void create_the_forms()
{
  create_form_Example();
}

