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
#include "handle.h"

extern void usepipe(char *pipedir, char *pipename, char *pipetype);
extern void init_geomview(int argc, char **argv);
extern void usage(char *me, char *morestuff);
extern int parse_common_arg(int *acp, char ***avp);
extern char *geomview_system_initfile;
extern vvec vv_load_path;
extern vvec vv_emodule_path;

#define load_path VVEC(vv_load_path, char *)
#define load_path_count VVCOUNT(vv_load_path)
#define emodule_path VVEC(vv_emodule_path, char *)
#define emodule_path_count VVCOUNT(vv_emodule_path)

extern int gv_nwins;
extern int gv_no_opengl;
