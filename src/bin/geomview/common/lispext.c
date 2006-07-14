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

#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
#include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

/*
 * geomview custom lisp object types
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "ooglutil.h"
#include "drawer.h"
#include "lisp.h"
#include "lispext.h"
#include "streampool.h"
#include "handleP.h"
#include "camera.h"
#include "geom.h"
#include "appearance.h"
#include "window.h"
#include "transform.h"
#include "fsa.h"
#include "lang.h"

extern HandleOps TransOps, GeomOps, CamOps, WindowOps;

LObject *L0, *L1;

static Fsa lang_fsa = NULL;
#define REJECT -1

/************************************************************************
 CAMERA LISP OBJECT
 ************************************************************************/
static CameraStruct *camcopy(CameraStruct *old)
{
  CameraStruct *new = OOGLNew(CameraStruct);
  if(old) *new = *old;
  else new->cam = NULL, new->h = NULL;
  if (new->cam) RefIncr((Ref*)(new->cam));
  if (new->h) RefIncr((Ref*)(new->h));
  return new;
}

int camfromobj(obj, x)
    LObject *obj;
    CameraStruct * *x;
{
  if (obj->type != LCAMERA) return 0;
  *x = LCAMERAVAL(obj);
  return 1;
}

LObject *cam2obj(x)
    CameraStruct * *x;
{
  CameraStruct *copy = camcopy(*x);
  return LNew( LCAMERA, &copy );
}

void camfree(x)
    CameraStruct * *x;
{
  if (*x) {
    if ((*x)->cam) CamDelete( (*x)->cam );
    if ((*x)->h) HandleDelete( (*x)->h );
  }
  OOGLFree(*x);
}

int cammatch(a, b)
    CameraStruct **a,**b;
{ 
  if ((*a)->h && ((*a)->h == (*b)->h)) return 1;
  if ((*a)->cam && ((*a)->cam == (*b)->cam)) return 1;
  return 0;
}

void camwrite(fp, x)
    FILE *fp;
    CameraStruct * *x;
{
  CamFSave( (*x)->cam, fp, "lisp output stream" );
}

void campull(a_list, x)
    va_list *a_list;
    CameraStruct * *x;
{
  *x = va_arg(*a_list, CameraStruct*);
}

LObject *camparse(Lake *lake)
{
  CameraStruct *new = OOGLNew(CameraStruct);
  new->h = NULL; new->cam = NULL;
  if (CamOps.strmin(POOL(lake), (Handle **)&(new->h),
		    (Ref **)(void *)&(new->cam)) == 0) {
    return Lnil;
  } else
    return LNew( LCAMERA, &new );
}

LType LCamerap = {
  "camera",
  sizeof(CameraStruct*),
  camfromobj,
  cam2obj,
  camfree,
  camwrite,
  cammatch,
  campull,
  camparse,
  LTypeMagic
  };

/************************************************************************
 * WINDOW LISP OBJECT 							*
 ************************************************************************/

static WindowStruct *wncopy(WindowStruct *old)
{
  WindowStruct *new = OOGLNew(WindowStruct);
  if(old) *new = *old;
  else new->wn = NULL, new->h = NULL;
  if (new->wn) RefIncr((Ref*)(new->wn));
  if (new->h) RefIncr((Ref*)(new->h));
  return new;
}

int wnfromobj(obj, x)
    LObject *obj;
    WindowStruct * *x;
{
  if (obj->type != LWINDOW) return 0;
  *x = LWINDOWVAL(obj);
  return 1;
}

LObject *wn2obj(x)
    WindowStruct * *x;
{
  WindowStruct *copy = wncopy(*x);
  return LNew( LWINDOW, &copy );
}

void wnfree(x)
    WindowStruct * *x;
{
  if (*x) {
    if ((*x)->wn) WnDelete( (*x)->wn );
    if ((*x)->h) HandleDelete( (*x)->h );
  }
  OOGLFree(*x);
}

