/* Form definition file generated with fdesign. */

#include "forms.h"
#include "panel.h"

FL_FORM *MainForm;

FL_OBJECT
        *QuitButton,
        *DrawButton,
        *UndoButton,
        *VertexButton;

void create_form_MainForm()
{
  FL_OBJECT *obj;
  MainForm = fl_bgn_form(FL_NO_BOX,280.0,180.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,280.0,180.0,"");
  QuitButton = obj = fl_add_button(FL_NORMAL_BUTTON,200.0,70.0,60.0,40.0,"Quit");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,QuitProc,0);
  DrawButton = obj = fl_add_button(FL_NORMAL_BUTTON,20.0,70.0,150.0,40.0,"New Line");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,DrawButtonProc,0);
  UndoButton = obj = fl_add_button(FL_NORMAL_BUTTON,20.0,20.0,90.0,40.0,"Undo");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,UndoButtonProc,0);
  VertexButton = obj = fl_add_lightbutton(FL_PUSH_BUTTON,130.0,20.0,130.0,40.0,"Vertices Only");
    fl_set_call_back(obj,VertexButtonProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,0.0,110.0,280.0,50.0,"Click RIGHT mouse button on an\n object to draw lines.");
    fl_set_object_lcol(obj,4);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,0.0,150.0,280.0,30.0,"GRAFFITI");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *PickInfoForm;

FL_OBJECT
        *DoneButton;

void create_form_PickInfoForm()
{
  FL_OBJECT *obj;
  PickInfoForm = fl_bgn_form(FL_NO_BOX,295.0,165.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,295.0,165.0,"");
  obj = fl_add_text(FL_NORMAL_TEXT,15.0,135.0,255.0,15.0,"The cursor is not pointing at an object.");
  DoneButton = obj = fl_add_button(FL_NORMAL_BUTTON,220.0,10.0,60.0,40.0,"OK");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,DoneButtonProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,5.0,115.0,280.0,15.0,"Click the right mouse button on any object");
  obj = fl_add_text(FL_NORMAL_TEXT,10.0,95.0,270.0,15.0,"in the geomview window to draw.  If you");
  obj = fl_add_text(FL_NORMAL_TEXT,5.0,75.0,270.0,15.0,"have no objects to draw on, you may load");
  obj = fl_add_text(FL_NORMAL_TEXT,20.0,55.0,240.0,15.0,"one with the geomview Load button.");
  fl_end_form();
}

/*---------------------------------------*/

void create_the_forms()
{
  create_form_MainForm();
  create_form_PickInfoForm();
}

