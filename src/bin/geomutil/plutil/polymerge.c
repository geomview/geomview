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

/*
 * Squeeze OFF files.
 * Merges collinear edges and coplanar faces.
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>	/* for qsort(), malloc(), atof() */

#include <ooglutil.h>	/* variable-length arrays, getopt() and stuff */

#ifdef NeXT
extern char *strdup(const char *);
#endif

#define	VSUB(a, b, dst)  (dst)->x = (a)->x - (b)->x, \
			 (dst)->y = (a)->y - (b)->y, \
			 (dst)->z = (a)->z - (b)->z

#define	VDOT(a, b)	 (a)->x*(b)->x + (a)->y*(b)->y + (a)->z*(b)->z
#define	VLENGTH(a)	 sqrt(VDOT(a, a))

#define VCROSS(a, b, dst) (dst)->x = (a)->y * (b)->z - (a)->z * (b)->y, \
			  (dst)->y = (a)->z * (b)->x - (a)->x * (b)->z, \
			  (dst)->z = (a)->x * (b)->y - (a)->y * (b)->x

#define	VSADD(a, s, b, dst) (dst)->x = (a)->x + (s)*(b)->x, \
			    (dst)->y = (a)->y + (s)*(b)->y, \
			    (dst)->z = (a)->z + (s)*(b)->z

#define	VSCALE(a, s)	    (a)->x *= (s), (a)->y *= (s), (a)->z *= (s)

float	EPS_LINEAR=	0.0001;	/* Max unit-vector cross product for collinearity */
float	EPS_NORMAL=	0.0009;	/* Max sin^2 theta between normals for coplanarity */
float	EPS_POINT=	0.00001;/* Max distance between coincident vertices */

#define	F_VERT	0x1
#define	F_EDGE	0x2
#define	F_FACE	0x4
#define	F_EVOLVER 0x8

typedef struct vertex V;
typedef struct face   F;
typedef struct faceedge Fe;
typedef struct edge   E;
typedef struct geodesic Geod;

typedef struct point {
	float x, y, z, w;
} P;


struct vertex {
	P p;
	int ref;
	int index;
	float *more;
};

struct faceedge {
	F *face;
	Fe *prev, *next;	/* double links in face-edges of this face */
	E  *edge;
	int sign;		/* direction w.r.t. edge */
	P n;			/* adopted surface normal at this edge */
	Fe *elink;
};

struct face {
	Fe *fedges;
	char *affix;
};

struct edge {
	V *v[2];		/* vertex pointers */
	E *link;		/* hash table link */
	P to;			/* normalized edge direction vector */
	Fe *feds;		/* faceedge's on this edge */
	int index;		/* for numbering edges for .fe format */
};

struct geodesic {
	int faceno, vertno;	/* Seed data.  Starting face, or failing that, starting vertex */
	F *face;		/* Current-face pointer */
	P base;			/* Geodesic base point */
	P dir;			/* Geodesic current direction vector */
	float length;		/* Total length of geodesic */
	float maxlength;	/* Maximum length of geodesic */
	vvec pts;		/* list of points (P) traced on geodesic */
};

static int nv, nf, tossedf;
static V *Vs;				/* All vertices */
static F *Fs;				/* All faces */
static E **ehash;			/* Array of hash-table head pointers */
static int nhash;
static int flags = F_VERT|F_EDGE|F_FACE;
static int debug = 0;
static int vdim = 3;
vvec geods;				/* List of geodesics to be traced */

static Fe *fedge( F *f, V *v0, V *v1 );
static void vmerge(void);
static void trace_geodesic(Geod *g);
static E *advance_geodesic(Geod *g, E *avoid);
static void add_geodesic(Geod *g, char *str);
static void reflectline(P *axis, P *vector, P *result);
static void rotation(P *va, P *vb, float T[3][3]);
static int vcmp(V **p, V **q);
static void deface(F *f);
static void normal_ize(F *f);
/*static E *echeck(int v0, int v1);*/
/*static void fecheck(Fe *fe);*/
static void femerge(Fe *fe1, Fe *fe2);
static void unfedge(Fe *fe);
static Fe *fedge(F *f, V *v0, V *v1);

#define	New(t)		(t *) malloc(sizeof(t))
#define NewN(t, N)	(t *) malloc((N)*sizeof(t))

