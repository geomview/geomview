/*
 * Copyright (C) 2006-2008 Claus-Justus Heine

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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define _FILE_OFFSET_BITS 64
#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#if HAVE_FCNTL_H && HAVE_FCNTL
# include <fcntl.h>

static const int o_nonblock =
#ifdef O_NONBLOCK
    O_NONBLOCK |
#endif
#ifdef O_NDELAY
    O_NDELAY |
#endif
#ifdef FNONBLK
    FNONBLK |
#endif
#ifdef FNDELAY
    FNDELAY |
#endif
    0;

#if !defined(O_NONBLOCK) && !defined(O_NDELAY) \
 && !defined(FNONBLK) && !defined(FNDELAY)
# error Do not know how to achieve non-blocking IO
#endif

#endif

#if HAVE_OFF64_T && HAVE_LSEEK64
# define HAVE_SOME_LSEEK 1
typedef off64_t offset_t;
# define LSEEK lseek64
# ifndef HAVE_DECL_LSEEK64
extern off64_t lseek64(int fd, off64_t offset, int whence);
# endif
#elif HAVE_LOFF_T && HAVE_LLSEEK
# define HAVE_SOME_LSEEK 1
typedef loff_t offset_t;
# define LSEEK llseek
# ifndef HAVE_DECL_LLSEEK
loff_t llseek(int fd, loff_t offset, int whence);
# endif
#elif HAVE_OFF_T && HAVE_LSEEK
# define HAVE_SOME_LSEEK 1
typedef off_t offset_t;
# define LSEEK lseek
# ifndef HAVE_DECL_LSEEK
off_t lseek(int fd, off_t offset, int whence);
# endif
#endif

#define POSIX_SHORTCUT (HAVE_READ && HAVE_SOME_LSEEK)

#include "iobuffer.h"

#ifndef DEBUG
# define DEBUG 0
#endif

#ifndef min
# define min(a, b) (((a) <= (b)) ? (a) : (b))
#endif

#ifndef BUFSIZ
# define BUFFER_SIZE 8192
#else
# define BUFFER_SIZE BUFSIZ
#endif

typedef struct IOBuffer
{
  struct IOBuffer *next;
  char buffer[BUFFER_SIZE];
} IOBuffer;

typedef struct IOBufferList
{
  IOBuffer *buf_head; /**< Head of linked list of buffers */
  IOBuffer *buf_tail; /**< Pointer to tail of buffer list */
  IOBuffer *buf_ptr;  /**< Pointer to current buffer */
  size_t   buf_pos;   /**< Byte pos into buf_ptr */
  size_t   tail_size; /**< Size used inside *buf_ptr */
  size_t   tot_pos;
  size_t   tot_size;
} IOBLIST;

struct IOBFILE
{
  FILE     *istream;  /**< The underlying stdio FILE */
  IOBLIST  ioblist;
  IOBLIST  ioblist_mark;
  int      can_seek:1;
  int      mark_wrap:1; /**< Set when the buffer no longer covers the
			     position of the mark. */
  int      mark_set:1;
  int      eof:2;
  int      ungetc;
  fpos_t   stdiomark;   /**< stdio mark interface */
#if POSIX_SHORTCUT
  offset_t posixmark;   /**< using read(), write() and a flavour of lseek() */
#endif
  size_t   mark_pos;    /**< Offset into our buffer structures (pipe, tty) */
  int      mark_ungetc; /**< Copy of what ungetc was when setmark() was called
			 */
  int      fd;
#if HAVE_FCNTL
  int      fflags;
#endif
#if DEBUG
  size_t   read_count;
#endif
};

#if DEBUG
static int n_total_buffers;
static int max_total_buffers;
static int n_fopen;
static int max_fopen;
static int n_fileopen;
static int max_fileopen;
static int n_popen;
static int max_popen;
#endif

static void iob_release_buffer(IOBLIST *ioblist)
{
  IOBuffer *iob, *prev;

  prev = ioblist->buf_head;
  iob  = prev->next;
  prev->next = NULL;
  while (iob) {
    prev = iob;
    iob  = iob->next;
    free(prev);
#if DEBUG
    n_total_buffers --;
#endif
  }
  memset(ioblist, 0, sizeof(*ioblist));
}

