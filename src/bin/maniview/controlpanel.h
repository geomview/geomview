/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
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
/* Header file generated with fdesign. */

/**** Callback routines ****/

extern void SaveButtonProc(FL_OBJECT *, long);
extern void DisplayButtonProc(FL_OBJECT *, long);
extern void InfoButtonProc(FL_OBJECT *, long);
extern void EnumerateButtonProc(FL_OBJECT *, long);
extern void TileButtonProc(FL_OBJECT *, long);
extern void HelpButtonProc(FL_OBJECT *, long);
extern void QuitButtonProc(FL_OBJECT *, long);
extern void LoadButtonProc(FL_OBJECT *, long);

extern void DisplayProc(FL_OBJECT *, long);
extern void DisplayProc(FL_OBJECT *, long);
extern void DisplayProc(FL_OBJECT *, long);
extern void DisplayProc(FL_OBJECT *, long);
extern void DisplayOKButtonProc(FL_OBJECT *, long);
extern void Attenuation2SliderProc(FL_OBJECT *, long);
extern void Attenuation3SliderProc(FL_OBJECT *, long);
extern void SoftshadeProc(FL_OBJECT *, long);
extern void Attenuation1SliderProc(FL_OBJECT *, long);
extern void DisplayProc(FL_OBJECT *, long);

extern void WorddepthProc(FL_OBJECT *, long);
extern void RadiusProc(FL_OBJECT *, long);
extern void EnumOKButtonProc(FL_OBJECT *, long);
extern void DrawRadiusProc(FL_OBJECT *, long);

extern void DDZProc(FL_OBJECT *, long);
extern void DDXYProc(FL_OBJECT *, long);
extern void DDResetProc(FL_OBJECT *, long);
extern void DDScaleProc(FL_OBJECT *, long);
extern void TileOKButtonProc(FL_OBJECT *, long);
extern void TileModeProc(FL_OBJECT *, long);
extern void TileModeProc(FL_OBJECT *, long);

extern void InfoOKButtonProc(FL_OBJECT *, long);

extern void LoadOKButtonProc(FL_OBJECT *, long);
extern void LoadCancelButtonProc(FL_OBJECT *, long);
extern void LoadProc(FL_OBJECT *, long);
extern void LoadProc(FL_OBJECT *, long);
extern void LoadProc(FL_OBJECT *, long);
extern void LoadShowBrowserProc(FL_OBJECT *, long);

extern void HelpOKButtonProc(FL_OBJECT *, long);

extern void SaveOKButtonProc(FL_OBJECT *, long);
extern void SaveCancelButtonProc(FL_OBJECT *, long);
extern void SaveGeomButtonProc(FL_OBJECT *, long);
extern void SaveGroupButtonProc(FL_OBJECT *, long);



/**** Forms and Objects ****/

extern FL_FORM *MainForm;

extern FL_OBJECT
        *SaveButton,
        *DisplayButton,
        *InfoButton,
        *EnumerateButton,
        *TileFormButton,
        *HelpButton,
        *QuitButton,
        *LoadButton;

extern FL_FORM *DisplayForm;

extern FL_OBJECT
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

extern FL_FORM *EnumForm;

extern FL_OBJECT
        *WorddepthCounter,
        *RadiusSlider,
        *EnumOKButton,
        *DrawRadiusSlider;

extern FL_FORM *TileForm;

extern FL_OBJECT
        *DDZDial,
        *DDXYPositioner,
        *DDResetButton,
        *DDScaleSlider,
        *TileOKButton,
        *DirichletDomainButton,
        *UsergeometryButton;

extern FL_FORM *InfoForm;

extern FL_OBJECT
        *InfoFormLabel,
        *InfoOKButton;

extern FL_FORM *LoadForm;

extern FL_OBJECT
        *LoadInput,
        *LoadOKButton,
        *LoadCancelButton,
        *loadtypegroup,
        *LoadGeomButton,
        *LoadCameraGeomButton,
        *LoadGroupButton,
        *LoadShowBrowser;

extern FL_FORM *HelpForm;

extern FL_OBJECT
        *HelpBrowser,
        *HelpOKButton;

extern FL_FORM *SaveForm;

extern FL_OBJECT
        *SaveInput,
        *SaveOKButton,
        *SaveCancelButton,
        *savetypegroup,
        *SaveGeomButton,
        *SaveGroupButton;



/**** Creation Routine ****/

extern void create_the_forms();
