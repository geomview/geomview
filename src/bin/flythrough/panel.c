/* Form definition file generated with fdesign. */

#include "flythrough.h"
#include <stdlib.h>
#include "panel.h"

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

void create_form_MainForm(void)
{
  FL_OBJECT *obj;

  if (MainForm)
     return;

  MainForm = fl_bgn_form(FL_NO_BOX,240,340);
  obj = fl_add_box(FL_UP_BOX,0,0,240,340,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  DodecScale = obj = fl_add_valslider(FL_HOR_SLIDER,10,313,226,22,"Scale Dodecahedron");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,ScaleProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,0,25,245,20,"Interactive Hyperbolic Flythrough");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,0,215,80,30,"LAYERS");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,5,140,230,15,"PATH");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  Quit = obj = fl_add_button(FL_NORMAL_BUTTON,190,50,40,30,"Quit");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,QuitProc,0);
  obj = fl_add_box(FL_SHADOW_BOX,0,285,245,5,"");
    fl_set_object_color(obj,FL_BLUE,FL_BLUE);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  obj = fl_add_box(FL_SHADOW_BOX,0,210,245,5,"");
    fl_set_object_color(obj,FL_BLUE,FL_BLUE);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  Info = obj = fl_add_button(FL_PUSH_BUTTON,10,50,170,30,"What's going on?");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,InfoProc,0);
  obj = fl_add_box(FL_SHADOW_BOX,0,130,245,5,"");
    fl_set_object_color(obj,FL_BLUE,FL_BLUE);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);

  PathGroup = fl_bgn_group();
  Direct = obj = fl_add_roundbutton(FL_RADIO_BUTTON,125,150,30,30,"Direct");
    fl_set_object_color(obj,FL_WHITE,FL_YELLOW);
    fl_set_object_lcolor(obj,223);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,PathProc,DIRECT);
  Equi = obj = fl_add_roundbutton(FL_RADIO_BUTTON,125,175,30,30,"Equidistant");
    fl_set_object_color(obj,FL_WHITE,FL_YELLOW);
    fl_set_object_lcolor(obj,248);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,PathProc,EQUI);
  Quarter = obj = fl_add_roundbutton(FL_RADIO_BUTTON,5,175,30,30,"Quarter Turn");
    fl_set_object_color(obj,FL_WHITE,FL_YELLOW);
    fl_set_object_lcolor(obj,135);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,PathProc,QUARTER);
  Loop = obj = fl_add_roundbutton(FL_RADIO_BUTTON,5,150,30,30,"Full Loop");
    fl_set_object_color(obj,FL_WHITE,FL_YELLOW);
    fl_set_object_lcolor(obj,135);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,PathProc,LOOP);
  fl_end_group();

  obj = fl_add_text(FL_NORMAL_TEXT,0,255,60,30,"STEPS");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_box(FL_SHADOW_BOX,0,248,245,5,"");
    fl_set_object_color(obj,FL_BLUE,FL_BLUE);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);

  GoGroup = fl_bgn_group();
  Go = obj = fl_add_roundbutton(FL_RADIO_BUTTON,20,88,55,45,"GO");
    fl_set_object_color(obj,FL_WHITE,FL_GREEN);
    fl_set_object_lcolor(obj,FL_GREEN);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,GoProc,1);
  Stop = obj = fl_add_roundbutton(FL_RADIO_BUTTON,110,88,55,45,"STOP");
    fl_set_object_color(obj,FL_WHITE,FL_RED);
    fl_set_object_lcolor(obj,FL_RED);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,GoProc,0);
  fl_end_group();


  TileGroup = fl_bgn_group();
  Level3 = obj = fl_add_roundbutton(FL_RADIO_BUTTON,195,215,30,30,"3");
    fl_set_object_color(obj,FL_WHITE,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,TilingProc,3);
  Level2 = obj = fl_add_roundbutton(FL_RADIO_BUTTON,155,215,30,30,"2");
    fl_set_object_color(obj,FL_WHITE,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,TilingProc,2);
  Level1 = obj = fl_add_roundbutton(FL_RADIO_BUTTON,115,215,30,30,"1");
    fl_set_object_color(obj,FL_WHITE,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,TilingProc,1);
  Level0 = obj = fl_add_roundbutton(FL_RADIO_BUTTON,75,215,30,30,"0");
    fl_set_object_color(obj,FL_WHITE,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,TilingProc,0);
  fl_end_group();


  SpeedGroup = fl_bgn_group();
  Speed2 = obj = fl_add_roundbutton(FL_RADIO_BUTTON,100,255,30,30,"20");
    fl_set_object_color(obj,FL_WHITE,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,SpeedProc,2);
  Speed3 = obj = fl_add_roundbutton(FL_RADIO_BUTTON,145,255,30,30,"40");
    fl_set_object_color(obj,FL_WHITE,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,SpeedProc,3);
  Speed4 = obj = fl_add_roundbutton(FL_RADIO_BUTTON,190,255,30,30,"80");
    fl_set_object_color(obj,FL_WHITE,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,SpeedProc,4);
  Speed1 = obj = fl_add_roundbutton(FL_RADIO_BUTTON,55,255,30,30,"10");
    fl_set_object_color(obj,FL_WHITE,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,SpeedProc,1);
  fl_end_group();

  obj = fl_add_text(FL_NORMAL_TEXT,5,5,240,20,"Not Knot: The Software");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_box(FL_SHADOW_BOX,0,84,245,5,"");
    fl_set_object_color(obj,FL_BLUE,FL_BLUE);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
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

void create_form_HelpForm(void)
{
  FL_OBJECT *obj;

  if (HelpForm)
     return;

  HelpForm = fl_bgn_form(FL_NO_BOX,530,340);
  obj = fl_add_box(FL_UP_BOX,0,0,530,340,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  HelpBrowser = obj = fl_add_browser(FL_NORMAL_BROWSER,10,35,510,275,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  obj = fl_add_text(FL_NORMAL_TEXT,10,10,435,20,"Not Knot: The Software   Interactive Hyperbolic Flythrough");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  Done = obj = fl_add_button(FL_NORMAL_BUTTON,455,10,60,20,"Done");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,DoneProc,0);

  DiagramGroup = fl_bgn_group();
  EucDiag = obj = fl_add_button(FL_RADIO_BUTTON,95,310,140,25,"Euclidean Diagram");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,DiagProc,EUC);
  HypDiag = obj = fl_add_button(FL_RADIO_BUTTON,305,310,150,25,"Hyperbolic Diagram");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,DiagProc,HYP);
  fl_end_group();

  fl_end_form();

}
/*---------------------------------------*/

void create_the_forms(void)
{
  create_form_MainForm();
  create_form_HelpForm();
}

