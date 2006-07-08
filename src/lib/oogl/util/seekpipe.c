/*
 * Copyright (C) 2003 Claus-Justus Heine

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 * seekpipe.c
 *
 * Implement seekable pipes using GNU's fopencookie() extension.
 *
 * Entry point:
 * 
 * FILE *seekpipe_open(FILE *stream);
 *
 * if fgetpos(stream, ...) succeeds, then simply return "stream". Otherwise
 * fiddle with GNU's fopencookie() stuff.
 *
 * _This_ stuff uses a circular ring-buffer of size at least
 * SP_RING_SZ.  In case that some advanced stdio allows really large
 * buffer size we check that SP_RING_SZ is at least as large as the
 * size argument to the cookie-read fct. That way one can at least
 * space back to the begin of the most recent read.
 *
 * The stuff is created with FILE *seekpipe_open(FILE *stream).  This
 * creates that circular ring-buffer. We don't care about buffering,
 * and try to use only stdio stuff. However, seekpipe_open() should be
 * called before doing any I/O. Afterwards it is possible to use
 * fgetpos() and fsetpos() to remember a position.
 *
 * If fsetpos() is called with a position outside the ringbuffer
 * you'll get a -ESPIPE error.
 *
 * Closing the the file-handle returned by seekpipe_open() closes also
 * the original stream, so be careful. OTOH, this way seekpipe_open()
 * can be a no-op for regular (i.e. seekable files).
 */

#define _GNU_SOURCE /* don't change, we really want it */

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define DEBUG 1
#if DEBUG
/* we use the environment variable SEEKPIPE_DEBUG to actually switch
 * on debugging, and SEEKPIPE_DEBUGFILE to control the name of the
 * file to write to.
 */
static const char *debugfilename = "/tmp/seekpipe-foo";
static FILE *debugfile;
#endif

#define SP_RING_SZ (1 << 20) /* (1 << 20)*/

#define SP_MAGIC  0x50534f4f /* hex for OOPS in LITTLE ENDIAN */

typedef struct {
  int            fd;
  FILE           *self;     /* our-selfes */
  off64_t        file_pos;  /* offset inside the file */
  off64_t        file_len;  /* how much is in the buffer */
  size_t         buff_wrap; /* "meeting" point */
  int            wrap;      /* has wrapped */
  unsigned long  magic1;
  char           buffer[SP_RING_SZ];
  unsigned long  magic2;
} SEEKPIPE;

#ifdef min
# undef min
#endif
#define min(a,b) ((a) <= (b) ? (a) : (b))

