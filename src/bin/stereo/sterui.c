/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 *
 * This file is part of Geomview.
 * 
 * Geomview is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * Geomview is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with Geomview; see the file COPYING.  If not, write
 * to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 * USA, or visit http://www.gnu.org.
 */

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";

/* Form definition file generated with fdesign. */

#include "forms.h"
#include "sterui.h"

FL_FORM *stereo;

FL_OBJECT
        *StereoBrowser,
        *SwapButton,
        *QuitButton,
        *BestViewText,
        *FocalButton,
        *HelpButton,
        *ConvSlider,
        *MoreButton;

void create_form_stereo()
{
  FL_OBJECT *obj;
  stereo = fl_bgn_form(FL_NO_BOX,223.0,199.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,223.0,199.0,"");
    fl_set_object_color(obj,9,47);
    fl_set_object_align(obj,FL_ALIGN_TOP);
  StereoBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,9.0,112.0,111.0,78.0,"");
    fl_set_call_back(obj,StereoProc,0);
  SwapButton = obj = fl_add_lightbutton(FL_PUSH_BUTTON,123.0,139.0,93.0,24.0,"Eye Swap");
    fl_set_call_back(obj,SwapProc,0);
  obj = fl_add_text(FL_NORMAL_TEXT,5.0,14.0,170.0,26.0,"Stereo View 1.0");
    fl_set_object_color(obj,9,0);
    fl_set_object_align(obj,FL_ALIGN_CENTER);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  QuitButton = obj = fl_add_button(FL_NORMAL_BUTTON,182.0,3.0,38.0,24.0,"Quit");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,QuitProc,0);
  BestViewText = obj = fl_add_box(FL_FLAT_BOX,10.0,69.0,200.0,40.0,"");
    fl_set_object_color(obj,9,47);
  FocalButton = obj = fl_add_button(FL_NORMAL_BUTTON,123.0,112.0,93.0,24.0,"Focal Plane");
    fl_set_call_back(obj,FocalProc,0);
  HelpButton = obj = fl_add_button(FL_NORMAL_BUTTON,176.0,168.0,40.0,23.0,"Help");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,HelpProc,1);
  ConvSlider = obj = fl_add_valslider(FL_HOR_NICE_SLIDER,8.0,50.0,198.0,21.0,"Convergence angle");
    fl_set_object_color(obj,9,11);
    fl_set_object_align(obj,FL_ALIGN_TOP);
    fl_set_call_back(obj,ConvProc,0);
  MoreButton = obj = fl_add_button(FL_NORMAL_BUTTON,124.0,167.0,49.0,24.0,"More..");
    fl_set_call_back(obj,MoreProc,1);
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *Help;

FL_OBJECT
        *HelpBrowser,
        *DoneButton;

void create_form_Help()
{
  FL_OBJECT *obj;
  Help = fl_bgn_form(FL_NO_BOX,475.0,450.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,475.0,450.0,"");
  HelpBrowser = obj = fl_add_browser(FL_NORMAL_BROWSER,10.0,30.0,460.0,410.0,"");
    fl_set_object_lsize(obj,FL_SMALL_FONT);
    fl_set_object_lstyle(obj,FL_FIXED_STYLE);
  DoneButton = obj = fl_add_button(FL_NORMAL_BUTTON,425.0,6.0,45.0,24.0,"Done");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,DoneProc,(long)Help);
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *More;

FL_OBJECT
        *OcularInput,
        *ScreenWidthInput,
        *DoneButton,
        *FixedCamButton,
        *CamNameInput;

void create_form_More()
{
  FL_OBJECT *obj;
  More = fl_bgn_form(FL_NO_BOX,255.0,140.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,255.0,140.0,"");
    fl_set_object_color(obj,9,47);
  OcularInput = obj = fl_add_input(FL_NORMAL_INPUT,10.0,102.0,110.0,26.0,"Ocular Separation");
    fl_set_object_color(obj,14,6);
    fl_set_object_align(obj,FL_ALIGN_RIGHT);
    fl_set_call_back(obj,OcularSepProc,0);
  ScreenWidthInput = obj = fl_add_input(FL_NORMAL_INPUT,10.0,72.0,110.0,24.0,"Screen Width");
    fl_set_object_color(obj,14,6);
    fl_set_object_align(obj,FL_ALIGN_RIGHT);
    fl_set_call_back(obj,ScreenWidthProc,0);
  DoneButton = obj = fl_add_button(FL_NORMAL_BUTTON,204.0,11.0,45.0,24.0,"Done");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,DoneProc,(long)More);
  obj = fl_add_text(FL_NORMAL_TEXT,139.0,92.0,78.0,16.0,"(in inches)");
    fl_set_object_color(obj,9,0);
  FixedCamButton = obj = fl_add_lightbutton(FL_PUSH_BUTTON,10.0,10.0,120.0,25.0,"Fixed camera");
  CamNameInput = obj = fl_add_input(FL_NORMAL_INPUT,10.0,41.0,110.0,24.0,"Camera Name");
    fl_set_object_color(obj,14,6);
    fl_set_object_align(obj,FL_ALIGN_RIGHT);
    fl_set_call_back(obj,CamNameProc,0);
  fl_end_form();
}

/*---------------------------------------*/

void create_the_forms()
{
  create_form_stereo();
  create_form_Help();
  create_form_More();
}

