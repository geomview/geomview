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


/* Authors: Stuart Levy, Tamara Munzner, Mark Phillips */

#include "lights.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "geom.h"
#include "list.h"
#include "geomclass.h"
#include "3d.h"
#include "pick.h"
#include "common.h"
#include "drawer.h"
#include "event.h"
#include "ui.h"
#include "motion.h"
#include "mouse.h"

#include "inst.h"

static Transform Tincr;
static int updateproc(Geom *g);
static int lightedit(Event *event);
static void normalize_color( Color *color, float *coeff );
static void set_ambient(Color *color, float intensity);
static void build_light_rack(LmLighting *lgt);
static void pick_light(Event *ev);

static void mouse2world(Point *axis, int objid);
void Rotation(double dx, double dy, Transform rot, int *target);

static void lxform(Transform t, Point3 axis);

typedef struct LightRack {
  Geom *list;
  Geom **lightgeoms;
  LtLight **lights;
  int lightcount;
  LmLighting *lgt;
} LightRack;

Geom *make_light(Point3 axis, Color *color);
static int countlights(LmLighting *lm);
static Geom *ray = NULL;
/*static int inited = 0; currently unused */
static LightRack lr;
static int lr_id = NOID;
static int apseq = -1;

static char raytxt[] = "VECT \
 10 20 0 \
 2 2 2 2 2 2 2 2 2 2 \
 0 0 0 0 0 0 0 0 0 0 \
 .050 .000 0	 .160 .000 10 \
 .040 .029 0	 .129 .094 10 \
 .015 .047 0	 .049 .152 10 \
-.015 .047 0	 -.049 .152 10 \
-.040 .029 0	 -.129 .094 10 \
-.050 .000 0	 -.160 .000 10 \
-.040 -.029 0	 -.129 -.094 10 \
-.015 -.047 0	 -.049 -.152 10 \
 .015 -.047 0	 .049 -.152 10 \
 .040 -.029 0	 .129 -.094 10";


void clight_init()
{
  Appearance *ap = drawerstate.ap;
  LmLighting *lgt;
  IOBFILE *f;

  f = iobfileopen(fmemopen(raytxt, strlen(raytxt), "rb"));
  if (f) {
    ray = GeomFLoad(f, "built-in light ray");
    if (! ray)
      printf("can't construct built-in light ray\n");
  }
  else
    printf("can't fstropen\n");
  iobfclose(f);

  lr.lights = NULL;
  lr.lightgeoms = NULL;
  lr.lightcount = 0;
  lr.list = NULL;

  ApGet(ap, AP_LGT, &lgt);
  if (lgt == NULL) {
    printf("current appearance has no lighting !!!\n");
  }
  apseq = drawerstate.apseq;
  build_light_rack(lgt); 

}


void colmult( c, scalar)
     Color *c;
     float scalar;
{
  c->r = c->r * scalar;
  c->g = c->g * scalar;
  c->b = c->b * scalar;
}



Color *
light_color()
{
  return &( lr.lights[uistate.current_light]->color );
}

float
light_intensity()
{
  return lr.lights[uistate.current_light]->intensity;
}

void set_light_intensity(float intens)
{
  lr.lights[uistate.current_light]->intensity = intens;
  if (uistate.current_light == 0)
    set_ambient( &(lr.lights[0]->color), lr.lights[0]->intensity );
  else {
    set_beam_color();
  }
  apseq = ++drawerstate.apseq;
  drawerstate.changed = true;
}

void set_beam_color()
{
  Color scaledcol;

  CoCopy( &lr.lights[uistate.current_light]->color, &scaledcol);
  colmult( &scaledcol, light_intensity());
  ApSet(lr.lightgeoms[uistate.current_light]->ap, 
	AP_MtSet, MT_EDGECOLOR, &scaledcol, MT_END, AP_END);
}

void set_light_color(Color *color)
{
  lr.lights[uistate.current_light]->color = *color;
  if (uistate.current_light == 0)
    /* ambient */
    set_ambient( &(lr.lights[0]->color), lr.lights[0]->intensity );
  else {
    /* not ambient --- also set color of light beam */
    set_beam_color();
  }
  apseq = ++drawerstate.apseq;
  drawerstate.changed = true;
}

