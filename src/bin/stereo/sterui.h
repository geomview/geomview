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

extern void StereoProc(FL_OBJECT *, long);
extern void SwapProc(FL_OBJECT *, long);
extern void QuitProc(FL_OBJECT *, long);
extern void FocalProc(FL_OBJECT *, long);
extern void HelpProc(FL_OBJECT *, long);
extern void ConvProc(FL_OBJECT *, long);
extern void MoreProc(FL_OBJECT *, long);

extern void DoneProc(FL_OBJECT *, long);

extern void OcularSepProc(FL_OBJECT *, long);
extern void ScreenWidthProc(FL_OBJECT *, long);
extern void DoneProc(FL_OBJECT *, long);
extern void CamNameProc(FL_OBJECT *, long);



/**** Forms and Objects ****/

extern FL_FORM *stereo;

extern FL_OBJECT
        *StereoBrowser,
        *SwapButton,
        *QuitButton,
        *BestViewText,
        *FocalButton,
        *HelpButton,
        *ConvSlider,
        *MoreButton;

extern FL_FORM *Help;

extern FL_OBJECT
        *HelpBrowser,
        *DoneButton;

extern FL_FORM *More;

extern FL_OBJECT
        *OcularInput,
        *ScreenWidthInput,
        *DoneButton,
        *FixedCamButton,
        *CamNameInput;



/**** Creation Routine ****/

extern void create_the_forms();
