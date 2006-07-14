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

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

#include <stdio.h>
#include <signal.h>
#include <math.h>
#include "geom.h"
#include "polylistP.h"
#include "3d.h"
#include "ooglutil.h"
#include "hinge.h"
#include "hui.h"

/*
  pl is the polyhedron we're dealing with.

  int pl->n_verts:		total number of vertices in the polyhedron
  HPoint3 pl->vl[]:		array containing vertices
  int pl->n_polys:		total number of faces in the polyhedron
  Poly pl->p[]:			array containing the faces
  int pl->p[i].n_vertices:	number of vertices in i-th face
  HPoint3 pl->p[i].v[j]->pt:	j-th vertex of i-th face
*/
PolyList *pl;


TransformPtr TT[MAXTHINGS];

float epsilon = .005;

/* pipes to and from geomview */
FILE *togv;
IOBFILE *fromgv;

char hingedatastring[] =
#include "hingedata.h"
;

/* the space we're in */
int space = HYPERBOLIC;

/* whether a rotation axis is currently defined */
int haveaxis = 0;

/* the angle to rotate through */
float angle = 90;

/* increments by which to rotate */
float hingeincr = 5.0;

/* whether to kill geomview when we exit or die */
int killgv = 0;

Point currentedge[2];
Point base = {0, 0, 0, 1};
Point tip = {0, 0, 1, 1};
Point axis = {0, 0, 1, 1};
Transform BaseT;

int mainpos[2];
int helppos[2];
int infopos[2];
int filepos[2];

char *file = NULL;

static void space_alignZ(Point *a, Point *b, Transform T);
static IOBFILE *Initialize(void);

#define NEXTARG ++argv; --argc

void usage(void)
{
  fprintf(stderr, "hinge [OPTIONS] [FILE]\n\
    OPTIONS:\n\
	-space {euclidean|hyperbolic}\n\
	-angle a	(set angle initially to a degrees)\n\
	-incr a		(hinge through increments of a degrees)\n\
	-killgv		(kill geomview upon exit)\n\
	-mpos x y	(position of main panel)\n\
	-hpos x y	(position of help panel)\n\
	-ipos x y	(position of info panel)\n\
	-fpos x y	(position of file panel)\n");
  exit(-1);
}

void die(int sig)
{
  QuitProc(NULL,0);
}

int main(int argc, char *argv[])
{
  signal(SIGHUP,	die);
  signal(SIGINT,	die);
  signal(SIGQUIT,	die);
  signal(SIGILL,	die);
  signal(SIGTRAP,	die);
  signal(SIGABRT,	die);
  signal(SIGBUS,	die);
  signal(SIGSEGV,	die);
  signal(SIGPIPE,	die);
  signal(SIGTERM,	die);
  
  NEXTARG;
  
  while (argc) {
    if (!strcmp(argv[0],"-killgv")) {
      killgv = 1;
    } else if (!strcmp(argv[0],"-space")) {
      NEXTARG;
      if (!argc) usage();
      if (!strcmp(argv[0],"hyperbolic")) {
	space = HYPERBOLIC;
      } else if (!strcmp(argv[0],"spherical")) {
	space = SPHERICAL;
      } else {;
	space = EUCLIDEAN;
      }
    } else if (!strcmp(argv[0], "-incr")) {
      NEXTARG;
      if (!argc) usage();
      hingeincr = atof(argv[0]);
    } else if (!strcmp(argv[0], "-angle")) {
      NEXTARG;
      if (!argc) usage();
      angle = atof(argv[0]);
    } else if (!strcmp(argv[0], "-mpos")) {
      NEXTARG;
      if (argc<2) usage();
      mainplacement = FL_PLACE_POSITION;
      mainpos[0] = atoi(argv[0]);
      mainpos[1] = atoi(argv[1]);
      NEXTARG;
    } else if (!strcmp(argv[0], "-fpos")) {
      NEXTARG;
      if (argc<2) usage();
      fileplacement = FL_PLACE_POSITION;
      filepos[0] = atoi(argv[0]);
      filepos[1] = atoi(argv[1]);
      NEXTARG;
    } else if (!strcmp(argv[0], "-ipos")) {
      NEXTARG;
      if (argc<2) usage();
      infoplacement = FL_PLACE_POSITION;
      infopos[0] = atoi(argv[0]);
      infopos[1] = atoi(argv[1]);
      NEXTARG;
    } else if (!strcmp(argv[0], "-hpos")) {
      NEXTARG;
      if (argc<2) usage();
      helpplacement = FL_PLACE_POSITION;
      helppos[0] = atoi(argv[0]);
      helppos[1] = atoi(argv[1]);
      NEXTARG;
    } else {
      file = argv[0];
      if (argc>1) {
	fprintf(stderr, "hinge: ignoring arguments after %s\n", file);
	break;
      }
    }
    
    NEXTARG;
  }
  
  hui_init();
  hui_main_loop(Initialize());
 
  return 0;
}