int main(int argc, char *argv[])
{
    int i;
    int tnv, tne, any;
    int binary = 0, vertsize = 0;
    char *C = "", *N = "", *four = "";
    extern int optind;
    extern char *optarg;
    IOBFILE *iobf;

    VVINIT(geods, Geod, 5);
    vvzero(&geods);
    while((i = getopt(argc, argv, "bdv:e:f:VEFg:")) != EOF) {
	switch(i) {
	case 'b': flags |= F_EVOLVER; break;
	case 'd': debug = 1; break;
	case 'v': EPS_POINT = atof(optarg); break;
	case 'e': EPS_LINEAR = atof(optarg); EPS_LINEAR *= EPS_LINEAR; break;
	case 'f': EPS_NORMAL = atof(optarg); EPS_NORMAL *= EPS_NORMAL; break;
	case 'V': flags &= ~F_VERT; break;
	case 'E': flags &= ~F_EDGE; break;
	case 'F': flags &= ~F_FACE; break;
	case 'g': add_geodesic(VVAPPEND(geods, Geod), optarg); break;
	default:
	    fprintf(stderr, "\
Usage: polymerge [-v vertex_thresh] [-e edge_thresh] [-f face_thresh]\n\
	[-V][-E][-F][-d][-b] [infile.off]\n\
Merges coincident vertices, collinear edges, coplanar faces of an OFF object.\n\
-v vertex_thresh: max separation for \"coincident\" vertices (default %g)\n\
-e edge_thresh	: max sin(theta) for \"collinear\" edges (default %g)\n\
-f face_thresh	: max sin(theta) for \"coplanar\" facet normals (default %.4g)\n\
-V  -E  -F	: don't try to merge vertices/edges/faces\n\
-b 		: create output in evolver .fe format\n\
-d 		: debug\n",
	EPS_POINT, sqrt(EPS_LINEAR), sqrt(EPS_NORMAL));
	    exit(1);
	}
    }
    if(optind < argc) {
	if(freopen(argv[argc-1], "rb", stdin) == NULL) {
	    fprintf(stderr, "polymerge: can't open input: ");
	    perror(argv[argc-1]);
	    exit(1);
	}
    }

    /* If we're tracing geodesics, we must have collinear edges merged
     * and facet-normals computed.
     */
    if(VVCOUNT(geods) > 0)
	flags |= F_EDGE;

    iobf = iobfileopen(stdin);

    vdim = 3;
    for(;;) {
	switch( iobfnextc(iobf, 0) ) {
	case 'N': N = "N"; vertsize += 3; goto swallow;
	case 'C': C = "C"; vertsize += 4; goto swallow;
	case '4':
		four = "4";
		vdim = 4;
		flags &= ~(F_EDGE|F_FACE);
		goto swallow;
      swallow:
	case '{': case '=': iobfgetc(iobf); break;
	case 'a':	/* Appearance?  Silently swallow keyword */
	    iobfexpectstr(iobf, "appearance");
	    if(iobfnextc(iobf, 0) == '{') {
		int c, brack = 0;
		do {
		    if((c = iobfgetc(iobf)) == '{') brack++;
		    else if(c == '}') brack--;
		} while(brack > 0 && c != EOF);
	    }
	    break;
	    
	default: goto done;
	}
    }
  done:
    iobfexpecttoken(iobf, "OFF");	/* swallow optional OFF prefix */
    if(iobfexpecttoken(iobf, "BINARY") == 0) {
	binary = 1;
	iobfnextc(iobf, 1);
	iobfgetc(iobf);
    }

    if(iobfgetni(iobf, 1, &nv, 0) <= 0 ||
	iobfgetni(iobf, 1, &nf, 0) <= 0 ||
	iobfgetni(iobf, 1, &nhash, 0) <= 0 || nv <= 0 || nf <= 0) {
	   fprintf(stderr, "polymerge: Input not an OFF file.\n");
	   exit(1);
    }

    nhash = (nf + nv + 4) / 2;
    if(nhash < 16) nhash = 17;
    if(!(nhash & 1)) nhash++;
    ehash = NewN(E *, nhash);
    memset((char *)ehash, 0, nhash*sizeof(E *));

    Vs = NewN(V, nv);
    Fs = NewN(F, nf);

    for(i = 0; i < nv; i++) {
	V *vp = &Vs[i];
	
	vp->p.w = 1;
	if(iobfgetnf(iobf, vdim, (float *)&vp->p, binary) < vdim) {
	  badvert:
	    fprintf(stderr, "polymerge: error reading vertex %d/%d\n", i,nv);
	    exit(1);
	}
	if(vertsize) {
	    Vs[i].more = NewN(float, vertsize);
	    if(iobfgetnf(iobf, vertsize, Vs[i].more, binary) < vertsize)
		goto badvert;
	}
	Vs[i].ref = 0;
	Vs[i].index = i;
    }

    /*
     * Combine vertices
     */
    if(flags & F_VERT)
	vmerge();

    /*
     * Load faces
     * The fedge() calls here assume we've already merged all appropriate
     * vertices.
     */

    tossedf = 0;
    for(i = 0; i < nf; i++) {
	F *fp = &Fs[i];
	Fe *fe;
	int nfv, k, c;
	int v, ov, v0;
	Fe head;
	char *cp;
	char aff[512];

	if(iobfgetni(iobf, 1, &nfv, binary) <= 0 || nfv <= 0) {
	    fprintf(stderr, "polymerge: error reading face %d/%d\n",
		i, nf);
	    exit(1);
	}
	head.prev = head.next = &head;
	fp->fedges = &head;
	iobfgetni(iobf, 1, &v, binary);
	if(v < 0 || v >= nv) {
	    fprintf(stderr, "polymerge: bad vertex %d on face %d\n", v, i);
	    exit(1);
	}
	v0 = ov = Vs[v].index;		/* Use common vertex if merged */
	for(k = 1; k < nfv; k++, ov = v) {
	    iobfgetni(iobf, 1, &v, binary);
	    if(v < 0 || v >= nv) {
		fprintf(stderr, "polymerge: bad vertex %d on face %d\n", v, i);
		exit(1);
	    }
	    v = Vs[v].index;		/* Use common vertex if merged */
	    if(ov == v)
		continue;
	    head.prev->next = fe = fedge(fp, &Vs[ov], &Vs[v]);
	    fe->prev = head.prev;
	    fe->next = &head;
	    head.prev = fe;
	}
	if(v != v0) {
	    fe = fedge(fp, &Vs[v], &Vs[v0]);
	    head.prev->next = fe;
	    fe->prev = head.prev;
	    fe->next = &head;
	    head.prev = fe;
	}
	head.next->prev = head.prev;
	if(head.next == &head) {
	    /*
	     * Degenerate face here
	     */
	    fp->fedges = NULL;
	    tossedf++;
	    if (debug)
		    printf("# Face %d degenerate already\n", i);
	} else {
	    head.prev->next = fp->fedges = head.next;
	}

	if(binary) {
	    int nfc;
	    float c;
	    iobfgetni(iobf, 1, &nfc, binary);
	    if(nfc > 4) {
		fprintf(stderr, "Bad face color count 0x%x", nfc);
		exit(1);
	    }
	    for(cp = aff; --nfc >= 0; cp += strlen(cp)) {
		iobfgetnf(iobf, 1, &c, binary);
		sprintf(cp, " %g", c);
	    }
	} else {
	    (void) iobfnextc(iobf, 1);
	    for(cp = aff; (c = iobfgetc(iobf)) != EOF && c != '\n' && cp < &aff[511]; )
		*cp++ = c;
	}
	*cp = '\0';
	fp->affix = (cp > aff) ? strdup(aff) : "";
    }

    /*
     * Compute face normals -- actually corner normals, since we avoid
     * assuming faces are planar.  As a side effect, we detect & join
     * collinear edges.
     */
    if(flags & F_EDGE) {
	for(i = 0; i < nf; i++) {
	    normal_ize(&Fs[i]);
	}
    }

    /* Trace geodesic(s) if we're given any. */
    for(i = 0; i < VVCOUNT(geods); i++) {
	trace_geodesic(&VVEC(geods, Geod)[i]);
    }

    /*
     * Locate edges bounding faces with the same normal direction.
     */
    if(flags & F_FACE) {
      do {
	any = 0;

	for(i = 0; i < nhash; i++) {
	    E *e;

	    for(e = ehash[i]; e != NULL; e = e->link) {
		Fe *fe, *fee;

		for(fe = e->feds; fe != NULL; fe = fe->elink) {
		    for(fee = fe->elink; fee != NULL; fee = fee->elink) {
			float dn;

			dn = VDOT(&fe->n, &fee->n);
			if(fabs(1 - dn*dn) < EPS_NORMAL) {
				/*
				 * OK, merge fee into fe
				 */
				femerge(fe, fee);
				any++;
				goto another;
			}
		    }
		}
	      another: ;
	    }
	}
	if (debug)
		printf("# %d faces merged this pass.\n", any);
      } while(any);
    }

    /*
     * Scan for unused edges.
     */
    for(i = 0; i < nv; i++)
	Vs[i].ref = 0;
    tne = 0;
    for(i = 0; i < nhash; i++) {
	E *e;
	for(e = ehash[i]; e != NULL; e = e->link) {
	    if(e->feds != NULL) {
		e->v[0]->ref++;
		e->v[1]->ref++;
		e->index = ++tne;
	    }
	}
    }
    /*
     * Renumber used vertices.
     */
    if(flags & 1) {
	tnv = 0;
	for(i = 0; i < nv; i++)
	    Vs[i].index = Vs[i].ref ? tnv++ : -i-1;
    } else {
	tnv = nv;
    }

    if(VVCOUNT(geods) > 0) {
	/* Emit all geodesics as a VECT object. */
	/* Don't emit anything else. */
	int i, j, totv = 0, totpl = 0;
	Geod *g;

	for(i = 0; i < VVCOUNT(geods); i++) {
	    g = &VVEC(geods, Geod)[i];
	    if(VVCOUNT(g->pts) > 1) {
		totv += VVCOUNT(g->pts);
		totpl++;
	    }
	}

	printf("VECT\n");
	printf("%d %d %d\n", totpl, totv, 0);
	for(i = 0; i < VVCOUNT(geods); i++) {
	    g = &VVEC(geods, Geod)[i];
	    if(VVCOUNT(g->pts) > 1) {
		printf("%d ", VVCOUNT(g->pts));
	    }
	}
	printf("\n");
	for(i = 0; i < totpl; i++)
	    printf("0 ");
	printf("\n\n");
	for(i = 0; i < VVCOUNT(geods); i++) {
	    P *pt;
	    g = &VVEC(geods, Geod)[i];
	    if(VVCOUNT(g->pts) > 1) {
		pt = VVEC(g->pts, P);
		for(j = VVCOUNT(g->pts); --j >= 0; pt++)
		    printf("%g %g %g\n", pt->x, pt->y, pt->z);
	    }
	}
	exit(0);
    }

if (flags & F_EVOLVER) /* Produce Brakke's evolver .fe format */
{
    int j=0;
    if (vdim == 4) printf("space_dimension 4\n");
    printf("vertices\n");
    for(i = 0; i < nv; i++) {
	V *v = &Vs[i];
	if(v->ref || !(flags & 1)) {
	    v->index = ++j;
	    printf("%d\t%#g %#g %#g", v->index, v->p.x, v->p.y, v->p.z);
	    if(vdim == 4) printf(" %g", v->p.w);
	    printf("\n");
	}
    }
    printf("\nedges\n");
    for(i = 0; i < nhash; i++) {
	E *e;
	for(e = ehash[i]; e != NULL; e = e->link)
	    if(e->feds != NULL)
		printf("%d\t%d %d\n", e->index,e->v[0]->index,e->v[1]->index);
    }

    printf("\nfaces\n");
    j=0;
    for(i=0; i<nf; i++) {
	Fe *fe, *fee;
	int k, nfv;

	fe = Fs[i].fedges;
	if(fe == NULL)
	    continue;
	for(fee = fe, k = 1; (fee = fee->next) != fe; k++)
	    ;
	if ((nfv = k)<3)
	    continue;	/* don't print faces of less than 3 sides */
	printf("%d", ++j);
	for(fee = fe, k = nfv; --k >= 0; fee = fee->next)
	    printf(" %d", (1-2*fee->sign)*fee->edge->index);
	printf("\n");
    }
}
else    /* Produce OFF format */
{
    printf("%s%s%sOFF\n%d %d %d\n", C, N, four, tnv, nf - tossedf, tne);
    for(i = 0; i < nv; i++) {
	V *v = &Vs[i];
	int k;
	if(v->ref || !(flags & F_VERT)) {
	    printf("%#g %#g %#g", v->p.x, v->p.y, v->p.z);
	    if(vdim == 4) printf(" %#g", v->p.w);
	    if(vertsize) {
		printf("  ");
		for(k = 0; k < vertsize; k++)
		    printf(" %#g", v->more[k]);
	    }
	    if(debug)
		printf("\t# %d [%d] #%d", v->index, v->ref, i);
	    printf("\n");
	}
    }
    printf("\n");
    /* ho hum */
    for(i=0; i<nf; i++) {
	Fe *fe, *fee;
	int k, nfv;

	fe = Fs[i].fedges;
	if(fe == NULL)
	    continue;
	for(fee = fe, k = 1; (fee = fee->next) != fe; k++)
	    ;
	nfv = k;
	printf("%d", nfv);
	for(fee = fe, k = nfv; --k >= 0; fee = fee->next)
	    printf(" %d", fee->edge->v[fee->sign]->index);
	printf("\t%s", Fs[i].affix);
	if(debug) {
	    printf(" #");
	    for(fee = fe, k = nfv; --k >= 0; fee = fee->next)
		    printf(" %d", (int)(fee->edge->v[fee->sign] - Vs));
	}
	printf("\n");
    }
}
    exit(1);
}

