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

#ifndef COMM_H
#define COMM_H

/*
 * comm_object() interprets a string to yield an object.
 * If the string is exactly the name of a file, open & read the file,
 * otherwise interpret it as something in that object's syntax.
 * The 'now' flag determines whether the file must be read immediately.
 *  COMM_NOW returns failure (0) if an object can't immediately be
 *	read from the file.  It reads at most one object from the file,
 *	then closes it.  If trying to guess the type of a file,
 *	you MUST use COMM_NOW.
 *  COMM_LATER reads an object if it can, but returns failure only
 *	if a file can't be opened.  However, it leaves the file open and
 *	listening for further data.  If opening something other than a plain
 *	file e.g. a named pipe, you MUST use COMM_LATER.
 */

#define	COMM_NOW	1
#define	COMM_LATER	0

extern HandleOps CommandOps, TransOps, NTransOps, GeomOps, CamOps, WindowOps;

extern void comm_init();

extern int comm_object(char *str, HandleOps *, Handle **hp, Ref **rp, int now);

extern int comm_read(char *kind, Pool *p);
extern int comm_route(char *str);	/* interpret str as command */
extern void gv_merge(HandleOps *ops, int id, Ref *window_or_camera);

		/* Maps string 'commands/geometry/transform/camera'
		 * into a HandleOps pointer, or NULL.
		 * Allows abbreviations.
		 */
extern HandleOps *str2ops(char *name);

extern void	loadfile(char *filename, HandleOps *defaultops, int guess);
extern emodule *emodule_run(emodule *em);
extern int	emodule_kill(emodule *em);
extern void	emodule_reap(emodule *em);

extern int	gv_debug;

#endif /* ! COMM_H */
