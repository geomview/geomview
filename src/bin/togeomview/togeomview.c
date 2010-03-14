/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2006-2007 Claus-Justus Heine
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/file.h>
#ifdef AIX
# define _BSD  1		/* Get FNDELAY from fcntl.h */
#endif
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#if HAVE_ERRNO_H
# include <errno.h>
#elif HAVE_SYS_ERRNO_H
# include <sys/errno.h>
#else
extern int errno;
#endif
#if HAVE_INET_SOCKETS || HAVE_INET6_SOCKETS
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#ifndef NeXT
#include <unistd.h>
#endif

#ifndef O_NONBLOCK
# define O_NONBLOCK O_NDELAY
#endif
#ifndef FNDELAY
# define FNDELAY O_NDELAY
#endif
#ifndef FNONBLK
# define FNONBLK FNDELAY
#endif

enum streamtype {
  nopipe = -1,
  namedpipe = 0,
  unixsocket,
  inetsocket,
  inet6socket
};

static void usage(const char *prog);

#define DFLT_PORT 29*31*37 /* 33263, so what */
#define DFLT_PORT_STR "33263"
#if HAVE_INET_SOCKETS
static void init_inetaddr(struct sockaddr_in *name,
			  const char *hostname,
			  int port);
#endif
#if HAVE_INET6_SOCKETS
static void init_inet6addr(struct sockaddr_in6 *name,
			   const char *hostname,
			   int port);
#endif

static void interrupt(int sig)
{
  fprintf(stderr, "Couldn't start geomview\n");
  exit(1);
}

/*
 * We've determined that no copy of geomview is running, so start one.
 */
static void start_gv(char **progtorun, char *toname,
		     bool asgeom, enum streamtype pipetype)
{
  char *args[1024];
  int i;
  static const char Mprefix[] = "/tmp/geomview/";
  char Mhow[] = "-Mcsin6";

  signal(SIGALRM, interrupt);
  for (i=0; progtorun[i] != NULL; i++)
    args[i] = progtorun[i];

  Mhow[2] = asgeom ? 'g' : 'c';
  switch (pipetype) {
  case namedpipe: Mhow[3] = 'p'; Mhow[4] = '\0'; break;
  case unixsocket: Mhow[3] = 's'; Mhow[4] = '\0'; break;
  case inetsocket: strcpy(Mhow+3, "sin"); break;
  case inet6socket: strcpy(Mhow+3, "sin6"); break;
  default: break;
  }
  args[i++] = Mhow;
#if HAVE_INET_SOCKETS || HAVE_INET6_SOCKETS
  if (pipetype == inetsocket || pipetype == inet6socket) {
    if ((toname = strrchr(toname, ':')) == NULL) {
      toname = DFLT_PORT_STR;
    } else {
      ++toname;
    }
    args[i++] = toname;
  } else
#endif
    args[i++] = strncmp(toname, Mprefix, sizeof(Mprefix)-1) != 0
      ? toname : toname + sizeof(Mprefix)-1;
  args[i] = NULL;
  
  if (fork() == 0) {
    close(0);
#if SETPGRP_VOID
    setpgrp();
#else
    setpgrp(0,getpid());
#endif
    execvp(progtorun[0], &args[0]);
    execvp("gv", &args[0]);
    perror("Couldn't exec geomview nor gv");
    kill(getppid(), SIGALRM);
    _exit(1);
  }
}

/*
 * Pipe fitting for linking Mathematica to geomview.
 * Starts geomview if not already running.
 */