int wnmatch(a, b)
    WindowStruct **a,**b;
{ 
  if ((*a)->h && ((*a)->h == (*b)->h)) return 1;
  if ((*a)->wn && ((*a)->wn == (*b)->wn)) return 1;
  return 0;
}

void wnwrite(FILE *fp, WindowStruct * *x)
{
  Pool *p = PoolStreamTemp("", NULL, fp, 1, &WindowOps);
  if(p == NULL)
    return;
  (void) WnStreamOut(p, (*x)->h, (*x)->wn);
  PoolDelete(p);
}

void wnpull(a_list, x)
    va_list *a_list;
    WindowStruct * *x;
{
  *x = va_arg(*a_list, WindowStruct*);
}

LObject *wnparse(Lake *lake)
{
  WindowStruct *new = OOGLNew(WindowStruct);
  new->h = NULL; new->wn = NULL;
  if (WindowOps.strmin(POOL(lake),(Handle **)&(new->h),
		       (Ref **)(void *)&(new->wn)) == 0) {
    return Lnil;
  } else
    return LNew( LWINDOW, &new );
}

LType LWindowp = {
  "window",
  sizeof(WindowStruct*),
  wnfromobj,
  wn2obj,
  wnfree,
  wnwrite,
  wnmatch,
  wnpull,
  wnparse,
  LTypeMagic
  };

/************************************************************************
 * GEOM LISP OBJECT 							*
 ************************************************************************/

static GeomStruct *geomcopy(GeomStruct *old)
{
  GeomStruct *new = OOGLNew(GeomStruct);
  if(old) *new = *old;
  else new->geom = NULL, new->h = NULL;
  if (new->geom) RefIncr((Ref*)(new->geom));
  if (new->h) RefIncr((Ref*)(new->h));
  return new;
}

int geomfromobj(obj, x)
    LObject *obj;
    GeomStruct * *x;
{
  if (obj->type != LGEOM) return 0;
  *x = LGEOMVAL(obj);
  return 1;
}

LObject *geom2obj(x)
    GeomStruct * *x;
{
  GeomStruct *copy = geomcopy(*x);
  return LNew( LGEOM, &copy );
}

void geomfree(x)
    GeomStruct * *x;
{
  if (*x) {
    if ((*x)->geom) GeomDelete( (*x)->geom );
    if ((*x)->h) HandleDelete( (*x)->h );
  }
  OOGLFree(*x);
}

int geommatch(a, b)
    GeomStruct **a,**b;
{ 
  if ((*a)->h && ((*a)->h == (*b)->h)) return 1;
  if ((*a)->geom && ((*a)->geom == (*b)->geom)) return 1;
  return 0;
}

void geomwrite(fp, x)
    FILE *fp;
    GeomStruct * *x;
{
  GeomFSave( (*x)->geom, fp, "lisp output stream" );
}

void geompull(a_list, x)
    va_list *a_list;
    GeomStruct * *x;
{
  *x = va_arg(*a_list, GeomStruct*);
}

LObject *geomparse(Lake *lake)
{
  GeomStruct *new = OOGLNew(GeomStruct);
  new->h = NULL; new->geom = NULL;
  if (GeomOps.strmin(POOL(lake), (Handle **)&(new->h), 
		     (Ref **)(void *)&(new->geom)) == 0) {
    return Lnil;
  } else {
    /* the following line should appear in all our parse routines.
       Actually, this whole procedure and others like it should
       be consolidated into one "ooglparse" thing... */
    if(new->h && !new->h->permanent) new->h = NULL;
    return LNew( LGEOM, &new );
  }
}

LType LGeomp = {
  "geometry",
  sizeof(GeomStruct*),
  geomfromobj,
  geom2obj,
  geomfree,
  geomwrite,
  geommatch,
  geompull,
  geomparse,
  LTypeMagic
  };

/************************************************************************
 * AP LISP OBJECT							*
 ************************************************************************/

