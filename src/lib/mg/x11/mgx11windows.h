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
extern void Xmg_setx11display(Display *dpy);
extern int Xmg_initx11device( void );
extern void Xmg_initzrange( void );
extern int Xmg_openwin( char * );
extern void Xmg_setwin( Window );
extern void Xmg_setparent( Window );
extern void Xmg_poswin(int, int, int, int);
extern void Xmg_prefposwin(int, int, int, int);
extern void Xmg_sizewin(int, int);
extern void Xmg_minsize(int, int);
extern void Xmg_winconstrain();
extern void Xmg_titlewin(char *);
extern void Xmg_closewin( mgx11win * );
extern void Xmg_flush( void );
extern void Xmg_newdisplaylist( void );
extern void Xmg_sortdisplaylist( void );
extern void Xmg_add(int, int, void *, void *);
extern void Xmg_showdisplaylist( void );
extern void Xmg_getwinsize( int *, int *, int *, int * );
