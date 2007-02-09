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

/* OOGL to VRML converter 
 * by Tamara Munzner
 */

#include <stdio.h>
#include <stdarg.h>
#include "geom.h"
#include "transform.h"
#include "polylistP.h"
#include "vectP.h"
#include "sphere.h"
#include "commentP.h"
#include "plutil.h"

#ifdef DEBUG
# undef DEBUG
# define DEBUG(expr)  (expr)
#else
# define DEBUG(expr)	/*nothing*/
#endif

FILE *f = NULL;
Appearance *ap = NULL;
int bezdice = 0;

struct linkap {
  Appearance *ap;
  struct linkap *prev;
};

struct linkap *toplap;

static void doap(Appearance *a);

static int indent = 0;

void fprintfindent(FILE *f, char *fmt, ...)
{
  va_list ap;

  fprintf(f, "%*s", indent, "");
  va_start(ap, fmt);
  vfprintf(f, fmt, ap);
  va_end(ap);
}

void fprintfindentinc(FILE *f, char *fmt, ...)
{
  va_list ap;

  fprintf(f, "%*s", indent, "");
  va_start(ap, fmt);
  vfprintf(f, fmt, ap);
  va_end(ap);
  indent += 2;
}

void fprintfdecindent(FILE *f, char *fmt, ...)
{
  va_list ap;

  indent -= 2;
  fprintf(f, "%*s", indent, "");
  va_start(ap, fmt);
  vfprintf(f, fmt, ap);
  va_end(ap);
  indent += 2;
}

#define PRINT_PADDING(f, padding) \
	fprintf((f), "%*s", (int)(indent + (padding)), "")

void
faceindex(PolyList *plist) {
  int i,j;
  Poly *pl = NULL;
  Vertex *base = plist->vl;
  for (i = 0, pl = plist->p; i < plist->n_polys; i++, pl++) {
    for (j = 0; j < pl->n_vertices; j++) {
      fprintf(f, " %d", (int) (pl->v[j]-base) );
    }
    fprintf(f, " -1");
    if (i != plist->n_polys-1) {
      fprintf(f, ",");
      if (i & 1) {
	fprintf(f, "\n");
	PRINT_PADDING(f, strlen("coordIndex ["));
      }
    }
  }
}