static ssize_t seekpipe_read(void *cookie, char *buf, size_t sz)
{
  SEEKPIPE *ff = cookie;
  size_t cp, bufflen, buffpos, rd = 0;
  ssize_t ret;
  long amount;
  off64_t buffoff;

  while (sz > 0) {
    if (rd > 0) { /* if we have just something, return that */
      return rd;
    } else if (ff->file_pos == ff->file_len) {
      /* normal operation, read from ff->fd and cache in
       * ring-buffer.
       *
       * We only read if rd == 0 in order to avoid artificial EOF
       * conditions
       */
      ret = read(ff->fd, buf, sz);
#if DEBUG
      if (debugfile && ret > 0)
	fwrite(buf, ret, 1, debugfile);
#endif

      if (ret > 0) {
	/* ok, copy to our cache. This is a ring buffer, we always
	 * append at the end. ff->wrap is important when serving
	 * request out of the cache.
	 */
	rd += ret;
	cp  = ret;
	do {
	  amount = min(SP_RING_SZ - ff->buff_wrap, cp);
	  memcpy(ff->buffer + ff->buff_wrap, buf, amount);
	  buf           += amount;
	  cp            -= amount;
	  ff->buff_wrap += amount;
	  ff->file_len  += amount;
	  ff->file_pos   = ff->file_len;
	  if (ff->buff_wrap == SP_RING_SZ) {
	    ff->buff_wrap = 0;
	    ff->wrap      = 1;
	  }
	} while (cp > 0);
	return rd;
      } else {
	return rd > 0 ? rd : ret;
      }
    } else {
      /* (partial) copy out of the cache */
      bufflen = ff->wrap ? SP_RING_SZ : ff->buff_wrap;
      buffoff = ff->file_len - bufflen;
      if (buffoff < 0) {
	return -1; /* cache inconsistency */
      }
      if (ff->file_pos < buffoff) {
	return -1; /* can't server this request */
      }
      /* otherwise start copying */
      
      buffpos = bufflen - (ff->file_len - ff->file_pos); /* abs. position */

      if (ff->wrap) {
	amount = min((long)sz, (long)(SP_RING_SZ - buffpos - ff->buff_wrap));
	if (amount > 0) {
	  memcpy(buf, &ff->buffer[ff->buff_wrap+buffpos], amount);
	  buf          += amount;
	  sz           -= amount;
	  ff->file_pos += amount;
	  rd           += amount;
	  buffpos      += amount;
	}
	/* adjust buffpos before going to lower part of buffer */
	buffpos -= SP_RING_SZ - ff->buff_wrap;
      }
      amount = min((long)sz, (long)(ff->buff_wrap - buffpos));
      if (amount > 0) {
	memcpy(buf, &ff->buffer[buffpos], amount);
	buf          += amount;
	sz           -= amount;
	ff->file_pos += amount;
	rd           += amount;
      }
    }
  }
  return rd;
}

static int seekpipe_seek(void *cookie, off64_t *pos, int whence)
{
  SEEKPIPE *ff = cookie;
  off64_t  new_pos, buffoff;
  size_t   bufflen;

  switch (whence) {
  case SEEK_CUR:
    new_pos = ff->file_pos + *pos;
    break;
  case SEEK_END:
    new_pos = ff->file_len - *pos;
    break;
  case SEEK_SET:
    new_pos = *pos;
    break;
  default:
    abort();
  }

  bufflen = ff->wrap ? SP_RING_SZ : ff->buff_wrap;
  buffoff = ff->file_len - bufflen;

  if (new_pos > ff->file_len || new_pos < buffoff) {
    /* fprintf(stderr, "*pos: %Ld, bufflen: %d, new_pos: %Ld, file_len: %Ld, buffoff: %Ld, whence: %d\n",
     *pos, bufflen, new_pos, ff->file_len, buffoff, whence); */
    *pos = ff->file_pos;
    errno = ESPIPE;
    return -1;
  }

  /* ok, request inside window */
  ff->file_pos = new_pos;
  *pos = new_pos;
  return 0;
}

static int seekpipe_close(void *cookie)
{
  SEEKPIPE *ff = cookie;

#if DEBUG
  if (debugfile) {
    fclose(debugfile);
    debugfile = NULL;
  }
#endif

  free(ff);
  return 0; /* leave original stream to caller */
}

cookie_io_functions_t ff_iof = {
  .read = seekpipe_read,
  .write = NULL,
  .seek = seekpipe_seek,
  .close = seekpipe_close,
};

FILE *seekpipe_open(int fd)
{
  SEEKPIPE *ff;

  ff = calloc(1, sizeof(*ff));

  ff->fd   = fd;
  ff->self = fopencookie(ff, "r", ff_iof);
  ff->magic1 =
    ff->magic2 = SP_MAGIC;

  if (ff->self == NULL) {
    free(ff);
  }

#if DEBUG
  if (getenv("SEEKPIPE_DEBUG")) {
    if (getenv("SEEKPIPE_DEBUGFILE")) {
      debugfilename = getenv("SEEKPIPE_DEBUGFILE");
    }
    debugfile = fopen(debugfilename, "w");
  }
#endif

  return ff->self;
}

