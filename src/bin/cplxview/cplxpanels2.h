/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
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
/* Header file generated with fdesign. */

/**** Callback routines ****/

extern void functioncallback(FL_OBJECT *, long);
extern void helpcallback(FL_OBJECT *, long);
extern void quitcallback(FL_OBJECT *, long);
extern void sliderscallback(FL_OBJECT *, long);
extern void coordtypecallback(FL_OBJECT *, long);
extern void meshlabelcallback(FL_OBJECT *, long);
extern void rangelabelcallback(FL_OBJECT *, long);
extern void domaincallback(FL_OBJECT *, long);
extern void domaincallback(FL_OBJECT *, long);
extern void domaincallback(FL_OBJECT *, long);
extern void domaincallback(FL_OBJECT *, long);

extern void quitcplxhelpcallback(FL_OBJECT *, long);

extern void slidercallback(FL_OBJECT *, long);
extern void slidercallback(FL_OBJECT *, long);
extern void slidershidecallback(FL_OBJECT *, long);

extern void coordcallback(FL_OBJECT *, long);
extern void coordcallback(FL_OBJECT *, long);
extern void coordcallback(FL_OBJECT *, long);
extern void coordtypehidecallback(FL_OBJECT *, long);

extern void usercoordcallback(FL_OBJECT *, long);
extern void usercoordcallback(FL_OBJECT *, long);
extern void usercoordhidecallback(FL_OBJECT *, long);

extern void meshcallback(FL_OBJECT *, long);
extern void meshcallback(FL_OBJECT *, long);
extern void meshlabelhidecallback(FL_OBJECT *, long);

extern void rangecallback(FL_OBJECT *, long);
extern void rangecallback(FL_OBJECT *, long);
extern void rangecallback(FL_OBJECT *, long);
extern void rangelabelhidecallback(FL_OBJECT *, long);



/**** Forms and Objects ****/

extern FL_FORM *cplxmainpanel;

extern FL_OBJECT
        *functionbox,
        *messagebox,
        *helpbutton,
        *quitbutton,
        *funclabelbutton,
        *coordtypebutton,
        *meshlabelbutton,
        *rangelabelbutton,
        *x1box,
        *y1box,
        *x0box,
        *y0box;

extern FL_FORM *cplxhelppanel;

extern FL_OBJECT
        *cplxhelpbrowser,
        *quitcplxhelpbutton;

extern FL_FORM *sliderspanel;

extern FL_OBJECT
        *abox,
        *bbox,
        *slidershidebox;

extern FL_FORM *coordtypepanel;

extern FL_OBJECT
        *rectcoordbox,
        *polarcoordbox,
        *usercoordbox,
        *coordtypehidebox;

extern FL_FORM *usercoordpanel;

extern FL_OBJECT
        *suvbox,
        *tuvbox,
        *usercoordhidebox;

extern FL_FORM *meshlabelpanel;

extern FL_OBJECT
        *m1box,
        *m2box,
        *meshlabelhidebox;

extern FL_FORM *rangelabelpanel;

extern FL_OBJECT
        *realrangebutton,
        *imagrangebutton,
        *d4rangebutton,
        *hiderangelabelbutton;



/**** Creation Routine ****/

extern void create_the_forms();
