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
#include <stdlib.h>
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

void
faceindex(PolyList *plist) {
  int i,j;
  Poly *pl = NULL;
  Vertex *base = plist->vl;
  for (i = 0, pl = plist->p; i < plist->n_polys; i++, pl++) {
    for (j = 0; j < pl->n_vertices; j++) {
      fprintf(f, " %d, ", (int) (pl->v[j]-base) );
    }
    fprintf(f, " -1,\n ");
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
  
  /* Vertices */
  fprintf(f, " Coordinate3 { point [\n");
  for (i = 0, v = plist->vl; i < plist->n_verts; i++, v++) {
    fprintf(f, " %g %g %g,  ", v->pt.x, v->pt.y, v->pt.z);
  }
  fprintf(f, "] } \n");

  /* Normals */
  if (shading == APF_FLAT) {
    PolyListComputeNormals(plist, PL_HASPN);
    fprintf(f, "NormalBinding { value PER_FACE }\n");
    DEBUG(fprintf(stderr, " per face normals! \n"));
    fprintf(f, " Normal { vector [ ");
    for (i = 0, pl= plist->p; i < plist->n_polys; i++, pl++) {
      fprintf(f, " %g %g %g,  ", pl->pn.x, pl->pn.y, pl->pn.z);
    }
    fprintf(f, "] }\n");
  } else if (shading == APF_SMOOTH) {
    PolyListComputeNormals(plist, PL_HASVN);
    fprintf(f, "NormalBinding { value PER_VERTEX_INDEXED }\n");    
    DEBUG(fprintf(stderr, " per vertex normals! \n"));
    fprintf(f, " Normal { vector [ ");
    for (i = 0, v = plist->vl; i < plist->n_verts; i++, v++) {
      fprintf(f, " %g %g %g,  ", v->vn.x, v->vn.y, v->vn.z);
    }
    fprintf(f, "] }\n");
  }

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
    fprintf(f, "MaterialBinding { value PER_FACE }\n");
    fprintf(f, " Material { diffuseColor [ ");
    for (i = 0, pl= plist->p; i < plist->n_polys; i++, pl++) {
      fprintf(f, " %g %g %g,  ", pl->pcol.r, pl->pcol.g, pl->pcol.b);
    }
    fprintf(f, "] }\n");
  } else if (plist->geomflags & PL_HASVCOL) {
    fprintf(f, "MaterialBinding { value PER_VERTEX_INDEXED }\n");    
    fprintf(f, " Material { diffuseColor [ ");
    for (i = 0, v = plist->vl; i < plist->n_verts; i++, v++) {
      fprintf(f, " %g %g %g,  ", v->vcol.r, v->vcol.g, v->vcol.b);
    }
    fprintf(f, "] }\n");
  } else if (toplap->ap->mat && toplap->ap->mat->valid & MTF_DIFFUSE) {
    /* reset color to backstop */
    fprintf(f, "Material { diffuseColor [ %g %g %g ] } ", 
	    toplap->ap->mat->diffuse.r, toplap->ap->mat->diffuse.g,
	    toplap->ap->mat->diffuse.b);
  }

  /* Connectivity */
  /* wish we could DEF fields not just nodes, then wouldn't 
     waste bandwidth with 3 chunks of identical numbers */
  fprintf(f," IndexedFaceSet {\n ");
  fprintf(f, " coordIndex [ \n");
  faceindex(plist);
  fprintf(f, " ] # end coordIndex \n "); 
  if (shading != APF_FLAT) {
    fprintf(f, " normalIndex [ \n");
    faceindex(plist);
    fprintf(f, " ] # end normalIndex \n "); 
    if (plist->geomflags & PL_HASVCOL) {
      fprintf(f, " materialIndex [ \n");
      faceindex(plist);
      fprintf(f, " ] # end materialIndex \n "); 
    }
  }
  fprintf(f, " } # end IndexedFaceSet \n "); 
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

  /* Vertices */
  fprintf(f, " Coordinate3 { point [\n");
  for (i = 0, v = pline->p; i < pline->nvert; i++, v++) {
    fprintf(f, " %g %g %g,  ", v->x, v->y, v->z);
  }
  fprintf(f, "] } \n");
  
  
  /* Colors */
  if (toplap->ap->mat && toplap->ap->mat->valid & MTF_EDGECOLOR)
    pline->ncolor = 0;  
  if (pline->ncolor >= 1) {
    if (pline->ncolor > 1)
	fprintf(f, " MaterialBinding { value PER_VERTEX_INDEXED } ");
    fprintf(f, " Material { diffuseColor [ ");
    for (i = 0; i < pline->ncolor; i++) {
      fprintf(f, " %g %g %g,  ", pline->c[i].r, pline->c[i].g, pline->c[i].b);
    }
    fprintf(f, "] }\n");
  } else if (toplap->ap && toplap->ap->mat && 
	     toplap->ap->mat->valid & MTF_EDGECOLOR) {
    fprintf(f, " Material { diffuseColor [ %g %g %g ] }\n", 
	    toplap->ap->mat->edgecolor.r, toplap->ap->mat->edgecolor.g, 
	    toplap->ap->mat->edgecolor.b);
  }

  /* Connectivity */
  fprintf(f," IndexedLineSet {\n ");
  fprintf(f, " coordIndex [ \n");
  lineindex(pline);
  fprintf(f, " ] # end coordIndex \n "); 
  /* appearance edgecolor overrides per-vertex colors */
  if (pline->ncolor > 1) {
    fprintf(f, " materialIndex [\n");
    n = -1;
    for (i = 0; i < pline->nvec; i++) {
      if (pline->vncolor[i] == 1) n++;
      for (j = 0; j < pline->vnvert[i]; j++) {
	fprintf(f, " %d, ", n);
	if (pline->vncolor[i] > 1) n++;
      }
      fprintf(f, " -1,\n ");
    }
    fprintf(f, " ] # end materialIndex \n ");   
  }
  fprintf(f, " } # end IndexedLineSet \n "); 
}

