/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
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

void create_form_cplxmainpanel(void)
{
  FL_OBJECT *obj;

  if (cplxmainpanel)
     return;

  cplxmainpanel = fl_bgn_form(FL_NO_BOX,420,290);
  obj = fl_add_box(FL_UP_BOX,0,0,420,290,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  obj = fl_add_text(FL_NORMAL_TEXT,100,20,240,30,"Complex Function Viewer");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE+FL_ENGRAVED_STYLE);
  functionbox = obj = fl_add_input(FL_NORMAL_INPUT,80,60,320,30,"Function:");
    fl_set_object_color(obj,FL_INDIANRED,FL_INDIANRED);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,functioncallback,0);
  messagebox = obj = fl_add_text(FL_NORMAL_TEXT,120,240,280,30,"This is a text box.");
    fl_set_object_boxtype(obj,FL_BORDER_BOX);
    fl_set_object_color(obj,FL_COL1,FL_RED);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  helpbutton = obj = fl_add_button(FL_NORMAL_BUTTON,350,200,50,30,"Help");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lcolor(obj,FL_RED);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,helpcallback,0);
  quitbutton = obj = fl_add_button(FL_NORMAL_BUTTON,20,20,50,30,"quit");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lcolor(obj,FL_RED);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,quitcallback,0);
  funclabelbutton = obj = fl_add_button(FL_NORMAL_BUTTON,270,200,70,30,"Sliders");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lcolor(obj,FL_RED);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,sliderscallback,0);
  coordtypebutton = obj = fl_add_button(FL_NORMAL_BUTTON,180,200,80,30,"Coordtype");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lcolor(obj,FL_RED);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,coordtypecallback,0);
  meshlabelbutton = obj = fl_add_button(FL_NORMAL_BUTTON,90,200,80,30,"Meshsize");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lcolor(obj,FL_RED);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,meshlabelcallback,0);
  rangelabelbutton = obj = fl_add_button(FL_NORMAL_BUTTON,20,200,60,30,"Range");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lcolor(obj,FL_RED);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,rangelabelcallback,0);
  obj = fl_add_text(FL_NORMAL_TEXT,20,100,100,30,"Domain:");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,240,90,30,"Messages:");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  x1box = obj = fl_add_counter(FL_NORMAL_COUNTER,300,130,100,30,"max(Re(z)):");
    fl_set_object_color(obj,FL_COL1,FL_BLACK);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_callback(obj,domaincallback,1);
  y1box = obj = fl_add_counter(FL_NORMAL_COUNTER,190,100,110,30,"max(Im(z)):");
    fl_set_object_color(obj,FL_COL1,FL_BLACK);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_callback(obj,domaincallback,2);
  x0box = obj = fl_add_counter(FL_NORMAL_COUNTER,90,130,100,30,"min(Re(z)):");
    fl_set_object_color(obj,FL_COL1,FL_BLACK);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_callback(obj,domaincallback,3);
  y0box = obj = fl_add_counter(FL_NORMAL_COUNTER,190,160,110,30,"min(Im(z)):");
    fl_set_object_color(obj,FL_COL1,FL_BLACK);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_callback(obj,domaincallback,4);
  fl_end_form();

}
/*---------------------------------------*/

FL_FORM *cplxhelppanel;

FL_OBJECT
        *cplxhelpbrowser,
        *quitcplxhelpbutton;

void create_form_cplxhelppanel(void)
{
  FL_OBJECT *obj;

  if (cplxhelppanel)
     return;

  cplxhelppanel = fl_bgn_form(FL_NO_BOX,480,250);
  obj = fl_add_box(FL_UP_BOX,0,0,480,250,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  cplxhelpbrowser = obj = fl_add_browser(FL_NORMAL_BROWSER,20,50,440,180,"");
    fl_set_object_color(obj,FL_COL1,FL_LEFT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  quitcplxhelpbutton = obj = fl_add_button(FL_NORMAL_BUTTON,410,10,50,30,"hide");
    fl_set_object_lcolor(obj,FL_RED);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,quitcplxhelpcallback,0);
  obj = fl_add_text(FL_NORMAL_TEXT,20,10,240,20,"Cplxview Help");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE+FL_ENGRAVED_STYLE);
  fl_end_form();

}
/*---------------------------------------*/

FL_FORM *sliderspanel;

FL_OBJECT
        *abox,
        *bbox,
        *slidershidebox;

void create_form_sliderspanel(void)
{
  FL_OBJECT *obj;

  if (sliderspanel)
     return;

  sliderspanel = fl_bgn_form(FL_NO_BOX,320,100);
  obj = fl_add_box(FL_UP_BOX,0,0,320,100,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  abox = obj = fl_add_valslider(FL_HOR_SLIDER,30,50,280,20,"a:");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_callback(obj,slidercallback,1);
  bbox = obj = fl_add_valslider(FL_HOR_SLIDER,30,70,280,20,"b:");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_callback(obj,slidercallback,2);
  obj = fl_add_text(FL_NORMAL_TEXT,10,10,230,20,"Optional function parameters:");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  slidershidebox = obj = fl_add_button(FL_NORMAL_BUTTON,260,10,50,30,"hide");
    fl_set_object_lcolor(obj,FL_RED);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,slidershidecallback,0);
  fl_end_form();

}
/*---------------------------------------*/

FL_FORM *coordtypepanel;

FL_OBJECT
        *rectcoordbox,
        *polarcoordbox,
        *usercoordbox,
        *coordtypehidebox;

