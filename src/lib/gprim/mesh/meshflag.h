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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef MESHFLAGDEFS
#define MESHFLAGDEFS

#include "mg.h"
#include "geomclass.h"

#define MESH_C		VERT_C
#define MESH_N		VERT_N
#define	MESH_4D		VERT_4D
#define MESH_U		VERT_ST
#define MESH_UWRAP	GEOMFLAG(0x001)
#define MESH_VWRAP	GEOMFLAG(0x002)
#define MESH_NQ		GEOMFLAG(0x020)
#define MESH_EVERT	GEOMFLAG(0x040)
#define	MESH_BINARY	GEOMFLAG(0x080)
#define	MESH_Z		GEOMFLAG(0x100)

#define MESH_MGWRAP(mfl) ((mfl) >> GEOMFL_SHIFT)

#endif /*MESHFLAGDEFS*/
