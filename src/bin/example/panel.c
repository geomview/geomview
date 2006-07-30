/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
#include "panel.h"

FL_FORM *Example;

FL_OBJECT
        *VelocitySlider;

void create_form_Example(void)
{
  FL_OBJECT *obj;

  if (Example)
     return;

  Example = fl_bgn_form(FL_NO_BOX,340,130);
  obj = fl_add_box(FL_UP_BOX,0,0,340,130,"");
    fl_set_object_lsize(obj,11);
  VelocitySlider = obj = fl_add_slider(FL_HOR_SLIDER,10,60,320,30,"Velocity");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_callback(obj,VelocityProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,10,10,320,30,"EXAMPLE");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
  fl_end_form();

}
/*---------------------------------------*/

void create_the_forms(void)
{
  create_form_Example();
}