static void iob_init_buffer(IOBLIST *ioblist)
{
  ioblist->buf_head       = malloc(sizeof(IOBuffer));
#if DEBUG
  n_total_buffers ++;
  if (n_total_buffers > max_total_buffers) {
    max_total_buffers = n_total_buffers;
    fprintf(stderr, "MAX BUFFERS: %d\n", max_total_buffers);
  }
#endif
  ioblist->buf_head->next = ioblist->buf_head;

  ioblist->buf_ptr  = ioblist->buf_head;
  ioblist->buf_tail = ioblist->buf_head;

  ioblist->tot_pos   = 0;
  ioblist->tot_size  = 0;
  ioblist->tail_size = 0;
  ioblist->buf_pos   = 0;
}

static void iob_copy_buffer(IOBLIST *to, IOBLIST *from)
{
  IOBuffer *iob;
  
  iob_init_buffer(to);
  for (iob = from->buf_head; iob->next != from->buf_head; iob = iob->next) {
    if (iob == from->buf_ptr) {
      to->buf_ptr = to->buf_tail;
    }
    memcpy(to->buf_tail->buffer, iob->buffer, BUFFER_SIZE);
    to->buf_tail->next = malloc(sizeof(IOBuffer));
#if DEBUG
    n_total_buffers ++;
    if (n_total_buffers > max_total_buffers) {
      max_total_buffers = n_total_buffers;
      fprintf(stderr, "MAX BUFFERS: %d\n", max_total_buffers);
    }
#endif
    to->buf_tail = to->buf_tail->next;
    to->buf_tail->next = to->buf_head;
  }
  to->tot_pos   = from->tot_pos;
  to->tot_size  = from->tot_size;
  to->tail_size = from->tail_size;
  to->buf_pos   = from->buf_pos;
}

FILE *iobfile(IOBFILE *iobf)
{
  return iobf ? iobf->istream : NULL;
}

int iobfileno(IOBFILE *iobf)
{
  return iobf ? iobf->fd : -1;
}

IOBFILE *iobfileopen(FILE *istream)
{
  IOBFILE *iobf;

  if (istream == NULL) {
    return NULL;
  }

  iobf = calloc(1, sizeof(IOBFILE));
  iobf->istream = istream;
  iobf->fd = fileno(istream);
  iobf->ungetc = EOF;

  if (iobf->fd >= 0) {
    /* Determine whether we have file positioning support */
    if (lseek(iobf->fd, 0, SEEK_CUR) != -1 && !isatty(iobf->fd)) {
      iobf->can_seek = -1;
    }

    /* If we have read(2) and write(2), then we disable stdio
     * completely and use the system calls directly.
     */
#if SETVBUF_REVERSED
    setvbuf(istream, _IONBF, NULL, 0);
#else
    setvbuf(istream, NULL, _IONBF, 0);
#endif
#if HAVE_FCNTL
    iobf->fflags = fcntl(iobf->fd, F_GETFL);
    if (iobf->fflags != -1 && (iobf->fflags & o_nonblock)) {
      iobf->fflags &= ~o_nonblock;
      if (fcntl(iobf->fd, F_SETFL, iobf->fflags) < 0) {
	fprintf(stderr, "iobfileopen(): unable to clear O_NONBLOCK: \"%s\"\n",
		strerror(errno));
      }
    }
  } else {
    iobf->fflags = -1;
#endif
  }

  iob_init_buffer(&iobf->ioblist);

  iobf->ungetc    = EOF;

#if DEBUG
  n_fileopen++;
  if (n_fileopen > max_fileopen) {
    max_fileopen = n_fileopen;
    fprintf(stderr, "max fileopen: %d\n", max_fileopen);
  }
#endif
  
  return iobf;
}

IOBFILE *iobfopen(const char *name, const char *mode)
{
  FILE *stream;

  if (strchr(mode, 'a') != NULL || strchr(mode, 'w') != NULL) {
    fprintf(stderr, "iobfopen(): Write mode is unsupported\n");
    return NULL;
  }
  stream = fopen(name, mode);
  if (stream == NULL)
    return NULL;

#if DEBUG
  n_fopen++;
  if (n_fopen > max_fopen) {
    max_fopen = n_fopen;
    fprintf(stderr, "max fopen: %d\n", max_fopen);
  }
#endif

  return iobfileopen(stream);
}

int iobfileclose(IOBFILE *iobf)
{
  iob_release_buffer(&iobf->ioblist);
  if (iobf->ioblist_mark.buf_head) {
    iob_release_buffer(&iobf->ioblist_mark);
  }
  free(iobf);

#if DEBUG
  n_fileopen--;
#endif

  return 0;
}

