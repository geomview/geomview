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

#include <tk.h>
#include <tcl.h>
#include <string.h>

extern int ndview_init(Tcl_Interp *interp);

int do_emodule_init(ClientData clientData, Tcl_Interp *interp,
		    int ac, const char **av)
{
  if (ac != 2) {
    Tcl_SetResult(interp, "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }
#define INIT(initfunc,string) if(!strcmp(av[1],string)) { if(initfunc(interp)==TCL_ERROR) return TCL_ERROR;} else
INIT(ndview_init,"ndview")
#undef INIT
  { /* last else clause */
    Tcl_SetResult(interp, "unknown module init", TCL_STATIC);
    return TCL_ERROR;
  }
  return TCL_OK;
}


int Tcl_AppInit(Tcl_Interp *interp) {
  if (Tcl_Init(interp) == TCL_ERROR)
    return TCL_ERROR;
  if (Tk_Init(interp) == TCL_ERROR)
    return TCL_ERROR;
  Tcl_CreateCommand(interp, "emodule_init", do_emodule_init, 0, 0);
  return TCL_OK;
}

int main(int argc, char **argv) {
  Tk_Main(argc, argv, Tcl_AppInit);
  return 0;
}