void
plisttoindface(Geom *pgeom)
{
  PolyList *plist = (PolyList *)pgeom;
  Poly *pl = NULL;
  Vertex *v = NULL;
  int i;
  int shading = toplap->ap->shading;

  /* Appearance */
  doap(toplap->ap);

  /* Geometry */
  fprintfindentinc(f, "geometry IndexedFaceSet {\n");
  fprintfindent(f, "solid FALSE\n"); /* 'cause we don't know */
  fprintfindent(f, "ccw TRUE\n");
  fprintfindent(f, "convex TRUE\n"); /* is this true? */
  fprintfindent(f, "normalPerVertex %s\n",
		    shading == APF_FLAT ? "FALSE" : "TRUE");
  fprintfindent(f, "colorPerVertex %s\n",
	 plist->geomflags & PL_HASVCOL ? "TRUE" : "FALSE");

  /* Vertices */
  fprintfindentinc(f, "coord Coordinate {\n");
  fprintfindent(f, "point [");
  for (i = 0, v = plist->vl; i < plist->n_verts; i++, v++) {
    fprintf(f, " %g %g %g", v->pt.x, v->pt.y, v->pt.z);
    if (i != plist->n_verts-1) {
      fprintf(f, ",");
      if (i & 1) {
	fprintf(f, "\n");
	PRINT_PADDING(f, strlen("point ["));
      }
    }
  }
  fprintfindent(f, "]\n");
  fprintfdecindent(f, "}\n");

  /* Normals */
  fprintfindentinc(f, "normal Normal {\n");
  fprintfindent(f, "vector [");
  if (shading == APF_FLAT) {
    PolyListComputeNormals(plist, PL_HASPN);
    for (i = 0, pl= plist->p; i < plist->n_polys; i++, pl++) {
      fprintf(f, " %g %g %g", pl->pn.x, pl->pn.y, pl->pn.z);
      if (i != plist->n_polys-1) {
	fprintf(f, ",");
	if (i & 1) {
	  fprintf(f, "\n");
	  PRINT_PADDING(f, strlen("vector ["));
	}
      }
    }
  } else if (shading == APF_SMOOTH) {
    PolyListComputeNormals(plist, PL_HASVN);
    for (i = 0, v = plist->vl; i < plist->n_verts; i++, v++) {
      fprintf(f, " %g %g %g", v->vn.x, v->vn.y, v->vn.z);
      if (i != plist->n_verts-1) {
	fprintf(f, ",");
	if (i & 1) {
	  fprintf(f, "\n");
	  PRINT_PADDING(f, strlen("vector ["));
	}
      }
    }
  }
  fprintf(f, " ]");
  fprintfdecindent(f, "}\n");

  /* Colors */
  if (plist->geomflags & PL_HASVCOL && plist->geomflags & PL_HASPCOL) {
    if (shading == APF_FLAT) { plist->geomflags &= ~PL_HASVCOL; }
    else { plist->geomflags &= ~PL_HASPCOL; }
  }
  /* Appearance diffuse color overrides object colors */

  /* We don't rely on the override bit, since Geomview never sets it
     when saving files.
   */

  if (toplap->ap->mat && toplap->ap->mat->valid & MTF_DIFFUSE) {
    plist->geomflags &= ~(PL_HASPCOL|PL_HASVCOL);
  }

  if (plist->geomflags & PL_HASPCOL) {
    fprintfindentinc(f, "color Color {\n");
    fprintfindent(f, "RGB [");
    for (i = 0, pl= plist->p; i < plist->n_polys; i++, pl++) {
      fprintf(f, " %g %g %g", pl->pcol.r, pl->pcol.g, pl->pcol.b);
      if (i != plist->n_polys-1) {
	fprintf(f, ",");
	if (i & 1) {
	  printf("\n");
	  PRINT_PADDING(f, strlen("RGB ["));
	}
      }
    }
    fprintf(f, " ]\n");
    fprintfdecindent(f, "}\n");
  } else if (plist->geomflags & PL_HASVCOL) {
    fprintfindentinc(f, "color Color {\n");
    fprintfindent(f, "RGB [");
    for (i = 0, v = plist->vl; i < plist->n_verts; i++, v++) {
      fprintf(f, " %g %g %g", v->vcol.r, v->vcol.g, v->vcol.b);
      if (i != plist->n_verts-1) {
	fprintf(f, ",");
	if (i & 1) {
	  printf("\n");
	  PRINT_PADDING(f, strlen("RGB ["));
	}
      }
    }
    fprintfindent(f, "]\n");
    fprintfdecindent(f, "}\n");
  }

  /* Connectivity, VRML 2 does not need extra-indices for colors and
     normals, it is able to use the vertex indices.
   */
  
  fprintfindent(f, "coordIndex [");
  faceindex(plist);
  fprintfindent(f, " ] # end coordIndex\n");
  fprintfdecindent(f, "} # end IndexedFaceSet\n"); 
}

void lineindex(Vect *pline) 
{
  int i,j;
  int n = 0;
  for (i = 0; i < pline->nvec; i++) {
    for (j = 0; j < pline->vnvert[i]; j++) {
      fprintf(f, " %d, ", n++);
    }
    fprintf(f, " -1,\n ");
  }
}

