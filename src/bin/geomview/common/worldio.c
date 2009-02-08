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


/* Authors: Stuart Levy, Tamara Munzner, Mark Phillips,
 * Celeste Fowler */

#include "mg.h"
#include "mgbuf.h"
#include "drawer.h"
#include "ui.h"
#include "geom.h"
#include "geomclass.h"
#include "instP.h"
#include "listP.h"
#include "streampool.h"
#include "comm.h"
#include "lang.h"
#include "main.h"
#include "transobj.h"
#include "ntransobj.h"
#include "worldio.h"
#include <string.h>
#include <stdlib.h>
#ifdef NeXT
#include <bsd/libc.h>
#else /* any other reasonable UNIX */
#include <unistd.h>
#endif


void 	set_emodule_path(char **dirs);
void 	set_load_path(char **dirs);

static void	save_dgeom(Pool *p, DGeom *dg, int doaliens);
static void	save_dgeom_geom(Pool *p, DGeom *dg, bool world, bool wrap, int cum);
static void	save_dview(Pool *p, DView *dv);
static int	nonidentity(Transform T);
static void	maybe_save_xform(Pool *p, char *cmd, char *name, Handle *, Transform T);
static char *	normalization_string(int n);
static char *	onoff_string(int n);

int
save_world(Pool *p, int id, bool comm, int wrap, int to_coords)
{
  Appearance *ap;
  int i, closeme = 0;
  FILE *fp;
  Transform T;
  DGeom *dg = NULL;
  Handle *h = NULL;

  if ((fp = PoolOutputFile(p)) == NULL) {
    return 0;
  }

#if 0
  /* Hack -- don't let StreamOut routines write Handle names. */
  /* "Temporary" workaround for library reading bug. -slevy */
  PoolSetOType(p, PO_DATA);
#endif

  if (id == NOID) {
    /* cH: hack, but otherwise we cannot save the universe, no aliens. */
    id = UNIVERSE;
  }
  
  if (!comm) {
    /* Write geometry */

    if (wrap) {
      if (id == UNIVERSE || ISCAM(id)) {
	PoolFPrint(p, fp, "{ # Base appearance\n");
	PoolIncLevel(p, 1);
	ApStreamOut(p, NULL, drawerstate.ap);
	PoolFPrint(p, fp, "# end base appearance\n");
	if (ISCAM(id)) {
	  PoolIncLevel(p, -1);
	  PoolFPrint(p, fp, "}\n");
	  return !ferror(fp);
	} else closeme = 1;
      }
    }
    if (id == UNIVERSE) {
      id = WORLDGEOM;
    }
    if (ISGEOM(id))
      dg = (DGeom *)drawer_get_object(id);
    if (id == WORLDGEOM) {
      if (wrap) {
	save_dgeom_geom(p, dg, true, wrap, to_coords);
      }
      PoolFPrint(p, fp, "{ LIST # World list \n");
      PoolIncLevel(p, 1);
      for(i = 1; i < dgeom_max; i++) {
	dg = (DGeom *)drawer_get_object(GEOMID(i));
	if (dg != NULL && dg->citizenship != ALIEN) 
	  save_dgeom_geom(p, dg, false, true, WORLDGEOM);
      }
      PoolIncLevel(p, -1);
      PoolFPrint(p, fp, "} #end of World List\n");
      if (wrap) {
	PoolIncLevel(p, -1);
	PoolFPrint(p, fp, "} # end of World and INST\n");
	if (closeme) {
	  PoolIncLevel(p, -1);
	  PoolFPrint(p, fp, "}\n");
	}
      }
    } else {
      save_dgeom_geom(p, dg, false, wrap, to_coords);
    }
    return !ferror(fp);
  }

  /* Write command language */


  PoolFPrint(p, fp, "(progn\n");
  PoolIncLevel(p, 1); /* Ensure embedded objects have { braces } */
  if (id == UNIVERSE || id == WORLDGEOM) {
    PoolFPrint(p, fp, "(merge-baseap ");
    ApStreamOut(p, NULL, drawerstate.ap);
    PoolFPrint(p, fp, ") # end base appearance\n");
    if (drawerstate.NDim > 0)
      PoolFPrint(p, fp, "(dimension %d)\n", drawerstate.NDim-1);
  }

  if (id != UNIVERSE && id != WORLDGEOM) {
    DObject *dobj = drawer_get_object(id);
    if (dobj) {
      if (ISGEOM(dobj->id))
	save_dgeom(p, (DGeom *)dobj, 1);
      else
	save_dview(p, (DView *)dobj);
    }
  } else {
 
    /* save data associated with the World dgeom */
    GeomGet( dgeom[0]->Item, CR_AXIS, T );
    GeomGet( dgeom[0]->Item, CR_AXISHANDLE, &h );
    maybe_save_xform(p, "xform-set", "worldgeom", h, T);
    maybe_save_xform(p, "xform-incr", "worldgeom",
		     dgeom[0]->incrhandle, dgeom[0]->Incr);

    GeomGet( dgeom[0]->Item, CR_APPEAR, &ap );
    if (ap != NULL) {
      PoolFPrint(p, fp, "(merge-ap \"worldgeom\" ");
      ApStreamOut( p, NULL, ap );
      PoolFPrint(p, fp, ") # end appearance \"worldgeom\"\n");
    }

    /* save the dgeoms.  Include aliens if id == UNIVERSE. */
    for (i=1; i<dgeom_max; ++i)
      if (dgeom[i] != NULL) save_dgeom(p, dgeom[i], id == UNIVERSE);

    /* save the dviews */
    for (i=0; i<dview_max; ++i)
      if (dview[i] != NULL) save_dview( p, dview[i] );
    
    /* save other stuff here ... */
    
    dg = (DGeom *)drawer_get_object(uistate.targetid);
    if (dg) {
      PoolFPrint(p, fp, "(ui-target \"%s\")\n", dg->name[1]);
    }
  }
  PoolIncLevel(p, -1);
  PoolFPrint(p, fp, ")\n");		/* terminates the (progn... */
  return !ferror(fp);
}