int main(int argc, char *argv[])
{
  int n, fd = -1;
  int asgeom = 0;
  char pipename[PATH_MAX];
  char buffer[BUFSIZ];
  static char *geomview[] = { "geomview", NULL };
  char **progtorun = geomview;
  char *tail;
  struct stat st;
  char *prog;
#ifdef NeXT
  enum streamtype pipetype = unixsocket;
#else
  enum streamtype pipetype = namedpipe;
#endif
  const char *todir = "/tmp/geomview";
  const char *toname = "OOGL";
  const char *hostname = "localhost";
  char *portstr = DFLT_PORT_STR;
  int port = DFLT_PORT;

  prog = argv[0];
  tail = strrchr(prog, '/');
  if (tail) tail++; else tail = argv[0];
  if (tail[0] == 'g') { asgeom = 1; }

  while (argc > 1 && argv[1][0] == '-') {
    for (tail = argv[1]; *++tail; ) {
      switch(*tail) {
      case 'M': asgeom = 1; break;
      case 'g': asgeom = 1; break;
      case 'c': asgeom = 0; break;
      case 'p': pipetype = namedpipe; break;
      case 's':
	pipetype = unixsocket;
	if (strncmp(tail, "sun", 3) == 0) {
	  tail += 2;
	} else if (strncmp(tail, "sin6", 4) == 0) {
	  pipetype = inet6socket;
	  tail += 3;
	} else if (strncmp(tail, "sin", 3) == 0) {
	  pipetype = inetsocket;
	  tail += 2;
	}
	break;
      default: usage(prog); break;
      }
    }
    argc--, argv++;
  }

#ifndef S_IFIFO
  if (pipetype == namedpipe) {
    fprintf(stderr, "Named pipes not supported.\n");
    exit(EXIT_FAILURE);
  }
#endif
#if !HAVE_UNIX_SOCKETS
  if (pipetype == unixsocket) {
    fprintf(stderr, "Unix domain sockets not supported.\n");
    exit(EXIT_FAILURE);
  }
#endif
#if !HAVE_INET_SOCKETS
  if (pipetype == inetsocket) {
    fprintf(stderr, "IPv4 sockets not supported.\n");
    exit(EXIT_FAILURE);
  }
#endif
#if !HAVE_INET6_SOCKETS
  if (pipetype == inetsocket) {
    fprintf(stderr, "IPv6 sockets not supported.\n");
    exit(EXIT_FAILURE);
  }
#endif

  if (argc > 1) {
    toname = argv[1];
    if (pipetype == inetsocket || pipetype == inet6socket) {
      portstr = strrchr(toname, ':');
      if (portstr == NULL) {
	portstr = DFLT_PORT_STR;
      } else {
	*portstr++ = '\0';
      }
      hostname = toname;
      port = atoi(portstr);
      toname = portstr;
    }
  } else if (pipetype == inetsocket || pipetype == inet6socket) {
    toname = portstr = DFLT_PORT_STR;
  }
  if (argc > 2) {
    progtorun = &argv[2];
  }

  if (pipetype == namedpipe || pipetype == unixsocket) {
    if (toname[0] == '/') {
      tail = strrchr(toname, '/');
      *tail = '\0';
      todir = toname;
      toname = tail + 1;
    }
    if (access(todir, W_OK) < 0) {
      mkdir(todir, 0777);
      chmod(todir, 0777);
    }
    sprintf(pipename, "%s/%s", todir, toname);

    if (stat(pipename, &st) >= 0) {
#if HAVE_UNIX_SOCKETS
      if (pipetype == unixsocket && (st.st_mode & S_IFMT) != S_IFSOCK) {
	unlink(pipename);
      }
#endif
#ifdef S_IFIFO
      if (pipetype == namedpipe && (st.st_mode & S_IFMT) != S_IFIFO) {
	unlink(pipename);
      }
#endif
    }
  } else {
    sprintf(pipename, "%s:%s", hostname, portstr);
  }

  if (pipetype != namedpipe) {
#if HAVE_UNIX_SOCKETS
    struct sockaddr_un un;
#endif
#if HAVE_INET_SOCKETS
    struct sockaddr_in in;
#endif
#if HAVE_INET6_SOCKETS
    struct sockaddr_in6 in6;
#endif
    struct sockaddr *s = NULL;
    int slen = 0;
    bool do_startgv = false;

    switch (pipetype) {
#if HAVE_UNIX_SOCKETS
    case unixsocket: 
      strncpy(un.sun_path, pipename, sizeof(un.sun_path)-1);
      un.sun_family = AF_UNIX;
      fd = socket(PF_UNIX, SOCK_STREAM, 0);
      s    = (struct sockaddr *)&un;
      slen = sizeof(un);
      do_startgv = true;
      break;
#endif
#if HAVE_INET_SOCKETS
    case inetsocket:
      init_inetaddr(&in, hostname, port);
      fd = socket(PF_INET, SOCK_STREAM, 0);
      s    = (struct sockaddr *)&in;
      slen = sizeof(in);
      /* only try to start gv if going through loop-back device */
      do_startgv = in.sin_addr.s_addr == htonl(INADDR_LOOPBACK);
      break;
#endif
#if HAVE_INET6_SOCKETS
    case inet6socket:
      init_inet6addr(&in6, hostname, port);
      fd = socket(PF_INET6, SOCK_STREAM, 0);
      s    = (struct sockaddr *)&in6;
      slen = sizeof(in6);
      do_startgv =
	memcmp(&in6.sin6_addr, &in6addr_loopback, sizeof(struct in6_addr))
	== 0;
      break;
#endif
    default:
      break;
    }
    if (connect(fd, s, slen) < 0) {
      if (errno != ECONNREFUSED && errno != ENOENT) {
	fprintf(stderr, "togeomview: Can't connect to ");
	perror(pipename);
	exit(EXIT_FAILURE);
      }
      if (do_startgv) {
	start_gv(progtorun, pipename, asgeom, pipetype);
	for (n = 0; connect(fd, s, slen) < 0; n++) {
	  if (n == 15) {
	    interrupt(0);
	  }
	  sleep(1);
	}
      } else {
	fprintf(stderr,
		"togeomview: unable to connect to \"%s\"; and "
		"starting Geomview on remote-hosts is not implemented yet.\n",
		pipename);
	exit(EXIT_FAILURE);
      }
    }
#ifdef S_IFIFO
  } else {
    /* Use named pipe */

    if (access(pipename, 0) < 0) {
      mknod(pipename, S_IFIFO, 0);
      chmod(pipename, 0666);
    }
    fd = open(pipename, O_WRONLY|O_NONBLOCK);
    if (fd >= 0) {
      fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) & ~(FNDELAY|FNONBLK|O_NONBLOCK));
    } else if (errno == ENXIO) {
      start_gv(progtorun, pipename, asgeom, pipetype);
      alarm(60);
      fd = open(pipename, O_WRONLY);
      alarm(0);
    }
