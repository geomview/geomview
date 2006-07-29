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

#undef stat
#undef fstat

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
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

#ifndef ENOENT
# define ENOENT 255
#endif
#ifndef ENOSYS
# define ENOSYS 255
#endif

int gv_stat(const char *path, struct stat *buf)
{
#if HAVE_LSTAT_EMPTY_STRING_BUG 
	if (path == NULL || *path == '\0') {
		errno = ENOENT;
		return -1;
	}
#endif
#if HAVE_STAT
	return stat(path, buf);
#else
	errno = ENOSYS;
	return -1;
#endif
}

int gv_fstat(int fd, struct stat *buf)
{
#if HAVE_FSTAT
	return fstat(fd, buf);
#else
	errno = ENOSYS;
	return -1;
#endif
}
