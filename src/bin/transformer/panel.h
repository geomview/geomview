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

extern void ApplyProc(FL_OBJECT *, long);
extern void SetProc(FL_OBJECT *, long);
extern void ReadProc(FL_OBJECT *, long);
extern void DoneProc(FL_OBJECT *, long);
extern void IdentityProc(FL_OBJECT *, long);
extern void TranslateButtonProc(FL_OBJECT *, long);
extern void RotateMode(FL_OBJECT *, long);
extern void TranslateMode(FL_OBJECT *, long);
extern void ScaleMode(FL_OBJECT *, long);
extern void RotateButtonProc(FL_OBJECT *, long);
extern void ScaleButtonProc(FL_OBJECT *, long);
extern void CoordSysProc(FL_OBJECT *, long);
extern void CoordSysProc(FL_OBJECT *, long);
extern void CoordSysProc(FL_OBJECT *, long);




/**** Forms and Objects ****/

extern FL_FORM *TransForm;

extern FL_OBJECT
        *t00,
        *t01,
        *t02,
        *t03,
        *t10,
        *t11,
        *t12,
        *t13,
        *t20,
        *t21,
        *t22,
        *t30,
        *t31,
        *t32,
        *t33,
        *ApplyButton,
        *SetButton,
        *ReadButton,
        *DoneButton,
        *IdentityButton,
        *targetGroup,
        *actGeom,
        *actCam,
        *TranslateGroup,
        *TranslationAxisX,
        *TranslationAxisY,
        *TranslationAxisZ,
        *TranslationMeasure,
        *TranslateButton,
        *ModeGroup,
        *RotateModeButton,
        *TranslateModeButton,
        *ScaleModeButton,
        *RotateGroup,
        *RotationAxisY,
        *RotationAxisZ,
        *RotateButton,
        *RotationMeasure,
        *RotationAxisX,
        *t23,
        *ScaleGroup,
        *ScaleMeasure,
        *ScaleButton,
        *ScaleX,
        *ScaleY,
        *ScaleZ,
        *CoordSysGroup,
        *CameraButton,
        *SelfButton,
        *WorldButton;

extern FL_FORM *fooform;



/**** Creation Routine ****/

extern void create_the_forms();