/* 
 * Save a DGeom as geometry.  Emit appearance and transform if wrap. 
 * if not 'world', also add geometry and close the braces.
 */
static void
save_dgeom_geom(Pool *p, DGeom *dg, bool world, bool wrap, int to_coords)
{
  FILE *fp = PoolOutputFile(p);
  Transform T;
  Handle *h = NULL;
  Geom *g = NULL;
  Appearance *ap = NULL;

  if (dg == NULL || dg->Item == NULL)
    return;

  if (wrap) {
    
    PoolFPrint(p, fp, "{ # %s\n", dg->name[1]);
    PoolIncLevel(p, 1);
    PoolFPrint(p, fp, "INST\n");
    
    drawer_get_transform(dg->id, T, to_coords);
    if (nonidentity(T) || h) {	/* Could say "transform" here */
      PoolFPrint(p, fp, "");
      TransStreamOut(p, h, T);	/* but TransStreamOut does that */
    }
    PoolFPrint(p, fp, "geom ");
    GeomGet(dg->Item, CR_APPEAR, &ap);
    if (ap) {
      ap = ApCopy(ap, NULL);
      ap->override = 0;
      if (ap->mat) ap->mat->override = 0;
      if (ap->lighting) ap->lighting->override = 0;
      PoolFPrint(p, fp, "");
      ApStreamOut(p, NULL, ap);
      ApDelete(ap);
    }
  }
  if (!world) {
    h = NULL;
    GeomGet(dg->Lgeom, CR_GEOM, &g);
    GeomGet(dg->Lgeom, CR_GEOMHANDLE, &h);
    GeomStreamOut(p, h, g);
    if (wrap) {
      PoolIncLevel(p, -1);
      PoolFPrint(p, fp, "} # end (geom and INST) %s\n", dg->name[1]);
    }
  }
}

static void
maybe_save_xform(Pool *p, char *cmd, char *name, Handle *h, Transform T)
{
  if (h || nonidentity(T)) {
    fprintf(PoolOutputFile(p), "(%s \"%s\" ", cmd, name);
    TransStreamOut(p, h, T);
    fprintf(PoolOutputFile(p), ")\n");
  }
}