static ApStruct *apcopy(ApStruct *old)
{
  ApStruct *new = OOGLNew(ApStruct);
  if(old) *new = *old;
  else new->ap = NULL, new->h = NULL;
  if (new->ap) RefIncr((Ref*)(new->ap));
#if 0
  /* no one uses ap handles yet, and callers don't bother
     to initialize the 'h' member to NULL, so don't do this
     yet */
  if (new->h) RefIncr((Ref*)(new->h));
#endif
  return new;
}

int apfromobj(obj, x)
    LObject *obj;
    ApStruct * *x;
{
  if (obj->type != LAP) return 0;
  *x = LAPVAL(obj);
  return 1;
}

LObject *ap2obj(x)
    ApStruct * *x;
{
  ApStruct *copy = apcopy(*x);
  return LNew( LAP, &copy );
}

void apfree(x)
    ApStruct * *x;
{
  if (*x) {
    if ((*x)->ap) ApDelete( (*x)->ap );
#if 0
    /* don't do the handle; see comment above */
    if ((*x)->h) HandleDelete( (*x)->h );
#endif
  }
  OOGLFree(*x);
}

int apmatch(a, b)
    ApStruct **a,**b;
{ 
#if 0
  /* don't do the handle; see comment above */
  if ((*a)->h && ((*a)->h == (*b)->h)) return 1;
#endif
  if ((*a)->ap && ((*a)->ap == (*b)->ap)) return 1;
  return 0;
}

void apwrite(FILE *fp, ApStruct * *x)
{
  ApFSave( (*x)->ap, (*x)->ap->handle, fp, "lisp output stream" );
}

void appull(a_list, x)
    va_list *a_list;
    ApStruct * *x;
{
  *x = va_arg(*a_list, ApStruct*);
}

LObject *apparse(Lake *lake)
{
  ApStruct *new = OOGLNew(ApStruct);
  new->h = NULL; new->ap = NULL;
  if (ApStreamIn(POOL(lake), &(new->h), &(new->ap)) == 0) {
    return Lnil;
  } else
    return LNew( LAP, &new );
}

LType LApp = {
  "ap",
  sizeof(ApStruct *),
  apfromobj,
  ap2obj,
  apfree,
  apwrite,
  apmatch,
  appull,
  apparse,
  LTypeMagic
  };


/************************************************************************
 * TRANSFORM LISP OBJECT						*
 ************************************************************************/

static TransformStruct *tmcopy(TransformStruct *old)
{
  TransformStruct *new = OOGLNew(TransformStruct);
  if(old) *new = *old;
  else new->h = NULL;
  if (new->h) RefIncr((Ref*)(new->h));
  return new;
}

int tmfromobj(obj, x)
    LObject *obj;
    TransformStruct * *x;
{
  if (obj->type != LTRANSFORM) return 0;
  *x = LTRANSFORMVAL(obj);
  return 1;
}

LObject *tm2obj(x)
    TransformStruct * *x;
{
  TransformStruct *copy = tmcopy(*x);
  return LNew( LTRANSFORM, &copy );
}

void tmfree(x)
    TransformStruct * *x;
{
  if (*x) {
    if ((*x)->h) HandleDelete( (*x)->h );
  }
  OOGLFree(*x);
}

int tmmatch(a, b)
    TransformStruct **a,**b;
{ 
  if ((*a)->h && ((*a)->h == (*b)->h)) return 1;
  if ((*a)->tm && ((*a)->tm == (*b)->tm)) return 1;
  return TmCompare( (*a)->tm, (*b)->tm, (float)0.0 );
}

void tmwrite(fp, x)
    FILE *fp;
    TransformStruct * *x;
{
  fputtransform(fp, 1, (float*)((*x)->tm), 0);
}

void tmpull(a_list, x)
    va_list *a_list;
    TransformStruct * *x;
{
  *x = va_arg(*a_list, TransformStruct*);
}

LObject *tmparse(Lake *lake)
{
  TransformStruct *new = OOGLNew(TransformStruct);
  new->h = NULL;
  if (TransOps.strmin(POOL(lake), (Handle **)&(new->h), (Ref **)(void *)&(new->tm)) == 0) {
    return Lnil;
  } else
    return LNew( LTRANSFORM, &new );
}