void set_light(int lightno)
{
  uistate.current_light = lightno;
  set_light_display(lightno);
}

int light_count()
{
  return lr.lightcount;
}

static void
set_ambient(Color *color, float intensity)
{
  Color scaledcolor;

  CoCopy(color, &scaledcolor);
  colmult(&scaledcolor, intensity);
  ApSet(drawerstate.ap, AP_LmSet, LM_AMBIENT, &scaledcolor, LM_END, AP_END);
}

/*-----------------------------------------------------------------------
 * Function:	light_edit_mode
 * Description:	enter/exit light editing mode
 * Args:	edit: 1 means enter, 0 means exit, 2 means toggle
 * Returns:	
 * Author:	mbp
 * Date:	Thu Dec 26 21:12:52 1991
 * Notes:	
 */
void light_edit_mode(int edit)
{

 if (edit == 2)
   edit = uistate.lights_shown ? 0 : 1;

 if (edit) {
   show_lights();
 }
 else {
   unshow_lights();
 }
 ui_light_button();
}

void unshow_lights()
{
/*  int i; currently unused */

  if (lr_id != NOID) {
    gv_delete( lr_id );
    lr_id = NOID;
  }
  ui_remove_mode(LIGHTEDIT);
  uistate.lights_shown = 0;
}

void show_lights()
{
  static char *name = "light rack";
  GeomStruct gs;

  lights_changed_check(); /* rebuild lightrack if lights changed */
  RefIncr((Ref *)lr.list); 
  gs.geom = lr.list;
  gs.h = NULL;
  lr_id = gv_new_alien( name, &gs );
  ui_add_mode(LIGHTEDIT, lightedit, T_NONE);
  uistate.lights_shown = 1;
  gv_event_mode( LIGHTEDIT );
}

static void build_light_rack(LmLighting *lgt)
{
  Point3 axis;
  int lightno;
  int i;
  LtLight *light, **lp;
  static LtLight ambient;

  if (lr.lightgeoms) OOGLFree(lr.lightgeoms);
  if (lr.lights) OOGLFree(lr.lights);
  if (lr.list) GeomDelete(lr.list); 

  lr.list = GeomCreate("list", CR_END);
  lr.lightcount = countlights(lgt)+1;
  lr.lightgeoms = OOGLNewNE(Geom *, lr.lightcount, "lightgeoms array");
  lr.lights = OOGLNewNE(LtLight *, lr.lightcount, "lights array");

  lr.lightgeoms[0] = NULL;
  lr.lights[0] = &ambient;
  LmGet(lgt, LM_AMBIENT, &(ambient.color));
  normalize_color( &(ambient.color), &(ambient.intensity) );

  lightno = 1;
  LM_FOR_ALL_LIGHTS(lgt, i,lp) {
    light = *lp;
    axis.x = light->position.x;
    axis.y = light->position.y;
    axis.z = light->position.z;
    lr.lights[lightno] = light;
    lr.lightgeoms[lightno] = make_light(axis, &(light->color));
    lr.list = ListAppend(lr.list, lr.lightgeoms[lightno]);
    ++lightno;
  }
  if(lr_id != NOID) {
    RefIncr((Ref *)lr.list); 
    {
      GeomStruct gs;
      gs.geom = lr.list;
      gs.h = NULL;
      gv_geometry( drawer_id2name(lr_id), &gs );
    }
  }
  ui_lights_changed();
}

#define max(a,b) (a)>(b)?(a):(b)

static void
normalize_color( color, coeff )
    Color *color;
    float *coeff;
{
    *coeff = max(color->r, color->g);
    *coeff = max(color->b, *coeff);

    if( *coeff != 0.0 ) {
	color->r /= *coeff;
	color->g /= *coeff;
	color->b /= *coeff;
    }
}

static int countlights(LmLighting *lm)
{
  int n = 0, i;
  LtLight **lp;
  LM_FOR_ALL_LIGHTS(lm, i,lp) {
    n++;
  }
  return n;
}