static void
save_dgeom(Pool *p, DGeom *dg, int doaliens)
{
  Geom *g=NULL;
  Appearance *ap=NULL;
  FILE *fp = PoolOutputFile(p);
  Handle *h = NULL;
  Transform T;
  char name[32];
  char *truename = dg->name[1];

  sprintf(name, "[%s]", dg->name[0]);	/* Temporary name -- unlikely to conflict */
  
  if (dg->citizenship == ALIEN) {
    if (!doaliens)
      return;
    PoolFPrint(p, fp, "(new-alien");
  } else {
    PoolFPrint(p, fp, "(new-geometry");
  }
  fprintf(fp, " \"%s\"	# %s\n", name, truename);
  PoolIncLevel(p, 1);
  PoolFPrint(p, fp, "");
  GeomGet( dg->Lgeom, CR_GEOM, &g );
  GeomGet( dg->Lgeom, CR_GEOMHANDLE, &h );
  GeomStreamOut( p, h, g );
  PoolIncLevel(p, -1);
  PoolFPrint(p, fp, ") # end geometry \"%s\" %s\n", name, truename);

  GeomGet( dg->Item, CR_APPEAR, &ap );
  if (ap != NULL) {
    PoolFPrint(p, fp, "(merge-ap \"%s\" ", name);
    ApStreamOut( p, NULL, ap );
    PoolFPrint(p, fp, ") # end appearance \"%s\" %s\n", name, truename);
  }

  TmIdentity(T);
  GeomGet( dg->Item, CR_AXIS, T );
  GeomGet( dg->Item, CR_AXISHANDLE, &h );
  maybe_save_xform(p, "xform-set", name, h, T);

  if (dg->NDT) {
    PoolFPrint(p, fp, "(ND-xform-set \"%s\" ", name);
    NTransStreamOut(p, NULL, dg->NDT);
    PoolFPrint(p, fp, ")\n");
  }

  PoolFPrint(p, fp, "(bbox-draw \"%s\" %s)\n",
	     name, onoff_string(dg->bboxdraw));
  if (dg->bboxap && dg->bboxap->mat) {
    Color *c = &(dg->bboxap->mat->edgecolor);
    PoolFPrint(p, fp, "(bbox-color \"%s\" %f %f %f)\n", name, c->r, c->g, c->b);
  }

  PoolFPrint(p, fp, "(normalization \"%s\" %s)\n", name,
	     normalization_string(dg->normalization));
  PoolFPrint(p, fp, "(name-object \"%s\" \"%s\")\n\n", name, truename);
}

static void
save_dview(Pool *p, DView *dv)
{
  char *name = dv->name[1];
  FILE *fp = PoolOutputFile(p);
  WnPosition wp;

  if (WnGet(dv->win, WN_CURPOS, &wp) > 0) {
    WnSet(dv->win, WN_PREFPOS, &wp, WN_END);
    PoolFPrint(p, fp, "(window default { position %d %d %d %d })\n",
	       wp.xmin,wp.xmax,wp.ymin,wp.ymax);
  }

  PoolFPrint(p, fp, "(camera \"%s\" ", name);
  CamStreamOut(p, dv->camhandle, dv->cam);
  PoolFPrint(p, fp, ") # camera \"%s\" ...\n", name);

  PoolFPrint(p, fp, "(backcolor \"%s\" %f %f %f)\n",
	     name, dv->backcolor.r,dv->backcolor.g,dv->backcolor.b);
  if (dv->cameradraw) {
    PoolFPrint(p, fp, "(camera-draw \"%s\" yes)\n", name);
  }

  if (dv->cluster != NULL) {
    cmap *cm;
    cent *ce;
    int i, j;
    PoolFPrint(p, fp, "(dimension %d)\n", drawerstate.NDim-1);

    PoolFPrint(p, fp, "(ND-axes \"%s\" \"%s\" %d %d %d %d)\n",
	    name, dv->cluster->name,
	    dv->NDPerm[0], dv->NDPerm[1], dv->NDPerm[2], dv->NDPerm[3]);

    if (dv->cluster->C2W != NULL) {
      PoolFPrint(p, fp, "(ND-xform-set \"%s\" ", name);
      TmNPrint(fp, dv->cluster->C2W);
      PoolFPrint(p, fp, ")\n");
    }

    PoolFPrint(p, fp, "(ND-color \"%s\" (\n", name);
    for(i = 0, cm = dv->NDcmap; i < dv->nNDcmap; i++, cm++) {
      PoolFPrint(p, fp, "\t( (");
      fputnf(fp, cm->axis->dim, cm->axis->v, 0);
      PoolFPrint(p, fp, ")\n");
      for(j = VVCOUNT(cm->cents), ce = VVEC(cm->cents,cent); --j >= 0; ce++) {
	PoolFPrint(p, fp, "\t\t%g\t", ce->v);
	fputnf(fp, 4, (float *)&ce->c, 0);
	fputc('\n', fp);
      }
      PoolFPrint(p, fp, "\t)\n");
    }
    PoolFPrint(p, fp, " ) ) # end ND-color \"%s\"\n", name);
  }

  PoolFPrint(p, fp, "(window \"%s\" ", name);
  WnStreamOut(p, NULL, dv->win);
  PoolFPrint(p, fp, ")\n");

  PoolFPrint(p, fp, "# end camera %s\n\n", name);
}

