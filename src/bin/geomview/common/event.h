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


/* Authors: Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef _EVENT_H
#define _EVENT_H

#include <stdio.h>
#include "common.h"

typedef struct {
  PFI motionproc;	/* current motion proc */
  char *epath;		/* pathname of external motion program*/
  FILE *efp;
  Pool *epool;		/* stream for communication to extern motion program */
  int epid;		/* pid of external motion pgm, or 0 if none */
} EventState;

extern EventState estate;

/*
 * For keyboard keys, dev number = ascii value
 *
 * Other dev number:
 */

#define ELEFTMOUSE	500
#define EMIDDLEMOUSE	501
#define ERIGHTMOUSE	502
/*
 * values for mouse events:
 *   1: button went down
 *  -1: button dragged while down
 *   0: button went up
 */

#define EDRAW		503

#define EMODEENTER	504
#define EMODEEXIT	505

#define EPICK		506
#define EUISELECT	507

#define PICKWORLD	0
#define PICKOBJECT	1

/* State of buttons, maintained in event.c and potentially usable by
 * motion or other modules.
 */
extern struct button {
	char shift;
	char ctrl;
	char left, middle, right;
} button;

extern void event_init();

extern void MainLoop();
extern void print_help();

extern float elapsed(float *since, float *nextsince);

#endif /* ! _EVENT_H */