Geom* make_light(Point3 axis, Color *color)
{
  Geom *g;
  Transform t;
  Appearance *ap;

  ap = ApCreate(AP_MtSet, MT_EDGECOLOR, color, MT_END, AP_END);
  lxform(t, axis);
  g = GeomCreate("inst", CR_GEOM, ray, CR_NOCOPY, CR_APPEAR, ap, CR_END);
  if (g == NULL) {
    fprintf(stderr, "Couldn't create inst\n");
    return NULL;
  }
  InstTransformTo((Inst *)g, t, NULL);
  return g;
}

static void lxform(Transform t, Point3 axis)
{
  Point3 x, y;
  static Point3 other1 = {0,0,1};
  static Point3 other2 = {0,1,0};
  Point3 *other;
  
  Pt3Unit(&axis);
  other = (Pt3Dot(&axis, &other1) < .5) ? &other1 : &other2;
  
  Pt3Cross(other, &axis, &x);
  Pt3Unit(&x);
  Pt3Cross(&axis, &x, &y);
  
  t[0][0] = x.x;
  t[0][1] = x.y;
  t[0][2] = x.z;
  t[0][3] = 0;
  
  t[1][0] = y.x;
  t[1][1] = y.y;
  t[1][2] = y.z;
  t[1][3] = 0;
  
  t[2][0] = axis.x;
  t[2][1] = axis.y;
  t[2][2] = axis.z;
  t[2][3] = 0;
  
  t[3][0] = 0;
  t[3][1] = 0;
  t[3][2] = 0;
  t[3][3] = 1;
}

#if LIGHTDEBUG
static char *
Point3String(Point3 *p)
{
  static char buf[80];
  sprintf(buf,"%f  %f  %f", p->x, p->y, p->z);
  return buf;
}
#endif

static int
lightedit(Event *event)
{
  float dx,dy;
  unsigned long int dt;
  Transform T;
  int target=WORLDGEOM;

  lights_changed_check(); /* rebuild lightrack if lights have changed */

  if (!uistate.lights_shown)
    show_lights();
  
  /* don't do anything with ambient light */
  if (uistate.current_light == 0) return 0; /* should this return 0 or 1 ? */
  
  switch (event->dev) {

  case ELEFTMOUSE:
    if (event->val > 0) {
      /* button went down */
      mousezero(event,NULL);
      TmIdentity(Tincr);
      drawer_updateproc( lr_id, NULL );
    }
    else if (event->val) {
      /* dragging while button down */
      mousedisp(event, &dx, &dy, &dt, &drawerstate.winpos);
      Rotation(dx, dy, T, &target);
      InstTransform((Inst *)lr.lightgeoms[uistate.current_light], T, NULL);
      PtTransform(T, &(lr.lights[uistate.current_light]->position),
		  &(lr.lights[uistate.current_light]->position));
#if LIGHTDEBUG
printf("light moved to %s\n", Point3String(&(lr.lights[uistate.current_light]->position)));
#endif
      apseq = ++drawerstate.apseq;
      drawerstate.changed = true;
    }
    else {
      /* button went up */
      mousedisp(event, &dx, &dy, &dt, &drawerstate.winpos);
      if ( (dx != 0) || (dy != 0) ) {
	Rotation(dx, dy, Tincr, &target);
	drawer_updateproc( lr_id, updateproc );
      }
      else
	drawer_updateproc( lr_id, NULL );
    }
    break;

  case EMIDDLEMOUSE:
  case ERIGHTMOUSE:
    pick_light(event);
    break;

  default:
    return 0;
    break;
  }
  return 1;
}

static int updateproc(Geom *g)
{
  /* don't do anything with ambient light */
  if (uistate.current_light == 0) return 42;

  InstTransform((Inst *)lr.lightgeoms[uistate.current_light], Tincr, NULL);
  PtTransform(Tincr, &(lr.lights[uistate.current_light]->position),
	      &(lr.lights[uistate.current_light]->position));
  apseq = ++drawerstate.apseq;
  drawerstate.changed = true;
  return 42;
}


