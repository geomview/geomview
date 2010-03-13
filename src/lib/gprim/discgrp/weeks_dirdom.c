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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

/* Comments on numerical issues:  Dirichlet() uses two constants,
namely VERTEX_EPSILON and MATRIX_EPSILON, to decide when two
real numbers are equal.  Setting the values of these constants
is tricky.  In situations where the polyhedron has very tiny
faces and the initial list of generators is sufficiently
precise (e.g. (40, 1) surgery on the figure eight knot) the
constants should be set fairly small.  In situations where the
polyhedron has no small faces and the generators are very
imprecise (e.g. (1,-1)(-5,1)(-5,1) surgery on the Borromean
rings, which gives the manifold of volume 0.94...) the
constants must be fairly large.   (The more complicated
Dehn fillings give less precise generators not because the
original solution to the gluing equations is less accurate,
but because the original set of generators is far from the
canonical set, and a lot of roundoff error accumulates in
the basepoint finding routine.)  */

/* #define PRECISE_GENERATORS 1 for high precision generators
and tiny faces, or #define PRECISE_GENERATORS 0 for low
precision generators and large faces. */
#define PRECISE_GENERATORS 0

/* Dirichlet() uses an algorithm based on checking that
that corresponding faces do in fact match up.  Specifically,
for each face it checks that (the image of) its matching
face does not extend beyond the perimeter of the given face.
It checks that the matching face does not
extend beyond any Dirichlet plane determined by a group
element (alpha)(beta), where alpha is the group element
associated with the matching face, and beta is the group
element associated with one of the faces bordering the given
face.  When the faces do match up
correctly the program can verify the fact very quickly, because
in the typical case where the edges are of order three the
group element (alpha)(beta) is exactly the group element
associated with a face bordering on the matching
face.  So the program can traverse the original face
counterclockwise while simultaneously traversing the matching
face clockwise and noting that group elements correspond
correctly.  If a group element fails to correspond, then the
program must check that no vertices of the matching face
lie beyond the Dirichlet plane corresponding to (alpha)(beta).
But even this is pretty fast--it just needs to evaluate some
inner products and check that none are positive.  In the event
that some vertices do lie beyond the Dirichlet plane
corresponding to (alpha)(beta), the program knows to add this
plane (and its inverse) to the collection of faces of the
polyhedron.  Thus the program adds only faces that are
actually needed.  This allows it to quickly find Dirichlet
domains whose vertices (and face planes) are too far from the
origin to be found by enumerating all group elements
at that distance (e.g. (40,1) Dehn filling on a knot
complement).

The results of this algorithm can be made rigorous by checking
that the sum of the dihedral angles around each edge add up
to 2pi and that the face pairings are sufficient to generate
the original group, but I haven't written the code to do this
yet.  (In fact, the condition that the face pairings generate
the original group might be satisfied automatically given
that we start with a generating set that includes all
group elements up to some norm.  Perhaps there's also a
way to prove that the edge angles must add up correctly.)
*/

/* Note:  Dirichlet() does not require inverses of generators	*/
/* to be present.  It provides any inverses which may be		*/
/* missing.														*/

#include "options.h"
#include "projective.h"
#include "complex.h"
#include "winged_edge.h"
#include "3d.h"
#include "dgflag.h"
#include <stdio.h>
#include "extern.h"
#include "discgrpP.h"

#define	free32(ptr)	free(ptr)
#define malloc32(size)	malloc(size)
#if PRECISE_GENERATORS
#define VERTEX_EPSILON	(1e9 * HARDWARE_PRECISION)
#define MATRIX_EPSILON	(1e7 * HARDWARE_PRECISION)
#else
#define VERTEX_EPSILON	1e-3
#define MATRIX_EPSILON	1e-5
#endif

#define MIN_DL_FILL_TONE  0
#define MAX_DL_FILL_TONE 64

typedef double vector[4];


/*static void		convert_generators(sl2c_matrix *m, proj_matrix **mm, int foo);*/
static void		make_cube(WEpolyhedron *poly);
/*static void		make_hypercube(WEpolyhedron *poly);*/
static void		initialize_polyhedron(WEpolyhedron *poly, proj_matrix *m, int n);
static int		find_Dirichlet_domain(WEpolyhedron *poly);
static int		check_face(WEpolyhedron *poly, WEface *face);
static int		all_dirty_faces_unmatched(WEpolyhedron *poly);
static int		unsophisticated_search(WEpolyhedron *poly);
static int		add_element(WEpolyhedron *poly, proj_matrix m);
static void		slice_off_cusps(WEpolyhedron *poly);
static int		add_face(WEpolyhedron *poly, proj_matrix m, WEface *face);
static void		cut_edges(WEpolyhedron *poly);
static void		adjust_f_e_ptrs(WEpolyhedron *poly);
static void		cut_faces(WEpolyhedron *poly, WEface *face);
static void		remove_dead_edges(WEpolyhedron *poly);
static void		remove_dead_vertices(WEpolyhedron *poly);

static void		number_faces(WEpolyhedron *poly);
/*static void		read_vertices(WEface *face, double (*foo)[3]);*/
/*static void		print_statistics(WEpolyhedron *poly);*/
static void		print_poly(WEpolyhedron *poly);
static void		print_vef(WEpolyhedron *poly);
/*static void		print_vertex_distances(WEpolyhedron *poly);*/
static void		print_vertices(WEpolyhedron *poly);
/*static void		print_edge_lengths(WEpolyhedron *poly);*/
/*static void		print_face_distances(WEpolyhedron *poly);*/
/*static void		saveOOGL(WEpolyhedron *poly);*/
/*static void		free_polyhedron(WEpolyhedron *poly);*/
static void		roundoff_message(char *message);

static int	matrix_epsilon_message_given,
			vertex_epsilon_message_given;

static point origin;
static int metric, debug;
static WEpolyhedron the_polyhedron;

/* we'd use the geometry library routines here if they were double precision */
extern double		DHPt3Dot3();
extern double		DHPt3Dot();

static WEedge edgeheap[1024];
static int heapcount = 0;

int proj_same_matrix();

void
do_weeks_code(WEpolyhedron **wepolyhedron, point oo, proj_matrix *gen_list,
	      int n, int met, int slice)
{
	int i;
	for (i=0; i<4; ++i)	origin[i] = oo[i];

	heapcount = 0;
	metric = met;
	*wepolyhedron = &the_polyhedron;
/*
	if (metric == DG_SPHERICAL) make_hypercube(*wepolyhedron);
	else make_cube(*wepolyhedron);
*/
	make_cube(*wepolyhedron);
	initialize_polyhedron(*wepolyhedron, gen_list, n);
	if (find_Dirichlet_domain(*wepolyhedron) == 0) 	*wepolyhedron = NULL;
	else {
	    if (debug == 2) print_poly(*wepolyhedron);
	    number_faces(*wepolyhedron);
	    }
	if (slice && (metric == DG_HYPERBOLIC)) slice_off_cusps(*wepolyhedron);
}

