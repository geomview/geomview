/* Form definition file generated with fdesign. */

#include "forms.h"
#include "cplxpanels2.h"

FL_FORM *cplxmainpanel;

FL_OBJECT
        *functionbox,
        *messagebox,
        *helpbutton,
        *quitbutton,
        *funclabelbutton,
        *coordtypebutton,
        *meshlabelbutton,
        *rangelabelbutton,
        *x1box,
        *y1box,
        *x0box,
        *y0box;

void create_form_cplxmainpanel()
{
  FL_OBJECT *obj;
  cplxmainpanel = fl_bgn_form(FL_NO_BOX,420.0,290.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,420.0,290.0,"");
  obj = fl_add_text(FL_NORMAL_TEXT,70.0,240.0,320.0,30.0,"Complex Function Viewer");
    fl_set_object_lsize(obj,FL_LARGE_FONT);
    fl_set_object_lstyle(obj,FL_ENGRAVED_STYLE);
  functionbox = obj = fl_add_input(FL_NORMAL_INPUT,100.0,200.0,280.0,30.0,"Function:");
    fl_set_object_color(obj,9,9);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,functioncallback,0);
  messagebox = obj = fl_add_text(FL_NORMAL_TEXT,120.0,20.0,280.0,30.0,"This is a text box.");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,47,1);
  helpbutton = obj = fl_add_button(FL_NORMAL_BUTTON,350.0,60.0,50.0,30.0,"Help");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lcol(obj,1);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,helpcallback,0);
  quitbutton = obj = fl_add_button(FL_NORMAL_BUTTON,20.0,240.0,30.0,30.0,"quit");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lcol(obj,1);
    fl_set_object_lsize(obj,FL_SMALL_FONT);
    fl_set_call_back(obj,quitcallback,0);
  funclabelbutton = obj = fl_add_button(FL_NORMAL_BUTTON,270.0,60.0,70.0,30.0,"Sliders");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lcol(obj,1);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,sliderscallback,0);
  coordtypebutton = obj = fl_add_button(FL_NORMAL_BUTTON,180.0,60.0,80.0,30.0,"Coordtype");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lcol(obj,1);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,coordtypecallback,0);
  meshlabelbutton = obj = fl_add_button(FL_NORMAL_BUTTON,90.0,60.0,80.0,30.0,"Meshsize");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lcol(obj,1);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,meshlabelcallback,0);
  rangelabelbutton = obj = fl_add_button(FL_NORMAL_BUTTON,20.0,60.0,60.0,30.0,"Range");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lcol(obj,1);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,rangelabelcallback,0);
  obj = fl_add_text(FL_NORMAL_TEXT,20.0,160.0,100.0,30.0,"Domain:");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,20.0,20.0,90.0,30.0,"Messages:");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  x1box = obj = fl_add_counter(FL_NORMAL_COUNTER,300.0,130.0,100.0,30.0,"max(Re(z)):");
    fl_set_object_color(obj,47,47);
    fl_set_object_align(obj,FL_ALIGN_LEFT);
    fl_set_call_back(obj,domaincallback,1);
  y1box = obj = fl_add_counter(FL_NORMAL_COUNTER,190.0,160.0,110.0,30.0,"max(Im(z)):");
    fl_set_object_color(obj,47,47);
    fl_set_object_align(obj,FL_ALIGN_LEFT);
    fl_set_call_back(obj,domaincallback,2);
  x0box = obj = fl_add_counter(FL_NORMAL_COUNTER,90.0,130.0,100.0,30.0,"min(Re(z)):");
    fl_set_object_color(obj,47,47);
    fl_set_object_align(obj,FL_ALIGN_LEFT);
    fl_set_call_back(obj,domaincallback,3);
  y0box = obj = fl_add_counter(FL_NORMAL_COUNTER,190.0,100.0,110.0,30.0,"min(Im(z)):");
    fl_set_object_color(obj,47,47);
    fl_set_object_align(obj,FL_ALIGN_LEFT);
    fl_set_call_back(obj,domaincallback,4);
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *cplxhelppanel;

