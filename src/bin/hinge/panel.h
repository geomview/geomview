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
/* Header file generated with fdesign. */

/**** Callback routines ****/

extern void QuitProc(FL_OBJECT *, long);
extern void UndoButtonProc(FL_OBJECT *, long);
extern void FileButtonProc(FL_OBJECT *, long);
extern void AngleInputProc(FL_OBJECT *, long);
extern void HelpButtonProc(FL_OBJECT *, long);
extern void InfoButtonProc(FL_OBJECT *, long);
extern void SpaceBrowserProc(FL_OBJECT *, long);
extern void ResetButtonProc(FL_OBJECT *, long);

extern void FileInputProc(FL_OBJECT *, long);
extern void FileOKButtonProc(FL_OBJECT *, long);
extern void FileCancelButtonProc(FL_OBJECT *, long);

extern void InfoOKButtonProc(FL_OBJECT *, long);

extern void HelpOKButtonProc(FL_OBJECT *, long);



/**** Forms and Objects ****/

extern FL_FORM *MainForm;

extern FL_OBJECT
        *QuitButton,
        *UndoButton,
        *FileButton,
        *AngleInput,
        *HelpButton,
        *InfoButton,
        *SpaceBrowser,
        *ResetButton,
        *MainFormLabel;

extern FL_FORM *FileForm;

extern FL_OBJECT
        *FileInput,
        *FileOKButton,
        *FileCancelButton;

extern FL_FORM *InfoForm;

extern FL_OBJECT
        *InfoFormLabel,
        *InfoOKButton;

extern FL_FORM *HelpForm;

extern FL_OBJECT
        *HelpBrowser,
        *HelpOKButton;



/**** Creation Routine ****/

extern void create_the_forms();
