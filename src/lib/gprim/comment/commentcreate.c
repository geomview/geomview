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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

/*
 * Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips,
 * Nathaniel Thurston
 */

/*
 * Comment creation, editing, retrieval and deletion.
 */

#include "commentP.h"
#include "transobj.h"

void
CommentDelete(Comment *comment)
{
    if (comment) {
	if (comment->name) OOGLFree(comment->name);
	if (comment->type) OOGLFree(comment->type);
	if (comment->data) OOGLFree(comment->data);
    }
}

Comment *
CommentCopy(Comment *comment) 
{
  Comment *nc;
  int datalength = comment->length;

  if (datalength == 0) datalength = strlen(comment->data)+1;
  nc = OOGLNewE(Comment, "CommentCopy: Comment");
  GGeomInit(nc, comment->Class, comment->magic, NULL);
  nc->name = OOGLNewNE(char, strlen(comment->name)+1, "Comment name");
  nc->type = OOGLNewNE(char, strlen(comment->type)+1, "Comment type");
  nc->data = OOGLNewNE(char, datalength, "Comment data");
  strcpy(nc->name, comment->name);
  strcpy(nc->type, comment->type);
  nc->length = comment->length;
  strcpy(nc->data, comment->data);
  return(nc);
}

Comment *
CommentCreate (Comment *exist, GeomClass *classp, va_list *a_list)
{
    Comment *comment;
    int attr;
    int copy = 1;

    if (exist == NULL) {
	comment = OOGLNewE(Comment, "CommentCreate comment");
	GGeomInit (comment, classp, COMMENTMAGIC, NULL);
	comment->name = NULL;
	comment->type = NULL;
	comment->length = 0;
	comment->data = NULL;
    } else {
	/* Check that exist is an comment. */
	comment = exist;
    }

    while ((attr = va_arg (*a_list, int))) {
	switch(attr) {
	default:
	    if (GeomDecorate(comment, &copy, attr, a_list)) {
		OOGLError (0, "CommentCreate: Undefined option: %d", attr);
		if (exist == NULL) GeomDelete ((Geom *)comment);
		return NULL;
	    }
	}
    }

    return comment;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