FL_OBJECT
        *cplxhelpbrowser,
        *quitcplxhelpbutton;

void create_form_cplxhelppanel()
{
  FL_OBJECT *obj;
  cplxhelppanel = fl_bgn_form(FL_NO_BOX,480.0,240.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,480.0,240.0,"");
  cplxhelpbrowser = obj = fl_add_browser(FL_NORMAL_BROWSER,20.0,20.0,440.0,180.0,"");
    fl_set_object_color(obj,47,15);
  quitcplxhelpbutton = obj = fl_add_button(FL_NORMAL_BUTTON,420.0,210.0,40.0,20.0,"hide");
    fl_set_object_lsize(obj,FL_SMALL_FONT);
    fl_set_call_back(obj,quitcplxhelpcallback,0);
  obj = fl_add_text(FL_NORMAL_TEXT,20.0,210.0,240.0,20.0,"Cplxview Help");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *sliderspanel;

FL_OBJECT
        *abox,
        *bbox,
        *slidershidebox;

void create_form_sliderspanel()
{
  FL_OBJECT *obj;
  sliderspanel = fl_bgn_form(FL_NO_BOX,320.0,90.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,320.0,90.0,"");
  abox = obj = fl_add_valslider(FL_HOR_SLIDER,30.0,30.0,280.0,20.0,"a:");
    fl_set_object_align(obj,FL_ALIGN_LEFT);
    fl_set_call_back(obj,slidercallback,1);
  bbox = obj = fl_add_valslider(FL_HOR_SLIDER,30.0,10.0,280.0,20.0,"b:");
    fl_set_object_align(obj,FL_ALIGN_LEFT);
    fl_set_call_back(obj,slidercallback,2);
  obj = fl_add_text(FL_NORMAL_TEXT,10.0,60.0,230.0,20.0,"Optional function parameters:");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  slidershidebox = obj = fl_add_button(FL_NORMAL_BUTTON,260.0,60.0,40.0,20.0,"hide");
    fl_set_object_lsize(obj,FL_SMALL_FONT);
    fl_set_call_back(obj,slidershidecallback,0);
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *coordtypepanel;

FL_OBJECT
        *rectcoordbox,
        *polarcoordbox,
        *usercoordbox,
        *coordtypehidebox;

void create_form_coordtypepanel()
{
  FL_OBJECT *obj;
  coordtypepanel = fl_bgn_form(FL_NO_BOX,330.0,150.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,330.0,150.0,"");
  rectcoordbox = obj = fl_add_button(FL_PUSH_BUTTON,20.0,80.0,190.0,30.0,"rectangular coordinates:");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,coordcallback,1);
  polarcoordbox = obj = fl_add_button(FL_PUSH_BUTTON,20.0,50.0,190.0,30.0,"polar coordinates:");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,coordcallback,2);
  usercoordbox = obj = fl_add_button(FL_PUSH_BUTTON,20.0,20.0,190.0,30.0,"user defined coordinates:");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,coordcallback,3);
  obj = fl_add_text(FL_NORMAL_TEXT,20.0,120.0,180.0,20.0,"domain specified by:");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  coordtypehidebox = obj = fl_add_button(FL_NORMAL_BUTTON,270.0,120.0,40.0,20.0,"hide");
    fl_set_object_lsize(obj,FL_SMALL_FONT);
    fl_set_call_back(obj,coordtypehidecallback,0);
  obj = fl_add_text(FL_NORMAL_TEXT,220.0,80.0,90.0,30.0,"z = x + i*y");
  obj = fl_add_text(FL_NORMAL_TEXT,220.0,50.0,100.0,30.0,"z = r*exp(i*t)");
  obj = fl_add_text(FL_NORMAL_TEXT,220.0,20.0,90.0,30.0,"z = s + i*t");
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *usercoordpanel;

FL_OBJECT
        *suvbox,
        *tuvbox,
        *usercoordhidebox;

