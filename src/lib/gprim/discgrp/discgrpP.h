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

#ifndef DISCGRPPDEF
#define DISCGRPPDEF

#include "geomclass.h"
#include "discgrp.h"
#include "3d.h"
#include "wa.h"
#include "winged_edge.h"

/* attributes */
#define DG_KEYWORDSIZE 	32
#define DG_NAMESIZE 	128
#define DG_LABELSIZE	256
#define	DG_WORDLENGTH	32
#define DG_MAXGENS	128		/* MAXGENS appears in fsa.h */


struct DiscGrpEl {
	int 		attributes;	/* type? fixed points, etc? */
	char 		word[DG_WORDLENGTH]; /* the 'string' in the generators? */
	Transform 	tform;
	ColorA		color;	
	struct DiscGrpEl	*inverse;
	};

typedef struct DiscGrpEl DiscGrpEl;

typedef struct keytokenpair {
	char key[32];
	int token;
	}  keytokenpair;

typedef struct {
	int attributes;		/* special, orthogonal, projective, etc */
				/* plus entry type ... */
	int dimn;		/* dimension of matrix */
	int sig;		/* signature of group */
	} matrixgroup;

typedef struct {
	int		num_el;
	matrixgroup	mgroup;
	DiscGrpEl 	*el_list;
	} DiscGrpElList;

typedef struct {

#define FRUS_LEFT	0
#define FRUS_RIGHT	1
#define FRUS_TOP	2
#define FRUS_BOTOM	3
	HPoint3		frustrum[4];	/* planes for culling against view frustum */


	Transform	m2w, w2m, c2w, w2c, c2m, m2c;
	}  dgviewinfo;

	
		
struct DiscGrp {
	GEOMFIELDS;
	char 		*name;
	char 		*comment;
	unsigned 	flag;		/* graphics flags */
	int 		attributes;	/* group attributes */
	int 		dimn;
	float		(*c2m)[4];	/* record the transform taking cam->model*/
	wa		*fsa;		/* word acceptor */
	DiscGrpElList	*gens;		/* generators */
	DiscGrpElList	*nhbr_list;	/* dirdom neighbors */
	DiscGrpElList	*big_list;	/* some arbitrary list of group el's */
	HPoint3		cpoint;		/* center point for dirichlet domain */
	Geom		*camgeom;	/* camera geom: moves w/ camera */
	Handle		*camgeomhandle;
	Geom		*ddgeom;	/* special geom*/
	Handle		*ddgeomhandle;
	Geom		*geom;		/* geometry for one fund domain */
	Handle		*geomhandle;
	float		scale;		/* shrink dirdom before drawing... */
	int		enumdepth;	/* max word length for big_list */
	float 		enumdist;	/* max distance grp el moves cpoint */
	float 		drawdist;	/* max distance grp el moves cpoint for drawing purposes (generally less than enumdist)_ */
	void		(*predraw)();
	dgviewinfo	viewinfo;	/* camera position, etc */
	} ;

extern HPoint3 DGorigin;

extern GeomClass *DiscGrpClass;

/* private methods */
extern void DiscGrpSetupDirdom( DiscGrp *discgrp);
extern DiscGrpEl *DiscGrpClosestGroupEl( DiscGrp *discgrp, HPoint3 *poi);
extern DiscGrpElList *DiscGrpExtractNhbrs( WEpolyhedron *wepoly );
extern Geom *DiscGrpDirDom( DiscGrp *dg);
extern Geom *WEPolyhedronToBeams( WEpolyhedron *poly, float alpha);
extern WEpolyhedron *DiscGrpMakeDirdom( DiscGrp *dg, HPoint3 *pt0, int slice);
extern DiscGrpElList *DiscGrpEnum( DiscGrp *dg, int (*constraintfn)());
extern void DiscGrpInitStandardConstraint( int depth, float d0, float d1);
extern int DiscGrpStandardConstraint( DiscGrpEl *dgel);
extern DiscGrpElList *DiscGrpElListDelete (DiscGrpElList *exist);
extern void DiscGrpAddInverses(DiscGrp *discgrp);

void DHPt3PerpBisect(/*point4 p0, point4 p1, point4 result, int metric*/);

extern int needstuneup();
extern void tuneup();
extern int is_new();
extern int insert_or_match_mat();
extern int push_new_stack();
extern int enumpush();
extern void init_stack();
extern void make_new_old();
extern int init_out_stack();
extern void delete_list();
extern int enumgetsize();
		
#endif /* ! DISCGRPPDEF */