int iobfclose(IOBFILE *iobf)
{
  int result;
 
  result = fclose(iobf->istream);

  (void)iobfileclose(iobf);

#if DEBUG
  n_fopen--;
#endif

  return result;
}

#if HAVE_POPEN
IOBFILE *iobpopen(const char *cmd, const char *mode)
{
  FILE *stream;

  if (strchr(mode, 'a') != NULL || strchr(mode, 'w') != NULL) {
    fprintf(stderr, "iobfopen(): Write mode is unsupported\n");
    return NULL;
  }
  stream = popen(cmd, mode);
  if (stream == NULL)
    return NULL;

#if DEBUG
  n_popen++;
  if (n_popen > max_popen) {
    max_popen = n_popen;
    fprintf(stderr, "max popen: %d\n", max_popen);
  }
#endif

  return iobfileopen(stream);
}

int iobpclose(IOBFILE *iobf)
{
  int result;

  result = pclose(iobf->istream);

  (void)iobfileclose(iobf);

#if DEBUG
  n_popen--;
#endif

  return result;
}
#endif

long iobftell(IOBFILE *iobf)
{
  long pos;

  if (!iobf->can_seek) {
    return ~0L;
  }
#if POSIX_SHORTCUT
  pos = (long)LSEEK(iobf->fd, 0, SEEK_CUR);
#else
  pos = ftell(iobf->istream);
#endif

  return pos < 0
    ? pos
    : (long)(pos - (iobf->ioblist.tot_size - iobf->ioblist.tot_pos));
}

int iobfseek(IOBFILE *iobf, long offset, int whence)
{
  if (iobf->can_seek) {
#if POSIX_SHORTCUT
    if ((long)LSEEK(iobf->fd, (offset_t)offset, whence) < 0) {
      return -1;
    }
#else
    if (fseek(iobf->istream, offset, whence) != 0) {
      return -1;
    }
#endif
    iob_release_buffer(&iobf->ioblist);
    iob_init_buffer(&iobf->ioblist);
    return 0;
  }
  return -1;
}

char *iobfgets(char *s, int size, IOBFILE *iobf)
{
  char *p = s;
  int c = 0;

  while (--size) {
    *p++ = c = iobfgetc(iobf);
    if (c == '\n' || c == EOF) {
      break;
    }
  }
  if (c == EOF) {
    p--;
  }
  *p = '\0';
  if (s == p && size) {
    return NULL;
  } else {
    return s;
  }
}

/* Copy the buffer contents between tot_pos and tot_pos - size t
 * ptr
 */
size_t iobfgetbuffer(IOBFILE *iobf, void *ptr, size_t size, int direction)
{
  IOBLIST *ioblist = &iobf->ioblist;
  IOBuffer *iob;
  int skip, i;
  size_t offset, cpsz, rval, tot_space = ioblist->tot_size - ioblist->tot_pos;
  char *buf = ptr;

  if (iobf->ungetc != EOF) {
    ++tot_space;
  }

  if (ptr == NULL) {
    return direction < 0 ? ioblist->tot_pos : tot_space;
  }

  if (direction < 0) {
    rval = size = min(size, ioblist->tot_pos);
    skip = (ioblist->tot_pos - size) / BUFFER_SIZE;
    for (i = 0, iob = ioblist->buf_head; i < skip; iob = iob->next, i++);
    offset = (ioblist->tot_pos - size) % BUFFER_SIZE;
    cpsz = min(size, BUFFER_SIZE - offset);
    memcpy(buf, iob->buffer + offset, cpsz);
    buf  += cpsz;
    size -= cpsz;
    while (size) {
      iob   = iob->next;
      cpsz  = min(size, BUFFER_SIZE);
      memcpy(buf, iob->buffer, cpsz);
      buf  += cpsz;
      size -= cpsz;
    }
  } else {
    rval = size = min(size, tot_space);
    if (size > 0 && iobf->ungetc != EOF) {
      *buf++ = iobf->ungetc;
      --size;
    }
    iob = ioblist->buf_ptr;
    offset = ioblist->buf_pos;
    cpsz = min(size, BUFFER_SIZE - offset);
    memcpy(buf, iob->buffer + offset, cpsz);
    size -= cpsz;
    buf  += cpsz;
    while (size) {
      iob   = iob->next;
      cpsz  = min(size, BUFFER_SIZE);
      memcpy(buf, iob->buffer, cpsz);
      buf  += cpsz;
      size -= cpsz;
    }   
  }  
  return rval;
}

