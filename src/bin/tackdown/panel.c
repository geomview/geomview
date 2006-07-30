/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
#include "panel.h"

FL_FORM *MainForm;

FL_OBJECT
        *TransButton,
        *worldButton;

void create_form_MainForm(void)
{
  FL_OBJECT *obj;

  if (MainForm)
     return;

  MainForm = fl_bgn_form(FL_NO_BOX,300,240);
  obj = fl_add_box(FL_UP_BOX,0,0,300,240,"");
    fl_set_object_lsize(obj,11);
  obj = fl_add_text(FL_NORMAL_TEXT,10,10,280,40,"Tack Down");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lcolor(obj,FL_RED);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,10,40,280,30,"Click button to fix target object in its current");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  TransButton = obj = fl_add_lightbutton(FL_NORMAL_BUTTON,10,150,130,40,"Tack Down");
    fl_set_object_color(obj,39,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,TransButtonProc,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,160,150,130,40,"Quit");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,QuitButtonProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,10,60,280,30,"location in the selected coordinate system");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  worldButton = obj = fl_add_button(FL_RADIO_BUTTON,0,200,150,40,"World Coordinates");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  obj = fl_add_button(FL_RADIO_BUTTON,150,200,150,40,"Universe Coordinates");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  obj = fl_add_text(FL_NORMAL_TEXT,10,90,280,30,"For best results, make sure normalization");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,10,110,280,30,"is off and the target object is not the world.");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  fl_end_form();

}
/*---------------------------------------*/

void create_the_forms(void)
{
  create_form_MainForm();
}

