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

 * This file implements setpos()/getpos() facilities for non-seekable
 * files by generating a secondary dynamic file buffering of arbitrary
 * size on top of stdio.
 */

/* As a special exception the author of this files permits the
 * distribution of this software with Geomview. As long as this file
 * is distributed with Geomview, the conditions of the GNU Lesser
 * Public Licence apply. See the file "COPYING" in the top-level
 * directory.
 */

#ifndef _CH_IOBUFFER_H_
#define _CH_IOBUFFER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct IOBFILE IOBFILE;

extern IOBFILE *iobfileopen(FILE *istream);
extern int iobfileclose(IOBFILE *iobf);
extern IOBFILE *iobfopen(const char *name, const char *mode);
extern int iobfclose(IOBFILE *iobf);
#if HAVE_POPEN
extern IOBFILE *iobpopen(const char *name, const char *mode);
extern int iobpclose(IOBFILE *iobf);
#endif
extern FILE *iobfile(IOBFILE *iobf);
extern size_t iobfread(void *ptr, size_t size, size_t nmemb, IOBFILE *iobf);
extern int iobfgetc(IOBFILE *iobf);
extern int iobfungetc(int c, IOBFILE *iobf);
extern void iobfrewind(IOBFILE *iobf);
extern long iobftell(IOBFILE *iobf);
extern int iobfseek(IOBFILE *stream, long offset, int whence);
extern char *iobfgets(char *s, int size, IOBFILE *iobf);
extern int iobfeof(IOBFILE *iobf);
extern int iobfileno(IOBFILE *iobf);

extern size_t
iobfgetbuffer(IOBFILE *iobf, void *ptr, size_t size, int direction);
extern int iobfsetmark(IOBFILE *iobf);
extern int iobfseekmark(IOBFILE *iobf);
extern int iobfclearmark(IOBFILE *iobf);

#endif