static size_t
iobfread_buffer(void *ptr, size_t size, IOBFILE *iobf)
{
  IOBLIST *ioblist = &iobf->ioblist;
  size_t tot_space, rq_sz, rq_sz_pos, rd_sz;
  char *buf = ptr;

  tot_space = ioblist->tot_size - ioblist->tot_pos;
  if (iobf->ungetc != EOF) {
    ++tot_space;
  }
  rq_sz = min(size, tot_space);

  if (rq_sz == 0) {
    return 0;
  }

  rd_sz = 0;

  if (iobf->ungetc != EOF) {
    buf[0] = iobf->ungetc & 0xff;
    iobf->ungetc = EOF;
    ++buf;
    ++rd_sz;
    --rq_sz;
#if DEBUG
    --iobf->read_count;
#endif
  }

  while (rq_sz) {
    rq_sz_pos = min(rq_sz, BUFFER_SIZE - ioblist->buf_pos);
    memcpy(buf, ioblist->buf_ptr->buffer + ioblist->buf_pos, rq_sz_pos);
    ioblist->buf_pos += rq_sz_pos;
    ioblist->tot_pos += rq_sz_pos;
    buf           += rq_sz_pos;
    rd_sz         += rq_sz_pos;
    rq_sz         -= rq_sz_pos;
    if (ioblist->buf_pos == BUFFER_SIZE &&
	ioblist->buf_ptr != ioblist->buf_tail) {
      /* advance to next buffer */
      ioblist->buf_ptr = ioblist->buf_ptr->next;
      ioblist->buf_pos = 0;
      if ((!iobf->mark_set || iobf->can_seek) &&
	  ioblist->buf_head->next->next != ioblist->buf_head) {
	/* Release buffers no longer needed. */
	ioblist->buf_tail->next = ioblist->buf_head->next;
	free(ioblist->buf_head);
#if DEBUG
	n_total_buffers --;
#endif
	ioblist->buf_head  = ioblist->buf_tail->next;
	ioblist->tot_pos  -= BUFFER_SIZE;
	ioblist->tot_size -= BUFFER_SIZE;
      }
    }
  }
  return rd_sz;
}

static void iob_check_space(IOBFILE *iobf)
{
  IOBLIST *ioblist = &iobf->ioblist;
  
  if (ioblist->tail_size < BUFFER_SIZE) {
    return;
  }
  
  if ((!iobf->can_seek && iobf->mark_set) ||
      ioblist->buf_head->next == ioblist->buf_head) {
    /* allocate up to two buffers in normal operation and as many as
     * needed for files without seek capabilities.
     */
    ioblist->buf_tail->next = malloc(sizeof(IOBuffer));
#if DEBUG
    n_total_buffers ++;
    if (n_total_buffers > max_total_buffers) {
      max_total_buffers = n_total_buffers;
      fprintf(stderr, "MAX BUFFERS: %d\n", max_total_buffers);
    }
#endif
    ioblist->buf_tail       = ioblist->buf_tail->next;
    ioblist->buf_tail->next = ioblist->buf_head;
    ioblist->tail_size      = 0;
  } else {
    /* rotate list */
    ioblist->buf_tail  = ioblist->buf_tail->next;
    ioblist->buf_head  = ioblist->buf_head->next;
    ioblist->tot_pos  -= BUFFER_SIZE;
    ioblist->tot_size -= BUFFER_SIZE;
    ioblist->tail_size = 0;
    iobf->mark_wrap = ~0;
  }
}

/* Flush the buffer as much as possible, but we have to be careful not
 * to discard data if tot_pos < tot_size. In this case we discard all
 * data < tot_pos and leave the rest as is.
 */
static void iob_flush_buffer(IOBLIST *ioblist)
{
  while (ioblist->buf_head != ioblist->buf_ptr) {
    ioblist->buf_tail->next = ioblist->buf_head->next;
    free(ioblist->buf_head);
#if DEBUG
    n_total_buffers --;
#endif
    ioblist->buf_head = ioblist->buf_tail->next;
    ioblist->tot_pos  -= BUFFER_SIZE;
    ioblist->tot_size -= BUFFER_SIZE;
  }
  /* Check for the special case where we have one and only one buffer
   * and tot_pos points to the very end of it. In this case, discard
   * this buffer, too. The other possibility would be to allocate a
   * second buffer.
   */
  if (ioblist->buf_head == ioblist->buf_head->next &&
      ioblist->tot_pos == BUFFER_SIZE) {
#if 1
    ioblist->tot_pos =
      ioblist->tot_size  =
      ioblist->buf_pos   =
      ioblist->tail_size = 0;
#else
    ioblist->buf_tail->next = malloc(sizeof(IOBuffer));
#if DEBUG
    n_total_buffers ++;
    if (n_total_buffers > max_total_buffers) {
      max_total_buffers = n_total_buffers;
      fprintf(stderr, "MAX BUFFERS: %d\n", max_total_buffers);
    }
#endif
    ioblist->buf_tail       = ioblist->buf_tail->next;
    ioblist->buf_tail->next = ioblist->buf_head;
    ioblist->tail_size =
      ioblist->buf_pos = 0;      
#endif
  }
}

