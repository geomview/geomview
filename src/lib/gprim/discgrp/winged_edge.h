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
#ifndef _winged_edge_
#define _winged_edge_

#ifndef _proj_matrix_
#define _proj_matrix_

typedef double proj_matrix[4][4];
typedef double point[4];

#endif

typedef struct WEvertex {
	double		x[4];	/* coordinates (x[3] == 1) */
	double		dist;	/* distance from slicing plane (used locally) */
	int		ideal;	/* is this an ideal vertex (used locally) */
	struct WEvertex	*next;
} WEvertex;

typedef struct WEedge {
	struct WEvertex	*v0,	/* tail */
			*v1;	/* tip  */
	struct WEedge	*e0L,	/* back left   */
			*e0R,	/* back right  */
			*e1L,	/* front left  */
			*e1R;	/* front right */
	struct WEface	*fL,	/* left face  */
			*fR;	/* right face */
	struct WEedge	*next;
} WEedge;

typedef struct WEface {
	int				order;
	int				fill_tone;
	struct WEedge	*some_edge;
	proj_matrix		group_element;
	struct WEface	*inverse;
	struct WEface	*next;	/* general use */
	struct WEface	*prv;	/* used for clean and dirty lists */
	struct WEface	*nxt;
} WEface;

typedef struct {
	int			num_vertices,
				num_edges,
				num_faces;
	WEvertex	*vertex_list;
	WEedge		*edge_list;
	WEface		*face_list;
	WEface		dirty0,	/* dummy faces to serve as start and	*/
				dirty1,	/* finish of doubly linked lists		*/
				clean0,
				clean1,
				pending0,
				pending1;
} WEpolyhedron;

#endif