void
  plinetoindline(Geom *pgeom)
{
  Vect *pline = (Vect *)pgeom;
  HPoint3 *v = NULL;
  int i, j, n;

  /* Appearance */
  doap(toplap->ap);

  /* Vertices */
  fprintfindentinc(f, "geometry IndexedLineSet {\n");
  
  fprintfindentinc(f, "coord Coordinate {\n");
  fprintfindent(f, "point [\n");
  for (i = 0, v = pline->p; i < pline->nvert; i++, v++) {
    fprintf(f, " %g %g %g", v->x, v->y, v->z);
    if (i != pline->nvert-1) {
      fprintf(f, ",");
      if (i & 1) {
	fprintf(f,"\n");
	PRINT_PADDING(f, strlen("point ["));
      }
    }
  }
  fprintfindent(f, "\n");
  fprintfdecindent(f, "}\n");
  
  /* Colors */
  if (toplap->ap->mat && toplap->ap->mat->valid & MTF_EDGECOLOR)
    pline->ncolor = 0;  
  if (pline->ncolor >= 1) {
    if (pline->ncolor > 1)
      fprintfindent(f, "colorPerVertex TRUE\n");
    else
      fprintfindent(f, "colorPerVertex FALSE\n");
    fprintfindentinc(f, "color Color {\n");
    fprintfindent(f, "RGB [");
    for (i = 0; i < pline->ncolor; i++) {
      fprintf(f, " %g %g %g", pline->c[i].r, pline->c[i].g, pline->c[i].b);
      if (i != pline->ncolor-1) {
	fprintf(f, ",");
	if (i & 1) {
	  printf("\n");
	  PRINT_PADDING(f, strlen("RGB ["));
	}
      }
    }
    fprintf(f, " ]\n");
    fprintfdecindent(f, "}\n");
  }

  /* Connectivity */
  fprintfindent(f, "coordIndex [ \n");
  lineindex(pline);
  fprintfindent(f, " ] # end coordIndex \n "); 
  /* appearance edgecolor overrides per-vertex colors */
  if (pline->ncolor > 1) {
    fprintfindent(f, "colorIndex [\n");
    n = -1;
    for (i = 0; i < pline->nvec; i++) {
      if (pline->vncolor[i] == 1) n++;
      for (j = 0; j < pline->vnvert[i]; j++) {
	fprintf(f, " %d, ", n);
	if (pline->vncolor[i] > 1) n++;
      }
      fprintf(f, " -1,\n ");
    }
    fprintfindent(f, "] # end materialIndex \n ");   
  }
  fprintfdecindent(f, "} # end IndexedLineSet \n "); 
}

static double det(double (*T)[3])
{
  return (T[0][0]*T[1][1]*T[2][2] +
	  T[1][0]*T[2][1]*T[0][2] +
	  T[2][0]*T[0][1]*T[1][2] -
	  T[2][0]*T[1][1]*T[0][2] -
	  T[1][0]*T[0][1]*T[2][2] -
	  T[0][0]*T[2][1]*T[1][2]);
}