#define MAGIC_SCALE .99
static void
slice_off_cusps(poly)
WEpolyhedron *poly;
{
  WEvertex *vptr, *vlist;
  Transform tlate;
  proj_matrix dtlate;
  HPoint3 tlatept;
  int i,j;

  /* make a copy of the vertex list as it stands now */
  vlist = (WEvertex *) malloc32(poly->num_vertices * sizeof(WEvertex));
  vptr = poly->vertex_list;
  i = 0;
  do {
    vlist[i] = *vptr;
    vlist[i].next = &vlist[i+1];
    vptr = vptr->next;
    i++;
  } while (vptr != NULL);
  vlist[i-1].next = NULL;

  /* now cycle through these vertices */
  vptr = vlist;
  do {
    vptr->ideal = ( (DHPt3Dot(vptr->x, vptr->x, metric)) >= -.00005) ? 1 : 0;
    if (vptr->ideal)	{
 	tlatept.x = vptr->x[0] * MAGIC_SCALE;
 	tlatept.y = vptr->x[1] * MAGIC_SCALE;
 	tlatept.z = vptr->x[2] * MAGIC_SCALE;
	tlatept.w = vptr->x[3];
	Tm3HypTranslateOrigin(tlate, &tlatept);
 	for (i=0;i<4;++i)	
 	    for (j=0;j<4;++j)	
		dtlate[j][i] = tlate[i][j];
	add_element(poly, dtlate);
	}
    vptr = vptr->next;
  } while (vptr != NULL);

}

#if 0
static void convert_generators(sl2c_generators, proj_generators_ptr, num_generators)
sl2c_matrix	*sl2c_generators;
proj_matrix	**proj_generators_ptr;
int			num_generators;
{
	int i;

	*proj_generators_ptr = (proj_matrix *) malloc32((MyInt32) num_generators * sizeof(proj_matrix));

	for (i=num_generators; --i>=0; )
		sl2c_to_proj(sl2c_generators[i], (*proj_generators_ptr)[i]);

	return;
}
#endif

#if 0
static void make_hypercube(polyhedron)
WEpolyhedron	*polyhedron;
{
	int			i;
	WEvertex	*initial_vertices[16];
	WEedge		*initial_edges[24];
	WEface		*initial_faces[12];
	static int	edata[24][8] = {
	{0, 4,  8,  4,  9,  6, 1,0},
	{2, 6,  4, 16,  6, 11, 0,8},
	{1, 5,  20,  8,  7,  9, 5,1},
	{3, 11, 10,  5, 22,  17, 7,6},		
	{0, 2,  0,  8,  1, 10, 0,2},
	{1, 3,  8,  20, 10,  3, 2,6},
	{4, 6,  12,  0, 11,  1, 3,0},
	{5, 13,  2,  9,  14, 21, 5, 4},
	{0, 1,  4,  0,  5,  2, 2,1},
	{4, 5,  0,  12,  2,  7, 1,4},
	{2, 3,  16,  4,  3,  5, 7,2},
	{6, 14,  6,  1,  18,  13, 3,8},

	{4, 4+8,  9,  6,  9+12,  6+12, 4,3},
	{2+8, 6+8,  4+12, 10+12,  11, 11+12, 8,10},
	{1+8, 5+8,  5+12,  8+12,  7+12,  7, 9,5},
	{3+8, 7+8, 10+12,  5+12, 11+12,  7+12, 10,9},
	{2, 2+8,  1,  10,  1+12, 10+12, 8,7},
	{1+8, 3+8,  8+12,  2+12, 3,  3+12, 6,9},
	{4+8, 6+8,  9+12,  0+12, 11+12,  11, 11,3},
	{5+8, 7+8,  2+12,  9+12,  3+12, 11+12, 9,11},
	{1, 1+8,  5,  2,  5+12,  2+12, 6,5},
	{4+8, 5+8,  0+12,  6+12,  7,  7+12, 4,11},
	{2+8, 3+8,  1+12,  4+12,  3+12,  3, 10,7},
	{6+8, 7+8,  6+12,  1+12,  7+12,  3+12, 11,10}};
	static int	fdata[12] = {0, 8,4,6,9,2,5,10,1,15,23,19};


	polyhedron->num_vertices	= 16;
	polyhedron->num_edges		= 24;
	polyhedron->num_faces		= 12;

/*
	vt	= (WEvertex *)	malloc32( 16 * sizeof(WEvertex));
	et	= (WEedge *)	malloc32( 24 * sizeof(WEedge));
	ft	= (WEface *)	malloc32( 12 * sizeof(WEface));
*/

	for (i=16; --i>=0; )
		initial_vertices[i]	= (WEvertex *)	malloc32((MyInt32) sizeof(WEvertex));
	for (i=24; --i>=0; )
		initial_edges[i]	= (WEedge *)	malloc32((MyInt32) sizeof(WEedge));
	for (i=12; --i>=0; )
		initial_faces[i]	= (WEface *)	malloc32((MyInt32) sizeof(WEface));

/*
	for (i=0; i<16; ++i)
		initial_vertices[i]	= &vt[i];
	for (i=0; i<24; ++i )
		initial_edges[i]	= &et[i];
	for (i=0; i <12; ++i )
		initial_faces[i]	= &ft[i];
*/

	polyhedron->vertex_list	= initial_vertices[0];
	polyhedron->edge_list	= initial_edges[0];
	polyhedron->face_list	= initial_faces[0];

	polyhedron->dirty0.nxt	= initial_faces[0];
	polyhedron->dirty0.prv	= NULL;	/* should be unnecessary */
	polyhedron->dirty1.nxt	= NULL;	/* should be unnecessary */
	polyhedron->dirty1.prv	= initial_faces[11];

	polyhedron->clean0.nxt	= &polyhedron->clean1;
	polyhedron->clean0.prv	= NULL;	/* should be unnecessary */
	polyhedron->clean1.nxt	= NULL;	/* should be unnecessary */
	polyhedron->clean1.prv	= &polyhedron->clean0;

	for (i=0; i<16; ++i ) {
		initial_vertices[i]->x[0] = (i & 8) ? 1.0 : -1.0;
		initial_vertices[i]->x[1] = (i & 4) ? 1.0 : -1.0;
		initial_vertices[i]->x[2] = (i & 2) ? 1.0 : -1.0;
		initial_vertices[i]->x[3] = (i & 1) ? 1.0 : -1.0;
		initial_vertices[i]->next = initial_vertices[i+1];
	}
	initial_vertices[15]->next = NULL;
	/* delete vertices # 7 and 8 */
	initial_vertices[6]->next = initial_vertices[9];
	polyhedron->num_vertices = 14;

	for (i=0; i< 24; ++i ) {
		initial_edges[i]->v0	= initial_vertices[edata[i][0]];
		initial_edges[i]->v1	= initial_vertices[edata[i][1]];
		initial_edges[i]->e0L	= initial_edges[edata[i][2]];
		initial_edges[i]->e0R	= initial_edges[edata[i][3]];
		initial_edges[i]->e1L	= initial_edges[edata[i][4]];
		initial_edges[i]->e1R	= initial_edges[edata[i][5]];
		initial_edges[i]->fL	= initial_faces[edata[i][6]];
		initial_edges[i]->fR	= initial_faces[edata[i][7]];
		initial_edges[i]->next	= initial_edges[i+1];
	}
	initial_edges[23]->next = NULL;

	for (i=0; i<12; ++i) {
		initial_faces[i]->order			= 4;
		initial_faces[i]->fill_tone		= -2;
		initial_faces[i]->some_edge		= initial_edges[fdata[i]];
		initial_faces[i]->inverse		= NULL;
		initial_faces[i]->next			= initial_faces[i+1];
		initial_faces[i]->prv			= initial_faces[i-1];
		initial_faces[i]->nxt			= initial_faces[i+1];
	}
	initial_faces[11]->next	= NULL;
	initial_faces[0]->prv	= &polyhedron->dirty0;
	initial_faces[11]->nxt	= &polyhedron->dirty1;

	return;
}
#endif

