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

#ifndef DISCGRPDEF
#define DISCGRPDEF

#include "bbox.h"
#include "dgflag.h"
#include "create.h"
#include "appearance.h"
#include "pick.h"
#include "streampool.h"

#ifndef NULL
#define NULL 0
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define DISCGRPMAGIC    GeomMagic('d',1)

typedef struct DiscGrp DiscGrp ;

extern DiscGrp *
DiscGrpPick( DiscGrp *, Pick *, Appearance *, Transform, TransformN *, int * );
extern GeomClass *DiscGrpMethods( void );
extern char     *DiscGrpName( void );
/*
extern DiscGrp  *DiscGrpFLoad( FILE *, char * );
extern DiscGrp  *DiscGrpLoad( char * );
*/
extern Geom  *DiscGrpImport (Pool * );
extern DiscGrp  *DiscGrpSave( DiscGrp *, char * );
extern DiscGrp  *DiscGrpFSave( DiscGrp *, FILE *, char * );
extern BBox     *DiscGrpBound( DiscGrp *, Transform T, TransformN *TN);
extern DiscGrp  *DiscGrpDraw( DiscGrp * );
/*
extern DiscGrpElList *DiscGrpElListCreate(DiscGrpElList *, ...);
extern DiscGrp  *DiscGrpEvert( DiscGrp * );
extern DiscGrp     *DiscGrpTransform( DiscGrp *, Transform );
extern DiscGrp     *DiscGrpTransformTo( DiscGrp *, Transform );
*/

/* tokens used by the create routines, not duplicated in create.h */
/* we'll use the '900' region: this ought to be registered somewhere */
/* DGELC == DiscGrpElListCreate
   DGC == DiscGrpCreate
*/
enum {
  DGCR_ATTRIBUTE = 900, /* int : attributes in both DGC and DGELC */
  DGCR_ATTRIBUTELIST,   /* int * : list of attributes in DGELC */
  DGCR_WORD,            /* char (*)[DG_WORDLENGTH] : used by
			 * DiscGrpElListCreate */
  DGCR_ELEM,            /* DiscGrpEl * : used in DGELC note that
			 * CR_ELEM is also used here */
  DGCR_CPOINT,          /* HPoint3 * :base  point of dirichlet domain: */

  DGCR_CAMGEOM,         /* Geom * :geometry used to represent the observer*/
  DGCR_CAMGEOMHANDLE,   /* Handle * */
  DGCR_HANDLE_CAMGEOM,  /* Handle *, Geom * */
 
  DGCR_DDGEOM,          /* Geom * :geometry for Dirichlet domain */
  DGCR_DDGEOMHANDLE,    /* Handle * */
  DGCR_HANDLE_DDGEOM,   /* Handle *, Geom * */
  DGCR_GENS,            /* DiscGrpElList * : generators */
  DGCR_BIGLIST,         /* DiscGrpElList * : general list */
  DGCR_ENUMDEPTH,       /* int : depth to compute wordlist */
  DGCR_ENUMDIST,        /* float : maximum distance of tiles */
  DGCR_SCALE,           /* float : scaling factor for dirdom */
  DGCR_FLAG,            /* the flag field of the DiscGrp */
  DGCR_NAME,            /* the flag field of the DiscGrp */
  DGCR_COMMENT,         /* the flag field of the DiscGrp */
  DGCR_DRAWDIST,        /* max distance grp el moves cpoint for
			 * drawing purposes (generally less than
			 * enumdist)_ */
  DGCR_END
};

#endif /*DISCGRPDEF*/

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
