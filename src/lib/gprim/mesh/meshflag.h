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

#define MESH_UWRAP	0x1
#define MESH_VWRAP	0x2
#define MESH_C		0x4
#define MESH_N		0x8
#define MESH_U		0x10
#define MESH_D		0x20
#define MESH_EVERT	0x40
#define	MESH_BINARY	0x80
#define	MESH_Z		0x100
#define	MESH_4D		0x200  /* Special 4-D flag to MeshCreate; not stored */

#endif /*MESHFLAGDEFS*/