#if _TEST_
int main(int argc, char *argv[])
{
  FILE   *fake;
  fpos_t pos, pos1;
  char   buf[1 << 20];
  int i;

  fake = seekpipe_open(fileno(stdin));

  if (fseek(fake, -1, SEEK_END)) perror("fseek");

  fread(buf, 1 << 17, 1, fake);

  memset(buf, 0, 16); 
  fread(buf, 16, 1, fake); buf[16] = '\0'; printf("READ: %s\n", buf);
  ungetc('a', fake);
  memset(&pos, 0, sizeof(pos));
  memset(&pos1, 0, sizeof(pos));
  if (fgetpos(fake, &pos)) perror("fgetpos");
  else printf("File position: %Ld\n", *(off64_t *)&pos);

  memset(buf, 0, 16); 
  fread(buf, 16, 1, fake); buf[16] = '\0'; printf("READ: %s\n", buf);
  if (fgetpos(fake, &pos)) perror("fgetpos");
  else printf("File position: %Ld\n", *(off64_t *)&pos);

  memset(buf, 0, 16); 
  fread(buf, 16, 1, fake); buf[16] = '\0'; printf("READ: %s\n", buf);
  if (fgetpos(fake, &pos1)) perror("fgetpos");
  else printf("File position: %Ld\n", *(off64_t *)&pos1);

  if (fsetpos(fake, &pos)) perror("fsetpos");
  else printf("File position: %Ld\n", *(off64_t *)&pos);

  memset(buf, 0, 16); 
  fread(buf, 16, 1, fake); buf[16] = '\0'; printf("READ: %s\n", buf);
  memset(buf, 0, 16); 
  fread(buf, 16, 1, fake); buf[16] = '\0'; printf("READ: %s\n", buf);

  if (fsetpos(fake, &pos)) perror("fsetpos");
  else printf("File position: %Ld\n", *(off64_t *)&pos);

  memset(buf, 0, 16); 
  fread(buf, 16, 1, fake); buf[16] = '\0'; printf("READ: %s\n", buf);
  memset(buf, 0, 16); 
  fread(buf, 16, 1, fake); buf[16] = '\0'; printf("READ: %s\n", buf);

  if (fgetpos(fake, &pos)) perror("fgetpos");
  else printf("File position: %Ld\n", *(off64_t *)&pos);

  if (fsetpos(fake, &pos1)) perror("fsetpos");
  else printf("File position: %Ld\n", *(off64_t *)&pos1);

  memset(buf, 0, 16); 
  fread(buf, 16, 1, fake); buf[16] = '\0'; printf("READ: %s\n", buf);
  memset(buf, 0, 16); 
  fread(buf, 16, 1, fake); buf[16] = '\0'; printf("READ: %s\n", buf);

  if (fgetpos(fake, &pos)) perror("fgetpos");
  else printf("File position: %Ld\n", *(off64_t *)&pos);

  fflush(fake);
  if (fseek(fake, -1, SEEK_END)) perror("fseek");

  if (fgetpos(fake, &pos)) perror("fgetpos");
  else printf("File position: %Ld\n", *(off64_t *)&pos);

  for (i = 0; i < 8192/64; i++)
    fread(buf, 64, 1, fake);

  if (fgetpos(fake, &pos)) perror("fgetpos");
  else printf("File position: %Ld\n", *(off64_t *)&pos);
   
  if (fsetpos(fake, &pos1)) perror("fsetpos");
  else printf("File position: %Ld\n", *(off64_t *)&pos1);

  if (fsetpos(fake, &pos)) perror("fsetpos");
  else printf("File position: %Ld\n", *(off64_t *)&pos);

  memset(buf, 0, 16); 
  fread(buf, 16, 1, fake); buf[16] = '\0'; printf("READ: %s\n", buf);
  memset(buf, 0, 16); 
  fread(buf, 16, 1, fake); buf[16] = '\0'; printf("READ: %s\n", buf);

  fclose(fake);

  exit(0);
}
#endif /* _TEST_ */

#if 0
Local Variables: ***
c-basic-offset: 2 ***
End: ***
#endif