LType LTransformp = {
  "transform",
  sizeof(TransformStruct *),
  tmfromobj,
  tm2obj,
  tmfree,
  tmwrite,
  tmmatch,
  tmpull,
  tmparse,
  LTypeMagic
  };

/************************************************************************
 * N-D TRANSFORM LISP OBJECT						*
 ************************************************************************/

static TmNStruct *tmncopy(TmNStruct *old)
{
  TmNStruct *new = OOGLNew(TmNStruct);
  if(old) *new = *old;
  else new->tm = NULL, new->h = NULL;
  if (new->tm) RefIncr((Ref*)(new->tm));
  if (new->h) RefIncr((Ref*)(new->h));
  return new;
}

int tmnfromobj(obj, x)
    LObject *obj;
    TmNStruct * *x;
{
  if (obj->type != LTRANSFORMN) return 0;
  *x = LTRANSFORMNVAL(obj);
  return 1;
}

LObject *tmn2obj( TmNStruct **x )
{
  TmNStruct *copy = tmncopy(*x);
  return LNew( LTRANSFORMN, &copy );
}

void tmnfree(TmNStruct **x)
{
  if (*x) {
    if ((*x)->tm) TmNDelete( (*x)->tm );
    if ((*x)->h) HandleDelete( (*x)->h );
  }
  OOGLFree(*x);
}

int tmnmatch(register TmNStruct **a, register TmNStruct **b)
{ 
  if ((*a)->h && ((*a)->h == (*b)->h)) return 1;
  if ((*a)->tm && ((*a)->tm == (*b)->tm)) return 1;
  return 0;
}

void tmnwrite(FILE *fp, TmNStruct **x)
{
  TmNPrint( fp, (*x)->tm );
}

void tmnpull(a_list, x)
    va_list *a_list;
    TmNStruct * *x;
{
  *x = va_arg(*a_list, TmNStruct*);
}

LObject *tmnparse(Lake *lake)
{
  TransformN *T = TmNRead(lake->streamin);
  TmNStruct *new;
  if(T) {
    new = OOGLNew(TmNStruct);
    new->h = NULL;
    new->tm = T;
    return LNew( LTRANSFORMN, &new );
  }
  return Lnil;
}

LType LTransformNp = {
  "ntransform",
  sizeof(TmNStruct*),
  tmnfromobj,
  tmn2obj,
  tmnfree,
  tmnwrite,
  tmnmatch,
  tmnpull,
  tmnparse,
  LTypeMagic
  };


/************************************************************************
 * ID LISP OBJECT							*
 ************************************************************************/

int idfromobj(obj, x)
    LObject *obj;
    int *x;
{
  if (obj->type == LSTRING) {
    *x = drawer_idbyname(LSTRINGVAL(obj));
    if (*x == NOID) return 0;
  } else if (obj->type == LID) {
    *x = LIDVAL(obj);
  } else return 0;
  return 1;
}

LObject *id2obj(x)
    int *x;
{
  return LNew( LID, x );
}

static void idfree(int *x)
{}

int idmatch(a, b)
    int *a,*b;
{
  return drawer_idmatch(*a,*b);
}

void idwrite(fp, x)
    FILE *fp;
    int *x;
{
  fprintf(fp, "%s", drawer_id2name(*x));
}

LObject *idparse(Lake *lake)
{
  LObject *obj = LSexpr(lake);
  int id;
  if (obj->type == LSTRING) {
    id  = drawer_idbyname(LSTRINGVAL(obj));
    if (id == NOID) return Lnil;
    OOGLFree(LSTRINGVAL(obj));
    obj->type = LID;
    obj->cell.i = id;
    return obj;
  } else {
    LFree(obj);
    return Lnil;
  }
}
    
static void idpull(a_list, x)
    va_list *a_list;
    int *x;
{
  *x = va_arg(*a_list, int);
}

