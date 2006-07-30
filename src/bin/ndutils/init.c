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

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Geometry Technologies, Inc.";
#endif

#include	<tcl.h>
#include	<tk.h>
#include 	"init.h"

IOBFILE *infile;

int
ndutils_init(Tcl_Interp *interp)
{
    Tk_Window w = Tk_MainWindow(interp);

/*Used by all three*/
    Tcl_CreateCommand(interp, "ObjExistCheck", ObjExistCheck, (ClientData) w, NULL);

/*Used by slicer*/
    Tcl_CreateCommand(interp, "GetData", GetData, (ClientData) w, NULL);
    Tcl_CreateCommand(interp, "GetDim", GetDim, (ClientData) w, NULL);
    Tcl_CreateCommand(interp, "CreateClipPlane", CreateClipPlane, (ClientData) w, NULL);
    Tcl_CreateCommand(interp, "UpdatePicture", UpdatePicture, (ClientData) w, NULL);
    Tcl_CreateCommand(interp, "GetFocusCam", GetFocusCam, (ClientData) w, NULL);
    Tcl_CreateCommand(interp, "SliceNDice", SliceNDice, (ClientData) w, NULL);

/*used by colormap*/
    Tcl_CreateCommand(interp, "GeomExDir", GeomExDir, (ClientData) w, NULL);
    Tcl_CreateCommand(interp, "setphoto", setphotoCmd, (ClientData) w, NULL);
    Tcl_CreateCommand(interp, "colors", colorsCmd, (ClientData) w, NULL);
    Tcl_CreateCommand(interp, "setmin", minCmd, (ClientData) w, NULL);
    Tcl_CreateCommand(interp, "setmax", maxCmd, (ClientData) w, NULL);

/*used by colormap and 3dsnapshot*/
    Tcl_CreateCommand(interp, "DoProjection", DoProjection, (ClientData) w, NULL);

    infile = iobfileopen(stdin);

    initSlicer();

    return TCL_OK;
}  