static IOBFILE *Initialize(void)
{
  int i;

  fromgv = iobfileopen(stdin);  

  LangInit(fromgv);

  for (i=0; i<MAXTHINGS; ++i) TT[i] = NULL;
  togv = stdout;
  TmCopy(TM_IDENTITY, BaseT);
  NewInst(0.0);
  fprintf(togv, "(progn\n\
  (normalization allgeoms none)\n\
  (lines-closer allcams 1)\n\
  (merge-baseap appearance { +edge })\n\
)\n");
  HingeSpace(space);
  

  if (file == NULL) {
    switch (space) {
    case HYPERBOLIC:
      file = "HingeDodec";
      break;
    default:
    case EUCLIDEAN:
      file = "HingeCube";
      break;
    }
  }
    
  if (file != NULL) {
    HingeLoad(file);
  }


  {
    Geom *g = NULL;
    IOBFILE *fp = iobfopen("hingedata", "r");
    if (fp == NULL)
      fp = iobfileopen(fmemopen(hingedatastring, sizeof(hingedatastring), "r"));
    if (fp == NULL) {
      OOGLError(0,"can't find the file \"hingedata\"; it must be\n\
in the current directory.  This will be fixed soon. [mbp]");
      exit(-1);
    }
    if (fp != NULL) {
      g = GeomFLoad(fp, "hinge data");
      iobfclose(fp);
    }
    if (g != NULL) {
      DefinePick(g);
    } else {
      OOGLError(1,"can't read hinge data\n");
      exit(-1);
    }
  }

  fprintf(togv, "(interest (pick world))\n");
  fflush(togv);

  return fromgv;
}

int WhichFace(HPoint3 *p, Transform T, PolyList *pl)
{
  int i;

  for (i=0; i<pl->n_polys; ++i) {
    if (CoPlanar(p, T, &(pl->p[i]))) return i;
  }
  return -1;
}

/*-----------------------------------------------------------------------
 * Function:	CoPlanar
 * Description:	test whether a point is coplanar with the image
 *		  of a polygon
 * Args:	*p: the point
 *		T: transform to apply to poly before test
 *		*poly: the polygon
 * Returns:	1 if yes, 0 if no
 * Author:	mbp
 * Date:	Thu Oct  1 10:06:39 1992
 * Notes:	Uses global variable "epsilon" for test.
 *		The plane of the polygon is assumed to be
 *		  that determined by its first 3 vertices.  This
 *		  assumes that those vertices are not collinear.
 *		  If they are collinear, this test may falsely
 *		  return 1.
 *
 */
