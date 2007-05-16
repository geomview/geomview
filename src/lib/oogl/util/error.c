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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include <stdarg.h>
#include <errno.h>
#include "ooglutil.h"


char	*_GFILE;	/* Name of file where error is found */
int	_GLINE;		/* Line number in file where error is found */
int	OOGL_Errorcode;	/* Unique integer error code */

int _OOGLError(int errorcode, char* fmt,...)
{
   va_list args;
   va_start(args, fmt);
   if(errorcode & OE_VERBOSE)
	fprintf(stderr, "Error <%d>: ",errorcode);
   if (fmt != NULL) vfprintf(stderr, fmt, args);
   fprintf(stderr, "\n");
   if(errorcode & OE_VERBOSE)
	fprintf(stderr, "File: %s, Line: %d\n\n",_GFILE,_GLINE);
   OOGL_Errorcode = errorcode;
   va_end(args);
   return 0;
}

/*\
 * Report syntax error
 */
void OOGLSyntax(IOBFILE *f, char *fmt, ...)
{
   static IOBFILE *oldf;
   static char oldtext[32];
   char *context;

   va_list args;
   va_start(args, fmt);
   vfprintf(stderr, fmt, args);
   context = iobfcontext(f);
   if(f == oldf && strcmp(context, oldtext) == 0) {
       fprintf(stderr, " [ditto]\n");
   } else {
       fprintf(stderr,
	       context[0] != '\0' ? ":\n%s" : " [no text available]\n",
	       context);
       oldf = f;
       memcpy(oldtext, context, sizeof(oldtext));
       oldtext[sizeof(oldtext)-1] = '\0';
   }
   va_end(args);
}

void OOGLWarn(char *fmt, ...)
{
  va_list args;
#if 0
  int level = 2;
  if(fmt[0] < ' ' && fmt[0] != '\0')
    level = *fmt++;
  /* Maybe we have a current warning level below which we don't print
   *
   * cH: seemingly not implemented.
   */
#endif

  va_start (args, fmt);
  
  vfprintf (stderr, fmt, args);
  fputc('\n', stderr);
  fflush(stderr);
  va_end (args);
}

const char *
sperrno(unsigned int err)
{
#if HAVE_STRERROR
  return strerror((int)err);
#else
# if !defined(__FreeBSD__) && !defined(__GLIBC__) && !defined(__CYGWIN__)
  extern int sys_nerr;
  extern char *sys_errlist[];
# endif
  static char errstr[16];
  
  if(err < sys_nerr)
    return(sys_errlist[err]);
  sprintf(errstr, "Error %d", err & 0xffff);
  return(errstr);
#endif
}

const char *
sperror(void)
{
#ifndef HAVE_DECL_ERRNO
	extern int errno;
#endif

	return sperrno(errno);
}