void create_form_coordtypepanel(void)
{
  FL_OBJECT *obj;

  if (coordtypepanel)
     return;

  coordtypepanel = fl_bgn_form(FL_NO_BOX,330,150);
  obj = fl_add_box(FL_UP_BOX,0,0,330,150,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  rectcoordbox = obj = fl_add_button(FL_PUSH_BUTTON,20,40,190,30,"rectangular coordinates:");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,coordcallback,1);
  polarcoordbox = obj = fl_add_button(FL_PUSH_BUTTON,20,70,190,30,"polar coordinates:");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,coordcallback,2);
  usercoordbox = obj = fl_add_button(FL_PUSH_BUTTON,20,100,190,30,"user defined coordinates:");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,coordcallback,3);
  obj = fl_add_text(FL_NORMAL_TEXT,20,10,180,20,"domain specified by:");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  coordtypehidebox = obj = fl_add_button(FL_NORMAL_BUTTON,270,10,50,30,"hide");
    fl_set_object_lcolor(obj,FL_RED);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,coordtypehidecallback,0);
  obj = fl_add_text(FL_NORMAL_TEXT,220,40,90,30,"z = x + i*y");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,220,70,100,30,"z = r*exp(i*t)");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,220,100,90,30,"z = s + i*t");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fl_end_form();

}
/*---------------------------------------*/

FL_FORM *usercoordpanel;

FL_OBJECT
        *suvbox,
        *tuvbox,
        *usercoordhidebox;

void create_form_usercoordpanel(void)
{
  FL_OBJECT *obj;

  if (usercoordpanel)
     return;

  usercoordpanel = fl_bgn_form(FL_NO_BOX,380,120);
  obj = fl_add_box(FL_UP_BOX,0,0,380,120,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  suvbox = obj = fl_add_input(FL_NORMAL_INPUT,80,50,280,30,"s(u,v) =");
    fl_set_object_color(obj,FL_INDIANRED,FL_INDIANRED);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,usercoordcallback,1);
  tuvbox = obj = fl_add_input(FL_NORMAL_INPUT,80,80,280,30,"t(u,v) =");
    fl_set_object_color(obj,FL_INDIANRED,FL_INDIANRED);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,usercoordcallback,2);
  obj = fl_add_text(FL_NORMAL_TEXT,10,10,220,30,"User defined coordinates:");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  usercoordhidebox = obj = fl_add_button(FL_NORMAL_BUTTON,310,10,50,30,"hide");
    fl_set_object_lcolor(obj,FL_RED);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,usercoordhidecallback,0);
  fl_end_form();

}
/*---------------------------------------*/

FL_FORM *meshlabelpanel;

FL_OBJECT
        *m1box,
        *m2box,
        *meshlabelhidebox;

void create_form_meshlabelpanel(void)
{
  FL_OBJECT *obj;

  if (meshlabelpanel)
     return;

  meshlabelpanel = fl_bgn_form(FL_NO_BOX,290,50);
  obj = fl_add_box(FL_UP_BOX,0,0,290,50,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  obj = fl_add_text(FL_NORMAL_TEXT,10,15,90,20,"Meshsize:");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  m1box = obj = fl_add_input(FL_INT_INPUT,100,10,50,30,"");
    fl_set_object_color(obj,FL_BOTTOM_BCOL,FL_INDIANRED);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER);
    fl_set_object_callback(obj,meshcallback,1);
  m2box = obj = fl_add_input(FL_NORMAL_INPUT,170,10,50,30,"x");
    fl_set_object_color(obj,FL_BOTTOM_BCOL,FL_INDIANRED);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,meshcallback,2);
  meshlabelhidebox = obj = fl_add_button(FL_NORMAL_BUTTON,230,10,50,30,"hide");
    fl_set_object_lcolor(obj,FL_RED);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,meshlabelhidecallback,0);
  fl_end_form();

}
/*---------------------------------------*/

FL_FORM *rangelabelpanel;

FL_OBJECT
        *realrangebutton,
        *imagrangebutton,
        *d4rangebutton,
        *hiderangelabelbutton;

void create_form_rangelabelpanel(void)
{
  FL_OBJECT *obj;

  if (rangelabelpanel)
     return;

  rangelabelpanel = fl_bgn_form(FL_NO_BOX,170,90);
  obj = fl_add_box(FL_UP_BOX,0,0,170,90,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  realrangebutton = obj = fl_add_button(FL_PUSH_BUTTON,10,50,50,30,"Re(f)");
    fl_set_object_color(obj,FL_COL1,FL_SLATEBLUE);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,rangecallback,1);
  imagrangebutton = obj = fl_add_button(FL_PUSH_BUTTON,60,50,50,30,"Im(f)");
    fl_set_object_color(obj,FL_COL1,FL_SLATEBLUE);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,rangecallback,2);
  d4rangebutton = obj = fl_add_button(FL_PUSH_BUTTON,110,50,50,30,"4D");
    fl_set_object_color(obj,FL_COL1,FL_SLATEBLUE);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,rangecallback,3);
  hiderangelabelbutton = obj = fl_add_button(FL_NORMAL_BUTTON,110,10,50,30,"hide");
    fl_set_object_lcolor(obj,FL_RED);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,rangelabelhidecallback,0);
  obj = fl_add_text(FL_NORMAL_TEXT,10,10,90,20,"Range:");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fl_end_form();

}
/*---------------------------------------*/

void create_the_forms(void)
{
  create_form_cplxmainpanel();
  create_form_cplxhelppanel();
  create_form_sliderspanel();
  create_form_coordtypepanel();
  create_form_usercoordpanel();
  create_form_meshlabelpanel();
  create_form_rangelabelpanel();
}