void  printxform(Transform T)
{
  if (T && memcmp(T, TM_IDENTITY, sizeof(Transform)) != 0) {
    fprintf(f, " MatrixTransform { matrix \n");
    fputtransform(f, 1, &T[0][0], 0);
    fprintf(f, " } \n");
  }
}

void doap(Appearance *a)
{
  if (a->mat && a->mat->valid) {
    Material *m = a->mat;
    float ka, kd, ks;
    ka = (m->valid & MTF_Ka) ? m->ka : 1.0;
    kd = (m->valid & MTF_Kd) ? m->kd : 1.0;
    ks = (m->valid & MTF_Ks) ? m->ks : 1.0;
    fprintf(f, "Material { ");
    if (m->valid & MTF_EMISSION) 
      fprintf(f, "emissiveColor %g %g %g \n", 
	      m->emission.r, m->emission.g, m->emission.b);
    if (m->valid & MTF_AMBIENT) 
      fprintf(f, "ambientColor %g %g %g \n",
	      ka*m->ambient.r, ka*m->ambient.g, ka*m->ambient.b);
    if (m->valid & MTF_DIFFUSE) 
      fprintf(f, "diffuseColor %g %g %g \n", 
	      kd*m->diffuse.r, kd*m->diffuse.g, kd*m->diffuse.b);
    if (m->valid & MTF_SPECULAR) 
      fprintf(f, "specularColor %g %g %g \n", 
	      ks*m->specular.r, ks*m->specular.g, ks*m->specular.b);
    if (m->valid & MTF_SHININESS) 
      fprintf(f, "shininess %g ", m->shininess);
    if (m->valid & MTF_ALPHA) 
      fprintf(f, "transparency %g ", 1.0 - m->diffuse.a);
    fprintf(f, "} #end Material \n");
  }
  if (a->lighting) {
    LtLight **lp, *l;
    int i;
    LM_FOR_ALL_LIGHTS(a->lighting, i,lp) {
      l = *lp;
      fprintf(f, "PointLight { on TRUE ");
      fprintf(f, "color %g %g %g ", 
	      l->color.r * l->intensity, 
	      l->color.g * l->intensity, 
	      l->color.b * l->intensity);
      fprintf(f, "location %g %g %g }\n",
	      l->position.x,l->position.y,l->position.z);
    }
  }
}

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
    fprintf(f, "Separator {\n");
    doap(a);
    nap = ApMerge(a, toplap->ap, 0);
    pushap(nap);
  }

  name = GeomName(where);
  if (!strcmp(name, "polylist")) {

    /* Might have internal Materials, isolate with Separator */
    fprintf(f, "Separator {\n");
    plisttoindface(where);
    fprintf(f, " } # end Separator \n");  


  } else if (!strcmp(name, "vect")) {

    /* Might have internal Materials, isolate with Separator */
    fprintf(f, "Separator {\n");
    plinetoindline(where);
    fprintf(f, " } # end Separator \n");  

  } else if (!strcmp(name, "sphere")) {
    fprintf(f, " Sphere { radius %g }\n", SphereRadius( (Sphere *)where));

  } else if (!strcmp(name, "comment")) {
    if (!strcmp(((Comment *)where)->type, "WWWInline")) {
      fprintf(f, "WWWInline { name \"%s\" bboxSize 0 0 0 bboxCenter 0 0 0 }\n",
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
	fprintf(f, "WWWAnchor { name \"%s\"\n", ((Comment *)child)->data);
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
    if (anchor) fprintf(f, " } # end WWWAnchor \n");

  } else if (!strcmp(name, "inst")) {
    GeomGet(where, CR_GEOM, &new);
    if (!new) return; 
    GeomGet(where, CR_TLIST, &tlist);
    if (!tlist) {

      /* a single xform */
      fprintf(f, "Separator { \n");
      GeomGet(where, CR_AXIS, &T);
      printxform(T);
      traverse(new);
      fprintf(f, "} #end Separator\n");

    } else {

      /* multiple xforms 
         We'll DEF the geometry for multiple USEs, but
	 we must output the first xform before DEFing the geometry.
	 NextTransform loops through the list of xforms, after the
	 cryptic but crucial set-up call to GeomIterate. 
       */

      GeomIter *it = GeomIterate( (Geom *)tlist, DEEP );
      NextTransform( it, T );
      fprintf(f, "Separator { \n");
      printxform(T);
      sprintf(usename, "instgeom%d", ++unique);
      fprintf(f, "  DEF %s Separator { ", usename);
      traverse(new);
      fprintf(f, "  } #end DEF \n } #end Separator \n");

      while(NextTransform( it, T )) {
	fprintf(f, "Separator { \n");	
	printxform(T);
	fprintf(f, " USE %s\n } #end Separator \n", usename);
      }
    }

  } else {
    if ((!strcmp(name, "bezier") || !strcmp(name, "bezierlist")) && bezdice) 
      GeomDice(where, bezdice, bezdice);
    /* Convert to all other geometric primitives to OFF. */
    new = AnyToPL(where, TM_IDENTITY);
    /* Might have internal Materials, isolate with Separator */
    fprintf(f, "Separator { \n");	
    plisttoindface(new);
    fprintf(f, " } # end Separator \n");  
  }

  if (a) {
    fprintf(f, " } # end Separator \n");  
    popap();
  }
}


int main(int argc, char *argv[])
{
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
  fprintf(f, "#VRML V1.0 ascii\n");
  fprintf(f, "Separator {\n");
  fprintf(f, " ShapeHints {vertexOrdering COUNTERCLOCKWISE faceType CONVEX}\n");
  traverse(g);
  fprintf(f, "} # end final Separator\n");

  return 0;
}