static void make_cube(polyhedron)
WEpolyhedron	*polyhedron;
{
	int			i;
	WEvertex	*initial_vertices[8];
	WEedge		*initial_edges[12];
	WEface		*initial_faces[6];
	static int	edata[12][8] = {
	{0, 4,  8,  4,  9,  6, 2, 4},
	{2, 6,  4, 10,  6, 11, 4, 3},
	{1, 5,  5,  8,  7,  9, 5, 2},
	{3, 7, 10,  5, 11,  7, 3, 5},
	{0, 2,  0,  8,  1, 10, 4, 0},
	{1, 3,  8,  2, 10,  3, 0, 5},
	{4, 6,  9,  0, 11,  1, 1, 4},
	{5, 7,  2,  9,  3, 11, 5, 1},
	{0, 1,  4,  0,  5,  2, 0, 2},
	{4, 5,  0,  6,  2,  7, 2, 1},
	{2, 3,  1,  4,  3,  5, 3, 0},
	{6, 7,  6,  1,  7,  3, 1, 3}};
	static int	fdata[6] = {4, 6, 0, 1, 0, 2};


	polyhedron->num_vertices	= 8;
	polyhedron->num_edges		= 12;
	polyhedron->num_faces		= 6;

	for (i=8; --i>=0; )
		initial_vertices[i]	= (WEvertex *)	malloc32((MyInt32) sizeof(WEvertex));
	for (i=12; --i>=0; )
		initial_edges[i]	= (WEedge *)	malloc32((MyInt32) sizeof(WEedge));
	for (i=6; --i>=0; )
		initial_faces[i]	= (WEface *)	malloc32((MyInt32) sizeof(WEface));

	polyhedron->vertex_list	= initial_vertices[0];
	polyhedron->edge_list	= initial_edges[0];
	polyhedron->face_list	= initial_faces[0];

	polyhedron->dirty0.nxt	= initial_faces[0];
	polyhedron->dirty0.prv	= NULL;	/* should be unnecessary */
	polyhedron->dirty1.nxt	= NULL;	/* should be unnecessary */
	polyhedron->dirty1.prv	= initial_faces[5];

	polyhedron->clean0.nxt	= &polyhedron->clean1;
	polyhedron->clean0.prv	= NULL;	/* should be unnecessary */
	polyhedron->clean1.nxt	= NULL;	/* should be unnecessary */
	polyhedron->clean1.prv	= &polyhedron->clean0;

	for (i=8; --i>=0; ) {
		initial_vertices[i]->x[0] = (i & 4) ? 17.0 : -17.0;
		initial_vertices[i]->x[1] = (i & 2) ? 17.0 : -17.0;
		initial_vertices[i]->x[2] = (i & 1) ? 17.0 : -17.0;
/*
	    if (metric & DG_SPHERICAL)	
		initial_vertices[i]->x[3] = 0.0;
	    else
*/
		initial_vertices[i]->x[3] = 1.0;
		initial_vertices[i]->next = initial_vertices[i+1];
	}
	initial_vertices[7]->next = NULL;

	for (i=12; --i>=0; ) {
		initial_edges[i]->v0	= initial_vertices[edata[i][0]];
		initial_edges[i]->v1	= initial_vertices[edata[i][1]];
		initial_edges[i]->e0L	= initial_edges[edata[i][2]];
		initial_edges[i]->e0R	= initial_edges[edata[i][3]];
		initial_edges[i]->e1L	= initial_edges[edata[i][4]];
		initial_edges[i]->e1R	= initial_edges[edata[i][5]];
		initial_edges[i]->fL	= initial_faces[edata[i][6]];
		initial_edges[i]->fR	= initial_faces[edata[i][7]];
		initial_edges[i]->next	= initial_edges[i+1];
	}
	initial_edges[11]->next = NULL;

	for (i=6; --i>=0; ) {
		initial_faces[i]->order			= 4;
		initial_faces[i]->fill_tone		= -2;
		initial_faces[i]->some_edge		= initial_edges[fdata[i]];
		initial_faces[i]->inverse		= NULL;
		if (i < 5) {
		  initial_faces[i]->next		= initial_faces[i+1];
		}
		if (i > 0) {
		  initial_faces[i]->prv			= initial_faces[i-1];
		}
		initial_faces[i]->nxt			= initial_faces[i+1];
	}
	initial_faces[5]->next	= NULL;
	initial_faces[0]->prv	= &polyhedron->dirty0;
	initial_faces[5]->nxt	= &polyhedron->dirty1;

	return;
}


static void initialize_polyhedron(polyhedron, proj_generators, num_generators)
WEpolyhedron	*polyhedron;
proj_matrix		*proj_generators;
int				num_generators;
{
	int		i;
	WEface	*face;

	for (i=num_generators; --i>=0; )
		{
		add_element(polyhedron, proj_generators[i]);
		if (debug == 2) print_poly(polyhedron);
		}

	/* make sure no faces of the original cube remain */
	for (face=polyhedron->face_list; face; face=face->next)
		if (debug)  if (face->fill_tone == -2) {
			fprintf(stderr, "A face of the original cube is inside the polyhedron\n");
			fprintf(stderr, "determined by the initial generators. This program\n");
			fprintf(stderr, "could be modified to deal with this situation, but\n");
			fprintf(stderr, "it's not ready yet.\n");
			/* the solution would be to tile to some small	*/
			/* radius (or increase the size of the cube!)	*/		
			return;
		}

	return;
}

