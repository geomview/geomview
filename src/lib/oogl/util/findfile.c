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

#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
#include "config.h"
#endif

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Geometry Technologies, Inc.";

/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

/* $Header: /home/mbp/geomview-git/geomview-cvs/geomview/src/lib/oogl/util/findfile.c,v 1.1 2000/08/15 16:33:25 mphillips Exp $ */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "ooglutil.h"

#if defined(unix) || defined(__unix)
# ifdef NeXT
#  include <fcntl.h>
# else
#  include <unistd.h>		/* needed for access() */
# endif
#else	/* Win32 */
# include <io.h>
# define R_OK  4		/* No Windows include file defines this?! */
#endif

#include <stdlib.h>


static char **dirlist = NULL;
static void dirprefix(char *file, char *dir);
char *envexpand(char *s);

/*-----------------------------------------------------------------------
 * Function:	filedirs
 * Description:	set the list of directories to search for files
 * Args:	dirs: NULL-terminated array of pointers to directory
 *		  strings
 * Author:	mbp
 * Date:	Wed Feb 12 14:09:48 1992
 * Notes:	This function sets the list of directories searched by
 *		findfile().   It makes an internal copy of these directories
 *		and expands all environment variables in them.
 */
void
filedirs(char *dirs[])
{
  char buf[1024], **p;
  int i,ndirs;

  if (dirlist) {
    for (p=dirlist; *p!=NULL; ++p) free(*p);
    OOGLFree(dirlist);
  }
  for (ndirs=0; dirs[ndirs]!=NULL; ++ndirs);
  dirlist = OOGLNewNE(char *,ndirs+1, "filedirs: dirlist");
  for (i=0; i<ndirs; ++i) {
    strcpy(buf, dirs[i]);
    envexpand(buf);
    dirlist[i] = strdup(buf);
  }
  dirlist[ndirs] = NULL;
}


/*-----------------------------------------------------------------------
 * Function:	getfiledirs
 * Description:	return the list of dirs set by the last call to filedirs()
 * Author:	mbp
 * Date:	Wed Feb 12 14:09:48 1992
 */
char **
getfiledirs()
{
  return dirlist;
}


/*-----------------------------------------------------------------------
 * Function:	findfile
 * Description:	resolve a filename into a pathname
 * Args:	*superfile: containing file
 *		*file: file to look for
 * Returns:	pointer to resolved pathname, or NULL if not found
 * Author:	mbp
 * Date:	Wed Feb 12 14:11:47 1992
 * Notes:
 *
 * findfile() tries to locate a (readable) file in the following way.
 *
 *    If file begins with a '/' it is assumed to be an absolute path.  In
 *    this case we expand any environment variables in file and test for
 *    existence, returning a pointer to the expanded path if the file is
 *    readable, NULL otherwise.
 *
 *    Now assume file does not begin with a '/'.
 *
 *    If superfile is non-NULL, we assume it is the pathname of a file
 *    (not a directory), and we look for file in the directory of that
 *    path.  Environment variables are expanded in file but not in
 *    superfile.
 *
 *    If superfile is NULL, or if file isn't found superfile directory,
 *    we look in each of the directories in the array last passed to
 *    filedirs().  Environment variables are expanded in file and in
 *    each of the directories last passed to filedirs().
 *
 *    We return a pointer to a string containing the entire pathname of
 *    the first location where file is found, or NULL if it is not found.
 *
 *    In all cases the returned pointer points to dynamically allocated
 *    space which will be freed on the next call to findfile().
 *
 *    File existence is tested with a call to access(), checking for read
 *    permission.
 */
char *
findfile(char *superfile, char *file)
{
  static char *path = NULL;
  register char **dirp;
  char pbuf[1024];

  if (path) {
    free(path);
    path = NULL;
  }
  if (file == NULL) return NULL;
  if (file[0] == '/' || file[0] == '$') {
    strcpy(pbuf, file);
    envexpand(pbuf);
    if (access(pbuf,R_OK)==0)
      return (path = strdup(pbuf));
    else
      return NULL;
  }
  if (superfile) {
    dirprefix(superfile, pbuf);
    strcat(pbuf, file);
    envexpand(pbuf);
    if (access(pbuf,R_OK)==0)
      return (path = strdup(pbuf));
  }
  if(dirlist == NULL) {
    if(access(file, R_OK) == 0)
	return (path = strdup(file));
  } else {
      for (dirp = dirlist; *dirp != NULL; dirp++) {
	sprintf(pbuf,"%s/%s", *dirp, file);
	envexpand(pbuf);
	if (access(pbuf,R_OK)==0)
	  return (path = strdup(pbuf));
      }
  }
  return (path = NULL);
}
    
/*-----------------------------------------------------------------------
 * Function:	dirprefix
 * Description:	get the directory prefix from a pathname
 * Args:	*path: the pathname
 *		*dir: pointer to location where answer is to be stored
 * Author:	mbp
 * Date:	Wed Feb 12 14:17:36 1992
 * Notes:	Answer always ends with a '/' if path contains a '/',
 *		otherwise dir is set to "".
 */
static void
dirprefix(char *path, char *dir)
{
  register char *end;

  strcpy(dir, path);
  end = dir + strlen(dir) - 1;
  while (end >= dir && *end != '/') --end;
  if (end >= dir) *(end+1) = '\0';
  else dir[0] = '\0';
}

/*-----------------------------------------------------------------------
 * Function:	envexpand
 * Description:	expand environment variables in a string
 * Args:	*s: the string
 * Returns:	s
 * Author:	mbp
 * Date:	Fri Feb 14 09:46:22 1992
 * Notes:	expansion is done inplace; there better be enough room!
 */
char *
envexpand(char *s)
{
  char *c, *env, *envend, *tail;

  c = s;
  if (*c == '~' && (env = getenv("HOME"))) {
    tail = strdup(c+1);
    strcpy(c, env);
    strcat(c, tail);
    c += strlen(env);
    free(tail);
  }
  while (*c != '\0') {
    if (*c == '$') {
      for(envend = c; isalnum(*++envend) || *envend == '_'; ) ;
      tail = strdup(envend);
      *envend = '\0';
      if((env = getenv(c+1)) == NULL) {
	OOGLError(1, "%s : No %s environment variable",s,c+1);
	strcpy(c,tail);
      } else {
	strcpy(c,env);
	strcat(c,tail);
	c += strlen(env);
      }
      free(tail);
    }
    else ++c;
  }
  return s;
}   