void iobfrewind(IOBFILE *iobf)
{
  rewind(iobf->istream);
#if 0
  if (1 || iobf->can_seek) {
    iob_release_buffer(&iobf->ioblist);
    iob_init_buffer(&iobf->ioblist);
  } else {
    iobf->ioblist.tot_pos =
      iobf->ioblist.buf_pos = 0;
    iobf->ioblist.buf_ptr =
      iobf->ioblist.buf_head;
  }
#else
  /* The fastest way is: simply move the position pointer to end of
     buffer */
  iobf->ioblist.tot_pos = iobf->ioblist.tot_size;
  iobf->ioblist.buf_pos = iobf->ioblist.tail_size;
#endif
  if (iobf->ioblist_mark.buf_head) {
    iob_release_buffer(&iobf->ioblist_mark);
  }
  iobf->mark_set  = 0;
  iobf->mark_wrap = 0;
  iobf->mark_pos  = ~0;
  memset(&iobf->stdiomark, ~0, sizeof(iobf->stdiomark));
  iobf->posixmark = -1;

  /* Clear status flags */
  iobf->ungetc = EOF;
  iobf->eof = 0;
}  

size_t iobfread(void *ptr, size_t size, size_t nmemb, IOBFILE *iobf)
{
  IOBLIST *ioblist = &iobf->ioblist;
  size_t rq_size = size * nmemb, rd_size, rd_tot;
  ssize_t tail_rd;
  char *buf = ptr;
#if HAVE_FCNTL
  int first = 1;
  int fcntl_err = 0;
#endif
  int cnt;

  if (size*nmemb == 0) {
    return 0;
  }

  rd_tot  =  0;
  tail_rd = ~0;
  cnt = 0;
  do {
    ++cnt;
    rd_size = iobfread_buffer(buf, rq_size, iobf);
    rq_size -= rd_size;
    rd_tot  += rd_size;
    buf     += rd_size;
    if (iobf->eof && rq_size) {
      iobf->eof = -1;
      break;
    }
    if (tail_rd && rq_size && !iobf->eof) {
      ssize_t tail_space;
      
      iob_check_space(iobf);
      tail_space = BUFFER_SIZE - ioblist->tail_size;
#if HAVE_FCNTL
      if (!iobf->can_seek) {
	if (first && iobf->fflags != -1) {
	  fcntl_err = fcntl(iobf->fd, F_SETFL, iobf->fflags | o_nonblock);
	}
	if (!first || (iobf->fd >= 0 && iobf->fflags == -1) || fcntl_err) {
	  tail_space = min(tail_space, (ssize_t)rq_size);
	}
      }
#else
      if (!iobf->can_seek && iobf->fd >= 0)
	  tail_space = min(tail_space, rq_size);
#endif
#if POSIX_SHORTCUT
      if (iobf->fd >= 0) {
	tail_rd = read(iobf->fd,
		       ioblist->buf_tail->buffer + ioblist->tail_size,
		       tail_space);
	if (tail_rd < 0) {
	  if (errno == EAGAIN) {
	    tail_rd = 0; /* probably a pipe */
	  } else {
	    tail_rd = 0; /* maybe do something else in this case */
	  }
	} else {
	  /* EOF is indicated by returning 0 for a request size > 0 */
	  if (tail_rd == 0 && tail_space > 0) {
	    iobf->eof = 1; /* eof is a counter! */
	  }
	}
	ioblist->tail_size += tail_rd;
	ioblist->tot_size  += tail_rd;
      } else {
#endif
	tail_rd = fread(ioblist->buf_tail->buffer + ioblist->tail_size,
			1, tail_space, iobf->istream);
      
	ioblist->tail_size += tail_rd;
	ioblist->tot_size  += tail_rd;
	if (tail_rd < tail_space && feof(iobf->istream)) {
	  iobf->eof = 1;
	}
      }
#if HAVE_FCNTL
      if (!iobf->can_seek && first && iobf->fflags != -1 && !fcntl_err) {
	first = 0;
	clearerr(iobf->istream);
	if ((fcntl_err = fcntl(iobf->fd, F_SETFL, iobf->fflags))
	    < 0) {
	  fprintf(stderr, "iobfread(): unable to clear O_NONBLOCK: \"%s\"\n",
		  strerror(errno));
	}
	if (tail_rd == 0 && rq_size) {
	  tail_rd = ~0; /* retry with blocking IO */
	  iobf->eof = 0;
	}
      }
#endif
    }
  } while (tail_rd && rq_size);
#if DEBUG
  iobf->read_count += rd_tot;
#endif
  return rd_tot / size;
}