static double getaxis(double (*mat)[3], double *axis)
{
  double tmp[3][3], ortho[3], tortho[3], det, max = 0.0;
  int i, j, maxi = 0, maxj = 0;
  int i0, i1, i2, j0, j1;

  for (i = 0; i < 3; i++) {  
    tmp[i][i] = mat[i][i] - 1.0;
    for (j = 0; j < 3; j++) {
      if (i != j) {
	tmp[i][j] = mat[i][j];
      }
      i0 = (i+1) % 3;
      i1 = (i+2) % 3;
      j0 = (j+1) % 3;
      j1 = (j+2) % 3;
      det = tmp[i0][j0]*tmp[i1][j1] - tmp[i1][j0]*tmp[i0][j1];
      if (fabs(det) > fabs(max)) {
	max = det;
	maxi = i;
	maxj = j;
      }
    }
  }
  i0 = (maxi+1) % 3;
  i1 = (maxi+2) % 3;
  j0 = (maxj+1) % 3;
  j1 = (maxj+2) % 3;
  axis[maxj] = 1.0;
  det = max;
  if (fabs(det) < 1e-12) {
    axis[j0] = axis[j1] = 0.0;
    return 0.0;
  }
  axis[j0] = (+ tmp[i1][j1]*tmp[i0][maxj]
	      - tmp[i0][j1]*tmp[i1][maxj]) / det;
  axis[j1] = (- tmp[i1][j0]*tmp[i0][maxj]
	      + tmp[i0][j0]*tmp[i1][maxj]) / det;
  i0 = 0; i1 = 1; i2 = 2;
  if (fabs(axis[i0]) < fabs(axis[i1])) {
    i = i0; i0 = i1; i1 = i;
  }
  if (fabs(axis[i0]) < fabs(axis[i2])) {
    i = i0; i0 = i2; i2 = i;
  }
  if (fabs(axis[i1]) < fabs(axis[i2])) {
    i = i1; i1 = i2; i2 = i;
  }
  axis[0] /= axis[i0];
  axis[1] /= axis[i0];
  axis[2] /= axis[i0];
  ortho[i0] = 0.0;
  ortho[i2] = 1.0;
  if (fabs(axis[i1]) > 1e-12) {
    ortho[i1] = -axis[i2]/axis[i1];
  } else {
    ortho[i1] = 1.0;
  }
#define SCP(v, w) (v[0]*w[0]+v[1]*w[1]+v[2]*w[2])
#define NRM2(v) SCP(v,v)
  tortho[0] = SCP(mat[0], ortho);
  tortho[1] = SCP(mat[1], ortho);
  tortho[2] = SCP(mat[2], ortho);
  return acos(SCP(ortho,tortho)/NRM2(ortho));
}

void  printxform(Transform T)
{
  int i, j;
  double scale;
  double mat[3][3];
  double axis[3], angle;

  if (T && memcmp(T, TM_IDENTITY, sizeof(Transform)) != 0) {
    /*
     * Brain-Dead (tm) VRML 2.0 doesn't know about MatrixTransform.
     * That's very sad.
     *
     * We only support uniform scalings, rotations and translations
     * Anything fancy created by the transformer yields undefined
     * results.
     */
    for (i = 0; i < 3; i++)
      for (j = 0; j < 3; j++)
	mat[i][j] = T[i][j];
    scale = pow(det(mat), 1.0/3.0);
    for (i = 0; i < 3; i++) {
      for (j = 0; j < 3; j++) {
	mat[i][j] /= scale;
      }
    }
    angle = getaxis(mat, axis);
    fprintfindent(f, "scale %g %g %g\n", scale, scale, scale);
    fprintfindent(f, "rotation %g %g %g %g\n",
	   axis[0], axis[1], axis[2], angle);
    fprintfindent(f, "translation %g %g %g\n",
	   T[3][0], T[3][1], T[3][2]);
  }
}

static void doap(Appearance *a)
{
  if (a->mat && a->mat->valid) {
    Material *m = a->mat;
    float ka, kd, ks;
    ka = (m->valid & MTF_Ka) ? m->ka : 1.0;
    kd = (m->valid & MTF_Kd) ? m->kd : 1.0;
    ks = (m->valid & MTF_Ks) ? m->ks : 1.0;
    fprintfindentinc(f, "appearance Appearance {\n");
    fprintfindentinc(f, "material Material {\n");
    if (m->valid & MTF_AMBIENT) 
      fprintfindent(f, "ambientIntensity %g\n", ka);
    if (m->valid & MTF_DIFFUSE) 
      fprintfindent(f, "diffuseColor %g %g %g\n", 
	      kd*m->diffuse.r, kd*m->diffuse.g, kd*m->diffuse.b);
    if (m->valid & MTF_EMISSION) 
      fprintfindent(f, "emissiveColor %g %g %g\n", 
	      m->emission.r, m->emission.g, m->emission.b);
    if (m->valid & MTF_SPECULAR) 
      fprintfindent(f, "specularColor %g %g %g\n", 
	      ks*m->specular.r, ks*m->specular.g, ks*m->specular.b);
    if (m->valid & MTF_SHININESS) 
      fprintfindent(f, "shininess %g\n", m->shininess/128.0);
    if (m->valid & MTF_ALPHA) 
      fprintfindent(f, "transparency %g\n", 1.0 - m->diffuse.a);
    fprintfdecindent(f, "} #end Material \n");
    fprintfdecindent(f, "} # end Appearance\n");
  }
}