#endif
  }
  if (fd < 0) {
    fprintf(stderr, "Can't open pipe to geomview: ");
    perror(pipename);
    exit(1);
  }
  while ((n = read(0, buffer, sizeof(buffer))) > 0) {
    if (write(fd, buffer, n) < n) {
      perror("Error writing to geomview");
      exit(1);
    }
  }
  if (!asgeom) {
    /* try to consume answers and echo them on STDOUT.
     *
     * The policy is somewhat questionable; the code below means that
     * each command piped to geomview via togeomview will at least
     * take 1/10 seconds, regardless whether there really comes an
     * answer or not.
     */
    fd_set sset;
    struct timeval tenth = { 0, 100000 };

    FD_ZERO(&sset);
    FD_SET(fd, &sset);

    for (;;) {
      if (select(fd+1, &sset, NULL, NULL, &tenth) == 1) {
	if ((n = read(fd, buffer, sizeof(buffer))) > 0) {
	  int dummy;
	  dummy = write(1, buffer, n);
	}
      } else {
	break;
      }
    }
  }

  exit(EXIT_SUCCESS);
}

static void usage(const char *prog)
{
  char buf[BUFSIZ];
  setbuf(stderr, buf);

  fprintf(stderr,
"Usage: %s [-c] [-g] [-ps[un|in[6]]] [PIPENAME|HOST:PORT [GVPROG ARGS...]]\n"
"Sends lisp-style commands or (with \"-g\") OOGL geometry data to Geomview\n"
"(or the program specified by \"GVPROG\"). The transport channel is a\n"
"named pipe (\"-p\" switch, default, except on the NeXT), a Unix domain\n"
"(\"-s[un]\" switch, default on the NeXT), or a TCP socket, either with the\n"
"IPv4 or IPv6 protocol (\"in[6]\" switch). For \"-p\" and \"-s\" a socket\n"
"or FIFO special file as given by \"PIPENAME\" is created in\n"
"\"/tmp/geomview/\". The default pipename is \"OOGL\". A copy of Geomview\n"
"or \"GVPROG\" is started if none listening on the chosen transport medium\n"
"is yet running. For TCP sockets this is done only when \"HOST\" refers to\n"
"the loopback device.\n"
"Examples:\n"
"	echo '(geometry fred < dodec.off)' | togeomview  sam\n"
"	togeomview -g <dodec.off\n"
"	cat my_geomview_script | togeomview bob  gv -wpos 300x300@500,500\n",
	  prog);
  exit(1);
}

#if HAVE_INET_SOCKETS
void init_inetaddr(struct sockaddr_in *name,
		   const char *hostname,
		   int port)
{
  struct hostent *hostinfo;
  
  name->sin_family = AF_INET;
  name->sin_port = htons(port);
  hostinfo = gethostbyname(hostname);
  if (hostinfo == NULL) {
    fprintf (stderr, "Unknown host %s.\n", hostname);
    exit(EXIT_FAILURE);
  }
  name->sin_addr = *(struct in_addr *) hostinfo->h_addr;
}
#endif

#if HAVE_INET6_SOCKETS
void init_inet6addr(struct sockaddr_in6 *name,
		    const char *hostname,
		    int port)
{
  struct hostent *hostinfo;
  
  name->sin6_family = AF_INET6;
  name->sin6_port = htons(port);
  hostinfo = gethostbyname(hostname);
  if (hostinfo == NULL) {
    fprintf (stderr, "Unknown host %s.\n", hostname);
    exit(EXIT_FAILURE);
  }
  name->sin6_addr = *(struct in6_addr *)hostinfo->h_addr;
}
#endif

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
