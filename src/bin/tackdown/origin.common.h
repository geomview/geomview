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
#ifndef ORIGIN_COMMON_H
#define ORIGIN_COMMON_H

/*
 * Routines beginning with "internals" are found in the common code.
 */
void internalsInit();
void internalsShow();
void internalsDone();
void internalsCancel();
void internalsQuit();

/*
 * These routines are to be found in the machine-specific code and will
 * be called from the common code 
 */

/* Disable the Show button */
void uiDisableShow();
/* Enable the Show button */
void uiEnableShow();

/* Disable the Done and Cancel buttons */
void uiDisableDoneCanel();
/* Enable the Done and Cancel buttons */
void uiEnableDoneCanel();

/* Put str1, str2, and str3 as the new instructions */
void uiSetInstructions(char *str1, char *str2, char *str3, char *str4);

/* Copy the name of the target object to name */
void uiGetTargetname(char *name);
/* Set the name of the target object to name */
void uiSetTargetname(char *name);

/* Display an error message (up to 3 lines long) */
void uiError(char *str1, char *str2, char *str3);

#endif