int CoPlanar(HPoint3 *p, Transform T, Poly *poly)
{
  HPoint3 v0;
  HPoint3 v1;
  HPoint3 v2;
  float det;
  int ans;

  HPt3Transform(T, &(poly->v[0]->pt), &v0);
  HPt3Transform(T, &(poly->v[1]->pt), &v1);
  HPt3Transform(T, &(poly->v[2]->pt), &v2);


  /* the following is the determinant of the 4x4 matrix
     whose 1st 3 rows are the homog coords of the polygon's
     1st 3 vertices, and whose last row is the homog coords
     of p */
  det = 
    p->x * (  v2.w*v1.y*v0.z
	    - v1.w*v2.y*v0.z
	    - v2.w*v0.y*v1.z
	    + v0.w*v2.y*v1.z
	    + v1.w*v0.y*v2.z
	    - v0.w*v1.y*v2.z ) +
    p->y * (- v2.w*v1.x*v0.z
	    + v1.w*v2.x*v0.z
	    + v2.w*v0.x*v1.z
	    - v0.w*v2.x*v1.z
	    - v1.w*v0.x*v2.z
	    + v0.w*v1.x*v2.z ) +
    p->z * (  v2.w*v1.x*v0.y
	    - v1.w*v2.x*v0.y
	    - v2.w*v0.x*v1.y
	    + v0.w*v2.x*v1.y
	    + v1.w*v0.x*v2.y
	    - v0.w*v1.x*v2.y ) +
    p->w * (- v2.x*v1.y*v0.z
	    + v1.x*v2.y*v0.z
	    + v2.x*v0.y*v1.z
	    - v0.x*v2.y*v1.z
	    - v1.x*v0.y*v2.z
	    + v0.x*v1.y*v2.z );

  ans = fabs(det) <= epsilon;
  return ans;
}


float HPt3EucDistance( HPoint3 *a, HPoint3 *b )
{
  Point3 aa, ab;
  float dist;

  HPt3ToPt3(a, &aa);
  HPt3ToPt3(b, &ab);
  dist = Pt3Distance( &aa, &ab );
  return dist;
}

int pt4equal(HPoint3 *a, HPoint3 *b)
{
  float dist;
  int ans;

  dist = HPt3EucDistance( a, b );
  ans = dist <= epsilon;
  return ans;
}

#if 0 /* commented out until spherical mode can be added; don't need
	 this now */
float space_distance(HPoint3 *a, HPoint3 *b)
{
  float dist;
  Point3 aa, bb;

  switch (space) {
  default:
  case EUCLIDEAN:
    dist = HPt3EucDistance( a, b );
    return dist;
    break;
  case HYPERBOLIC:
    dist = HPt3HypDistance( a, b );
    return dist;
    break;
  }
}

float HPt3HypDistance( HPoint3 *a, HPoint3 *b )
{
  float ab, aa, bb, dist, p, s;
  extern double	acosh(double);

  ab = MinkDot(a,b);
  aa = MinkDot(a,a);
  bb = MinkDot(b,b);
  p  = ab*ab / (aa * bb);
  if (p < 1) return 0;
  s = sqrt(p);
  dist = 2 * acosh( (double)s );
  return dist;
}

float MinkDot( HPoint3 *a, HPoint3 *b )
{
  return a->x*b->x + a->y*b->y + a->z*b->z - a->w*b->w;
}
#endif


/*-----------------------------------------------------------------------
 * Function:	PolyContainsEdge
 * Description:	test whether the image of a polygon contain an edge
 * Args:	e[]: array of two points --- the endpoints of the edge
 *		T: transform to apply to the polygon
 *		*poly: the polygon
 * Returns:	-1, 0, or 1 (see below)
 * Author:	mbp
 * Date:	Thu Oct  1 12:28:36 1992
 * Notes:	returns 1 if the polygon contains the edge with the vertices
 *		  in the same order, -1 if opposite order, 0 if polygon doesn't
 *		  contain the edge at all.
 */
int PolyContainsEdge(Point e[], Transform T, Poly *poly)
{
  int i,ans;
  HPoint3 v;

  ans = 0;
  for (i=0; i<poly->n_vertices; ++i) {
    HPt3Transform(T, &(poly->v[i]->pt), &v);
    if (pt4equal(&e[0], &v)) {
      HPt3Transform(T, &(poly->v[(i+1)%poly->n_vertices]->pt), &v);
      if (pt4equal(&e[1], &v)) {
	ans = 1;
	goto done;
      }
      HPt3Transform(T, &(poly->v[(i-1+poly->n_vertices)%poly->n_vertices]->pt), &v);
      if (pt4equal(&e[1], &v)) {
	ans = -1;
	goto done;
      }
    }
  }
 done:
  return ans;
}