/*
 * Add a new faceedge
 */
static Fe *fedge(F *f, V *v0, V *v1)
{
    Fe *fe;
    E *e;
    int t;
    int i0, i1;
    float r;

    fe = New(Fe);
    fe->face = f;
    fe->sign = 0;
    i0 = v0->index;
    i1 = v1->index;
    if(i0 > i1) {
	V *tv = v0;
	v0 = v1;
	v1 = tv;
	i0 = v0->index;
	i1 = v1->index;
	fe->sign = 1;
    }

    t = (unsigned long)(v0->index + v1->index + v0->index*v1->index) % nhash;
					/* Symmetric hash function */
    for(e = ehash[t]; e != NULL; e = e->link)
	if(e->v[0]->index == i0 && e->v[1]->index == i1)
	    goto gotit;

    e = New(E);
    e->v[0] = v0;
    e->v[1] = v1;
    e->feds = NULL;
    VSUB(&v0->p, &v1->p, &e->to);
    r = VDOT(&e->to, &e->to);
    if(r != 0) {
	r = 1/sqrt(r);
	e->to.x *= r;  e->to.y *= r;  e->to.z *= r;
    } else if (debug)
	printf("# Coincident: %d == %d [%g %g %g]\n", v0->index, v1->index, v0->p.x,v0->p.y,v0->p.z);
    e->link = ehash[t];
    ehash[t] = e;
 gotit:
    fe->edge = e;
    fe->elink = e->feds;
    e->feds = fe;
    return fe;
}