static void pick_light(Event *ev)
{
  int i;
  DView *dv = dview[uistate.mousefocus];
  DGeom *dg = dgeom[INDEXOF(lr_id)];
  Transform V, T, Tnorm, Tmodel, Tnet, Tt;
  float xpick, ypick;
  Pick *pick = NULL;

  CamView(dv->cam, V); 
  /* Top-level modelling transform */
  GeomPosition( dv->Item, T );
  TmConcat( T,V, T );
  pick = PickSet(NULL, PA_THRESH, .02, PA_END);
  for (i=1; i<lr.lightcount; ++i) {
    TmIdentity(T);
    GeomPosition( dg->Item, Tmodel );
    GeomPosition( dg->Inorm, Tnorm );
    TmConcat( Tnorm,Tmodel, Tt );
    TmConcat( Tt,T, Tnet );	/* Now Tnet = complete geom-to-screen proj'n */
    mousemap(ev->x, ev->y, &xpick, &ypick, &drawerstate.winpos);
    if(GeomMousePick( lr.lightgeoms[i], pick, (Appearance *)NULL,
		      Tnet, NULL, NULL, xpick, ypick )) {
      uistate.current_light = i;
      set_light(i);
    }
  }
}

void add_light()
{
  LmLighting *lgt;

  ApGet(drawerstate.ap, AP_LGT, &lgt);
  LmAddLight( lgt, LtCreate(LT_END) );
  ++drawerstate.apseq;
  uistate.current_light = lr.lightcount; 
  lights_changed_check();
}


void delete_light()
{
  LmLighting *lgt;

  ApGet(drawerstate.ap, AP_LGT, &lgt);
  LmRemoveLight( lgt, lr.lights[uistate.current_light] );
  ++drawerstate.apseq;
  if ( uistate.current_light > (lr.lightcount - 2) ) {
    uistate.current_light--;
  }
  lights_changed_check();
}

void lights_changed_check()
{
  LmLighting *lgt;
/*  Appearance *ap; currently unused */

  if (drawerstate.apseq != apseq) {
    ApGet( drawerstate.ap, AP_LGT, &lgt );
    build_light_rack( lgt );
    apseq = drawerstate.apseq;    
    drawerstate.changed = true;
/*      
    if (uistate.lights_shown) {
      GeomStruct gs;
      gs.geom = lr.list;
      gs.h = NULL;
      gv_geometry( lr_id, &gs );
    }

      Not just unnecessary (happens in build_light_rack) but wrong: 
      this is a hidden GeomDelete! 
*/
  }
}

/* This is a complete hack.  This module should use the drawer_transform stuff,
 * but I'm too lazy to fix it now --njt
 */
static void
mouse2world(Point *axis, int objid)
{
    Transform C2W, T, Tinv;
    DObject *dobj;

    if((dobj = drawer_get_object(objid)) == NULL)
	return;

    CamGet( dview[uistate.mousefocus]->cam, CAM_C2W, C2W );
    PtTransform( C2W, axis, axis );

    if(ISCAM(objid)) {
	CamGet( ((DView *)dobj)->cam, CAM_W2C, Tinv );
	PtTransform( Tinv, axis, axis );
    } else if(((DGeom *)dobj)->citizenship == ORDINARY) {
	GeomGet(dgeom[0]->Item, CR_AXIS, T);
	/* Assert: dgeom[0]->Inorm uses the identity matrix */
	TmInvert(T, Tinv);
	PtTransform( Tinv, axis, axis );
    }
}
void
Rotation(double dx, double dy, Transform rot, int *target)
{
  double ang;
  Point axis;

  if (dx == 0 && dy == 0) {
    TmIdentity(rot);
    return;
  }

  axis.x = -dy;  axis.y = dx;  axis.z = 0;
  axis.w = 0;
  ang = atan( sqrt( dx*dx + dy*dy ) );

  mouse2world(&axis, *target);

  TmRotate( rot, ang, (Point3 *)(void *)&axis );
}
