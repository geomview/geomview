/* Form definition file generated with fdesign. */

#include "forms.h"
#include "ginsuF.h"

FL_FORM *ClipPanel;

FL_OBJECT
        *SliceButton,
        *ExitButton,
        *InfoButton,
        *TargInput,
        *Ainput,
        *Binput,
        *Cinput,
        *Dinput,
        *EditButton;

void create_form_ClipPanel()
{
  FL_OBJECT *obj;
  ClipPanel = fl_bgn_form(FL_NO_BOX,200.0,320.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,200.0,320.0,"");
  SliceButton = obj = fl_add_button(FL_NORMAL_BUTTON,70.0,70.0,50.0,30.0,"Slice!");
  obj = fl_add_text(FL_NORMAL_TEXT,20.0,105.0,140.0,20.0,"ax + by + cz = d");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  ExitButton = obj = fl_add_button(FL_NORMAL_BUTTON,145.0,285.0,50.0,30.0,"Exit");
  InfoButton = obj = fl_add_button(FL_NORMAL_BUTTON,10.0,70.0,50.0,30.0,"Info");
  obj = fl_add_text(FL_NORMAL_TEXT,50.0,280.0,90.0,30.0,"Ginsu!");
    fl_set_object_lcol(obj,4);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
    fl_set_object_lstyle(obj,FL_ENGRAVED_STYLE);
  TargInput = obj = fl_add_input(FL_NORMAL_INPUT,10.0,30.0,175.0,30.0,"TARGET CLIP OBJECT");
    fl_set_object_color(obj,9,10);
    fl_set_object_align(obj,FL_ALIGN_BOTTOM);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  Ainput = obj = fl_add_input(FL_NORMAL_INPUT,10.0,250.0,165.0,30.0,"a");
    fl_set_object_align(obj,FL_ALIGN_RIGHT);
  Binput = obj = fl_add_input(FL_NORMAL_INPUT,10.0,210.0,165.0,30.0,"b");
    fl_set_object_align(obj,FL_ALIGN_RIGHT);
  Cinput = obj = fl_add_input(FL_NORMAL_INPUT,10.0,170.0,165.0,30.0,"c");
    fl_set_object_align(obj,FL_ALIGN_RIGHT);
  Dinput = obj = fl_add_input(FL_NORMAL_INPUT,10.0,130.0,165.0,30.0,"d");
    fl_set_object_align(obj,FL_ALIGN_RIGHT);
  EditButton = obj = fl_add_button(FL_PUSH_BUTTON,130.0,70.0,50.0,30.0,"Edit");
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *InfoPanel;

FL_OBJECT
        *MyBrowser,
        *CloseButton;

void create_form_InfoPanel()
{
  FL_OBJECT *obj;
  InfoPanel = fl_bgn_form(FL_NO_BOX,270.0,220.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,270.0,220.0,"");
  MyBrowser = obj = fl_add_browser(FL_NORMAL_BROWSER,10.0,10.0,250.0,160.0,"");
  obj = fl_add_text(FL_NORMAL_TEXT,30.0,180.0,205.0,30.0,"Ginsu Info");
    fl_set_object_lcol(obj,4);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
    fl_set_object_lstyle(obj,FL_ENGRAVED_STYLE);
  CloseButton = obj = fl_add_button(FL_NORMAL_BUTTON,200.0,180.0,60.0,30.0,"Close");
  fl_end_form();
}

/*---------------------------------------*/

void create_the_forms()
{
  create_form_ClipPanel();
  create_form_InfoPanel();
}