static int check_face(polyhedron, face)
WEpolyhedron	*polyhedron;
WEface			*face;
{
	/* We want to see whether the image of the matching face	*/
	/* is entirely contained within the given face.  To do this	*/
	/* we'll examine the edges of the given face one at a time	*/
	/* and make sure the image of the matching face doesn't		*/
	/* extend past any edge.  We do this as follows.  Let alpha	*/
	/* be the inverse of the group element associated with the	*/
	/* given face, and beta be the group element associated		*/
	/* with one of its neighboring faces.  We want to make sure	*/
	/* that none of the vertices of the matching face extend	*/
	/* beyond the Dirichlet plane determined by (alpha)(beta).	*/
	/* This will be true for all beta iff the image of the		*/
	/* matching face is contained within the given face.		*/
	/*															*/
	/* We want this routine to run correctly in all cases and	*/
	/* to run quickly in the typical case.  In the typical case	*/
	/* the faces will already match exactly, and the edges will	*/
	/* all be of order three, so (alpha)(beta) will in fact		*/
	/* be the group element associated with one of the faces	*/
	/* bordering on the matching face.  So in this case we		*/
	/* simply traverse the given face in the counterclockwise	*/
	/* while simultaneously traversing the matching face in the	*/
	/* clockwise direction, and check that each (alpha)(beta)	*/
	/* coincides with a gamma bordering the matching face.  If	*/
	/* this fails (as will occur when the faces do not coincide	*/
	/* or the edges are not of order 3) then our fallback plan	*/
	/* is to check that all vertices of the matching face lie	*/
	/* on the correct side of the Dirichlet plane determined by	*/
	/* (alpha)(beta).											*/
	/*															*/
	/* (Note: there may be a better way to structure the		*/
	/* algorithm so that we check edges instead of faces.  For	*/
	/* a typical edge (of order three) we want to check that	*/
	/* the product of the three surrounding group elements is	*/
	/* the identity.  But that will wait for another day.)		*/

	WEface		*match_face;
	WEedge		*edge,
			*edge0,
			*placeholder;
	WEvertex	*vertex;
	double		(*alpha)[4],	/* these are proj_matrices with	*/
			(*beta)[4],		/* no storage attached			*/
			(*gamma)[4];
	vector		planne;
	proj_matrix	alphabeta;
	int             alphabeta_found;
	point		gorigin;

	/* If the given face doesn't have an active matching face	*/
	/* (i.e. its mate lies entirely outside the polyhedron)		*/
	/* then we put it at the end of the dirty list and hope it	*/
	/* goes away.							*/
	/* Dec. 8, 92: don't bother with putting the original faces of  */
	/* the cube on the dirty list. */
	if (face->inverse == NULL) {
	    if (face->fill_tone != -2)	{
		face->nxt = &polyhedron->dirty1;
		face->prv = polyhedron->dirty1.prv;
		polyhedron->dirty1.prv->nxt = face;
		polyhedron->dirty1.prv = face;
		polyhedron->pending0.nxt = &polyhedron->pending1;
		polyhedron->pending1.prv = &polyhedron->pending0;
	   	}

		/* Be sure the dirty queue contains at least one	*/
		/* face with an active inverse face.  If it doesn't	*/
		/* call a routine to try to create some new faces.	*/
		/* If that fails, print a message and exit.			*/
		while (all_dirty_faces_unmatched(polyhedron))
		{
		if (debug) 
		    fprintf(stderr, "searching for more group elements\n");
		    if ( ! unsophisticated_search(polyhedron)) {
			if (debug) fprintf(stderr, "The dirty list in Dirichlet.c contains only unmatched faces.\n");
			return(0);
			}
		}
		return(1);
	}

	match_face = face->inverse;
	alpha = match_face->group_element;

	/* The value of edge0 is maintained from one use of the	*/
	/* inner loop to the next to improve efficiency in the	*/
	/* case where the faces match exactly.					*/
	edge0 = match_face->some_edge;

	/* traverse the face */
	edge = face->some_edge;
	do {
		if (edge->fL == face) {	/* edge points counterclockwise	*/
			beta = edge->fR->group_element;
			edge = edge->e1L;
		}
		else {		/* edge points clockwise	*/
			beta = edge->fL->group_element;
			edge = edge->e0R;
		}

		proj_mult(alpha, beta, alphabeta);

		/* is alphabeta the matrix associated with a	*/
		/* neighbor of match_face?						*/
		alphabeta_found = 0;
		placeholder = edge0;
		do {
			/* edge points counterclockwise */
			if (edge0->fL == match_face) {	
				gamma = edge0->fR->group_element;
				edge0 = edge0->e0L;	/* traverse CLOCKWISE */
			}
			else {		/* edge points clockwise */
				gamma = edge0->fL->group_element;
				edge0 = edge0->e1R;	/* traverse CLOCKWISE */
			}

			if (proj_same_matrix(alphabeta, gamma)) {
				alphabeta_found = 1;
				break;
			}
		}
		while (edge0 != placeholder);

		/* If alphabeta_found == 1 we know the image of the	*/
		/* match_face doesn't extend beyond this edge.  If	*/
		/* alphabeta_found == 0 we need to investigate		*/
		/* further by checking whether the vertices of the	*/
		/* match_face all lie on the correct side of the	*/
		/* Dirichlet plane determined by alphabeta.			*/
		if ( ! alphabeta_found) {
			/* see comments in add_face() */
			/* first get the image of origin */
			matvecmul4(alphabeta, origin, gorigin);
			DHPt3PerpBisect( origin, gorigin, planne,metric);

			do {
				if (edge0->fL == match_face) {	/* CCL */
					vertex = edge0->v0;
					edge0 = edge0->e1L;
				}
				else {							/* CL  */
					vertex = edge0->v1;
					edge0 = edge0->e0R;
				}

				if (DHPt3Dot(planne, vertex->x,metric) > VERTEX_EPSILON) {
					/* add the new group element */
					add_element(polyhedron, alphabeta);

					/* if face has been modified, go home */
					if (polyhedron->pending0.nxt != face)
						return(1);

					/* otherwise if match_face still exists,	*/
					/* keep checking face						*/
					if (face->inverse) {
						edge0 = match_face->some_edge;
						break;
					}

					/* otherwise put face on the dirty list and	*/
					/* go home									*/
					face->nxt = &polyhedron->dirty1;
					face->prv = polyhedron->dirty1.prv;
					polyhedron->dirty1.prv->nxt = face;
					polyhedron->dirty1.prv = face;
					polyhedron->pending0.nxt = &polyhedron->pending1;
					polyhedron->pending1.prv = &polyhedron->pending0;
					return(1);
				}
			}
			while (edge0 != placeholder);
		}
	}
	while (edge != face->some_edge);
	
	return(1);
}