static int nonidentity(Transform T)
{
  return memcmp(T, TM_IDENTITY, sizeof(TM_IDENTITY));
}

static char *
normalization_string(int n)
{
  switch (n) {
  case NONE: return "none";
  case ALL: return "all";
  case EACH: return "each";
  default: return "???";
  }
}

static char *
onoff_string(int n)
{
  if (n) return "on";
  else return "off";
}


int worldio(HandleOps *ops, Pool *p, int to_coords, int id)
{
  int ok = -1;
  Transform T;
  TransformN *TN;
  bool wrap = true;
  
  /* no data attached to a handle has been saved yet */
  HandlesSetObjSaved(false);

  if (to_coords == SELF) {
    wrap = false;
  }
  if (ops == &CommandOps) {
    ok = save_world(p, id, true, wrap, to_coords);
  } else if (ops == &GeomOps) {
    if (ISTYPE(T_NONE, id)) {
      /* bounding-box requested */
      DGeom *dg;

      id = GEOMID(INDEXOF(id));
      if ((dg = (DGeom *)drawer_get_object(id)) != NULL && dg->bboxvalid) {
	Geom *bbox;
	GeomGet(dg->Lbbox, CR_GEOM, &bbox);
	if (bbox != NULL) {
	  ok = GeomStreamOut(p, NULL, bbox);
	}
      }
    } else {
      ok = save_world(p, id, false, wrap, to_coords);
    }
  } else if (ops == &CamOps && ISCAM(id)) {
    DView *dv = (DView *)drawer_get_object(id);
    if (dv) ok = CamStreamOut(p, dv->camhandle, dv->cam);
  } else if (ops == &WindowOps) {
    DView *dv = (DView *)drawer_get_object(ISCAM(id) ? id : FOCUSID);
    WnWindow *win;
    if (dv && dv->mgctx) {
      mgctxselect(dv->mgctx);
      if ((ok = mgctxget(MG_WINDOW, &win)) > 0)
	ok = WnStreamOut(p, NULL, win);
    }
  } else if (ops == &TransOps) {
    drawer_get_transform(id, T, to_coords);
    ok = TransStreamOut(p, NULL, T);
  } else if (ops == &NTransOps) {
    TN = drawer_get_ND_transform(id, to_coords);
    ok = NTransStreamOut(p, NULL, TN);
    TmNDelete(TN);
  }    
  fflush(PoolOutputFile(p));

  return ok;
}

static int
unique(char *new, char **already, int nalready)
{
  int i;
  if (new == NULL)
    return 0;
  for(i = 0; i < nalready; i++)
    if (strcmp(new, already[i]) == 0)
      return 0;
  return 1;
}

static void
setapath(LList *list, vvec *vvpath, void (*setfunc)(char **))
{
#define MAXDIRS 100
  int i, k;
  char *td, *dirs[MAXDIRS+1];
  for(i = 0; i < MAXDIRS && list; list = list->cdr) {
    if (list->car == NULL)
      continue;
    td = LSTRINGVAL(list->car);
    if (strcmp(td, "+") == 0) {
      for(k = 0; k < VVCOUNT(*vvpath) && i < MAXDIRS; k++) {
	td = VVEC(*vvpath, char *)[k];
	if (unique(td, dirs, i))
	  dirs[i++] = strdup(td);
      }
    } else {
      if (unique(td, dirs, i))
	dirs[i++] = td;
    }
  }
  dirs[i] = NULL;
  (*setfunc)(dirs);
#undef MAXDIRS
}

