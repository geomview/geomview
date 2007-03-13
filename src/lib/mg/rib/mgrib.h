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
/*
 * MG context attributes specific to Renderman rib driver (mgrib)
 *
 */
#ifndef _GV_MGRIB_H_
#define _GV_MGRIB_H_

#define   MG_RIBWINID		300 /* token: int */
#define   MG_RIBBORN		301 /* token: int (boolean) */
#define   MG_RIBFILE		302 /* token: FILE* */

#define   MG_RIBLINEMODE	303 /* token: int */
#define   MG_RIBPOLYGON		304 /* possible value for MG_RIBLINEMODE */
#define   MG_RIBCYLINDER	305 /* possible value for MG_RIBLINEMODE */
#define   MG_RIBPRMANLINE	306 /* possible value for MG_RIBLINEMODE */

#define   MG_RIBFORMAT		307 /* token: int */
#define   MG_RIBASCII		308 /* possible value for MG_RIBFORMAT */
#define   MG_RIBBINARY		309 /* possible value for MG_RIBFORMAT */

#define   MG_RIBFILEPATH	MG_RIBDISPLAYNAME /* obsolete. token: char* */

#define   MG_RIBDISPLAY		311 /* token: int */
#define   MG_RIBTIFF		312 /* possible value for MG_RIBDISPLAY */
#define   MG_RIBFRAME		313 /* possible value for MG_RIBDISPLAY */

#define   MG_RIBDISPLAYNAME	314 /* token: char* */

#define   MG_RIBBACKING		315 /* token: int */
#define   MG_RIBDOBG		316 /* possible value for MG_RIBBACKING */
#define   MG_RIBNOBG		317 /* possible value for MG_RIBBACKING */

#define   MG_RIBSHADER		318 /* token: int */
#define   MG_RIBSTDSHADE	319 /* possible value for MG_RIBSHADER */
#define   MG_RIBEXTSHADE	320 /* possible value for MG_RIBSHADER */

#define   MG_RIBSCENE		321 /* RIB 1.0: scene name: char* */
#define   MG_RIBCREATOR		322 /* RIB 1.0: creator:    char* */
#define   MG_RIBFOR		323 /* RIB 1.0: user:       char* */
#define   MG_RIBDATE		324 /* RIB 1.0: date:       char* */

/* following are functions wich need to be accessed by main program */
void mgrib_flushbuffer();     /* flushes tokens from buffer to file */

#endif

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
