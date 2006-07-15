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
/* Include this file after forms.h, but before Forms-designer-generated code. */

#if defined(FL_INCLUDE_VERSION) || defined(FL_FORMS_H)

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef XFORMS
# define XFORMS 1
#endif

#define foreground()	/* nothing */
#define winset(w)	fl_winset(w)
#define winpop()	(fl_winget() && XRaiseWindow(fl_display, fl_winget()))
#define	minsize(x, y)	fl_winminsize(fl_winget(), x, y)
#define	maxsize(x, y)	fl_winmaxsize(fl_winget(), x, y)

#if FL_INCLUDE_VERSION <= 75  /* Old XForms */

#define FL_INITIALIZE(name) { \
	static int argc = 1;	\
	static char *argv[] = {name, 0};	\
	fl_initialize(argv[0],argv[0], NULL, 0, &argc, argv);	\
    }

#define DrawButton DrawBtn      /* Avoid name conflict with XForms typedef! */
#define FLXWRAPPER(xfunc, formsfunc) \
 FL_OBJECT *xfunc(int kind, float x, float y, float w, float h, char *name) { \
   return formsfunc(kind, x, y, w, h, name); \
 }

#else /* Modern XForms */

#define	FL_INITIALIZE(name) { \
	static int argc = 1;			\
	static char *argv[] = {name, 0};		\
	fl_initialize(&argc, argv, argv[0], 0, 0);	\
    }

extern inline void flx_end_form() {
  FL_OBJECT *obj = NULL;
  if(fl_current_form)
    obj = fl_current_form->first;
  for( ; obj; obj = obj->next) {
    obj->y = fl_current_form->h - obj->y - obj->h;
    if(obj->col1 == 47) obj->col1 = FL_COL1;
    if(obj->col2 == 47) obj->col2 = FL_COL1;
  }
  fl_end_form();
}

#define fl_end_form() flx_end_form()

#endif

#else /* Non-XForms */

#define FL_INITIALIZE(name)	/* nothing */

#endif