LType LIdp = {
  "id",
  sizeof(int),
  idfromobj,
  id2obj,
  idfree,
  idwrite,
  idmatch,
  idpull,
  LSexpr,
  LTypeMagic
  };


/************************************************************************
 * KEYWORD LISP OBJECT							*
 ************************************************************************/

int keywordfromobj(obj, x)
    LObject *obj;
    int *x;
{
  if (obj->type == LSTRING) {
    *x = (int)(long)fsa_parse(lang_fsa, LSTRINGVAL(obj));
    if (*x == REJECT) return 0;
  } else if (obj->type == LKEYWORD) {
    *x = LKEYWORDVAL(obj);
  } else return 0;
  return 1;
}

LObject *keyword2obj(x)
    int *x;
{
  return LNew( LKEYWORD, x );
}

static int keywordmatch(a, b)
    int *a,*b;
{
  return *a == *b;
}

void keywordwrite(fp, x)
    FILE *fp;
    int *x;
{
  fprintf(fp, "%s", keywordname(*x));
}

static void keywordfree(void *value)
{}

static void keywordpull(a_list, x)
    va_list *a_list;
    int *x;
{
  *x = va_arg(*a_list, int);
}

LObject *keywordparse(Lake *lake)
{
  LObject *obj = LSexpr(lake);
  int key;
  if (obj->type == LSTRING) {
    key  = (int)(long)fsa_parse(lang_fsa, LSTRINGVAL(obj));
    if (key == REJECT) return Lnil;
    OOGLFree(LSTRINGVAL(obj));
    obj->type = LKEYWORD;
    obj->cell.i = key;
    return obj;
  } else {
    LFree(obj);
    return Lnil;
  }
}

LType LKeywordp = {
  "keyword",
  sizeof(int),
  keywordfromobj,
  keyword2obj,
  keywordfree,
  keywordwrite,
  keywordmatch,
  keywordpull,
  LSexpr,
  LTypeMagic
  };


/************************************************************************
 * STRINGS LISP OBJECT							*
 * (a "strings" object is a string with possibly embedded spaces)	*
 ************************************************************************/

int stringsfromobj(obj, x)
    LObject *obj;
    char * *x;
{
  if (obj->type != LSTRING  && obj->type != LSTRINGS) return 0;
  *x = LSTRINGVAL(obj);
  return 1;
}

static LObject *stringsparse(Lake *lake)
{
  char *tok;
  int toklen, c, first=1;
  static char *delims = "()";
  vvec svv;

  VVINIT(svv, char, 80);

  *VVINDEX(svv, char, 0) = '\0';
  while ( LakeMore(lake,c) ) {
    tok = iobfdelimtok( delims, lake->streamin, 0 );
    toklen = strlen(tok);
    vvneeds(&svv, strlen(VVEC(svv,char))+toklen+2);
    if (!first) strcat(VVEC(svv,char), " ");
    else first = 0;
    strcat(VVEC(svv,char), tok);
  }
  VVCOUNT(svv) = strlen(VVEC(svv,char))+1;
  vvtrim(&svv);
  tok = VVEC(svv, char);
  return LNew( LSTRINGS, &tok );
}

LType LStringsp;		/* initialized in lispext_init() */

/**********************************************************************/
/**********************************************************************/

void lispext_init()
{
  LStringsp = *(LSTRING);
  LStringsp.name = "strings";
  LStringsp.fromobj = stringsfromobj;
  LStringsp.parse = stringsparse;

  {
    int zero=0, one=1;
    L0 = LNew( LINT, &zero );
    L1 = LNew( LINT, &one );
  }

  lang_fsa = fsa_initialize(NULL, (void*)REJECT);
  return;
}

void define_keyword(char *word, int value)
{
  fsa_install(lang_fsa, word, (void*)(long)value);
}

/* returns < 0 if asked to parse something that isn't a keyword. */
int parse_keyword(char *word)
{
  return (int)(long)fsa_parse(lang_fsa, word);
}