/*
 * Remove a faceedge from its edge list
 */
static void unfedge(Fe *fe)
{
    Fe **fepp;
    
    for(fepp = &fe->edge->feds; *fepp != NULL; fepp = &(*fepp)->elink) {
	if(*fepp == fe) {
	    *fepp = fe->elink;
	    break;
	}
    }
    free(fe);
}

/*
 * Merge two faces
 * We delete these face-edges from both faces
 */
static void femerge(Fe *fe1, Fe *fe2)
{
    F *f1, *f2;
    Fe *tfe;

    if(fe1->face == fe2->face) {
	if (debug)
		printf("# Merging two edges of face %d -- tossing it.\n",
		       (int)(fe1->face - Fs));
	deface(fe1->face);
	return;
    }

    if(fe1->sign == fe2->sign) {
	/*
	 * Messy.  To merge these, we need to reverse all the links
	 * in one of the two faces.
	 */
	Fe *xfe;
	tfe = fe2;
	do {
	    tfe->sign ^= 1;
	    xfe = tfe->next;
	    tfe->next = tfe->prev;
	    tfe->prev = xfe;
	    tfe = xfe;
	} while(tfe != fe2);
    }
    fe1->prev->next = fe2->next;
    fe2->next->prev = fe1->prev;
    fe1->next->prev = fe2->prev;
    fe2->prev->next = fe1->next;

    f1 = fe1->face;
    f2 = fe2->face;
    if(f1->fedges == fe1)
	f1->fedges = fe2->next;
	if(debug)
	    printf("# Merged face %d into %d (vertices %d %d) n1.n2 %g\n",
		   (int)(f2-Fs), (int)(f1-Fs),
		   fe1->edge->v[fe1->sign]->index,
		   fe1->edge->v[1 - fe1->sign]->index,
		   VDOT(&fe1->n, &fe2->n));
    tfe = fe2->next;
    if(tfe == NULL) {
	fprintf(stderr, "polymerge: face f2 already deleted?\n");
    } else {
	do {
	    tfe->face = f1;
	} while((tfe = tfe->next) != fe1->next);
    }
    f2->fedges = NULL;
    tossedf++;
    unfedge(fe1);
    unfedge(fe2);

    /*
     * Join collinear edges, recompute normals (might have changed a bit).
     */
    normal_ize(f1);
}

