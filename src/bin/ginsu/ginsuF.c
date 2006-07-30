/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
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

void create_form_ClipPanel(void)
{
  FL_OBJECT *obj;

  if (ClipPanel)
     return;

  ClipPanel = fl_bgn_form(FL_NO_BOX,200,320);
  obj = fl_add_box(FL_UP_BOX,0,0,200,320,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  SliceButton = obj = fl_add_button(FL_NORMAL_BUTTON,70,220,50,30,"Slice!");
    fl_set_object_lcolor(obj,FL_RED);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,195,140,20,"ax + by + cz = d");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  ExitButton = obj = fl_add_button(FL_NORMAL_BUTTON,145,5,50,30,"Exit");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  InfoButton = obj = fl_add_button(FL_NORMAL_BUTTON,10,220,50,30,"Info");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  obj = fl_add_text(FL_NORMAL_TEXT,50,10,90,30,"Ginsu!");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE+FL_ENGRAVED_STYLE);
  TargInput = obj = fl_add_input(FL_NORMAL_INPUT,10,260,175,30,"TARGET CLIP OBJECT");
    fl_set_object_color(obj,FL_INDIANRED,FL_PALEGREEN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_BOTTOM);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  Ainput = obj = fl_add_input(FL_NORMAL_INPUT,10,40,165,30,"a");
    fl_set_object_color(obj,FL_BOTTOM_BCOL,FL_MAGENTA);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT);
  Binput = obj = fl_add_input(FL_NORMAL_INPUT,10,80,165,30,"b");
    fl_set_object_color(obj,FL_BOTTOM_BCOL,FL_MAGENTA);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT);
  Cinput = obj = fl_add_input(FL_NORMAL_INPUT,10,120,165,30,"c");
    fl_set_object_color(obj,FL_BOTTOM_BCOL,FL_MAGENTA);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT);
  Dinput = obj = fl_add_input(FL_NORMAL_INPUT,10,160,165,30,"d");
    fl_set_object_color(obj,FL_BOTTOM_BCOL,FL_MAGENTA);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT);
  EditButton = obj = fl_add_button(FL_PUSH_BUTTON,130,220,50,30,"Edit");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fl_end_form();

}
/*---------------------------------------*/

FL_FORM *InfoPanel;

FL_OBJECT
        *CloseButton,
        *MyBrowser;

void create_form_InfoPanel(void)
{
  FL_OBJECT *obj;

  if (InfoPanel)
     return;

  InfoPanel = fl_bgn_form(FL_NO_BOX,270,220);
  obj = fl_add_box(FL_UP_BOX,0,0,270,220,"");
    fl_set_object_lsize(obj,11);
  CloseButton = obj = fl_add_button(FL_NORMAL_BUTTON,200,10,60,30,"Close");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  MyBrowser = obj = fl_add_browser(FL_NORMAL_BROWSER,10,50,250,160,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  obj = fl_add_text(FL_NORMAL_TEXT,30,10,205,30,"Ginsu Info");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE+FL_ENGRAVED_STYLE);
  fl_end_form();

}
/*---------------------------------------*/

void create_the_forms(void)
{
  create_form_ClipPanel();
  create_form_InfoPanel();
}