static int find_Dirichlet_domain(polyhedron)
WEpolyhedron	*polyhedron;
{
	/* By the time this routine is called the faces			*/
	/* corresponding to the initial generators should all	*/
	/* be on the dirty list, and the clean list should be	*/
	/* empty.  The faces are examined one at a time to see	*/
	/* whether (the image of) the matching face is entirely	*/
	/* contained within the given face.  If it is, the face	*/
	/* is put on the clean list.  If it's not, a new face	*/
	/* is added to the polyhedron.  New faces, and old		*/
	/* faces which have been cut, are put on the dirty list.*/

	WEface	*face;

	while (polyhedron->dirty0.nxt != &polyhedron->dirty1) {
		/* pull a face off the dirty list */
		face = polyhedron->dirty0.nxt;
		polyhedron->dirty0.nxt = face->nxt;
		face->nxt->prv = &polyhedron->dirty0;

		/* put the face on the pending list so that if	*/
		/* add_face() feels the need to put it on the	*/
		/* dirty list, it will have a list to remove	*/
		/* it from	*/
		face->nxt = &polyhedron->pending1;
		face->prv = &polyhedron->pending0;
		polyhedron->pending0.nxt = face;
		polyhedron->pending1.prv = face;

		/* check the face */
		if (check_face(polyhedron, face) == 0)
		  /* don't worry if original faces hang around */
		  if (face->fill_tone != -2) return 0;

		/* if the face is still on the pending list,	*/
		/* move it to the clean list 			*/
		if (polyhedron->pending0.nxt == face) {
			face->nxt = polyhedron->clean0.nxt;
			face->prv = &polyhedron->clean0;
			polyhedron->clean0.nxt->prv = face;
			polyhedron->clean0.nxt = face;
		}
	}

	return 1;
}




static int all_dirty_faces_unmatched(polyhedron)
WEpolyhedron *polyhedron;
{
	WEface	*face;
	int realdirty = 0;

	/* If at least one face on the dirty list has a	*/
	/* matching face, or the dirty list is empty,	*/
	/* return 0.  Otherwise return 1.				*/
	/* Make a change to this routine to ignore faces of the original cube */

	if (polyhedron->dirty0.next == &polyhedron->dirty1)
		return(0); /* shouldn't occur in practice */

	for (face = polyhedron->dirty0.nxt; face != &polyhedron->dirty1; face = face->nxt)
		{
		if (face->fill_tone != -2)	/* not face of original cube */
		    {
		    if ( face->inverse) return(0);
		    else realdirty = 1;		/* there exist unmatched, real faces */
		    }
		}

	return(realdirty);
}


/* unsophisticated_search() returns 1 if it		*/
/* manages to add a new face, 0 if it doesn't.	*/					

static int unsophisticated_search(polyhedron)
WEpolyhedron *polyhedron;
{
	/* This routine serves as a backup for the more efficient	*/
	/* algorithm which is normally used.  On rare occasions		*/
	/* (e.g. for wh_left) the usual algorithm ends up with only	*/
	/* unmatched faces on the dirty list, so it cannot proceed.	*/
	/* This routine tries looking at all products of two		*/
	/* elements in an effort to scare up a new face.			*/

	WEface		*face0,
				*face1;
	proj_matrix	alpha;

	for (face0=polyhedron->face_list; face0; face0=face0->next)
		for (face1=polyhedron->face_list; face1; face1=face1->next) {
			proj_mult(face0->group_element, face1->group_element, alpha);
			if (add_element(polyhedron, alpha))
				return(1);
		}
	return(0);
}




/* Faces are created in matched pairs (corresponding to a	*/
/* group element and its inverse) by add_element().  If one	*/
/* element of a pair is killed, the inverse pointer of its	*/
/* mate is set to NULL.										*/

/* add_element() returns 1 if it adds a face, 0 if it doesn't */

static int add_element(polyhedron, m0)
WEpolyhedron	*polyhedron;
proj_matrix		m0;
{
	proj_matrix	m1;
	WEface		*new_face0,
				*new_face1;
	int			result0,
				result1,
				order2 = 0;

	/* compute the inverse matrix */
	proj_invert(m0, m1);
    	if (proj_same_matrix(m0, m1))	order2 = 1;

	/* create the new faces */
	new_face0 = (WEface *) malloc32((MyInt32) sizeof(WEface));
	new_face1 = (WEface *) malloc32((MyInt32) sizeof(WEface));

	/* set their inverse pointers */
	new_face0->inverse = new_face1;	
	new_face1->inverse = new_face0;	

	/* attempt to add the faces */
	/* If for any reason one (or both) is unnecessary,	*/
	/* add_face() will free the one and adjust the		*/
	/* inverse pointer of the other.					*/
	if (order2)
	    {
	    new_face0->inverse = new_face0;
	    result0 = add_face(polyhedron, m0, new_face0);
	    return(result0 );
	    }
	else
	    {
	    result0 = add_face(polyhedron, m0, new_face0);
	    result1 = add_face(polyhedron, m1, new_face1);
	    return(result0 || result1);
	    }

}


/* add_face() returns 1 if it adds a face, 0 if it doesn't */

static int add_face(polyhedron, matrix, new_face)
WEpolyhedron	*polyhedron;
proj_matrix		matrix;
WEface			*new_face;
{
	vector		planne, gorigin;
	WEvertex	*vertex;
	int			face_is_needed;

	/* get the normal (in the Minkowski metric)	*/
	/* to the Dirichlet hyperplane				*/

	/* first get the image of origin */
	matvecmul4(matrix,origin, gorigin);
 	DHPt3PerpBisect(origin, gorigin, planne,metric);
	/* Compute the "distance" from each vertex to the	*/
	/* Dirichlet plane.  Count the number of vertices	*/
	/* on or beyond the plane.  Vertices within			*/
	/* VERTEX_EPSILON of the plane are assumed to lie	*/
	/* on the plane.									*/
	face_is_needed = 0;
	for (vertex=polyhedron->vertex_list; vertex; vertex=vertex->next) {
		vertex->dist = DHPt3Dot( planne, vertex->x, metric);
		if (vertex->dist > VERTEX_EPSILON)
			face_is_needed = 1;
		else if (vertex->dist > - VERTEX_EPSILON) {
			if (fabs(vertex->dist) > 1e-2 * VERTEX_EPSILON && ! vertex_epsilon_message_given) {
				if (debug) roundoff_message("VERTEX_EPSILON");
				vertex_epsilon_message_given = 1;
			}
			vertex->dist = 0.0;
		}
	}

	if ( ! face_is_needed) {
		if (new_face->inverse)
			new_face->inverse->inverse = NULL;
		free32(new_face); 
		return(0);
	}

	/* put a vertex in the middle of each cut edge			*/
	cut_edges(polyhedron);

	/* set the new face */
	new_face->order = 0;
	new_face->fill_tone = -1;
	proj_copy(new_face->group_element, matrix);

	/* install the new face in the polyhedron */
	adjust_f_e_ptrs(polyhedron);
	cut_faces(polyhedron, new_face);
	remove_dead_edges(polyhedron);
	remove_dead_vertices(polyhedron);

	/* put the new face on the face lists */
	new_face->next			= polyhedron->face_list;
	polyhedron->face_list	= new_face;
	new_face->nxt				= polyhedron->dirty0.nxt;
	new_face->prv				= &polyhedron->dirty0;
	polyhedron->dirty0.nxt->prv	= new_face;
	polyhedron->dirty0.nxt		= new_face;

	polyhedron->num_faces++;

	return(1);
}