int iobfgetc(IOBFILE *iobf)
{
  int c = EOF;
  unsigned char c_char;

  if (iobf->eof != -1 && iobfread(&c_char, 1, 1, iobf) == 1) {
    c = c_char;
  }

  return c;
}

int iobfsetmark(IOBFILE *iobf)
{
  IOBLIST *ioblist = &iobf->ioblist;
  int result = 0;

  if (iobf->mark_set) {
    iobfclearmark(iobf);
  }

  /* FIXME: generate error if EOF condition is set? */
  if (iobf->eof == -1) {
    return -1;
  }

  iob_flush_buffer(ioblist);
  
  iobf->mark_set    = ~0;
  iobf->mark_wrap   = 0;
  iobf->mark_pos    = ioblist->tot_pos;
  iobf->mark_ungetc = iobf->ungetc;

  if (iobf->can_seek) {
#if POSIX_SHORTCUT
    if ((iobf->posixmark = LSEEK(iobf->fd, 0, SEEK_CUR)) < 0) {
      result = -1;
    }
#else
    result = fgetpos(iobf->istream, &iobf->stdiomark);
#endif
    iob_copy_buffer(&iobf->ioblist_mark, &iobf->ioblist);
  }

  return result;
}

int iobfseekmark(IOBFILE *iobf)
{
  IOBLIST *ioblist;

  if (!iobf->mark_set) {
    return -1;
  }

  if (iobf->mark_wrap) { /* implies can_seek */
#if POSIX_SHORTCUT
    if (LSEEK(iobf->fd, iobf->posixmark, SEEK_SET) != iobf->posixmark) {
      return -1;
    }
#else
    if (fsetpos(iobf->istream, &iobf->stdiomark) != 0) {
      return -1;
    }
#endif
    iob_release_buffer(&iobf->ioblist);
    iob_copy_buffer(&iobf->ioblist, &iobf->ioblist_mark);
    iobf->mark_wrap = 0;
  }
  
  ioblist = &iobf->ioblist;

  ioblist->buf_ptr = ioblist->buf_head;
  ioblist->tot_pos = iobf->mark_pos;
  ioblist->buf_pos = iobf->mark_pos % BUFFER_SIZE;

  iobf->ungetc = iobf->mark_ungetc;

  /* Clear status flags on success */
  if (iobf->eof == -1) {
    iobf->eof = 1;
  }
#if DEBUG
  iobf->read_count = ioblist->tot_pos;
#endif
  return 0;
}

int iobfclearmark(IOBFILE *iobf)
{
  if (!iobf->mark_set) {
    return -1;
  }
  
  iobf->mark_set    = 0;
  iobf->mark_wrap   = 0;

  if (iobf->ioblist_mark.buf_head != NULL) {
    iob_release_buffer(&iobf->ioblist_mark);
  }

  return 0;
}

int iobfungetc(int c, IOBFILE *iobf)
{
  if (c == EOF) { /* ??? */
    iobf->ungetc = EOF;
  } else {
    iobf->ungetc = c & 0xff;
    if (iobf->eof == -1){
      iobf->eof = 1;
    }
  }
  return c;
}

int iobfeof(IOBFILE *iobf)
{
  if (iobf->ungetc != EOF) {
    return 0;
  } else if (iobf->ioblist.tot_pos < iobf->ioblist.tot_size) {
    return 0;
  } else if (iobf->eof == -1) {
    if (POSIX_SHORTCUT && iobf->fd >= 0) {
      return 1;
    } else if (feof(iobf->istream)) {
      return 1;
    }
    iobf->eof = 0;
    return 0;
  } else {
    return 0;
  }
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
