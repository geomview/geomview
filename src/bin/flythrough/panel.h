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

extern void ScaleProc(FL_OBJECT *, long);
extern void QuitProc(FL_OBJECT *, long);
extern void InfoProc(FL_OBJECT *, long);
extern void PathProc(FL_OBJECT *, long);
extern void PathProc(FL_OBJECT *, long);
extern void PathProc(FL_OBJECT *, long);
extern void PathProc(FL_OBJECT *, long);
extern void GoProc(FL_OBJECT *, long);
extern void GoProc(FL_OBJECT *, long);
extern void TilingProc(FL_OBJECT *, long);
extern void TilingProc(FL_OBJECT *, long);
extern void TilingProc(FL_OBJECT *, long);
extern void TilingProc(FL_OBJECT *, long);
extern void SpeedProc(FL_OBJECT *, long);
extern void SpeedProc(FL_OBJECT *, long);
extern void SpeedProc(FL_OBJECT *, long);
extern void SpeedProc(FL_OBJECT *, long);

extern void DoneProc(FL_OBJECT *, long);
extern void DiagProc(FL_OBJECT *, long);
extern void DiagProc(FL_OBJECT *, long);



/**** Forms and Objects ****/

extern FL_FORM *MainForm;

extern FL_OBJECT
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

extern FL_FORM *HelpForm;

extern FL_OBJECT
        *HelpBrowser,
        *Done,
        *DiagramGroup,
        *EucDiag,
        *HypDiag;



/**** Creation Routine ****/

extern void create_the_forms();
