/* Form definition file generated with fdesign. */

#include "forms.h"
#include "controlpanel.h"

#include "discgrpP.h"
#include "dgflag.h"
#include "maniview.h"

FL_FORM *MainForm;

FL_OBJECT
        *SaveButton,
        *DisplayButton,
        *InfoButton,
        *EnumerateButton,
        *TileFormButton,
        *HelpButton,
        *QuitButton,
        *LoadButton;

void create_form_MainForm()
{
  FL_OBJECT *obj;
  MainForm = fl_bgn_form(FL_NO_BOX,250.0,460.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,250.0,460.0,"");
  obj = fl_add_text(FL_NORMAL_TEXT,60.0,400.0,130.0,50.0,"Maniview");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_color(obj,7,47);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_lstyle(obj,FL_ENGRAVED_STYLE);
  SaveButton = obj = fl_add_button(FL_PUSH_BUTTON,20.0,270.0,210.0,40.0,"Save");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,SaveButtonProc,0);
  DisplayButton = obj = fl_add_button(FL_PUSH_BUTTON,20.0,230.0,210.0,40.0,"Display");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,DisplayButtonProc,0);
  InfoButton = obj = fl_add_button(FL_PUSH_BUTTON,20.0,70.0,210.0,40.0,"Info");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,InfoButtonProc,0);
  EnumerateButton = obj = fl_add_button(FL_PUSH_BUTTON,20.0,190.0,210.0,40.0,"Enumerate");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,EnumerateButtonProc,0);
  TileFormButton = obj = fl_add_button(FL_PUSH_BUTTON,20.0,150.0,210.0,40.0,"Basic Tile");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,TileButtonProc,0);
  HelpButton = obj = fl_add_button(FL_PUSH_BUTTON,20.0,110.0,210.0,40.0,"Help");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,HelpButtonProc,0);
  QuitButton = obj = fl_add_button(FL_PUSH_BUTTON,20.0,30.0,210.0,40.0,"Quit");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,QuitButtonProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,40.0,370.0,170.0,30.0,"A 3-Manifold Viewer");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  LoadButton = obj = fl_add_button(FL_PUSH_BUTTON,20.0,310.0,210.0,40.0,"Load");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,LoadButtonProc,0);
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *DisplayForm;

FL_OBJECT
        *CullzButton,
        *CentercamButton,
        *DirdomButton,
        *ShowcamButton,
        *DisplayOKButton,
        *Attenuation2Slider,
        *Attenuation3Slider,
        *SoftshadeButton,
        *Attenuation1Slider,
        *DrawGeomButton;

void create_form_DisplayForm()
{
  FL_OBJECT *obj;
  DisplayForm = fl_bgn_form(FL_NO_BOX,320.0,380.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,320.0,380.0,"");
  CullzButton = obj = fl_add_button(FL_PUSH_BUTTON,150.0,170.0,100.0,30.0,"cull");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,DisplayProc,DG_ZCULL);
  CentercamButton = obj = fl_add_button(FL_PUSH_BUTTON,50.0,230.0,100.0,30.0,"centercam");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,DisplayProc,DG_CENTERCAM);
  DirdomButton = obj = fl_add_button(FL_PUSH_BUTTON,150.0,230.0,110.0,30.0,"draw dirdom");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,DisplayProc,DG_DRAWDIRDOM);
  ShowcamButton = obj = fl_add_button(FL_PUSH_BUTTON,50.0,200.0,100.0,30.0,"showcam");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,DisplayProc,DG_DRAWCAM);
  obj = fl_add_text(FL_NORMAL_TEXT,90.0,260.0,120.0,40.0,"Toggles");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_lstyle(obj,FL_ENGRAVED_STYLE);
  DisplayOKButton = obj = fl_add_button(FL_NORMAL_BUTTON,250.0,310.0,50.0,45.0,"OK");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,DisplayOKButtonProc,0);
  Attenuation2Slider = obj = fl_add_valslider(FL_HOR_SLIDER,60.0,70.0,230.0,30.0,"fogfree");
    fl_set_object_align(obj,FL_ALIGN_LEFT);
    fl_set_call_back(obj,Attenuation2SliderProc,0);
  obj = fl_add_box(FL_FRAME_BOX,10.0,310.0,210.0,50.0,"Display Settings");
    fl_set_object_color(obj,7,47);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  Attenuation3Slider = obj = fl_add_valslider(FL_HOR_SLIDER,60.0,20.0,230.0,30.0,"fog");
    fl_set_object_align(obj,FL_ALIGN_LEFT);
    fl_set_call_back(obj,Attenuation3SliderProc,0);
  SoftshadeButton = obj = fl_add_button(FL_PUSH_BUTTON,20.0,170.0,130.0,30.0,"software shading");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,SoftshadeProc,0);
  Attenuation1Slider = obj = fl_add_valslider(FL_HOR_SLIDER,60.0,120.0,230.0,30.0,"atten1");
    fl_set_object_align(obj,FL_ALIGN_LEFT);
    fl_set_call_back(obj,Attenuation1SliderProc,0);
  DrawGeomButton = obj = fl_add_button(FL_PUSH_BUTTON,150.0,200.0,110.0,30.0,"draw geom");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,DisplayProc,DG_DRAWGEOM);
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *EnumForm;