#if 0
static void dolight(Appearance *a)
{
  if (a->lighting) {
    LtLight **lp, *l;
    int i;
    LM_FOR_ALL_LIGHTS(a->lighting, i,lp) {
      l = *lp;
      fprintfindentinc(f, "PointLight {\n");
      fprintfindent(f, "on TRUE\n");
      fprintfindent(f, "color %g %g %g\n", 
	     l->color.r,
	     l->color.g,
	     l->color.b);
      fprintfindent(f, "intensity %g\n", l->intensity);
      fprintfindent(f, "location %g %g %g\n",
	     l->position.x,l->position.y,l->position.z);
      fprintfdecindent(f, "}\n");
    }
  }
}
#endif

void
pushap(Appearance *nap)
{
  struct linkap *newlap = (struct linkap *) malloc(1 * sizeof(struct linkap));
  newlap->ap = NULL;
  newlap->ap = ApCopy(nap, newlap->ap);
  newlap->prev = toplap;
  toplap = newlap;
}

void
popap()
{
  struct linkap *oldlap = toplap;
  toplap = oldlap->prev;
  ApDelete(oldlap->ap);
  free(oldlap);
}

static void
  traverse(Geom *where)
{
  Geom *new = NULL;
  Geom *tlist = NULL;
  Appearance *a, *nap = NULL;
  Transform T;
  char usename[16];
  char *name = NULL;
  static int unique = -1;

  if (!where) return;
  
  /* Appearances: We need certain values for our own internal use
     (shading, diffuse material) so we have to keep a stack of
     appearances around. But we refrain from printing out the entire
     contents of the appearance on top of the stack at every level to
     save both cycles and bandwidth. So we just print out new
     appearances in doap, secure that Separator nodes will isolate
     their effects. */
  GeomGet(where, CR_APPEAR, &a);
  if (a) {
    nap = ApMerge(a, toplap->ap, 0);
    pushap(nap);
  }

  name = GeomName(where);
  if (!strcmp(name, "polylist")) {

    /* Might have internal Materials, isolate with Separator */
    fprintfindentinc(f, "Shape {\n");
    plisttoindface(where);
    fprintfdecindent(f, "} # end Shape\n");  

  } else if (!strcmp(name, "vect")) {

    /* Might have internal Materials, isolate with Separator */
    fprintfindentinc(f, "Shape {\n");
    plinetoindline(where);
    fprintfdecindent(f, " } # end Shape \n");  

  } else if (!strcmp(name, "sphere")) {
    fprintfindent(f, " Sphere { radius %g }\n", SphereRadius( (Sphere *)where));

  } else if (!strcmp(name, "comment")) {
    if (!strcmp(((Comment *)where)->type, "WWWInline")) {
      fprintfindent(f, "Inline { url \"%s\" bboxSize 0 0 0 bboxCenter 0 0 0 }\n",
	      ((Comment *)where)->data);

    } else if (!strcmp(((Comment *)where)->type, "HREF")) {
      /* no-op. we've already dealt with it at level of its 
	 list or inst parent. */
    }

  } else if (!strcmp(name, "list")) {
    /* must lookahead due to fundamental structural difference 
       between weboogl and vrml:
       in weboogl, anchors affect siblings and parent. 
       in vrml, anchors affect children.
       immediate child might be HREF comment, 
       in which case we need to write out a WWWAnchor node up here
       not down inside of list when we would normally come across it.
       */
    Geom *child = NULL;
    char *newname;
    int anchor = 0;
    new = where;
    while(new) {
      GeomGet(new, CR_CAR, &child);
      newname = GeomName(child);
      if (newname && !strcmp(newname, "comment") && 
	  !strcmp( ((Comment *)child)->type, "HREF")) {
	fprintfindentinc(f, "Anchor { url \"%s\"\n", ((Comment *)child)->data);
	anchor = 1;
      }
      GeomGet(new, CR_CDR, &child);
      new = child;
    }

    /* done with lookahead, now really go through the list */
    new = where;
    while(new) {
      GeomGet(new, CR_CAR, &child);
      traverse(child);
      GeomGet(new, CR_CDR, &child);
      new = child;
    }
    if (anchor) fprintfdecindent(f, " } # end Anchor \n");

  } else if (!strcmp(name, "inst")) {
    GeomGet(where, CR_GEOM, &new);
    if (!new) return; 
    GeomGet(where, CR_TLIST, &tlist);
    if (!tlist) {

      /* a single xform */
      fprintfindentinc(f, "Transform {\n");
      GeomGet(where, CR_AXIS, &T);
      printxform(T);
      fprintfindentinc(f, "children [\n");
      traverse(new);
      fprintfdecindent(f, "] # children\n");
      fprintfdecindent(f, "} # Transform\n");

    } else {

      /* multiple xforms 
         We'll DEF the geometry for multiple USEs, but
	 we must output the first xform before DEFing the geometry.
	 NextTransform loops through the list of xforms, after the
	 cryptic but crucial set-up call to GeomIterate. 
       */

      GeomIter *it = GeomIterate( (Geom *)tlist, DEEP );
      NextTransform( it, T );
      fprintfindentinc(f, "Transform {\n");
      printxform(T);
      sprintf(usename, "instgeom%d", ++unique);
      fprintfindent(f, "children DEF %s Group { ", usename);
      traverse(new);
      fprintfdecindent(f, "} #end Group DEF\n");
      fprintfdecindent(f, "} #end Transform \n");

      while(NextTransform( it, T )) {
	fprintfindentinc(f, "Transform {\n");
	printxform(T);
	fprintfindent(f, "children USE %s\n", usename);
	fprintfdecindent(f, "} # Transform\n");
      }
    }

  } else {
    if ((!strcmp(name, "bezier") || !strcmp(name, "bezierlist")) && bezdice) 
      GeomDice(where, bezdice, bezdice);
    /* Convert to all other geometric primitives to OFF. */
    new = AnyToPL(where, TM_IDENTITY);
    /* Might have internal Materials, isolate with Separator */
    fprintfindentinc(f, "Shape {\n");
    plisttoindface(new);
    fprintfdecindent(f, "} # end Shape\n");  
  }

  if (a) {
    popap();
  }
}


