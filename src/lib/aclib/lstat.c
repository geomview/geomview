/* Copyright (C) 2006 Claus-Justus Heine.
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

#undef lstat

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_STRING_H
# include <string.h>
#endif
#if HAVE_LIMITS_H
# include <limits.h>
#endif
#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#if HAVE_ERRNO_H
# include <errno.h>
#endif

#if !HAVE_DECL_ERRNO
extern int errno;
#endif

#ifndef PATH_MAX
# define PATH_MAX 1024
#endif
#ifndef ENOENT
# define ENOENT 255
#endif
#ifndef ENAMETOOLONG
# define ENAMETOOLONG 255
#endif
#ifndef ENOSYS
# define ENOSYS 255
#endif

int gv_lstat(const char *path, struct stat *buf)
{
#if !LSTAT_FOLLOWS_SLASHED_SYMLINK
	size_t pathlen;
	char mypath[PATH_MAX+1];
#endif

#if HAVE_LSTAT_EMPTY_STRING_BUG 
	if (path == NULL || *path == '\0') {
		errno = ENOENT;
		return -1;
	}
#endif
#if !LSTAT_FOLLOWS_SLASHED_SYMLINK
	pathlen = strlen(path);
	if (pathlen > 0 && path[pathlen-1] == '/') {
		if (pathlen == PATH_MAX) {
			errno = ENAMETOOLONG;
			return -1;
		}
		memcpy(mypath, path, pathlen);
		path = mypath;
		mypath[pathlen++] = '.';
		mypath[pathlen++] = '\0';
	}
#endif
#if HAVE_LSTAT
	return lstat(path, buf);
#else
	errno = ENOSYS;
	return -1;
#endif
}
