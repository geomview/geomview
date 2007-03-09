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

#include <stdio.h>
#include "commentP.h"
#include "streampool.h"
#include "handleP.h"

static char *
fbalanced(IOBFILE *file)
{
    int depth = 1;
    int bufsize = 10240;
    char *buf = OOGLNewNE(char, bufsize, "Comment data");
    char *bufp = buf;
    if (iobfexpectstr(file, "{")) return NULL;
    do {
	int c = EOF;

	if (bufp - buf >= bufsize - 2)
	    buf = OOGLRenewNE(char, buf, bufsize += 10240, "Comment data");
	while (bufp - buf < bufsize - 2) {
	  *bufp++ = c = iobfgetc(file);
	  if (c == '{' || c == '}') {
	    break;
	  }
	}
	switch(c) {
	    case '}': depth--; break;
	    case '{': depth++; break;
	}
    } while (depth > 0);
    *--bufp = '\0';
    return OOGLRenewNE(char, buf, strlen(buf)+1, "Comment data");
}

Geom *
CommentImport( Pool *p )
{
    char *str;
    Comment *comment;
    IOBFILE *file;

    if (p == NULL || (file = PoolInputFile(p)) == NULL)
	return NULL;

    if (strcmp(GeomToken(file), "COMMENT"))
	return NULL;

    comment = (Comment *)GeomCCreate(NULL, CommentMethods(), NULL);

    if ((str = iobftoken(file, 0)) == NULL) return NULL;
    comment->name = OOGLNewNE(char, strlen(str)+1, "Comment name");
    strcpy(comment->name, str);
    if ((str = iobftoken(file, 0)) == NULL) return NULL;
    comment->type = OOGLNewNE(char, strlen(str)+1, "Comment type");
    strcpy(comment->type, str);
    if (iobfnextc(file, 0) == '{' ) {
      comment->data = fbalanced(file); /* read until '}' */
    } else {
	if (iobfgetni(file, 1, &comment->length, 0) != 1) return NULL;
	if (comment->length == 0) return NULL;
	if (iobfexpectstr(file, " ")) return NULL;
	comment->data = OOGLNewNE(char, comment->length, "Comment data");
	if (iobfread(comment->data, comment->length, 1, file) != 1) return NULL;
    }
    return (Geom *)comment;
}

int
CommentExport( Comment *comment, Pool *pool )
{
    if (comment == NULL || pool == NULL || pool->outf == NULL)
	return 0;

    (void) CommentFSave(comment, pool->outf, "");
    return 1;
}