#define PRETTY(x)  ((int)(x) - 0x10000000)

#if 0
static void fecheck(Fe *fe)
{
    Fe *fee;
    F *f;
    E *e;

    if(fe == NULL)
	return;
    f = fe->face;
    fprintf(stderr,"0x%p: on face %d (%p); ", (void *)fe, f - Fs, (void *)f);
    fee = fe;
    do {
	fprintf(stderr," %s%x[%d%s%d] ", (f->fedges == fee) ? "*" : "",
		PRETTY(fee),
		fee->edge->v[0]->index,
		fee->sign ? "<-" : "->",
		fee->edge->v[1]->index);

	if(fee->face != f)
	    fprintf(stderr," Fe %p: face %d (%p) != %p\n",
		    (void *)PRETTY(fee), fee->face - Fs, (void *)fee->face, (void *)f);
	if(fee->next->prev != fee)
	    fprintf(stderr," Fe %x: next %x next->prev %x\n",
		PRETTY(fee), PRETTY(fee->next), PRETTY(fee->next->prev));
	e = fee->edge;
	fee = fee->next;
    } while(fee != fe);
    fprintf(stderr, "\n");
}
#endif

#if 0
static E *echeck(int v0, int v1)
{
    E *e;

    int t = (v0 + v1 + v0*v1) % nhash;
					/* Symmetric hash function */
    if(v0 > v1) v0 ^= v1, v1 ^= v0, v0 ^= v1;
    for(e = ehash[t]; e != NULL; e = e->link) {
	if(e->v[0] == &Vs[v0] && e->v[1] == &Vs[v1]) {
	    Fe *fe;
	    fprintf(stderr, "E 0x%p %d-%d (%d-%d)  %p...\n",
		    (void *)e, v0,v1, e->v[0]->index, e->v[1]->index, (void *)PRETTY(e->feds));
	    for(fe = e->feds; fe != NULL; fe = fe->elink) {
		fecheck(fe);
	    }
	}
    }
    
    return e;
}
#endif

