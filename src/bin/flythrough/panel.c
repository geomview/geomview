/* Form definition file generated with fdesign. */

#include "forms.h"
#include "panel.h"
#include "flythrough.h"

FL_FORM *MainForm;

FL_OBJECT
        *DodecScale,
        *Quit,
        *Info,
        *PathGroup,
        *Direct,
        *Equi,
        *Quarter,
        *Loop,
        *GoGroup,
        *Go,
        *Stop,
        *TileGroup,
        *Level3,
        *Level2,
        *Level1,
        *Level0,
        *SpeedGroup,
        *Speed2,
        *Speed3,
        *Speed4,
        *Speed1;

void create_form_MainForm()
{
  FL_OBJECT *obj;
  MainForm = fl_bgn_form(FL_NO_BOX,245.0,345.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,245.0,345.0,"");
  DodecScale = obj = fl_add_valslider(FL_HOR_SLIDER,10.0,10.0,226.0,22.0,"Scale Dodecahedron");
    fl_set_object_align(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,ScaleProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,0.0,300.0,245.0,20.0,"Interactive Hyperbolic Flythrough");
    fl_set_object_lcol(obj,4);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,0.0,100.0,80.0,30.0,"LAYERS");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,5.0,190.0,230.0,15.0,"PATH");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  Quit = obj = fl_add_button(FL_NORMAL_BUTTON,190.0,265.0,40.0,30.0,"Quit");
    fl_set_call_back(obj,QuitProc,0);
  obj = fl_add_box(FL_BORDER_BOX,0.0,55.0,245.0,5.0,"");
    fl_set_object_color(obj,4,4);
  obj = fl_add_box(FL_BORDER_BOX,0.0,130.0,245.0,5.0,"");
    fl_set_object_color(obj,4,4);
  Info = obj = fl_add_button(FL_PUSH_BUTTON,10.0,265.0,170.0,30.0,"What's going on?");
    fl_set_call_back(obj,InfoProc,0);
  obj = fl_add_box(FL_BORDER_BOX,0.0,210.0,245.0,5.0,"");
    fl_set_object_color(obj,4,4);
  PathGroup = fl_bgn_group();
  Direct = obj = fl_add_roundbutton(FL_RADIO_BUTTON,125.0,165.0,30.0,30.0,"Direct");
    fl_set_object_lcol(obj,223);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,PathProc,DIRECT);
  Equi = obj = fl_add_roundbutton(FL_RADIO_BUTTON,125.0,140.0,30.0,30.0,"Equidistant");
    fl_set_object_lcol(obj,248);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,PathProc,EQUI);
  Quarter = obj = fl_add_roundbutton(FL_RADIO_BUTTON,5.0,140.0,30.0,30.0,"Quarter Turn");
    fl_set_object_lcol(obj,135);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,PathProc,QUARTER);
  Loop = obj = fl_add_roundbutton(FL_RADIO_BUTTON,5.0,165.0,30.0,30.0,"Full Loop");
    fl_set_object_lcol(obj,135);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,PathProc,LOOP);
  fl_end_group();
  obj = fl_add_text(FL_NORMAL_TEXT,0.0,60.0,60.0,30.0,"STEPS");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_box(FL_BORDER_BOX,0.0,92.0,245.0,5.0,"");
    fl_set_object_color(obj,4,4);
  GoGroup = fl_bgn_group();
  Go = obj = fl_add_roundbutton(FL_RADIO_BUTTON,20.0,212.0,55.0,45.0,"GO");
    fl_set_object_color(obj,7,2);
    fl_set_object_lcol(obj,2);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,GoProc,1);
  Stop = obj = fl_add_roundbutton(FL_RADIO_BUTTON,110.0,212.0,55.0,45.0,"STOP");
    fl_set_object_color(obj,7,1);
    fl_set_object_lcol(obj,1);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,GoProc,0);
  fl_end_group();
  TileGroup = fl_bgn_group();
  Level3 = obj = fl_add_roundbutton(FL_RADIO_BUTTON,195.0,100.0,30.0,30.0,"3");
    fl_set_call_back(obj,TilingProc,3);
  Level2 = obj = fl_add_roundbutton(FL_RADIO_BUTTON,155.0,100.0,30.0,30.0,"2");
    fl_set_call_back(obj,TilingProc,2);
  Level1 = obj = fl_add_roundbutton(FL_RADIO_BUTTON,115.0,100.0,30.0,30.0,"1");
    fl_set_call_back(obj,TilingProc,1);
  Level0 = obj = fl_add_roundbutton(FL_RADIO_BUTTON,75.0,100.0,30.0,30.0,"0");
    fl_set_call_back(obj,TilingProc,0);
  fl_end_group();
  SpeedGroup = fl_bgn_group();
  Speed2 = obj = fl_add_roundbutton(FL_RADIO_BUTTON,100.0,60.0,30.0,30.0,"20");
    fl_set_call_back(obj,SpeedProc,2);
  Speed3 = obj = fl_add_roundbutton(FL_RADIO_BUTTON,145.0,60.0,30.0,30.0,"40");
    fl_set_call_back(obj,SpeedProc,3);
  Speed4 = obj = fl_add_roundbutton(FL_RADIO_BUTTON,190.0,60.0,30.0,30.0,"80");
    fl_set_call_back(obj,SpeedProc,4);
  Speed1 = obj = fl_add_roundbutton(FL_RADIO_BUTTON,55.0,60.0,30.0,30.0,"10");
    fl_set_call_back(obj,SpeedProc,1);
  fl_end_group();
  obj = fl_add_text(FL_NORMAL_TEXT,5.0,320.0,240.0,20.0,"Not Knot: The Software");
    fl_set_object_lcol(obj,4);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_box(FL_BORDER_BOX,0.0,256.0,245.0,5.0,"");
    fl_set_object_color(obj,4,4);
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *HelpForm;

FL_OBJECT
        *HelpBrowser,
        *Done,
        *DiagramGroup,
        *EucDiag,
        *HypDiag;

void create_form_HelpForm()
{
  FL_OBJECT *obj;
  HelpForm = fl_bgn_form(FL_NO_BOX,530.0,340.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,530.0,340.0,"");
  HelpBrowser = obj = fl_add_browser(FL_NORMAL_BROWSER,10.0,30.0,510.0,275.0,"");
  obj = fl_add_text(FL_NORMAL_TEXT,10.0,310.0,435.0,20.0,"Not Knot: The Software   Interactive Hyperbolic Flythrough");
    fl_set_object_lcol(obj,4);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  Done = obj = fl_add_button(FL_NORMAL_BUTTON,455.0,310.0,60.0,20.0,"Done");
    fl_set_call_back(obj,DoneProc,0);
  DiagramGroup = fl_bgn_group();
  EucDiag = obj = fl_add_button(FL_RADIO_BUTTON,95.0,5.0,140.0,25.0,"Euclidean Diagram");
    fl_set_call_back(obj,DiagProc,EUC);
  HypDiag = obj = fl_add_button(FL_RADIO_BUTTON,305.0,5.0,150.0,25.0,"Hyperbolic Diagram");
    fl_set_call_back(obj,DiagProc,HYP);
  fl_end_group();
  fl_end_form();
}

/*---------------------------------------*/

void create_the_forms()
{
  create_form_MainForm();
  create_form_HelpForm();
}