/* for debugging, just to make sure we understand what we have... */
void WritePolyListInfo(PolyList *pl)
{
  int i,j;
  FILE *fp = fopen("hinge.out", "w");

  fprintf(fp, "%1d vertices\n\n", pl->n_verts);
  for (i=0; i<pl->n_verts; ++i) {
    fprintf(fp, "vert[%2d] = (%6f, %6f, %6f, %6f)\n",
	    i,
	    pl->vl[i].pt.x,
	    pl->vl[i].pt.y,
	    pl->vl[i].pt.z,
	    pl->vl[i].pt.w);
  }
  fprintf(fp, "\n");
  fprintf(fp, "%1d faces\n\n", pl->n_polys);
  for (i=0; i<pl->n_polys; ++i) {
    fprintf(fp, "face %1d has %1d vertices:\n", i, pl->p[i].n_vertices);
    for (j=0; j<pl->p[i].n_vertices; ++j) {
      fprintf(fp, "\t(%6f, %6f, %6f, %6f)\n",
	      pl->p[i].v[j]->pt.x,
	      pl->p[i].v[j]->pt.y,
	      pl->p[i].v[j]->pt.z,
	      pl->p[i].v[j]->pt.w);
    }
  }
  fclose(fp);
}

int
HingeLoad(char *file)
{
  Geom *g;

  if (strlen(file)<=0) return 0;

  /*
    The following kludge uses geomview to get the data; we tell
    geomview to load it, then dump it back to us, then delete it.
   */
  fprintf(togv,"(progn\n\
  (geometry thing { < %s })\n\
  (echo \"{\")\n\
  (write geometry - thing self)\n\
  (echo \"}\")\n\
  (delete thing)\n\
)\n", file);
  fflush(togv);

  /*
   * We now read the object we just told geomview to give us.
   */
  g = GeomFLoad(fromgv, "Hinge: pipe from geomview");

  if (g == NULL) {
    fprintf(stderr, "Hinge: error reading geometry from file %s\n", file);
  } else {
    char buf[80];
    DefineThing(g);
    sprintf(buf,"%s",file);
    hui_message(buf);
    haveaxis = 0;
    ShowAxis();
  }
  pl = (PolyList *)g;
  return g != NULL;
}
  

int
NewInst(float ang)
{
  int n = NewTTindex();
  fprintf(togv,
	  "(progn (read geometry { define T%1d { LIST } } )\n\
(geometry \"geom%1d\" { INST tlist { LIST { :T%1d } } geom :thing } ) )\n",
	  n, n, n);
  fflush(togv);
  Inst(n, 0.0);
  return n;
}

void
ComputeTransform(Transform T, Transform BaseT, float ang)
{
  if (ang == 0) {
    TmCopy(BaseT, T);
  } else {
    Rotation(T, &currentedge[0], &currentedge[1], RADIANS(ang));
    TmConcat(BaseT, T, T);
  }
}

int
NewTTindex()
{
  int i;
  for (i=0; i<MAXTHINGS && TT[i]!=NULL; ++i);
  if (i==MAXTHINGS) return -1;
  TT[i] = (TransformPtr)malloc(sizeof(Transform));
  TmIdentity(TT[i]);
  return i;
}

void
DefineAxis(Point *a, Point *b)
{
  haveaxis = 1;
  currentedge[0] = *a;
  currentedge[1] = *b;
  base = *a;
  tip = *b;
  HPt3Sub(b, a, &axis);
}

void
Rotation(Transform T, Point *a, Point *b, float angle)
{
  Transform A;

  space_alignZ(a, b, A);
  TmInvert(A,A);
  TmRotateZ(T, angle);
  TmConjugate(T, A, T);
}

void
DefineThing(Geom *g)
{
  fprintf(togv,"(read geometry { define thing {\n");
  GeomFSave(g, togv, "hinge output pipe (define thing...)");
  fprintf(togv, "} } )\n");
  fflush(togv);
}
 
void
DefinePick(Geom *g)
{

  fprintf(togv, "(geometry \"pick\" { INST tlist { LIST { :edgeT } } geom { \n");
  GeomFSave(g, togv, "hinge output pipe (geometry pick...)");
  fprintf(togv, " } } )\n");

  fflush(togv);
}
 