static void cut_edges(polyhedron)
WEpolyhedron	*polyhedron;
{
	int			i;
	double		d0,
				d1,
				t,
				s;
	WEedge		*edge,
				*new_edge,
				*nbr_edge;
	WEvertex	*new_vertex;	

	for (edge=polyhedron->edge_list; edge; edge=edge->next) {
		d0 = edge->v0->dist;
		d1 = edge->v1->dist;
		if ((d0 < 0.0 && d1 > 0.0) || (d0 > 0.0 && d1 < 0.0)) {
			new_vertex	= (WEvertex *)	malloc32((MyInt32) sizeof(WEvertex));
			new_edge	= (WEedge *)	malloc32((MyInt32) sizeof(WEedge));

			t = -d0/(d1 - d0);
			s = 1.0 - t;
			for (i=4; --i>=0; )
				new_vertex->x[i] = s * edge->v0->x[i] + t * edge->v1->x[i];
			new_vertex->dist = 0.0;
			new_vertex->next = polyhedron->vertex_list;
			polyhedron->vertex_list = new_vertex;
			polyhedron->num_vertices++;

			new_edge->v1 = edge->v1;
			new_edge->v0 = new_vertex;
			edge->v1 = new_vertex;

			nbr_edge = edge->e1L;
			new_edge->e1L = nbr_edge;
			if (nbr_edge->e0L == edge)
				nbr_edge->e0L = new_edge;
			else
				nbr_edge->e1R = new_edge;

			nbr_edge = edge->e1R;
			new_edge->e1R = nbr_edge;
			if (nbr_edge->e0R == edge)
				nbr_edge->e0R = new_edge;
			else
				nbr_edge->e1L = new_edge;

			new_edge->e0L = edge;
			new_edge->e0R = edge;
			edge->e1L = new_edge;
			edge->e1R = new_edge;

			new_edge->fL = edge->fL;
			new_edge->fR = edge->fR;

			edge->fL->order++;
			edge->fR->order++;

			new_edge->next = polyhedron->edge_list;
			polyhedron->edge_list = new_edge;
			polyhedron->num_edges++;
		}
	}
	return;
}


static void adjust_f_e_ptrs(polyhedron)
WEpolyhedron *polyhedron;
{
	WEedge	*edge;

	/* make sure each good face "sees" a good edge */

	for (edge=polyhedron->edge_list; edge; edge=edge->next)
		if (edge->v0->dist < 0.0 || edge->v1->dist < 0.0) {
			edge->fL->some_edge = edge;
			edge->fR->some_edge = edge;
		}

	return;
}


/* check each old face:									*/
/* (1) if a face is entirely >= 0, remove it			*/
/*	   (this can be checked immediately, because each	*/
/*	   good face sees a good edge at this point)		*/
/*	   (check that removing dead faces doesn't			*/
/*	   change the group)								*/
/* (2) if a face is entirely <=0, leave it alone		*/
/*	   (but make sure any 0-0 edges "see" the new face	*/
/*		and the new face sees the 0-0 edges)			*/
/*		also set order of new face						*/
/* (3) otherwise bisect the face with a new edge, and	*/
/*	   make sure the new edge "sees" the new new face	*/
/*	   and the old face "sees" a valid edge				*/

static void cut_faces(polyhedron, new_face)
WEpolyhedron	*polyhedron;
WEface			*new_face;
{
	int			zero_count,
				count,
				count1 = 0,
				count3 = 0;
	double		d0, d1;
	WEvertex	*v01, *v23;
	WEedge		*edge,
				*next_edge,
				*new_edge,
				*e0 = NULL, *e1 = NULL, *e2 = NULL, *e3 = NULL;
	WEface		*f_list,
				*face;

	/* to facilitate removing unneeded faces, first move all	*/
	/* the faces onto f_list, then move the good faces back		*/
	/* onto polyhedron->face_list as they are processed			*/
	f_list = polyhedron->face_list;
	polyhedron->face_list = NULL;

	/* we'll count the order of the new_face as we go */
	new_face->order = 0;

	while (f_list) {
		/* pull a face off f_list */
		face = f_list;
		f_list = f_list->next;

		/* Is the face entirely >= 0 ? */
		/* Note: adjust_f_e_ptrs() has been called,	*/
		/* so all good faces see good edges.		*/
		/* Some fL and fR pointers on 0-0 edges may	*/
		/* temporarily be left dangling when face is freed.	*/
		if (face->some_edge->v0->dist >= 0
		 && face->some_edge->v1->dist >= 0) {
			if (face->inverse)
				face->inverse->inverse = NULL;
			face->prv->nxt = face->nxt;
			face->nxt->prv = face->prv;
			free32(face); 
			--polyhedron->num_faces;
			continue;
		}

		/* cut the face if necessary */

		/* counts number of vertices at dist 0 (two consecutive */
		/* vertices at dist 0 are counted as one (or none)	*/
		/* because we don't need to cut such a face)		*/ 
		zero_count = 0;

		/* We'll traverse the face counterwise to find the	*/
		/* edges going in and out of the "zero vertices" (i.e.	*/
		/* the vertices at dist 0)								*/
		/* e0 = negative to zero								*/
		/* e1 = zero to positive								*/
		/* e2 = positive to zero								*/
		/* e3 = zero to negative								*/

		count = 0;	/* for use in finding order of new face */
		edge = face->some_edge;
		do {
			/* which way does the edge point? */
			/* edge points counterclockwise	*/
			if (edge->fL == face) {	
				d0 = edge->v0->dist;
				d1 = edge->v1->dist;
				next_edge = edge->e1L;
			}
			else {		/* edge points clockwise	*/
				d0 = edge->v1->dist;
				d1 = edge->v0->dist;
				next_edge = edge->e0R;
			}

			if (d0 == 0.0) {
				if (d1 == 0.0) {
					if (edge->fL == face)
						edge->fR = new_face;
					else
						edge->fL = new_face;
					new_face->some_edge = edge;
					new_face->order++;
					break;
					}
				zero_count++;
				if (d1 < 0.0) {
					e3 = edge;
					count3 = count;
				}
				else {	/* d1 > 0.0 */
					e1 = edge;
					count1 = count;
				}
			}
			else if (d1 == 0.0) {
				if (d0 < 0.0)
					e0 = edge;
				else	/* d0 > 0.0 */
					e2 = edge;
			}

			edge = next_edge;
			count++;
		}
		while (edge != face->some_edge);

		if (zero_count == 2) { /* we need to make a cut */
			new_edge = &edgeheap[heapcount++];
			new_edge = (WEedge *) malloc32((MyInt32) sizeof(WEedge));
			polyhedron->num_edges++;
			new_edge->next = polyhedron->edge_list;
			polyhedron->edge_list = new_edge;

			/* v01 = vertex between edges e0 and e1	*/
			/* v23 = vertex between edges e2 and e3	*/
			v01 = (e0->v0->dist == 0.0) ? e0->v0 : e0->v1;
			v23 = (e2->v0->dist == 0.0) ? e2->v0 : e2->v1;

			new_edge->v0 = v01;
			new_edge->v1 = v23;
			new_edge->e0L = e0;
			new_edge->e0R = e1;
			new_edge->e1L = e3;
			new_edge->e1R = e2;
			new_edge->fL = face;
			new_edge->fR = new_face;

			if (e0->v0 == v01)
				e0->e0R = new_edge;
			else
				e0->e1L = new_edge;

			if (e1->v0 == v01)
				e1->e0L = new_edge;
			else
				e1->e1R = new_edge;

			if (e2->v0 == v23)
				e2->e0R = new_edge;
			else
				e2->e1L = new_edge;

			if (e3->v0 == v23)
				e3->e0L = new_edge;
			else
				e3->e1R = new_edge;

			new_face->some_edge = new_edge;
			new_face->order++;

			face->order = (count1 - count3 + face->order)%face->order + 1;

			/* transfer face to dirty list */
			face->prv->nxt = face->nxt;
			face->nxt->prv = face->prv;
			face->nxt	= polyhedron->dirty0.nxt;
			face->prv	= &polyhedron->dirty0;
			polyhedron->dirty0.nxt->prv	= face;
			polyhedron->dirty0.nxt		= face;
		}

		/* put the face back on polyhedron->face_list */
		face->next = polyhedron->face_list;
		polyhedron->face_list = face;
	}

	return;
}