FL_OBJECT
        *WorddepthCounter,
        *RadiusSlider,
        *EnumOKButton,
        *DrawRadiusSlider;

void create_form_EnumForm()
{
  FL_OBJECT *obj;
  EnumForm = fl_bgn_form(FL_NO_BOX,400.0,250.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,400.0,250.0,"");
  WorddepthCounter = obj = fl_add_counter(FL_NORMAL_COUNTER,160.0,130.0,130.0,40.0,"worddepth");
    fl_set_object_align(obj,FL_ALIGN_LEFT);
    fl_set_call_back(obj,WorddepthProc,0);
  RadiusSlider = obj = fl_add_valslider(FL_HOR_SLIDER,130.0,80.0,240.0,30.0,"tesselation radius");
    fl_set_object_align(obj,FL_ALIGN_LEFT);
    fl_set_call_back(obj,RadiusProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,30.0,190.0,220.0,40.0,"Enumerate Group");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_color(obj,7,47);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_lstyle(obj,FL_ENGRAVED_STYLE);
  EnumOKButton = obj = fl_add_button(FL_NORMAL_BUTTON,330.0,180.0,50.0,45.0,"OK");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,EnumOKButtonProc,0);
  DrawRadiusSlider = obj = fl_add_valslider(FL_HOR_SLIDER,130.0,30.0,240.0,30.0,"draw radius");
    fl_set_object_align(obj,FL_ALIGN_LEFT);
    fl_set_call_back(obj,DrawRadiusProc,0);
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *TileForm;

FL_OBJECT
        *DDZDial,
        *DDXYPositioner,
        *DDResetButton,
        *DDScaleSlider,
        *TileOKButton,
        *DirichletDomainButton,
        *UsergeometryButton;

void create_form_TileForm()
{
  FL_OBJECT *obj;
  TileForm = fl_bgn_form(FL_NO_BOX,400.0,320.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,400.0,320.0,"");
  obj = fl_add_box(FL_FLAT_BOX,10.0,10.0,380.0,170.0,"");
  DDZDial = obj = fl_add_dial(FL_LINE_DIAL,310.0,90.0,70.0,70.0,"z");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_align(obj,FL_ALIGN_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,DDZProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,30.0,140.0,100.0,20.0,"Center Point");
    fl_set_object_lcol(obj,137);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,40.0,120.0,90.0,20.0,"Chooser");
    fl_set_object_lcol(obj,137);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  DDXYPositioner = obj = fl_add_positioner(FL_NORMAL_POSITIONER,190.0,90.0,80.0,70.0,"(x,y)");
    fl_set_object_align(obj,FL_ALIGN_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,DDXYProc,0);
  DDResetButton = obj = fl_add_button(FL_NORMAL_BUTTON,40.0,90.0,90.0,30.0,"Reset");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,DDResetProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,20.0,260.0,130.0,40.0,"Basic Tile");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_color(obj,7,47);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_lstyle(obj,FL_ENGRAVED_STYLE);
  DDScaleSlider = obj = fl_add_valslider(FL_HOR_SLIDER,120.0,30.0,240.0,30.0,"scale factor");
    fl_set_object_align(obj,FL_ALIGN_LEFT);
    fl_set_call_back(obj,DDScaleProc,0);
  TileOKButton = obj = fl_add_button(FL_NORMAL_BUTTON,330.0,260.0,50.0,45.0,"OK");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,TileOKButtonProc,0);
  obj = fl_add_box(FL_FLAT_BOX,20.0,190.0,360.0,50.0,"");
  obj = fl_add_text(FL_NORMAL_TEXT,30.0,200.0,60.0,30.0,"MODE:");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  DirichletDomainButton = obj = fl_add_button(FL_NORMAL_BUTTON,110.0,200.0,120.0,30.0,"Dirichlet domain");
    fl_set_call_back(obj,TileModeProc,DIRDOM_MODE);
  UsergeometryButton = obj = fl_add_button(FL_NORMAL_BUTTON,260.0,200.0,110.0,30.0,"User geometry");
    fl_set_call_back(obj,TileModeProc,USER_GEOM);
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
  InfoForm = fl_bgn_form(FL_NO_BOX,330.0,310.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,330.0,310.0,"");
  InfoFormLabel = obj = fl_add_text(FL_NORMAL_TEXT,80.0,240.0,150.0,40.0,"Maniview");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  obj = fl_add_text(FL_NORMAL_TEXT,11.0,214.0,300.0,30.0,"by");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  obj = fl_add_text(FL_NORMAL_TEXT,11.0,184.0,300.0,30.0,"Charlie Gunn");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  obj = fl_add_text(FL_NORMAL_TEXT,11.0,154.0,300.0,30.0,"The Geometry Center");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  InfoOKButton = obj = fl_add_button(FL_NORMAL_BUTTON,260.0,200.0,50.0,45.0,"OK");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,InfoOKButtonProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,11.0,108.0,300.0,30.0,"Maniview and Geomview are ");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  obj = fl_add_text(FL_NORMAL_TEXT,10.0,90.0,300.0,30.0,"available from");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  obj = fl_add_text(FL_NORMAL_TEXT,12.0,72.0,300.0,30.0,"www.geomview.org");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  obj = fl_add_text(FL_NORMAL_TEXT,12.0,10.0,300.0,30.0,"For usage instructions hit the ``Help'' button.");
    fl_set_object_align(obj,FL_ALIGN_CENTER);
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *LoadForm;

