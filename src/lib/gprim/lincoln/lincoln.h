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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

/*	Winged edge data structure represented in Lincoln file format


		\               /	 
		 \             /
		  \           /
		   \         /
		 e00\       /e10
		     \     /
		      \   /
			v0
			|
			|
			|
        f0		|e                 f1
			|
			|
			|
			|
			v1
                       / \
                      /   \
                  e01/     \e11
                    /       \
                   /         \
                  /           \

*/

#define BUFSIZE 256

struct position  {
	float x,y,z,w;
		};

struct equation	{
	float x,y,z,w;
		};

/*
struct vertexchars	{  information will be stored here
	to give the characteristics of an vertex (e.g. pointtype, color,...) }
*/

struct vertex	{	
	struct position pos;
	struct edge *e;
	int order;
/*	struct vertexchars chars;		*/
		};

/*
struct edgechars	{  information will be stored here
	to give the characteristics of an edge (e.g. thickness, color,...) }
*/

struct edge	{
	struct vertex *v0,*v1;
	struct face   *f0,*f1;
	struct edge   *e00,*e01,*e10,*e11;
/*	struct edgechars chars;			*/
		};

struct facechars	{  
	int color;	};

struct face	{	
	struct equation eqn;
	struct edge *e;
	int order;
	struct facechars chars;			
		};

struct data    { char title[BUFSIZE]; int dim, nvertices, nedges, nfaces, nobjects ;} ;
