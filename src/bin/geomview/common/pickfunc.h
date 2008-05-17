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
#define PICKFUNC(name, body, NDbody)					\
  static inline LObject *pickbody(char *coordsys, char *id,		\
				  HPoint3 point, int pn,		\
				  HPoint3 vertex, int vn,		\
				  HPoint3 edge[2], int en,		\
				  HPoint3 face[], int fn,		\
				  int ppath[], int ppn,			\
				  int vi, int ei[2], int ein, int fi)	\
  {									\
    do {								\
      body;								\
    } while (false);							\
    return Lt;								\
  }									\
  static inline LObject *pickNDbody(char *coordsys, char *id,		\
				    float *point, int pn,		\
				    float *vertex, int vn,		\
				    float *edge, int en,		\
				    float *face, int fn,		\
				    int ppath[], int ppn,		\
				    int vi, int ei[2], int ein, int fi)	\
  {									\
    do {								\
      NDbody;								\
    } while (false);							\
    return Lt;								\
  }									\
  LObject *name(Lake *lake, LList *args)				\
  {									\
    char *coordsys = NULL, *id = NULL;					\
    float *varpoint = NULL;  int pn;					\
    float *varvertex = NULL; int vn;					\
    float *varedge = NULL; int en;					\
    float *varface = NULL; int fn;					\
    int *varppath = NULL; int ppn;					\
    int vi;								\
    int ei[2]; int ein = 2;						\
    int fi;								\
    LObject *res;							\
									\
    LDECLARE(("pick", LBEGIN,						\
	      LSTRING,			&coordsys,			\
	      LSTRING,			&id,				\
	      LHOLD, LVARARRAY, LFLOAT,	&varpoint, &pn,			\
	      LHOLD, LVARARRAY, LFLOAT,	&varvertex, &vn,		\
	      LHOLD, LVARARRAY, LFLOAT,	&varedge, &en,			\
	      LHOLD, LVARARRAY, LFLOAT,	&varface, &fn,			\
	      LHOLD, LVARARRAY, LINT,	&varppath, &ppn,		\
	      LINT,			&vi,				\
	      LHOLD, LARRAY, LINT,	ei, &ein,			\
	      LINT,			&fi,				\
	      LEND));							\
									\
    if (varpoint == NULL) {						\
      return Lnil;							\
    }									\
									\
    if (pn <= 4) {							\
      HPoint3 point;							\
      HPoint3 vertex;							\
									\
      if (varpoint) {							\
	point = *(HPoint3 *)varpoint;					\
      }									\
      if (varvertex) {							\
	vertex = *(HPoint3 *)varvertex;					\
      }									\
      res = pickbody(coordsys, id,					\
		     point, pn,						\
		     vertex, vn,					\
		     (HPoint3 *)varedge, en,				\
		     (HPoint3 *)varface, fn,				\
		     varppath, ppn, vi, ei, ein, fi);			\
    } else {								\
      res = pickNDbody(coordsys, id,					\
		       varpoint, pn,					\
		       varvertex, vn, varedge, en, varface, fn,		\
		       varppath, ppn, vi, ei, ein, fi);			\
    }									\
									\
    if (varpoint) OOGLFree(varpoint);					\
    if (varvertex) OOGLFree(varvertex);					\
    if (varedge) OOGLFree(varedge);					\
    if (varface) OOGLFree(varface);					\
    if (varppath) OOGLFree(varppath);					\
									\
    return res;								\
  }

/*
  Note: the "if (1)" business above is to prevent a warning
  about the following statement ("return Lt") not being reached
  if the body itself returns.
*/

/* Note: Don't use DEFPICKFUNC any more.  Use the newer PICKFUNC
 * instead.  DEFPICKFUNC is provided for backward compatibility.
 *
 * cH: the above comment seemingly is ignored by everybody ...
 */
#define DEFPICKFUNC(helpstr,						\
		    coordsys,						\
		    id,							\
		    point, pn,						\
		    vertex, vn,						\
		    edge, en,						\
		    face, fn,						\
		    ppath, ppn,						\
		    vi,							\
		    ei, ein,						\
		    fi,							\
		    body,						\
		    NDbody)						\
  static inline LObject *pickbody(char *coordsys, char *id,		\
				  HPoint3 point, int pn,		\
				  HPoint3 vertex, int vn,		\
				  HPoint3 edge[2], int en,		\
				  HPoint3 face[], int fn,		\
				  int ppath[], int ppn,			\
				  int vi, int ei[2], int ein, int fi)	\
  {									\
    do {								\
      body;								\
    } while (false);							\
    return Lt;								\
  }									\
  static inline LObject *pickNDbody(char *coordsys, char *id,		\
				    float *point, int pn,		\
				    float *vertex, int vn,		\
				    float *edge, int en,		\
				    float *face, int fn,		\
				    int ppath[], int ppn,		\
				    int vi, int ei[2], int ein, int fi)	\
  {									\
    do {								\
      NDbody;								\
    } while (false);							\
    return Lt;								\
  }									\
  LDEFINE(pick, LVOID, helpstr)						\
  {									\
    char *coordsys = NULL, *id = NULL;					\
    float *var##point = NULL;  int pn;					\
    float *var##vertex = NULL; int vn;					\
    float *var##edge = NULL; int en;					\
    float *var##face = NULL; int fn;					\
    int *var##ppath = NULL; int ppn;					\
    int vi;								\
    int ei[2]; int ein = 2;						\
    int fi;								\
    LObject *res;							\
									\
    LDECLARE(("pick", LBEGIN,						\
	      LSTRING,			&coordsys,			\
	      LSTRING,			&id,				\
	      LHOLD, LVARARRAY, LFLOAT,	&var##point, &pn,		\
	      LHOLD, LVARARRAY, LFLOAT,	&var##vertex, &vn,		\
	      LHOLD, LVARARRAY, LFLOAT,	&var##edge, &en,		\
	      LHOLD, LVARARRAY, LFLOAT,	&var##face, &fn,		\
	      LHOLD, LVARARRAY, LINT,	&var##ppath, &ppn,		\
	      LINT,			&vi,				\
	      LHOLD, LARRAY, LINT,	ei, &ein,			\
	      LINT,			&fi,				\
	      LEND));							\
									\
    if (var##point == NULL) {						\
      return Lnil;							\
    }									\
									\
    if (pn <= 4) {							\
      HPoint3 point;							\
      HPoint3 vertex = { 0.0, 0.0, 0.0 };				\
									\
      if (var##point) {							\
	point = *(HPoint3 *)var##point;					\
      }									\
      if (var##vertex) {						\
	vertex = *(HPoint3 *)var##vertex;				\
      }									\
      res = pickbody(coordsys, id,					\
		     point, pn,						\
		     vertex, vn,					\
		     (HPoint3 *)var##edge, en,				\
		     (HPoint3 *)var##face, fn,				\
		     var##ppath, ppn, vi, ei, ein, fi);			\
    } else {								\
      res = pickNDbody(coordsys, id,					\
		       var##point, pn,					\
		       var##vertex, vn, var##edge, en, var##face, fn,	\
		       var##ppath, ppn, vi, ei, ein, fi);		\
    }									\
									\
    if (var##point) OOGLFree(var##point);				\
    if (var##vertex) OOGLFree(var##vertex);				\
    if (var##edge) OOGLFree(var##edge);					\
    if (var##face) OOGLFree(var##face);					\
    if (var##ppath) OOGLFree(var##ppath);				\
									\
    return res;								\
  }

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */

  