static void
set_vvpath(vvec *vv, char **ents)
{
  int i;
  for(i = VVCOUNT(*vv); --i >= 0; )
    free(VVEC(*vv, char *)[i]);

  VVCOUNT(*vv) = 0;
  while(ents && *ents)
    *VVAPPEND(*vv, char *) = strdup(*ents++);
  vvtrim(vv);
}

LDEFINE(set_load_path, LVOID,
	"(set-load-path      (PATH1 ... PATHN))\n\
	Sets search path for command, geometry, etc. files.  The PATHi\n\
	are strings giving the pathnames of directories to be searched.\n\
	The special directory name \"+\" is replaced by the existing path,\n\
	so e.g. (set-load-path (mydir +)) prepends mydir to the path.")
{
  LList *list;
  LDECLARE(("set-load-path", LBEGIN,
	    LLITERAL, LLIST, &list,
	    LEND));
  setapath(list, &vv_load_path, set_load_path);
  return Lt;
}

void set_load_path(char **dirs)
{
  int filepanel;

  set_vvpath(&vv_load_path, dirs);

  filedirs(dirs);			/* Inform OOGL/refcomm library */

  filepanel = ui_name2panel("Files");	/* Inform UI */
  if (ui_panelshown(filepanel))
    ui_showpanel(filepanel, 1);
}

LDEFINE(load_path, LLIST,
	"(load-path)\n\
	Returns the current search path for command, geometry, etc. files.\n\
	Note: to actually see the value returned by this function\n\
	you should wrap it in a call to echo: (echo (load-path)).\n\
	See also set-load-path.")
{
  /*
    For backward compatibility this command takes an optional argument
    which specifies a new load-path.  This will disappear in a future version.
  */
  LList *list = NULL;
  int i;
  LDECLARE(("load-path", LBEGIN,
	    LOPTIONAL,
	    LLITERAL, LLIST, &list,
	    LEND));

  if (list != NULL) {
    gv_set_load_path(list);
    return Lt;
  }

  for (i=0; i<load_path_count; ++i) {
    char *dir = strdup(load_path[i]);
    list = LListAppend(list, LNew(LSTRING, &dir));
  }
  return LNew(LLIST, &list);
}

LDEFINE(set_emodule_path, LVOID,
	"(set-emodule-path      (PATH1 ... PATHN))\n\
	Sets the search path for external modules.  The PATHi should\n\
	be pathnames of directories containing, for each module, the\n\
	module's executable file and a .geomview-<modulename> file\n\
	which contains an (emodule-define ...) command for that\n\
	module.  This command implicitly calls (rehash-emodule-path)\n\
	to rebuild the application brower from the new path setting.\n\
	The special directory name \"+\" is replaced by the existing path,\n\
	so e.g. (set-emodule-path (mydir +)) prepends mydir to the path.")
{
#define MAXDIRS 100
  LList *list;
  LDECLARE(("set-emodule-path", LBEGIN,
	    LLITERAL, LLIST, &list,
	    LEND));
  setapath(list, &vv_emodule_path, set_emodule_path);
  return Lt;
}

void set_emodule_path(char **dirs)
{
  set_vvpath(&vv_emodule_path, dirs);
  gv_rehash_emodule_path();
}

LDEFINE(rehash_emodule_path, LLIST,
	"(rehash-emodule-path)\n\
	Rebuilds the application (external module) browser by reading\n\
	all .geomview-* files in all directories on the emodule-path.\n\
	Primarily intended for internal use; any applications defined\n\
	by (emodule-define ...) commands outside of the .geomview-*\n\
	files on the emodule-path will be lost.  Does not sort the\n\
	entries in the brower; see (emodule-sort) for that.")
{
  int i;
  char pat[512], **files, **fp;
  LDECLARE(("rehash-emodule-path", LBEGIN,
	    LEND));

  gv_emodule_clear();
  for (i=0; i<emodule_path_count; ++i) {
    sprintf(pat, "%s/.geomview-*", emodule_path[i]);
    fp = files = ooglglob(pat);
    while (fp && *fp) {
      if (access(*fp, R_OK) == 0) {
	char *emod_dir_saved = uistate.emod_dir;
	uistate.emod_dir = emodule_path[i];
	loadfile(*fp, &CommandOps, 0);
	uistate.emod_dir = emod_dir_saved;
      }
      ++fp;
    }
    ooglblkfree(files);
    OOGLFree(files);
  }
  return Lt;
}