static void remove_dead_edges(polyhedron)
WEpolyhedron *polyhedron;
{
	WEedge	*e_list,
			*edge;

	/* Move all edges onto e_list, then move the good edges		*/
	/* back onto polyhedron->edge_list.							*/

	e_list = polyhedron->edge_list;
	polyhedron->edge_list = NULL;

	while (e_list) {
		/* pull an edge off e_list */
		edge = e_list;
		e_list = e_list->next;

		/* if it has one or more bad vertices, throw it away */
		if (edge->v0->dist > 0.0 || edge->v1->dist > 0.0) {

			/* first tidy up at vertices of dist 0.0 */
			if (edge->v0->dist == 0.0) {
				if (edge->e0L->e0R == edge)
					edge->e0L->e0R = edge->e0R;
				else
					edge->e0L->e1L = edge->e0R;

				if (edge->e0R->e0L == edge)
					edge->e0R->e0L = edge->e0L;
				else
					edge->e0R->e1R = edge->e0L;
			}
			if (edge->v1->dist == 0.0) {
				if (edge->e1L->e1R == edge)
					edge->e1L->e1R = edge->e1R;
				else
					edge->e1L->e0L = edge->e1R;

				if (edge->e1R->e1L == edge)
					edge->e1R->e1L = edge->e1L;
				else
					edge->e1R->e0R = edge->e1L;
			}

			/* throw away the edge */
			free32(edge);
			--polyhedron->num_edges;
		}
		else {	/* put it back on polyhedron->edge_list */
			edge->next = polyhedron->edge_list;
			polyhedron->edge_list = edge;
		}
	}
	return;
}


static void remove_dead_vertices(polyhedron)
WEpolyhedron *polyhedron;
{
	WEvertex	*v_list,
				*vertex;

	/* Move all vertices onto v_list, then move the		*/
	/* good vertices back onto polyhedron->vertex_list.	*/

	v_list = polyhedron->vertex_list;
	polyhedron->vertex_list = NULL;

	while (v_list) {
		/* pull an vertex off v_list */
		vertex = v_list;
		v_list = v_list->next;

		/* if it's been cut off, throw it away */
		if (vertex->dist > 0.0) {
			free32(vertex); 
			--polyhedron->num_vertices;
		}
		else {	/* put it back on polyhedron->vertex_list */
			vertex->next = polyhedron->vertex_list;
			polyhedron->vertex_list = vertex;
		}
	}
	return;
}


static void number_faces(polyhedron)
WEpolyhedron *polyhedron;
{
	int			count;
	WEface		*face;

	/* at this point just assign consecutive indices to the	*/
	/* faces, and let the routines for making display lists,*/
	/* saving in other file formats, etc. set the actual	*/
	/* fill tones											*/

	count = 0;
	for (face=polyhedron->face_list; face; face=face->next) {
		/* skip faces which were already assigned fill_tones	*/
		/* when their inverses were found						*/
		if (face->fill_tone >= 0)
			continue;

		if (face->inverse == NULL) {
		    if (debug) fprintf(stderr, "unmatched faces in Dirichlet.c\n");
		}

		face->fill_tone				= count;
		if (face->inverse) face->inverse->fill_tone	= count;
		count++;
	}

	return;
}

#if 0
static void read_vertices(face, v)
WEface	*face;
double	(*v)[3];
{
	int		i;
	WEedge	*edge;

	edge = face->some_edge;
	do {
		if (edge->fL == face) {	/* edge points counterclockwise	*/
			for (i=3; --i>=0; )
				(*v)[i] = edge->v0->x[i];
			v++;
			edge = edge->e1L;
		}
		else {					/* edge points clockwise		*/
			for (i=3; --i>=0; )
				(*v)[i] = edge->v1->x[i];
			v++;
			edge = edge->e0R;
		}
	}
	while (edge != face->some_edge);

	return;
}
#endif

static void print_poly(polyhedron)
WEpolyhedron *polyhedron;
{
	print_vef(polyhedron);	
	print_vertices(polyhedron);
}

#if 0
static void print_statistics(polyhedron)
WEpolyhedron *polyhedron;
{
	print_vef(polyhedron);
	print_vertex_distances(polyhedron);
	print_edge_lengths(polyhedron);
	print_face_distances(polyhedron);
	return;
}
#endif

static void print_vef(polyhedron)
WEpolyhedron *polyhedron;
{
	if (debug) fprintf(stderr, "%d vertices, %d edges, %d faces\n",
		polyhedron->num_vertices,
		polyhedron->num_edges,
		polyhedron->num_faces);

	if (polyhedron->num_vertices - polyhedron->num_edges + polyhedron->num_faces != 2) {
		if (debug) fprintf(stderr, "Euler characteristic error in Dirichlet.c\n");
		return;
	}

	return;
}