static void normal_ize(F *f)
{
    Fe *fe;

    fe = f->fedges;
    if(fe == NULL)
	return;
    if(fe->prev == fe->next) {
	if (debug)
	    printf("# Face %d already degenerate -- tossing it.\n",
		   (int)(f - Fs));
	deface(f);
	return;
    }

    do {		/* loop over edges on this face */
	P n;
	float r;

	for(;;) {
	    Fe *fn, *fee;
	    P *pp, *qp;

	    pp = &fe->edge->to;
	    fee = fe->next;
	    qp = &fee->edge->to;
	    VCROSS(pp, qp, &n);
	    r = VDOT(&n, &n);
	    if(r > EPS_LINEAR)
		break;
	    /*
	     * Join collinear edges; produce a new edge
	     */
	    fn = fedge(f, fe->edge->v[fe->sign], fee->edge->v[1-fee->sign]);
	    fee->next->prev = fe->prev->next = fn;
	    fn->next = fee->next;
	    fn->prev = fe->prev;
	    if(fe == f->fedges || fee == f->fedges)
		f->fedges = fn;	/* preserve headiness */
	    unfedge(fee);
	    if(fee != fe)
		unfedge(fe);
	    if(fn->prev == fn->next) {
		/*
		 * This face became degenerate -- toss it.
		 */
		if (debug)
			printf("# degenerate face %d\n", (int)(f - Fs));
		deface(f);
		return;
	    }
	    fe = fn;
	}

	r = 1/sqrt(r);
	if(n.x < 0 ||  (n.x == 0 && n.y < 0) || (n.y == 0 && n.z < 0))
	    r = -r;		/* Canonicalize */
	fe->n.x = n.x*r;  fe->n.y = n.y*r;  fe->n.z = n.z*r;
    } while((fe = fe->next) != f->fedges);
}

/*
 * Delete a face, erasing all edges.
 */
static void deface(F *f)
{
    Fe *fe, *fee;

    fe = f->fedges;
    if(fe == NULL)
	return;

    do {
	fee = fe->next;
	unfedge(fe);
	fe = fee;
    } while(fe != f->fedges);
    f->fedges = NULL;
    tossedf++;
}

static int vcmp(V **p, V **q)
{
    V *vp, *vq;
    float d;

    vp = *p;
    vq = *q;
    d = vp->p.x - vq->p.x;
    if(d < 0) return -1;
    if(d > 0) return 1;
    d = vp->p.y - vq->p.y;
    if(d < 0) return -1;
    if(d > 0) return 1;
    d = vp->p.z - vq->p.z;
    if(d < 0) return -1;
    if(d > 0) return 1;
    if(vp->p.w == vq->p.w) return 0;
    return(vp->p.w < vq->p.w ? -1 : 1);
}

static void vmerge(void)
{
    V **vp;
    int i, j;
    V *a, *b;
    int nexti;

    vp = NewN(V *, nv);
    for(i = 0, a = Vs; i < nv; i++) {
	a->ref = 0;
	vp[i] = a++;
    }
    qsort(vp, nv, sizeof(V *), (int (*)())vcmp);

    /*
     * Now all matches will occur within X-runs
     */
    for(i = 0; i < nv; i = nexti ? nexti : i+1) {
	nexti = 0;
	a = vp[i];
	if(a->ref)
	    continue;
	for(j = i; ++j < nv; ) {
	    b = vp[j];
	    if(b->ref)
		continue;
	    if(b->p.x - a->p.x > EPS_POINT)
		break;

	    if(fabs(a->p.y - b->p.y) < EPS_POINT
			&& fabs(a->p.z - b->p.z) < EPS_POINT
			&& (vdim == 3 || fabs(a->p.w - b->p.w) < EPS_POINT)) {
		if (debug)
		    printf("# Vtx %d->%d\n", b->index, a->index);
		b->index = a->index;
		b->ref++;
	    } else if(!nexti)
		nexti = j;
	}
    }
    free(vp);
}

/*
 * Add a geodesic, described by a string.
 */
static void add_geodesic(Geod *g, char *str)
{
    int ok;
    float r;

    VVINIT(g->pts, P, 20);
    ok = sscanf(str, "%d,%f,%f,%f,%f,%f,%f,%f",
	&g->faceno, &g->base.x, &g->base.y, &g->base.z,
	&g->dir.x, &g->dir.y, &g->dir.z, &g->maxlength);

    if(ok < 7) {
	fprintf(stderr, "Geodesic must have form:  faceno,X0,Y0,Z0,X1,Y1,Z1\n\
where X0,Y0,Z0 define the base point, X1,Y1,Z1 the direction,\n\
and faceno is the integer index of the face on which the base-point lies.\n");
	exit(1);
    }

    /* Convert X1,Y1,Z1 into direction vector. */
    VSUB(&g->dir, &g->base, &g->dir);
    r = VLENGTH(&g->dir);
    if(r)
	VSCALE(&g->dir, 1/r);
}