int main(int argc, char *argv[]) {
  Geom *g;
  int flat = 1;

  f = stdout;

  while (argc > 1 && argv[1][0] == '-' && argv[1][1] != '\0') {
    if (argv[1][1] == 'b') {
      bezdice = atoi(argv[2]); argc--; argv++;
    } else if (argv[1][1] == 's') {
      flat = 0; 
    }
    argc--; argv++;
  }

  if(argc > 1 && argv[1][0] == '-' && argv[1][1] != '\0') {
    fprintf(stderr, "\
Usage: %s [filename]  -- convert OOGL file to VRML format\n\
Writes to standard output.  Reads from stdin if no file specified.\n",
	argv[0]);
    exit(1);
  }
  pushap(ApCreate(AP_SHADING, flat ? APF_FLAT : APF_SMOOTH, AP_END));
  if(argc <= 1) {
    argv[1] = "-";
  }
  g = strcmp(argv[1], "-") ? 
    GeomLoad(argv[1]) : GeomFLoad(iobfileopen(stdin), "standard input");
  fprintfindent(f, "#VRML V2.0 utf8\n\n");
  fprintfindentinc(f, "Group {\n");
  fprintfindentinc(f, "children [\n");
  traverse(g);
  fprintfdecindent(f, "] # children\n");
  fprintfdecindent(f, "} # Group\n");

  return 0;
}