#if 0
static void saveOOGL(polyhedron)
WEpolyhedron *polyhedron;
{
	FILE *fp = fopen("/tmp/test.off","w");
	GeomFSave(WEPolyhedronToPolyList(polyhedron), fp, "/tmp/test.off");
	fclose(fp);
}
#endif

static void print_vertices(polyhedron)
WEpolyhedron *polyhedron;
{
	WEvertex	*vertex;
	fprintf(stderr, "Vertices:\n");
	for (vertex=polyhedron->vertex_list; vertex; vertex=vertex->next) 
	    fprintf(stderr, "%f\t%f\t%f\t%f\n",vertex->x[0], vertex->x[1], vertex->x[2], vertex->x[3]);
}

#if 0
static void print_vertex_distances(polyhedron)
WEpolyhedron *polyhedron;
{
	WEvertex	*vertex;
	int			ideal_vertex_present,
				finite_vertex_present,
				i;
	double		norm,
				max,
				min,
				ideal_point_norm;

	/* Note:  the following statement revealed that "infinity"	*/
	/* is at a distance of about 17 on a Mac.					*/
	/* fprintf(stderr, "infinity = %lf\n", acosh(1.0/ideal_point_norm));	*/

	ideal_point_norm = sqrt(1e5 * HARDWARE_PRECISION);
	ideal_vertex_present  = 0;
	finite_vertex_present = 0;
	min = cosh(17.0);
	max = 0.0;

    if (metric == DG_HYPERBOLIC)	{
	for (vertex=polyhedron->vertex_list; vertex; vertex=vertex->next) {
		norm = sqrt(fabs(DHPt3Dot3(vertex->x, vertex->x, metric)));
		if (norm < ideal_point_norm) {
			vertex->ideal = 1;
			ideal_vertex_present = 1;
		}
		else {
			vertex->ideal = 0;

			for (i=4; --i>=0; )
				vertex->x[i] /= norm;

			if (vertex->x[3] < min)
				min = vertex->x[3];

			if (vertex->x[3] > max)
				max = vertex->x[3];

			finite_vertex_present = 1;
		}
	}

	if (finite_vertex_present) {
		fprintf(stderr, "closest  vertex     %.6f\n", acosh(min));
		if (ideal_vertex_present)
			fprintf(stderr, "furthest finite vertex %.6f\n", acosh(max));
		else
			fprintf(stderr, "furthest vertex     %.6f\n", acosh(max));
	}
	else
		fprintf(stderr, "all vertices are at infinity\n");

	}
	return;
}
#endif

/* Note:  print_edge_lengths() assumes print_vertex_distances()	*/
/* has already been called to normalize the vertices.			*/

#if 0
static void print_edge_lengths(polyhedron)
WEpolyhedron *polyhedron;
{
	WEedge	*edge;
	int		finite_edge_present,
			infinite_edge_present;
	double	dot,
			min,
			max;

	min = cosh(17.0);
	max = 1.0;
	finite_edge_present   = 0;
	infinite_edge_present = 0;

	for (edge=polyhedron->edge_list; edge; edge=edge->next)
		if (edge->v0->ideal || edge->v1->ideal)
			infinite_edge_present = 1;
		else {
			finite_edge_present = 1;
			dot = -DHPt3Dot3( edge->v0->x,edge->v1->x, metric);
			if (dot < min)
				min = dot;
			if (dot > max)
				max = dot;
		}

	if (finite_edge_present)
		if (infinite_edge_present) {
			fprintf(stderr, "shortest finite edge %.6f\n", acosh(min));
			fprintf(stderr, "longest  finite edge %.6f\n", acosh(max));
		}
		else {
			fprintf(stderr, "shortest edge       %.6f\n", acosh(min));
			fprintf(stderr, "longest  edge       %.6f\n", acosh(max));
		}
	else
		fprintf(stderr, "all edges are infinite\n");
	
	return;
}
#endif

#if 0
static void print_face_distances(polyhedron)
WEpolyhedron *polyhedron;
{
	WEface	*face;
	double	min,
			max;

	min = 1e308;	/* ieee max double */
	max = 0;

	for (face=polyhedron->face_list; face; face=face->next) {
		if (face->group_element[3][3] < min)
			min = face->group_element[3][3];
		if (face->group_element[3][3] > max)
			max = face->group_element[3][3];
	}

	fprintf(stderr, "closest  face plane %.6f\n", 0.5 * acosh(min));
	fprintf(stderr, "furthest face plane %.6f\n", 0.5 * acosh(max));

	return;
}
#endif

#if 0
static void free_polyhedron(polyhedron)
WEpolyhedron *polyhedron;
{
	WEvertex	*dead_vertex;
	WEedge		*dead_edge;
	WEface		*dead_face;

	while (polyhedron->vertex_list) {
		dead_vertex = polyhedron->vertex_list;
		polyhedron->vertex_list = dead_vertex->next;
		free32(dead_vertex); 
	}

	while (polyhedron->edge_list) {
		dead_edge = polyhedron->edge_list;
		polyhedron->edge_list = dead_edge->next;
		free32(dead_edge); 
	}

	while (polyhedron->face_list) {
		dead_face = polyhedron->face_list;
		polyhedron->face_list = dead_face->next;
		free32(dead_face); 
	}

	return;
}
#endif

static void roundoff_message(epsilon_name)
char *epsilon_name;
{
#if PRECISE_GENERATORS
	fprintf(stderr,"\nWARNING:  roundoff error is getting a bit large.  (%s)\n", epsilon_name);
#else
	fprintf(stderr,"\nWARNING:  roundoff error is getting perilously large.  (%s)\n", epsilon_name);
#endif
	fprintf(stderr,"To verify the correctness of the final Dirichlet domain,\n");
	fprintf(stderr,"move off to another point in the Dehn filling plane, then\n");
	fprintf(stderr,"return to the present point, recompute the Dirichlet domain,\n");
	fprintf(stderr,"and see whether you get the same number of vertices, edges\n");
	fprintf(stderr,"and faces.  (Moving to a different point and then returning\n");
	fprintf(stderr,"will randomize the roundoff error.)\n\n");

	return;
}

int proj_same_matrix(m0, m1)
proj_matrix	m0,
			m1;
{
	int		i, j;
	double	diff;

	for (i=4; --i>=0; )
		for (j=4; --j>=0; ) {
			diff = fabs(m0[i][j] - m1[i][j]);
			if (diff > MATRIX_EPSILON)
				return(0);
			if (diff > 1e-2 * MATRIX_EPSILON && ! matrix_epsilon_message_given) {
				if (debug) roundoff_message("MATRIX_EPSILON");
				matrix_epsilon_message_given = 1;
			}
		}
	return(1);
}
