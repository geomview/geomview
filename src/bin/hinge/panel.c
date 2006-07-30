/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
#include "panel.h"

FL_FORM *MainForm;

FL_OBJECT
        *QuitButton,
        *UndoButton,
        *FileButton,
        *HelpButton,
        *InfoButton,
        *SpaceBrowser,
        *ResetButton,
        *MainFormLabel,
        *AngleInput;

void create_form_MainForm(void)
{
  FL_OBJECT *obj;

  if (MainForm)
     return;

  MainForm = fl_bgn_form(FL_NO_BOX,300,360);
  obj = fl_add_box(FL_UP_BOX,0,0,300,360,"");
    fl_set_object_lsize(obj,11);
  QuitButton = obj = fl_add_button(FL_NORMAL_BUTTON,15,305,75,40,"Quit");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_callback(obj,QuitProc,0);
  UndoButton = obj = fl_add_button(FL_NORMAL_BUTTON,15,145,75,40,"Undo");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_callback(obj,UndoButtonProc,0);
  FileButton = obj = fl_add_button(FL_NORMAL_BUTTON,15,105,75,40,"File");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_callback(obj,FileButtonProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,150,245,86,30,"Space");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  HelpButton = obj = fl_add_button(FL_NORMAL_BUTTON,15,185,75,40,"Help");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_callback(obj,HelpButtonProc,0);
  InfoButton = obj = fl_add_button(FL_NORMAL_BUTTON,15,225,75,40,"Info");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_callback(obj,InfoButtonProc,0);
  SpaceBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,125,280,140,60,"");
    fl_set_object_lsize(obj,FL_HUGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER);
    fl_set_object_lstyle(obj,FL_TIMES_STYLE);
    fl_set_object_callback(obj,SpaceBrowserProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,25,55,265,25,"Click the right mouse button on an edge");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,30,75,245,25,"to select, then on a face to rotate.");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  ResetButton = obj = fl_add_button(FL_NORMAL_BUTTON,15,265,75,40,"Reset");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_callback(obj,ResetButtonProc,0);
  MainFormLabel = obj = fl_add_text(FL_NORMAL_TEXT,10,10,280,40,"Hinge");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_HUGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,120,150,81,34,"Angle");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  AngleInput = obj = fl_add_input(FL_NORMAL_INPUT,125,190,70,40,"");
    fl_set_object_color(obj,FL_BLUE,FL_CYAN);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_callback(obj,AngleInputProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,200,200,80,20,"degrees");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fl_end_form();

}
/*---------------------------------------*/

FL_FORM *FileForm;

FL_OBJECT
        *FileInput,
        *FileOKButton,
        *FileCancelButton;

void create_form_FileForm(void)
{
  FL_OBJECT *obj;

  if (FileForm)
     return;

  FileForm = fl_bgn_form(FL_NO_BOX,490,110);
  obj = fl_add_box(FL_UP_BOX,0,0,490,110,"");
    fl_set_object_lsize(obj,11);
  FileInput = obj = fl_add_input(FL_NORMAL_INPUT,20,50,440,40,"");
    fl_set_object_color(obj,FL_BOTTOM_BCOL,FL_CYAN);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_callback(obj,FileInputProc,0);
  FileOKButton = obj = fl_add_button(FL_RETURN_BUTTON,355,10,95,35,"OK");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_callback(obj,FileOKButtonProc,0);
  FileCancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,225,10,100,35,"Cancel");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_callback(obj,FileCancelButtonProc,0);
  fl_end_form();

}
/*---------------------------------------*/

FL_FORM *InfoForm;

FL_OBJECT
        *InfoFormLabel,
        *InfoOKButton;

void create_form_InfoForm(void)
{
  FL_OBJECT *obj;

  if (InfoForm)
     return;

  InfoForm = fl_bgn_form(FL_NO_BOX,310,290);
  obj = fl_add_box(FL_UP_BOX,0,0,310,290,"");
    fl_set_object_lsize(obj,11);
  InfoFormLabel = obj = fl_add_text(FL_NORMAL_TEXT,5,15,300,30,"Hinge");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,5,55,300,30,"by");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,5,85,300,30,"Mark Phillips");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,5,115,300,30,"The Geometry Center");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  InfoOKButton = obj = fl_add_button(FL_NORMAL_BUTTON,245,10,50,45,"OK");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_callback(obj,InfoOKButtonProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,5,161,300,30,"Hinge and Geomview are available to");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,4,179,300,30,"the public from");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,6,197,300,30,"www.geomview.org");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,6,259,300,30,"For usage instructions hit the ``Help'' button.");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  fl_end_form();

}
/*---------------------------------------*/

FL_FORM *HelpForm;

FL_OBJECT
        *HelpBrowser,
        *HelpOKButton;

void create_form_HelpForm(void)
{
  FL_OBJECT *obj;

  if (HelpForm)
     return;

  HelpForm = fl_bgn_form(FL_NO_BOX,360,340);
  obj = fl_add_box(FL_UP_BOX,0,0,360,340,"");
    fl_set_object_lsize(obj,11);
  obj = fl_add_text(FL_NORMAL_TEXT,115,15,125,35,"Hinge Help");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  HelpBrowser = obj = fl_add_browser(FL_NORMAL_BROWSER,10,60,335,275,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  HelpOKButton = obj = fl_add_button(FL_NORMAL_BUTTON,280,15,50,40,"OK");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_callback(obj,HelpOKButtonProc,0);
  fl_end_form();

}
/*---------------------------------------*/

void create_the_forms(void)
{
  create_form_MainForm();
  create_form_FileForm();
  create_form_InfoForm();
  create_form_HelpForm();
}