void create_form_usercoordpanel()
{
  FL_OBJECT *obj;
  usercoordpanel = fl_bgn_form(FL_NO_BOX,380.0,110.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,380.0,110.0,"");
  suvbox = obj = fl_add_input(FL_NORMAL_INPUT,80.0,40.0,280.0,30.0,"s(u,v) =");
    fl_set_object_color(obj,9,9);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,usercoordcallback,1);
  tuvbox = obj = fl_add_input(FL_NORMAL_INPUT,80.0,10.0,280.0,30.0,"t(u,v) =");
    fl_set_object_color(obj,9,9);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,usercoordcallback,2);
  obj = fl_add_text(FL_NORMAL_TEXT,10.0,70.0,220.0,30.0,"User defined coordinates:");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  usercoordhidebox = obj = fl_add_button(FL_NORMAL_BUTTON,320.0,80.0,40.0,20.0,"hide");
    fl_set_object_lsize(obj,FL_SMALL_FONT);
    fl_set_call_back(obj,usercoordhidecallback,0);
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *meshlabelpanel;

FL_OBJECT
        *m1box,
        *m2box,
        *meshlabelhidebox;

void create_form_meshlabelpanel()
{
  FL_OBJECT *obj;
  meshlabelpanel = fl_bgn_form(FL_NO_BOX,290.0,40.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,290.0,40.0,"");
  obj = fl_add_text(FL_NORMAL_TEXT,10.0,10.0,90.0,20.0,"Meshsize:");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  m1box = obj = fl_add_input(FL_INT_INPUT,100.0,10.0,50.0,20.0,"");
    fl_set_object_color(obj,8,9);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
    fl_set_call_back(obj,meshcallback,1);
  m2box = obj = fl_add_input(FL_NORMAL_INPUT,170.0,10.0,50.0,20.0,"x");
    fl_set_object_color(obj,8,9);
    fl_set_call_back(obj,meshcallback,2);
  meshlabelhidebox = obj = fl_add_button(FL_NORMAL_BUTTON,240.0,10.0,40.0,20.0,"hide");
    fl_set_object_lsize(obj,FL_SMALL_FONT);
    fl_set_call_back(obj,meshlabelhidecallback,0);
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *rangelabelpanel;

FL_OBJECT
        *realrangebutton,
        *imagrangebutton,
        *d4rangebutton,
        *hiderangelabelbutton;

void create_form_rangelabelpanel()
{
  FL_OBJECT *obj;
  rangelabelpanel = fl_bgn_form(FL_NO_BOX,170.0,80.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,170.0,80.0,"");
  realrangebutton = obj = fl_add_button(FL_PUSH_BUTTON,10.0,10.0,50.0,30.0,"Re(f)");
    fl_set_object_color(obj,47,12);
    fl_set_call_back(obj,rangecallback,1);
  imagrangebutton = obj = fl_add_button(FL_PUSH_BUTTON,60.0,10.0,50.0,30.0,"Im(f)");
    fl_set_object_color(obj,47,12);
    fl_set_call_back(obj,rangecallback,2);
  d4rangebutton = obj = fl_add_button(FL_PUSH_BUTTON,110.0,10.0,50.0,30.0,"4D");
    fl_set_object_color(obj,47,12);
    fl_set_call_back(obj,rangecallback,3);
  hiderangelabelbutton = obj = fl_add_button(FL_NORMAL_BUTTON,120.0,50.0,40.0,20.0,"hide");
    fl_set_object_lsize(obj,FL_SMALL_FONT);
    fl_set_call_back(obj,rangelabelhidecallback,0);
  obj = fl_add_text(FL_NORMAL_TEXT,10.0,50.0,90.0,20.0,"Range:");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fl_end_form();
}

/*---------------------------------------*/

void create_the_forms()
{
  create_form_cplxmainpanel();
  create_form_cplxhelppanel();
  create_form_sliderspanel();
  create_form_coordtypepanel();
  create_form_usercoordpanel();
  create_form_meshlabelpanel();
  create_form_rangelabelpanel();
}

