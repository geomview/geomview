/*
 * Copyright (C) 2006 Claus-Justus Heine

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; see the file COPYING.  If not, write to the
 Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 * Clean stdio replacement for fmemopen, using a temporary file. No
 * more fiddling with stdio internals. Gnah.
 */

/* As a special exception the author of this files permits the
 * distribution of this software with Geomview. As long as this file
 * is distributed with Geomview, the conditions of the GNU Lesser
 * Public Licence apply. See the file "COPYING" in the top-level
 * directory.
 */

#include <stdio.h>

/* Come on, folks, let's just do it the CleanWay (tm): open a
 * temporary file, write the string to that file, rewind the file,
 * unlink the file (then fclose() will remove it). Grin. A
 * brain-damaged system might fail to actually delete that file, but
 * we do not care.
 */
FILE *fmemopen (void *buf, size_t size, const char *mode)
{
  FILE *tmp = tmpfile();

  if (*mode != 'r') {
    return NULL;
  }
  if (fwrite(buf, size, 1, tmp) != 1) {
    fclose(tmp);
    return NULL;
  }
  if (fseek(tmp, 0L, SEEK_SET) != 0) {
    fclose(tmp);
    return NULL;
  }
  return tmp;
}