LDEFINE(emodule_path, LLIST,
	"(emodule-path)\n\
	Returns the current search path for external modules.\n\
	Note: to actually see the value returned by this function\n\
	you should wrap it in a call to echo: (echo (emodule-path)).\n\
        See also set-emodule-path.")
{
  LList *list = NULL;
  int i;
  LDECLARE(("emodule-path", LBEGIN,
	    LEND));

  for (i=0; i<emodule_path_count; ++i) {
    char *dir = strdup(emodule_path[i]);
    list = LListAppend(list, LNew(LSTRING, &dir));
  }
  return LNew(LLIST, &list);
}

LDEFINE(emodule_defined, LSTRING,
	"(emodule-defined \"modulename\")\n\
	If the given external-module name is known, returns the name of\n\
	the program invoked when it's run as a quoted string; otherwise\n\
	returns nil.  ``(echo (emodule-defined \"name\"))'' prints the string.")
{
  char *name;
  emodule *em;
  LDECLARE(("emodule-defined", LBEGIN,
	    LSTRING, &name,
	    LEND));
  return ui_emodule_index(name, &em) < 0 ? Lnil : LTOOBJ(LSTRING)(&em->text);
}

LDEFINE(all, LLIST,
	"(all geometry)  returns a list of names of all geometry objects.\n\
(all camera)	returns a list of names of all cameras.\n\
(all emodule defined)  returns a list of all defined external modules.\n\
(all emodule running)  returns a list of all running external modules.\n\
Use e.g. ``(echo (all geometry))'' to print such a list.")
{
  char *type, *subtype = "";
  int i, wantrunning;
  DGeom *dg;
  DView *dv;
  emodule *em;
  LList *l = NULL;

  LDECLARE(("all", LBEGIN,
	    LSTRING, &type,
	    LOPTIONAL,
	    LSTRING, &subtype,
	    LEND));
    
  if (strncmp(type, "geom", 4) == 0) {
    LOOPGEOMS(i, dg)
      l = LListAppend(l, LTOOBJ(LSTRING)(&dg->name[1]));
  } else if (strncmp(type,  "cam", 3) == 0) {
    LOOPVIEWS(i, dv)
      l = LListAppend(l, LTOOBJ(LSTRING)(&dv->name[1]));
  } else if (strncmp(type, "emodule", 7) == 0) {
    wantrunning = (strncmp(subtype, "run", 3) == 0);
    em = VVEC(uistate.emod, emodule);
    for(i = VVCOUNT(uistate.emod); --i >= 0; em++)
      if ((em->pid != 0) == wantrunning)
	l = LListAppend(l, LTOOBJ(LSTRING)(&em->name));
  } else {
    OOGLError(0, "all: expected \"geometry\" or \"camera\" or \"emodule defined\" or \"emodule running\", got: %s %s\n",
	      type, subtype);
    return Lnil;
  }
  return LNew(LLIST, &l);
}

LDEFINE(camera_prop, LVOID,
	"(camera-prop { geometry object }   [projective])\n\
	Specify the object to be shown when drawing other cameras.\n\
	By default, this object is drawn with its origin at the camera,\n\
	and with the camera looking toward the object's -Z axis.\n\
	With the \"projective\" keyword, the camera's viewing projection is\n\
	also applied to the object; this places the object's Z=-1 and Z=+1 at\n\
	near and far clipping planes, with the viewing area -1<={X,Y}<=+1.\n\
	Example:  (camera-prop { < cube } projective)")
{
  Keyword proj = NO_KEYWORD;
  GeomStruct *gs = NULL;

  LDECLARE(("camera-prop", LBEGIN,
	    LGEOM, &gs,
	    LOPTIONAL,
	    LKEYWORD, &proj,
	    LEND));

  if (proj != NO_KEYWORD && proj != PROJECTIVE_KEYWORD) {
    OOGLError(1, "Expected either \"%s\" or nothing, but got \"%s\".\n",
	      keywordname(PROJECTIVE_KEYWORD), keywordname(proj));
    return Lnil;
  }
  
  if (gs && gs->geom) {
    REFINCR(gs->geom);
    REFINCR(gs->h);
    GeomDelete(drawerstate.camgeom);
    drawerstate.camgeom = gs->geom;
    drawerstate.camproj = (proj == PROJECTIVE_KEYWORD);
    drawerstate.changed = true;
    return Lt;
  }

  return Lnil;
}