/* Generate 3x3 rotation matrix which takes va -> vb
 * Assumes va and vb both unit vectors.
 */
static void rotation(P *va, P *vb, float T[3][3])
{
    float adotb = VDOT(va, vb);
    float ab_1 = adotb - 1;
    float apb, apb2;
    int i, j;
    P aperpb;

    VSADD(vb, -adotb, va, &aperpb);
    apb2 = VDOT(&aperpb, &aperpb);
    if(apb2 == 0) {
	float dot;
	if(adotb >= 0) {
	    /* Either some vector is zero, or they're identical.  No rot'n. */
	    memset(T, 0, 9*sizeof(float));
	    T[0][0] = T[1][1] = T[2][2] = 1;
	    return;
	}
	/* Oppositely directed vectors.  Pick an arbitrary plane and
	 * rotate 180 degrees in it.
	 */
	if(fabs(va->x) < .7) aperpb.x = 1;
	else aperpb.y = 1;
	dot = VDOT(&aperpb, va);
	VSADD(&aperpb, dot, va, &aperpb);
	apb2 = VDOT(&aperpb, &aperpb);
    }
    apb = VDOT(&aperpb, vb) / apb2;
    for(i = 0; i < 3; i++) {
	float ai = (&va->x)[i];
	float api = (&aperpb.x)[i];
	for(j = 0; j < 3; j++) {
	    T[i][j] = (&va->x)[j] * (ai*ab_1 - api*apb)
		    + (&aperpb.x)[j] * (api*ab_1/apb2 + ai*apb);
	}
	T[i][i] += 1;
    }
}

/*
 * result = vector reflected in axis
 */
static void reflectline(P *axis, P *vector, P *result)
{
    float mag = VDOT(axis, axis);
    float along = VDOT(axis, vector);
    *result = *vector;
    VSCALE(result, -1);
    VSADD(result, 2*along/mag, axis, result);
}
    

/*
 * Carry this geodesic forward across one facet.
 * Update g->face to the new face, if any, or NULL if we encountered a boundary.
 * Update g->base and g->dir to be a point and direction on the new face.
 * Return pointer to the edge with which we found an intersection
 *  (with g->base presumably being some point on that edge).
 * Avoid considering an intersection with the edge that led us here, if any.
 *
 * Return a pointer to the edge we found the intersection with, NULL if none.
 */