FL_OBJECT
        *LoadInput,
        *LoadOKButton,
        *LoadCancelButton,
        *loadtypegroup,
        *LoadGeomButton,
        *LoadCameraGeomButton,
        *LoadGroupButton,
        *LoadShowBrowser;

void create_form_LoadForm()
{
  FL_OBJECT *obj;
  LoadForm = fl_bgn_form(FL_NO_BOX,470.0,200.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,470.0,200.0,"");
  LoadInput = obj = fl_add_input(FL_NORMAL_INPUT,10.0,10.0,440.0,40.0,"");
    fl_set_object_color(obj,9,9);
  LoadOKButton = obj = fl_add_button(FL_RETURN_BUTTON,360.0,60.0,95.0,35.0,"OK");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,LoadOKButtonProc,0);
  LoadCancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,250.0,60.0,100.0,35.0,"Cancel");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,LoadCancelButtonProc,0);
  obj = fl_add_box(FL_FRAME_BOX,205.0,150.0,160.0,40.0,"Load Panel");
    fl_set_object_color(obj,7,47);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_box(FL_FRAME_BOX,30.0,150.0,160.0,30.0,"LOAD TYPE:");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  loadtypegroup = fl_bgn_group();
  LoadGeomButton = obj = fl_add_button(FL_RADIO_BUTTON,30.0,90.0,160.0,30.0,"Load Geom");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,LoadProc,LOAD_GEOM);
  LoadCameraGeomButton = obj = fl_add_button(FL_RADIO_BUTTON,30.0,60.0,160.0,30.0,"Load Camera Geom");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,LoadProc,LOAD_CAMGEOM);
  LoadGroupButton = obj = fl_add_button(FL_RADIO_BUTTON,30.0,120.0,160.0,30.0,"Load Group");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,LoadProc,LOAD_GROUP);
  fl_end_group();
  LoadShowBrowser = obj = fl_add_button(FL_NORMAL_BUTTON,250.0,105.0,205.0,35.0,"Show Files");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,LoadShowBrowserProc,0);
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
  HelpForm = fl_bgn_form(FL_NO_BOX,470.0,350.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,470.0,350.0,"");
  obj = fl_add_text(FL_NORMAL_TEXT,20.0,290.0,200.0,40.0,"Maniview Help");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_color(obj,7,47);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  HelpBrowser = obj = fl_add_browser(FL_NORMAL_BROWSER,10.0,10.0,450.0,270.0,"");
  HelpOKButton = obj = fl_add_button(FL_NORMAL_BUTTON,410.0,290.0,50.0,40.0,"OK");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,HelpOKButtonProc,0);
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *SaveForm;

FL_OBJECT
        *SaveInput,
        *SaveOKButton,
        *SaveCancelButton,
        *savetypegroup,
        *SaveGeomButton,
        *SaveGroupButton;

void create_form_SaveForm()
{
  FL_OBJECT *obj;
  SaveForm = fl_bgn_form(FL_NO_BOX,490.0,180.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,490.0,180.0,"");
  SaveInput = obj = fl_add_input(FL_NORMAL_INPUT,30.0,20.0,440.0,40.0,"");
    fl_set_object_color(obj,9,9);
  SaveOKButton = obj = fl_add_button(FL_RETURN_BUTTON,360.0,70.0,95.0,35.0,"OK");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,SaveOKButtonProc,0);
  SaveCancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,240.0,70.0,100.0,35.0,"Cancel");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_call_back(obj,SaveCancelButtonProc,0);
  obj = fl_add_box(FL_FRAME_BOX,270.0,125.0,150.0,40.0,"Save Panel");
    fl_set_object_color(obj,7,47);
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_box(FL_FRAME_BOX,40.0,130.0,110.0,30.0,"Options:");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  savetypegroup = fl_bgn_group();
  SaveGeomButton = obj = fl_add_button(FL_RADIO_BUTTON,40.0,100.0,110.0,30.0,"Save Geom");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,SaveGeomButtonProc,0);
  SaveGroupButton = obj = fl_add_button(FL_RADIO_BUTTON,40.0,70.0,110.0,30.0,"Save Matrices");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,SaveGroupButtonProc,0);
  fl_end_group();
  fl_end_form();
}

/*---------------------------------------*/

void create_the_forms()
{
  create_form_MainForm();
  create_form_DisplayForm();
  create_form_EnumForm();
  create_form_TileForm();
  create_form_InfoForm();
  create_form_LoadForm();
  create_form_HelpForm();
  create_form_SaveForm();
}