LDEFINE(write, LVOID,
	"(write {command|geometry|camera|transform|ntransform|window|bbox} FILENAME [ID|(ID ...)] [self|world|universe|otherID])\n\
	write description of ID in given format to FILENAME.  Last\n\
	parameter chooses coordinate system for geometry & transform:\n\
	self: just the object, no transformation or appearance (geometry only)\n\
	world: the object as positioned within the World.\n\
	universe: object's position in universal coordinates;\n\
	includes Worldtransform\n\
	other ID: the object transformed to otherID's coordinate system.\n\
\n\
	A filename of \"-\" is a special case: data are written to the\n\
	stream from which the 'write' command was read.  For external\n\
	modules, the data are sent to the module's standard input.\n\
	For commands not read from an external program, \"-\" means\n\
	geomview's standard output.  (See also the \"command\"\n\
	command.)\n\
\n\
	The ID can either be a single id or a parenthesized list of\n\
	ids, like \"g0\" or \"(g2 g1 dodec.off)\".")
{
  Pool *op, *p;
  HandleOps *ops;
  char *opsname;
  LObject *idobj;
  char *fname;
  Lake *hiawatha;
  int coords = UNIVERSE, val = 1, id;
  bool temppool = false;
  bool do_bbox = false;

  LDECLARE(("write", LBEGIN,
	    LLAKE, &hiawatha,
	    LSTRING, &opsname,
	    LSTRING, &fname,
	    LHOLD, LLOBJECT, &idobj,
	    LOPTIONAL,
	    LID, &coords,
	    LEND));
  
  p = POOL(hiawatha);

  if (strcmp(opsname, "bbox") == 0) {
    do_bbox = true;
    ops = &GeomOps;
  }
  if (!do_bbox && (ops = str2ops(opsname)) == NULL) {
    OOGLError(0, "write: expected one of command|geometry|camera|transform|ntransform|window, got \"%s\"", opsname);
    return Lnil;
  }

  if (fname[0] == '-') {
    if (PoolOutputFile(p)) {
      op = p;
    } else {
      op = PoolStreamTemp(fname, NULL,  stdout, 1, &CommandOps);
      temppool = true;
    }
  } else {
    op = PoolStreamTemp(fname, NULL, NULL, 1, &CommandOps);
    temppool = true;
  }

  if (op == NULL || PoolOutputFile(op) == NULL) {
    fprintf(stderr, "write: cannot open \"%s\": %s\n", fname, sperror());
    return Lnil;
  }

  if (idobj->type == LSTRING || idobj->type == LSYMBOL) {
    if (!LFROMOBJ(LID)(idobj, &id)) {
      fprintf(stderr, "write: expects ID or list of IDs in arg position 4\n");
      return Lnil;
    }
    if (do_bbox) {
      id = ID(T_NONE, INDEXOF(id));
    }
    val = (worldio(ops, op, coords, id) == 1);
  } else if (idobj->type == LLIST) {
    LList *list;
    for( list = LLISTVAL(idobj); list && list->car; list = list->cdr ) {
      if (!LFROMOBJ(LID)(list->car, &id)) {
	fprintf(stderr, "write: expects ID or list of IDs in arg position 2\n");
	return Lnil;
      }
      if (do_bbox) {
	id = ID(T_NONE, INDEXOF(id));
      }
      val &= (worldio(ops, op, coords, id) == 1);
    }
  }
  if (temppool) {
    PoolClose(op);
    PoolDelete(op);
  }
  if (!val) {
    fprintf(stderr, "write failed\n");
    return Lnil;
  } else
    return Lt;
}


/***************************************************/
/* Snapshot code ***********************************/
/***************************************************/

extern snap_entry snapshot_table[];