static E *
advance_geodesic(Geod *g, E *avoid)
{
    Fe *fed, *bestfed = NULL;
    E *edge;
    P n;
    P lb, bestbase = { 0, }, bestdir = { 0, };
    float r, u, t, bestt;
    int i;
    struct point2 {
	float u, v;
    } lbase, ldir;

    if(g->face == NULL)
	return NULL;

    /* Test for intersections with all the edges on this face. */
    bestt = 1e20;
    fed = g->face->fedges;
    do {
	edge = fed->edge;
	if(edge != avoid) {
	    /* We have at hand:
	     * edge->v[1], the base point of the edge; let's call it
	     *			the origin of our local coordinate system
	     * edge->to,	"e", a unit vector pointing along the edge
	     * fed->n,		"N", the face surface normal as a unit vector
	     * Let's call local coordinates on this face "u" and "v",
	     * with "u" positive along the edge and "v" perp. to it.
	     */

	    /* Construct the other basis vector, "n" = N cross e */
	    VCROSS(&fed->n, &edge->to, &n);

	    /* Transform base point g->base to local coords */
	    VSUB(&g->base, &edge->v[1]->p, &lb); /* Subtract local origin */

	    lbase.u = VDOT(&edge->to, &lb);
	    lbase.v = VDOT(&n, &lb);

	    /* Transform direction g->dir to local coords too.
	     * Since we want to squash everything down onto the face,
	     * let's just ignore the components in the direction of N.
	     */
	    ldir.u = VDOT(&edge->to, &g->dir);
	    ldir.v = VDOT(&n, &g->dir);

	    /*
	     * Where does lbase + t*ldir cross the v=0 axis?
	     * If this t comes closer than any other so far, take it.
	     */
	    if(ldir.v != 0) {
		t = -lbase.v/ldir.v;
		u = lbase.u + t * ldir.u;
		if(t > 0 && t < bestt && u >= 0) {
		    /* Final check: does our new point actually
		     * lie within the length of the edge?
		     */
		    P edgevec;
		    VSUB(&edge->v[1]->p, &edge->v[0]->p, &edgevec);

		    if(VDOT(&edgevec,&edgevec) >= u*u) {
			bestt = t;
			bestfed = fed;

			/* Compute new base point */
			VSADD(&edge->v[1]->p, u, &edge->to, &bestbase);
			/* Transform direction back into 3-space.
			 * It'll be almost the same, but possibly not quite, in
			 * case it didn't lie quite in the plane of the face.
			 */
			bestdir.x = edge->to.x * ldir.u + n.x * ldir.v;
			bestdir.y = edge->to.y * ldir.u + n.y * ldir.v;
			bestdir.z = edge->to.z * ldir.u + n.z * ldir.v;
		    }
		}
	    }
	}
        fed = fed->next;
    } while(fed != g->face->fedges);

    if(bestfed) {
	/* We did find some edge intersection. */
	/* Update base point and direction, and increment length. */
	P piece;
	VSUB(&bestbase, &g->base, &piece);
	g->length += VLENGTH(&piece);

	g->base = bestbase;
	g->dir = bestdir;

	/* Add base point to list of vertices on the geodesic. */
	*VVAPPEND(g->pts, P) = g->base;

	/* Whither next? Look for a different face on the same edge.
	 * The face-edges which share an edge are linked together via
	 * their "elink" fields, and edge->feds points to the first one.
	 */
	edge = bestfed->edge;
	fed = edge->feds;
	while(fed == bestfed)
	    fed = fed->elink;
	
	if(fed != NULL) {
	    /* We have a neighboring face on this edge. */
	    float T[3][3];
	    float dir[3];
 
	    /* Remember which face. */
	    g->face = fed->face;

	    /* Construct a rotation
	     * from plane of old face (bestfed->n)
	     * into plane of new face (fed->n).
	     */
	    rotation(&bestfed->n, &fed->n, T);
	    
	    /* dir = bestdir * T */
	    for(i = 0; i < 3; i++) {
		dir[i]  = bestdir.x*T[0][i] + bestdir.y*T[1][i]
			+ bestdir.z*T[2][i];
	    }
	    g->dir.x = dir[0];
	    g->dir.y = dir[1];
	    g->dir.z = dir[2];
	} else {
	    /* There's no neighboring face.  Must have reached a boundary. */
	    g->face = NULL;
	}

	/* Re-normalize direction in case it wasn't quite a unit vector. */
	r = VLENGTH(&g->dir);
	if(r != 0 && r != 1)
	    VSCALE(&g->dir, 1/r);

	return bestfed->edge;
    } else {
	/* Somehow, we didn't find an intersection with any of the edges
	 * of this face.  Maybe we were pointing in the wrong direction
	 * (away from the face rather than into it)?
	 */
	return NULL;
    }
}
   
static void
trace_geodesic(Geod *g)
{
    E *edge, *nextedge;
    int maxsteps = 1000000;
    float maxlength = 1e10;

    /* Bootstrap.
     * Take our facet-number, and initialize
     * g->face from it.
     */
    if(g->faceno < 0 || g->faceno >= nf) {
	fprintf(stderr,
	    "Can't believe geodesic starting from face #%d, ignoring it.\n",
	    g->faceno);
	return;		/* Forget it. */
    }

    /* Stuff the initial point into the geodesic path. */
    *VVAPPEND(g->pts, P) = g->base;

    g->face = &Fs[g->faceno];

    /* Interpret length limit.  How far shall we trace this geodesic?
     * Negative -> facet count, positive -> Euclidean length, zero -> default.
     */
    if(g->maxlength < 0) {
	maxsteps = -g->maxlength;
    } else if(g->maxlength > 0) {
	maxlength = g->maxlength;
    } else {
	maxsteps = 100;
    }

    edge = NULL;		/* Initially, no reason to avoid any edge. */
    do {
	nextedge = advance_geodesic(g, edge);

	if(nextedge == NULL && g->face != NULL) {
	    /* Maybe we ran into a facet with flipped orientation,
	     * and turned away from the facet rather than toward it.
	     * Try again, flipping the direction about the last edge we found.
	     */
	    reflectline(&edge->to, &g->dir, &g->dir);

	    nextedge = advance_geodesic(g, edge);
	}
	    
	/* Remember which edge we struck, so as to avoid considering
	 * an intersection with it on the next face.
	 */
	edge = nextedge;

	/* Could be smarter.  What if we've encountered a boundary,
	 * so advance_geodesic() left g->face equal to NULL?
	 * We could search the surface for another polygon which just
	 * doesn't happen to meet this one edge-to-edge -- as when two
	 * pieces of surface are irregularly butted up against each other --
	 * and use that to find a new face to walk over.
	 * But, not right now.
	 */
    } while(edge != NULL && --maxsteps > 0 && g->length < maxlength);
}
