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
/*
 * gvinit.h: generated automatically from file "gvinit"
 *
 *  to modify, edit "gvinit" and type "make gvinit.h"
 */
" (progn\n\
  (merge-baseap {\n\
	lighting {\n\
		replacelights		\n\
		attenmult  1		\n\
		attenmult2  1\n\
		light {\n\
		    color 1 1 1\n\
		    position  0 0 0 1	\n\
		    location camera	\n\
		}\n\
		light {\n\
		    color .5 1 1\n\
		    position  .5 .5 0 1\n\
		    location camera\n\
		}\n\
	}\n\
  })\n\
  (normalization target none)\n\
  (soft-shader focus on)\n\
  )\n\
  (geometry maniview { : dghandle})\n\n"
