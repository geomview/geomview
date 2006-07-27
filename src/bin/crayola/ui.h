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
#ifndef UI_H
#define UI_H

/* Stop listening to user events */
void uiFreeze();

/* Resume listening to user events */
void uiThaw();

/* Returns nonzero if we are doing the named thing, zero otherwise */
int uiGet();
int uiSet();
int uiSetAll();
int uiEliminateColor();

/* Update the color display on the ui */
void uiChangeColor(ColorA *color);

/* Get the color from the ui */
void uiCurrentColor(ColorA *color);

/* Ask the user a question with two possible responses */
int uiQuery(char *ques1, char *ques2, char *ques3, char *res1, char *res2);

#endif
