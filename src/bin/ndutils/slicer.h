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
#ifndef TFILE2_H
#define TFILE2_H

#include	<tcl.h>
#include	<tk.h>

void initSlicer(void);

int GetFocusCam(ClientData clientdata, Tcl_Interp * interp,
		int argc, const char **argv);
int GetDim(ClientData clientdata, Tcl_Interp * interp,
	   int argc, const char **argv);
int CreateClipPlane(ClientData clientdata, Tcl_Interp * interp,
		    int argc, const char **argv);
int GetData(ClientData clientdata, Tcl_Interp * interp,
	    int argc, const char **argv);
int UpdatePicture(ClientData clientdata, Tcl_Interp * interp,
		  int argc, const char **argv);
int SliceNDice(ClientData clientdata, Tcl_Interp * interp,
	       int argc, const char **argv);

#endif
