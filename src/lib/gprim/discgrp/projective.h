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
#ifndef _projective_
#define _projective_

#ifndef _proj_matrix_
#define _proj_matrix_

typedef double proj_matrix[4][4];
typedef double point[4];

#endif

#if PROTOTYPES
/* in sl2c_to_proj.c */
extern void		sl2c_to_proj(/*sl2c_matrix, proj_matrix*/);
extern void		proj_to_sl2c(/*proj_matrix, sl2c_matrix*/);
extern void		proj_mult(/*proj_matrix, proj_matrix, proj_matrix*/);
extern void		proj_copy(/*proj_matrix, proj_matrix*/);
extern void		proj_invert(/*proj_matrix, proj_matrix*/);
#endif

#endif