int
general_snapshot(char *fname, int id, DView *view, 
		 WnWindow *win, WnPosition *wp,
		 int (*function)(void))
{
  Camera *cam = NULL;
  const Appearance *ap;
  mgcontext *ctx;
  FILE *f = NULL;
  WnPosition vp;
  int mgspace;
  int failed;

  f = (fname[0] == '|') ? popen(fname+1, "wb") : fopen(fname, "wb");
  if (f == NULL)
    return -1;

  mgctxget(MG_CAMERA, &cam);
  mgctxget(MG_SPACE, &mgspace);
#if 0
  /* Copy so that changed flags are set */
  ap = ApCopy(mggetappearance(NULL), NULL);
#else
  ap = mggetappearance();
#endif
  mgctxget(MG_WINDOW, &win);
  win = WnCopy(win);
  vp.xmin = vp.ymin = 0;
  vp.xmax = wp->xmax - wp->xmin + 1;
  vp.ymax = wp->ymax - wp->ymin + 1;
  WnSet(win, WN_CURPOS, wp, WN_VIEWPORT, &vp, WN_END);
    
  function();
  ctx = mgctxcreate(MG_CAMERA, cam,
		    MG_APPEAR, ap,
		    MG_WINDOW, win,
		    MG_BACKGROUND, &view->backcolor,
		    MG_BUFFILE, f,
		    MG_SPACE, mgspace,
		    MG_END);
  mgreshapeviewport();	/* Make camera aspect match window */
  {
    /* Try to be as realistic as possible when dumping a snapshot.
     * Create a drawing context, install (hopefully) all the
     * attributes that draw_view() doesn't reset on its own,
     * plug the ctx into the camera, and force a redraw.
     * Then undo the subterfuge.
     */
    bool oldredraw = view->redraw;
    bool oldchanged = view->changed;
    mgcontext *oldctx = view->mgctx;
    view->mgctx = ctx;
    view->redraw = true;
    gv_draw(view->id);
    view->redraw = oldredraw, view->changed = oldchanged;
    view->mgctx = oldctx;
  }
  fflush(f);
    
  mgctxdelete(ctx);
#if 0
  ApDelete(ap);
#endif
  mgctxselect(view->mgctx);	/* Revert to previous device */
    
  failed = (fname[0] != '|') ? fclose(f) : pclose(f);
  return failed;
}


LDEFINE(snapshot, LINT,
	"(snapshot       CAM-ID     FILENAME [FORMAT [XSIZE [YSIZE]]])\n"
	"Save a snapshot of CAM-ID in the FILENAME (a string).")
{
  DView *dv;
  WnWindow *wn = NULL;
  WnPosition wp;
  int id, xsize = 0, ysize = 0;
  char *fname, *format;
  int i, failed = -1;

  if (snapshot_table[0].name)
    format = snapshot_table[0].name;
  else
    format = "ppm";
  LDECLARE(("snapshot", LBEGIN,
	    LID, &id,
	    LSTRING, &fname,
	    LOPTIONAL, LSTRING, &format,
	    LINT, &xsize,
	    LINT, &ysize,
	    LEND));
    
  if (!ISCAM(id) || 
      (dv = (DView *)drawer_get_object(id)) == NULL 
      || dv->mgctx == NULL)
    {
      OOGLError(0, "snapshot: id %d: no such camera", id);
      return LCopy(L1);
    }
  mgctxselect(dv->mgctx);
  mgctxget(MG_WINDOW, &wn);
  wp.xmin = wp.ymin = 0;  wp.xmax = wp.ymax = 399;  /* Avoid catastrophe */
  WnGet(wn, WN_CURPOS, &wp);

  if (xsize > 0 || ysize > 0) {
    int dx = wp.xmax - wp.xmin + 1;
    int dy = wp.ymax - wp.ymin + 1;
    if (xsize <= 0)
      xsize = (dx * ysize + dy/2) / dy;
    else if (ysize <= 0)
      ysize = (dy * xsize + dx/2) / dx;
    wp.xmax = wp.xmin + xsize - 1;
    wp.ymax = wp.ymin + ysize - 1;
  }

  for (i=0; snapshot_table[i].function!=NULL; i++)
    if (!strcmp(format, snapshot_table[i].name))
      {
	failed = snapshot_table[i].function(fname, id, dv, wn, &wp);
	break;
      }
  if (!strcmp(format, "ppm")) {
    failed = general_snapshot(fname, id, dv, wn, &wp, mgdevice_BUF);
  } else if (!strcmp(format, "ps")) {
    failed = general_snapshot(fname, id, dv, wn, &wp, mgdevice_PS);
  }
  if (failed == -1)
    OOGLError(0, "snapshot: unknown file format %s", format);
  return failed ? Lnil : Lt;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
