/* Form definition file generated with fdesign. */

#include "forms.h"
#include "panel.h"

FL_FORM *MainForm;

FL_OBJECT
        *QuitButton,
        *UndoButton,
        *FileButton,
        *AngleInput,
        *HelpButton,
        *InfoButton,
        *SpaceBrowser,
        *ResetButton,
        *MainFormLabel;

void create_form_MainForm()
{
  FL_OBJECT *obj;
  MainForm = fl_bgn_form(FL_NO_BOX,300.0,360.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,300.0,360.0,"");
  QuitButton = obj = fl_add_button(FL_NORMAL_BUTTON,15.0,15.0,75.0,40.0,"Quit");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,QuitProc,0);
  UndoButton = obj = fl_add_button(FL_NORMAL_BUTTON,15.0,175.0,75.0,40.0,"Undo");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,UndoButtonProc,0);
  FileButton = obj = fl_add_button(FL_NORMAL_BUTTON,15.0,215.0,75.0,40.0,"File");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,FileButtonProc,0);
  AngleInput = obj = fl_add_input(FL_NORMAL_INPUT,156.0,130.0,70.0,40.0,"");
    fl_set_object_color(obj,4,6);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_align(obj,FL_ALIGN_TOP);
    fl_set_call_back(obj,AngleInputProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,151.0,175.0,80.0,20.0,"(degrees)");
  obj = fl_add_text(FL_NORMAL_TEXT,150.0,192.0,81.0,34.0,"Angle");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  obj = fl_add_text(FL_NORMAL_TEXT,150.0,85.0,86.3,30.0,"Space");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  HelpButton = obj = fl_add_button(FL_NORMAL_BUTTON,15.0,135.0,75.0,40.0,"Help");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,HelpButtonProc,0);
  InfoButton = obj = fl_add_button(FL_NORMAL_BUTTON,15.0,95.0,75.0,40.0,"Info");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,InfoButtonProc,0);
  SpaceBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,125.0,20.0,140.0,60.0,"");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
    fl_set_call_back(obj,SpaceBrowserProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,25.0,280.0,265.0,25.0,"Click the right mouse button on an edge");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  obj = fl_add_text(FL_NORMAL_TEXT,30.0,260.0,245.0,25.0,"to select, then on a face to rotate.");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  ResetButton = obj = fl_add_button(FL_NORMAL_BUTTON,15.0,55.0,75.0,40.0,"Reset");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,ResetButtonProc,0);
  MainFormLabel = obj = fl_add_text(FL_NORMAL_TEXT,10.0,310.0,280.0,40.0,"Hinge");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *FileForm;

FL_OBJECT
        *FileInput,
        *FileOKButton,
        *FileCancelButton;

void create_form_FileForm()
{
  FL_OBJECT *obj;
  FileForm = fl_bgn_form(FL_NO_BOX,490.0,110.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,490.0,110.0,"");
  FileInput = obj = fl_add_input(FL_NORMAL_INPUT,20.0,20.0,440.0,40.0,"");
    fl_set_object_color(obj,13,6);
    fl_set_call_back(obj,FileInputProc,0);
  FileOKButton = obj = fl_add_button(FL_RETURN_BUTTON,355.0,65.0,95.0,35.0,"OK");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,FileOKButtonProc,0);
  FileCancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,225.0,65.0,100.0,35.0,"Cancel");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,FileCancelButtonProc,0);
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *InfoForm;

FL_OBJECT
        *InfoFormLabel,
        *InfoOKButton;

void create_form_InfoForm()
{
  FL_OBJECT *obj;
  InfoForm = fl_bgn_form(FL_NO_BOX,310.0,295.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,310.0,295.0,"");
  InfoFormLabel = obj = fl_add_text(FL_NORMAL_TEXT,5.0,250.0,300.0,30.0,"Hinge");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  obj = fl_add_text(FL_NORMAL_TEXT,5.0,210.0,300.0,30.0,"by");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  obj = fl_add_text(FL_NORMAL_TEXT,5.0,180.0,300.0,30.0,"Mark Phillips");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  obj = fl_add_text(FL_NORMAL_TEXT,5.0,150.0,300.0,30.0,"The Geometry Center");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  obj = fl_add_text(FL_NORMAL_TEXT,6.0,50.0,300.0,30.0,"(IP address 128.101.25.31)");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  InfoOKButton = obj = fl_add_button(FL_NORMAL_BUTTON,245.0,240.0,50.0,45.0,"OK");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,InfoOKButtonProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,5.0,104.0,300.0,30.0,"Hinge and Geomview are available to");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  obj = fl_add_text(FL_NORMAL_TEXT,4.0,86.0,300.0,30.0,"the public from");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  obj = fl_add_text(FL_NORMAL_TEXT,6.0,68.0,300.0,30.0,"www.geomview.org");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  obj = fl_add_text(FL_NORMAL_TEXT,6.0,6.0,300.0,30.0,"For usage instructions hit the ``Help'' button.");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *HelpForm;

FL_OBJECT
        *HelpBrowser,
        *HelpOKButton;

void create_form_HelpForm()
{
  FL_OBJECT *obj;
  HelpForm = fl_bgn_form(FL_NO_BOX,360.0,345.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,360.0,345.0,"");
  obj = fl_add_text(FL_NORMAL_TEXT,115.0,295.0,125.0,35.0,"Hinge Help");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  HelpBrowser = obj = fl_add_browser(FL_NORMAL_BROWSER,10.0,10.0,335.0,275.0,"");
  HelpOKButton = obj = fl_add_button(FL_NORMAL_BUTTON,280.0,290.0,50.0,40.0,"OK");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,HelpOKButtonProc,0);
  fl_end_form();
}

/*---------------------------------------*/

void create_the_forms()
{
  create_form_MainForm();
  create_form_FileForm();
  create_form_InfoForm();
  create_form_HelpForm();
}

