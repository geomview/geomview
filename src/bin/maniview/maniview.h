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
#ifndef HUI_H
#define HUI_H

#include "forms.h"
#include "controlpanel.h"

#define CHANGED 	1
#define NEW_SPACE	2
#define NEW_AP		4
#define SOFTSHADE	8
#define DIRDOM		16

#define LOAD_GROUP	1
#define LOAD_GEOM	2
#define LOAD_CAMGEOM	4
#define CHECK_PIPE	8

#define DIRDOM_MODE	1
#define USER_GEOM	2

extern void ui_message(char *s);

extern int changed;
extern int myindex, space;
extern DiscGrp *dg;
extern float atk[][7];
extern float radius[];
extern float depth[];
extern int softshade;
extern float bkgdblend;
extern int loadtype;
extern int tilemode;
extern float myscale;
extern FILE *ifp;

extern int mainplacement;
extern int helpplacement;
extern int infoplacement;
extern int loadplacement;
extern int saveplacement;
extern int displayplacement;
extern int dirdomplacement;
extern int enumplacement;
extern int quitplacement;

extern void loadgroup(char *filename);
extern FILE *get_input_fp( char *filename, int lt );

#endif /* HUI_H */

