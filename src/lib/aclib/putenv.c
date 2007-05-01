/*
 * The following copyright notice applies to everything in this
 * file from here on down.
 */

/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getenv.c	5.5 (Berkeley) 6/27/88";
#endif /* LIBC_SCCS and not lint */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * _findenv --
 *	Returns pointer to value associated with name, if any, else NULL.
 *	Sets offset to be the offset of the name/value combination in the
 *	environmental array, for use by setenv(3)/putenv(3) and unsetenv(3).
 *	Explicitly removes '=' in argument name.
 */
static char *_findenv(char *name, int *offset)
{
  extern char **environ;
  int len;
  char **P, *C;

  for (C = name, len = 0; *C && *C != '='; ++C, ++len);
  for (P = environ; *P; ++P)
    if (!strncmp(*P, name, len))
      if (*(C = *P + len) == '=') {
	*offset = P - environ;
	return(++C);
      }
  return(NULL);
}
#if defined(LIBC_SCCS) && !defined(lint)
/* static char sccsid[] = "@(#)setenv.c	5.2 (Berkeley) 6/27/88"; */
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>

/* Note: we do not use OOGLNew() & friends because libc would neither do so. */
static inline void *xmalloc(size_t size)
{
  void *ptr;
  
  if ((ptr = malloc(size)) == NULL) {
    fprintf(stderr, "malloc(%d) failed in putenv().\n", size);
    exit(EXIT_FAILURE);
  }
  return ptr;
}

static inline void *xrealloc(void *oldptr, size_t newsize)
{
  void *ptr;
  
  if ((ptr = realloc(oldptr, newsize)) == NULL) {
    fprintf(stderr, "realloc(%p, %d) failed in putenv().\n", oldptr, newsize);
    exit(EXIT_FAILURE);
  }
  return ptr;
}

/*
 * putenv --
 *	Put a string of the form "name=value" into the environment.
 *	[Adapted from BSD routine setenv(name, value, rewrite).]
 */
putenv(char *name)
{
  extern char **environ;
  static int alloced;			/* if allocated space before */
  char *value;
  char *C;
  int l_value, offset;

  value = strchr(name, '=');
  value = value ? value+1 : name;
  l_value = strlen(value);
  if ((C = _findenv(name, &offset))) {	/* find if already exists */
    if (strlen(C) >= l_value) {	/* old larger; copy over */
      while (*C++ = *value++);
      return(0);
    }
  }
  else {					/* create new slot */
    int	cnt;
    char	**P;

    for (P = environ, cnt = 0; *P; ++P, ++cnt);
    if (alloced) {			/* just increase size */
      environ = (char **)realloc((char *)environ,
				 (u_int)(sizeof(char *) * (cnt + 2)));
      if (!environ)
	return(-1);
    }
    else {				/* get new space */
      alloced = 1;		/* copy old entries into it */
      P = (char **)xmalloc((u_int)(sizeof(char *) *
				  (cnt + 2)));
      if (!P)
	return(-1);
      memcpy(P, environ, cnt * sizeof(char *));
      environ = P;
    }
    environ[cnt + 1] = NULL;
    offset = cnt;
  }
  for (C = name; *C && *C != '='; ++C);	/* no `=' in name */
  if (!(environ[offset] =			/* name + `=' + value */
	xmalloc((u_int)((int)(C - name) + l_value + 2))))
    return(-1);
  for (C = environ[offset]; (*C = *name++) && *C != '='; ++C);
  for (*C++ = '='; *C++ = *value++;);
  return(0);
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
