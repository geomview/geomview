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

extern void ShowButtonProc(FL_OBJECT *, long);
extern void DoneButtonProc(FL_OBJECT *, long);
extern void CancelButtonProc(FL_OBJECT *, long);
extern void QuitButtonProc(FL_OBJECT *, long);




/**** Forms and Objects ****/

extern FL_FORM *MainForm;

extern FL_OBJECT
        *ShowButton,
        *DoneButton,
        *CancelButton,
        *Instruc1,
        *Instruc2,
        *Instruc3,
        *TargetInput,
        *Instruc4;

extern FL_FORM *ErrorForm;

extern FL_OBJECT
        *Error1,
        *Error2,
        *Error3,
        *OkButton;



/**** Creation Routine ****/

extern void create_the_forms();