void
Inst(int n, float ang)
{
  ComputeTransform(TT[n], BaseT, ang);
  fprintf(togv,"(read geometry { define T%1d TLIST\n",n);
  fputtransform(togv, 1, (float*)(TT[n]), 0);
  fprintf(togv," } )\n");
  fflush(togv);
}

void
Undo()
{
  int i;

  for (i=0; i<MAXTHINGS && TT[i]!=NULL; ++i);
  if (i==MAXTHINGS || i<=1) return;

  --i;
/*  OOGLFree(TT[i]); */
  free(TT[i]);
  TT[i] = NULL;
  fprintf(togv,"(delete \"geom%1d\")\n", i);
  fflush(togv);
}

void
Reset()
{
  int i;

  fprintf(togv,"(progn\n");
  for (i=1; i<MAXTHINGS && TT[i]!=NULL; ++i) {
    /*  OOGLFree(TT[i]); */
    free(TT[i]);
    TT[i] = NULL;
    fprintf(togv,"(delete \"geom%1d\")\n", i);
  }
  haveaxis = 0;
  ShowAxis();
  fprintf(togv,")\n");
  fflush(togv);
}

void
  ShowAxis()
{
  Transform Tloc, Tsize, Tnet;
  Point3 b1;
  float b1halflen, radius;
  
  if (haveaxis) {
    
    space_alignZ(&currentedge[0], &currentedge[1], Tloc);
    HPt3TransPt3(Tloc, &currentedge[1], &b1);
    TmInvert(Tloc, Tloc);
    b1halflen = .5 * Pt3Length(&b1);
    
    TmTranslate( Tsize, (float)0, (float)0, b1halflen );
    radius = .05 * b1halflen;
    CtmScale( Tsize, radius, radius, b1halflen );
    
    TmConcat(Tsize, Tloc, Tnet);
    
    fprintf(togv,"(read geometry { define edgeT TLIST\n");
    fputtransform(togv, 1, (float*)Tnet, 0);
    fprintf(togv," } )\n");
    
  } else {
    
    fprintf(togv,"(read geometry { define edgeT { LIST } })\n");
    
  }
  
  fflush(togv);
}

void space_translate_origin(Point *pt, Transform T)
{
  switch(space) {
  case HYPERBOLIC:
    TmHypTranslateOrigin(T, pt);
    break;
  case SPHERICAL:
    TmSphTranslateOrigin(T, pt);
    break;
  default:
  case EUCLIDEAN: TmTranslateOrigin(T, pt); break;
  }
}
  
void HingeSpace(int s)
{
  switch (s) {
  case HYPERBOLIC:
    space = HYPERBOLIC;
    fprintf(togv, "\
(progn\n\
  (space hyperbolic)\n\
  (camera-reset allcams)\n\
  (merge-baseap appearance { +edge shading constant })\n\
)\n");
    fflush(togv);
    break;
  case SPHERICAL:
    space = SPHERICAL;
    fprintf(togv, "\
(progn\n\
  (space spherical)\n\
  (camera-reset allcams)\n\
  (merge-baseap appearance { +edge shading constant })\n\
)\n");
    fflush(togv);
    break;
  case EUCLIDEAN:
  default:
    space = EUCLIDEAN;
    fprintf(togv, "\
(progn\n\
  (space euclidean)\n\
  (merge-baseap appearance { +edge shading flat })\n\
)\n");
    fflush(togv);
    break;
  }
}

/*
  returns T = transform taking a to 0 and b to
  a point on the +Z axis
  */
static void space_alignZ(Point *a, Point *b, Transform T)
{
  Transform Ta, R;
  Point b1;
  
  /* Ta = transform taking a to origin */
  space_translate_origin(a, Ta);
  TmInvert(Ta,Ta);
  
  /* b1 = image of b under Ta */
  HPt3Transform(Ta, b, &b1);
  
  /* R = transform rotating b1 to +Z axis */
  TmRotateTowardZ(R, &b1);
  
  /* answer T is Ta * R */
  TmConcat(Ta, R, T);
}
