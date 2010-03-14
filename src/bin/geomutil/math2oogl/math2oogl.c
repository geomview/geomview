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

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

# include "config.h"

/* math2oogl: convert Mathematica graphics object to OOGL format.
   SurfaceGraphics and MeshGraphics => MESH, Graphics3D => OFF,
   BezierPatch => BEZuvn.

   Note that we expect the graphics objects to have been processed as
   in OOGL.m, i.e. provide dimension and meshrange information and
   print out the colors before the points for SurfaceGraphics objects,
   and convert the characters "(){}, " to a newline.

   Original converter by Nils McCarthy, 
   Geomview pipefitting by Stuart Levy
   SurfaceGraphics converter additions by Tamara Munzner.
   MeshGraphics and BezierPatch converter additions by Silvio Levy
*/



/*
 * Pipe fitting for linking Mathematica to geomview.
 * Starts geomview if not already running.
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(unix) || defined(__unix) || defined(__unix__)
#include <signal.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/un.h>
#endif
#include <sys/errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef AIX
#define _BSD 1		/* Get FNDELAY from <fcntl.h> */
#endif
#include <fcntl.h>

#ifdef NeXT
#include <bsd/libc.h>
#else  /* any other reasonable unix */
#include <unistd.h>
#endif

#ifdef SVR4	/* What should this be called? */
#include <sys/systeminfo.h>
#endif

#ifndef FNDELAY
# define FNDELAY  O_NDELAY
#endif

#ifndef FNONBLK		/* Next 3.0 lacks these in <sys/fcntl.h> */
# define FNONBLK  FNDELAY
#endif

#ifndef O_NONBLOCK
# define O_NONBLOCK FNONBLK
#endif

#ifndef FD_CLOEXEC
#define FD_CLOEXEC 1
#endif

extern int errno;

char *todir = "/tmp/geomview";
char *toname = "Mathematica";

char giveup[] = "Geomview graphics: math2oogl: Couldn't start geomview on ";

/* Fool some Linux distros which are too eager to improve the quality
 * of source code and turn on FORTIFY_SOURCE by default for the
 * C-compiler gcc.
 */
static int ign_write(int fd, void *buffer, size_t size)
{
  return write(fd, buffer, size);
}

static int ign_dup(int fd)
{
  return dup(fd);
}

static void interrupt(int sig) { 
    char myname[1024];
    ign_write(2, giveup, sizeof(giveup));

#ifdef SVR4
	/* jkirk@keck.tamu.edu reports no gethostname() in Solaris (SVR4).
	 * Use sysinfo() instead.
	 */
    sysinfo(SI_HOSTNAME, myname, sizeof(myname));
#else
    gethostname(myname, sizeof(myname));
#endif

    ign_write(2, myname, strlen(myname));
    ign_write(2, "\n", 1);
    exit(1);
}

void start_gv(char **gvpath, char *pipename, char *toname)
{
    char *args[1024];
    int i = 0;
    char **gvp;

    signal(SIGALRM, interrupt);
    args[i++] = gvpath[0];

#ifdef NeXT
    args[i++] = "-Mc";  args[i++] = toname;
#else /* sgi */
    args[i++] = "-c";   args[i++] = pipename;
#endif
	/* Copy remaining args through trailing NULL */
    for(gvp = gvpath; (args[i++] = *++gvp); )
	;

    if(fork() == 0) {
	int savederr = dup(2);
	static char whynot[] = "Geomview graphics: math2oogl: Couldn't find ";

	fcntl(savederr, F_SETFD, FD_CLOEXEC);	/* close this on exec */
	close(0); close(1); close(2);


	open("/dev/null", O_RDWR);	/* Open /dev/null as file descriptors */
	ign_dup(0);  ign_dup(0);	/* 0(stdin), 1(stdout) and 2(stderr) */
		/* (Could just close them, but that seems to poison geomview
		 * if it tries to report an error.)
		 */

#if SETPGRP_VOID
	setpgrp();
#else
	setpgrp(0,getpid());
#endif
	execvp(gvpath[0], &args[0]);
	ign_write(savederr, whynot, sizeof(whynot));
	ign_write(savederr, gvpath[0], strlen(gvpath[0]));
	ign_write(savederr, "\n", 1);
	execvp("geomview", &args[0]);
	execvp("gv", &args[0]);

	dup2(savederr, 2);
	kill(getppid(), SIGALRM);
	interrupt(0);
	_exit(1);
    }
}

void startgv(char **gvpath)
{
    int usesock;
    int n, fd = -1;
    char pipename[BUFSIZ];
    struct sockaddr_un un;

    if(access(todir, W_OK) < 0) {
	mkdir(todir, 0777);
	chmod(todir, 0777);
    }
    sprintf(pipename, "%s/%s", todir, toname);

#ifdef NeXT
    usesock = 1;
#else
    usesock = 0;
#endif

    if(usesock) {
	strncpy(un.sun_path, pipename, sizeof(un.sun_path)-1);
	un.sun_family = AF_UNIX;
	fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(connect(fd, (struct sockaddr *)(&un), sizeof(un)) < 0) {
	    if(errno != ECONNREFUSED && errno != ENOENT) {
		fprintf(stderr, "togeomview: Can't connect to ");
		perror(pipename);
		exit(1);
	    }

	    start_gv(gvpath, pipename, toname);
	    for(n = 0; connect(fd, (struct sockaddr *)(&un), sizeof(un)) < 0; n++) {
		if(n == 60)
		    interrupt(0);
		sleep(1);
	    }
	}
    } else {
	/* Use named pipe */
	if(access(pipename, 0) < 0) {
	    mknod(pipename, S_IFIFO, 0);
	    chmod(pipename, 0666);
	}
	fd = open(pipename, O_WRONLY|O_NONBLOCK);
	if(fd >= 0) {
	    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) & ~(FNDELAY|FNONBLK|O_NONBLOCK));
	} else if(errno == ENXIO) {
	    start_gv(gvpath, pipename, toname);
	    alarm(60);
	    fd = open(pipename, O_WRONLY);
	    alarm(0);
	}
    }
    if(fd < 0) {
	fprintf(stderr, "Can't open pipe to geomview: ");
	perror(pipename);
	exit(1);
    }

    /* Now stdout writes to pipe. */
    dup2(fd, 1);
}


/*
 * Mathematica->OOGL conversion
 */

typedef float Color[3];

#define ismajor isupper

/*
 * The following table should always obey the convention that "major"
 * tokens (corresponding to the cases in the big switch below)
 *  are represented by capitals
 */

enum st {
  IGNORE='i',
  POLYGON='p',
  LINE='l',
  COLOR='c',
  NUMBER='n',
  MESH='m',
  MESHRANGE='k',
  DIMENSIONS='d',
  SG='S',
  MG='M',
  G3='G',
  BG='B'
};

struct line {
  char *data;
  enum st token;
  struct line *next;
};

struct line *lines=NULL;
int size[2];
float range[4];

void usage()
{
  fprintf(stderr,"Usage:  math2oogl [-togeomview <objectname>]");
  exit(1);
}

int main(int ac,char **av)
{
  int togv = 0;
  struct line *lastline=NULL;
  struct line *prev=NULL;
  char buf[1024];
  int npolypoints=0, npolys=0, npolycolors=0;
  int nvectpoints=0, nvects=0, nvectcolors=0;
  struct line *curline, *globline;
  int numnums=0;
  int numcols=0;
  enum st state=IGNORE;
  Color **colors = NULL;
  int i,j,q,ok;
  float xincr, yincr;
  int complex = 0, toss = 0;

  if (ac >= 4 && (!strcmp(av[1],"-togeomview"))) {
    startgv(&av[3]);
    printf("(geometry %s\n", av[2]);
    togv=1;
  } else if (ac > 1) usage();

  for(;;) {
    int c;
    char *k;
	/* Swallow all delimiters, " (){},\n\r".
	 * It's not strictly correct to ignore {} nesting, but...
	 */
    for(k = buf; (c = getchar()) > ' ' && c != ',' && c != '(' && c != ')'
			    && c != '{' && c != '}' && k < buf+sizeof(buf)-1;
			k++) {
	/* Handle escaped newlines, which Mathematica inserts just rarely
	 * enough to be hard to notice!
	 */
	if(c == '\\') {
	    if((c = getchar()) == '\n') {
		while((c = getchar()) == ' ' || c == '\t')
		    ;
	    }
	}
	*k = c;
    }
    if(c == EOF)
	break;
    *k = '\0';
    if (k == buf || !strcmp(buf,"List"))
      continue;

    if (!lastline)
      lastline = lines = malloc(sizeof(struct line));
    else {
      if (toss) lastline = prev;
      toss = 0;
      prev = lastline;
      lastline = lastline->next = malloc (sizeof(struct line));
    }
    lastline->next = NULL;
    lastline->data = malloc(strlen(buf)+1);
    
    strcpy (lastline->data, buf);
    if (isalpha(*lastline->data)) {
      if (!strcmp(lastline->data, "Graphics3D"))
	lastline->token = G3;
      else if (!strcmp(lastline->data, "SurfaceGraphics"))
	lastline->token = SG;
      else if (!strcmp(lastline->data, "MeshGraphics"))
	lastline->token = MG;
      else if (!strcmp(lastline->data, "BezierPatch")||
          !strcmp(lastline->data, "BezierGraphics`BezierPatch"))
	lastline->token = BG;
      else if (!strcmp(lastline->data, "Polygon"))
	lastline->token = POLYGON;
      else if (!strcmp(lastline->data, "Point"))
	lastline->token = LINE; /* degenerate vector */
      else if (!strcmp(lastline->data, "Line"))
	lastline->token = LINE;
      else if (!strcmp(lastline->data, "RGBColor"))
	lastline->token = COLOR;
      else if (!strcmp(lastline->data, "MeshRange"))
	lastline->token = MESHRANGE;
      else if (!strcmp(lastline->data, "Dimensions"))
	lastline->token = DIMENSIONS;
      else if (!strcmp(lastline->data, "Complex")) {
	lastline->token = IGNORE;
	complex = 1;
	toss = 1;
      } else {
	lastline->token = IGNORE;
      }
    } else if (isdigit(*lastline->data) || '-'==*lastline->data) {
      lastline->token = NUMBER;
      /* grab real part, toss imaginary part */
      if (complex) {
	if (complex == 1) {
	  complex++;
	} else if (complex == 2) {
	  lastline->token = IGNORE;
	  complex = 0;
	  toss = 1;
	}
      }
    }
  }
  
  /* we might have multiple graphics objects */
  printf ("{ LIST\n\n");    
  
  ok = 1;
  globline = lines;
  for (globline = lines;globline;) {
    switch (globline->token) {
    case SG: /* SurfaceGraphics */
      globline=globline->next;
      if (globline->token == DIMENSIONS) {
	globline=globline->next;
	for (i = 0; i < 2 && ok; i++, globline = globline->next)
	  if (globline->token == NUMBER) 
	    size[i] = atoi(globline->data);
	  else ok = 0;
      } else 
	ok = 0;
      if (!ok) {
	fprintf(stderr, "can't read mesh dimensions!\n");
	return 1;
      }
      if (globline->token == MESHRANGE) {
	globline=globline->next;
	for (i = 0; i < 4 && ok; i++, globline = globline->next) {
	  if (globline->token == NUMBER) 
	    range[i] = atof(globline->data);
	  else ok = 0;
	}
      } else ok = 0;
      if (!ok) {
	fprintf(stderr, "can't read mesh range!\n");
	return 1;
      }
      
      /* 
	in SurfaceGraphics object, first we get all the colors,
	then all the points: they're not interleaved as in 
	the Graphics3D object. so we need to store them.
	
	points array is size[0] x size[1], 
	color array is size[0]-1 x size[1]-1 (per face not per vertex)
	so repeat the face color for the extra vertices.

	*/
      
      /* if there are colors, store them off */
      if (globline->token == COLOR) {
	numcols = (size[0]-1) * (size[1]-1);
	colors = malloc((size[0]-1)*sizeof(Color*));
	for (i = 0; i < size[0]-1;i++)
	  colors[i] = malloc((size[1]-1)*sizeof(Color));
	ok = 1;
	for (i = 0; i < size[0]-1; i++) {
	  for (j=0; j < size[1]-1; j++) {
	    while (globline->token == COLOR && ok)
	      for (q=0; q < 3 && ok; q++) {
		globline = globline->next;	     
		if (globline->token != NUMBER) {
		  ok = 0;
		  break;
		} else {
		  colors[i][j][q]=atof(globline->data);
		}
	      }
	    globline=globline->next;
	  }
	}
	if (!ok) {
	  fprintf(stderr, "can't read mesh color array!\n");
	  return 1;
	}
      }
      /* Each number we get is just the z coordinate. 
	 Figure out x and y values based on meshrange and size.
	 We don't use a ZMESH because the grid isn't necessarily 
	 an array starting at 0,0: it depends on meshrange
       */	 
      xincr = (range[1] - range[0]) / (size[0]-1);
      yincr = (range[3] - range[2]) / (size[1]-1);
      
      if (numcols)
	printf("\n\n{ CMESH \n");
      else 
	printf("\n\n{ MESH \n");
      printf("\n%d %d\n",size[0], size[1]);
      ok = 1;
      for (i = 0; i < size[0]; i++){
	for (j = 0; j < size[1]; j++) {
	  if (globline->token!=NUMBER) {
	    ok = 0; break;
	  }
	  printf(" %f ", range[0] + xincr*j); /* x */
	  printf(" %f ", range[2] + yincr*i); /* y */
	  printf(" %s ", globline->data);      /* z */
	  /* MESH vertices need RGBA colors 
	     since OOGL MESHes have per-vertex colors, repeat some.
	     repeat last i index, first j index.
	   */
	  if (numcols) {
	    printf(" %f %f %f 1\n", colors[i<size[0]-1?i:i-1][j?j-1:0][0],
		   colors[i<size[0]-1?i:i-1][j?j-1:0][1],
		   colors[i<size[0]-1?i:i-1][j?j-1:0][2]);
	  } else printf("\n");
	  globline=globline->next;
	}
      }
      if (!ok) {
	fprintf(stderr, "can't read mesh points array!\n");
	return 1;
      } else 
	printf("} #end of MESH\n"); 
      break;
    case MG:  /* MeshGraphics */
      globline=globline->next;
      if (globline->token == DIMENSIONS) {
	globline=globline->next;
	for (i = 0; i < 2 && ok; i++, globline = globline->next)
	  if (globline->token == NUMBER) 
	    size[i] = atoi(globline->data);
	  else ok = 0;
      } else 
	ok = 0;
      if (!ok) {
	fprintf(stderr, "can't read mesh dimensions!\n");
	return 1;
      }
      
      /* 
	in MeshGraphics object, first we get all the colors,
	then all the points: they're not interleaved as in 
	the Graphics3D object. so we need to store them.
	
	points array is size[0] x size[1], 
	color array is size[0]-1 x size[1]-1 (per face not per vertex)
	so repeat the face color for the extra vertices.

	*/
      
      /* if there are colors, store them off */
      if (globline->token == COLOR) {
	numcols = (size[0]-1) * (size[1]-1);
	colors = malloc((size[0]-1)*sizeof(Color*));
	for (i = 0; i < size[0]-1;i++)
	  colors[i] = malloc((size[1]-1)*sizeof(Color));
	ok = 1;
	for (i = 0; i < size[0]-1; i++) {
	  for (j=0; j < size[1]-1; j++) {
	    while (globline->token == COLOR && ok)
	      for (q=0; q < 3 && ok; q++) {
		globline = globline->next;	     
		if (globline->token != NUMBER) {
		  ok = 0;
		  break;
		} else {
		  colors[i][j][q]=atof(globline->data);
		}
	      }
	    globline=globline->next;
	  }
	}
	if (!ok) {
	  fprintf(stderr, "can't read mesh color array!\n");
	  return 1;
	}
      }
      /* Each number we get is just the x,y or z coordinate. 
	 We don't use a ZMESH because the grid isn't necessarily 
	 an array starting at 0,0: it depends on meshrange
       */	 
      
      if (numcols)
	printf("\n\n{ CMESH \n");
      else 
	printf("\n\n{ MESH \n");
      printf("\n%d %d\n",size[0], size[1]);
      ok = 1;
      for (i = 0; i < size[0]; i++){
	for (j = 0; j < size[1]; j++) {
	  if (globline->token!=NUMBER) {
	    ok = 0; break;
	  }
	  printf(" %s ", globline->data);      /* x */
          globline=globline->next;
	  if (globline->token!=NUMBER) {
	    ok = 0; break;
	  }
	  printf(" %s ", globline->data);      /* y */
          globline=globline->next;
	  if (globline->token!=NUMBER) {
	    ok = 0; break;
	  }
	  printf(" %s ", globline->data);      /* z */
	  /* MESH vertices need RGBA colors 
	     since OOGL MESHes have per-vertex colors, repeat some.
	     repeat last i index, first j index.
	   */
	  if (numcols) {
	    printf(" %f %f %f 1\n", colors[i<size[0]-1?i:i-1][j?j-1:0][0],
		   colors[i<size[0]-1?i:i-1][j?j-1:0][1],
		   colors[i<size[0]-1?i:i-1][j?j-1:0][2]);
	  } else printf("\n");
	  globline=globline->next;
	}
      }
      if (!ok) {
	fprintf(stderr, "can't read mesh points array!\n");
	return 1;
      } else 
	printf("} #end of MESH\n"); 
    break;
    case BG:  /* BezierGraphics */
      globline=globline->next;
      if (globline->token == DIMENSIONS) {
	globline=globline->next;
	for (i = 0; i < 2 && ok; i++, globline = globline->next)
	  if (globline->token == NUMBER) 
	    size[i] = atoi(globline->data);
	  else ok = 0;
      } else 
	ok = 0;
      if (!ok) {
	fprintf(stderr, "can't read Bezier patch dimensions!\n");
	return 1;
      }
      
      /* 
	in BezierGraphics object, first we get all the colors,
	then all the points.
	
	points array is size[0] x size[1], 
	color array is 2x2.

	*/
      
      /* if there are colors, store them off */
      if (globline->token == COLOR) {
	numcols = 2*2;
	colors = malloc(2*sizeof(Color*));
	for (i = 0; i < 2;i++)
	  colors[i] = malloc(2*sizeof(Color));
	ok = 1;
	for (i = 0; i < 2; i++) {
	  for (j=0; j < 2; j++) {
	    while (globline->token == COLOR && ok)
	      for (q=0; q < 3 && ok; q++) {
		globline = globline->next;	     
		if (globline->token != NUMBER) {
		  ok = 0;
		  break;
		} else {
		  colors[i][j][q]=atof(globline->data);
		}
	      }
	    globline=globline->next;
	  }
	}
	if (!ok) {
	  fprintf(stderr, "can't read mesh color array!\n");
	  return 1;
	}
      }
      /* Each number we get is just the x,y or z coordinate.  */
      
      if (numcols)
	printf("\n\n{ CBEZ");
      else 
	printf("\n\n{ BEZ");
      printf("%d%d3\n",size[1]-1, size[0]-1); /* reverse order */
      ok = 1;
      for (i = 0; i < size[0]; i++){
	for (j = 0; j < size[1]; j++) {
	  if (globline->token!=NUMBER) {
	    ok = 0; break;
	  }
	  printf(" %s ", globline->data);      /* x */
          globline=globline->next;
	  if (globline->token!=NUMBER) {
	    ok = 0; break;
	  }
	  printf(" %s ", globline->data);      /* y */
          globline=globline->next;
	  if (globline->token!=NUMBER) {
	    ok = 0; break;
	  }
	  printf(" %s ", globline->data);      /* z */
	  /* print colors */
	  if (numcols) {
	    printf(" %f %f %f 1\n", colors[i][j][0], colors[i][j][1],
		   colors[i][j][2]);
	  } else printf("\n");
	  globline=globline->next;
	}
      }
      if (!ok) {
	fprintf(stderr, "can't read control points array!\n");
	return 1;
      } else 
	printf("} #end of BEZ\n"); 
    break;
    case G3: /* Graphics3D */
      globline=globline->next;
      npolypoints=npolys=npolycolors=0;
      nvectpoints=nvects=nvectcolors=0;
      numnums=numcols=0;
      state = IGNORE;
      for (curline=globline;;curline=curline->next) {
	if (!curline || (curline->token != NUMBER)) {
	  
	  if (state == LINE) {
	    nvects++;
	    nvectpoints += numnums/3;
	    if (numcols)
	      nvectcolors++;
	    numcols = 0;
	  } else if (state == POLYGON) {
	    npolys++;
	    npolypoints += numnums/3;
	    if (numcols)
	      npolycolors++;
	  }
	  if (!curline || ismajor(curline->token))
	    break; /* go back to main loop */
	  
	  if (curline->token == COLOR)
	    numcols++;
	  
	  state=curline->token;
	  numnums=0;
	} else			/* it's a number */
	  numnums++;
      }      
      if (npolys) {
	
	printf ("\n{ = OFF\n");
	printf ("%d %d %d\n",npolypoints, npolys, 0);
	
	{ /* vertex list */
	  struct line *curline;
	  enum st state=IGNORE;
	  char coordnum=0;

	  for (curline=globline; curline && !ismajor(curline->token);
	       curline=curline->next) {
	    if (curline->token != NUMBER) {
	      state = curline->token;
	      coordnum=0;
	      printf("\n");
	    } else
	      if (state == POLYGON) {
		printf("%s",curline->data);
		if(coordnum++%3 == 2)
		  printf("\n");
		else
		  printf(" ");
	      }
	  }
	}
	{ /* face list (maybe includes color) */
	  Color c;
	  char hascolor = 0;	/* so far. */
	  char thiscolor=1;	/* if this polygon has had its color
				 * tacked on. */
	  char cnum=3;		/* 3 == not doing color now. */
	  struct line *curline;
	  enum st state=IGNORE;
	  int pointnum=0;
	  
	  c[0] = c[1] = c[2] = 0;
	  for (curline=globline;;curline=curline->next) {
	    if (!thiscolor && hascolor &&
		(!curline || curline->token != NUMBER)) {
	      /* OFF faces need just RGB colors, no alpha! */
	      printf(" %f %f %f",c[0],c[1],c[2]);
	      thiscolor = 1;
	    }
	    if (!curline || ismajor(curline->token))
	      break; /* back to main loop */
	    switch (curline->token) {
	    case COLOR:
	      cnum = 0;
	      state = COLOR;
	      hascolor = 1;
	      break;
	    case POLYGON:
	      {
		struct line *countline;
		int numvert=0;
		
		thiscolor = !hascolor;
		
		cnum = 3;
		state = POLYGON;
		for (countline = curline->next; countline; countline=countline->next) {
		  if (countline->token != NUMBER)
		    break;
		  numvert++;
		}
		
		printf("\n%d",numvert/3);
	      }
	      break;
	    case NUMBER:
	      if (state == POLYGON) {
		if (pointnum++%3 == 2)
		  printf(" %d",pointnum/3-1);
	      } else if (state==COLOR) {
		if (cnum<3)
		  c[(int)cnum++] = atof(curline->data);
	      }
	      break;
	    default:
	      state = curline->token;
	      break;
	    }
	  }
	}
	printf("\n\n} #end of OFF\n");
      }
      if (nvects) {
	printf("\n{ = VECT\n");
	printf("%d %d %d\n",nvects, nvectpoints, nvectcolors);
	{
	  struct line *curline;
	  enum st state=IGNORE;
	  int numvert=0, numcol=0;
	  for (curline=globline;;curline=curline->next) {
	    if (numvert && (!curline || curline->token != NUMBER)) {
	      printf("%d ",numvert/3);
	      numvert = 0;
	    }
	    if (!curline || ismajor(curline->token))
	      break; /* back to main loop */
	    if(curline->token == NUMBER && state==LINE)
	      numvert++;
	    else
	      state=curline->token;
	  }
	  printf("\n\n");
	  for (curline=globline; curline && !ismajor(curline->token);
	       curline=curline->next) {
	    if (curline->token == LINE) {
	      printf("%d ",numcol?1:0);
	      numcol = 0;
	    }
	    if(curline->token == COLOR)
	      numcol++;
	  }
	  printf("\n\n");
	}
	{
	  struct line *curline;
	  enum st state=IGNORE;
	  int coordno=0;
	  for (curline=globline; curline && !ismajor(curline->token);
	       curline=curline->next) {
	    if (curline->token != NUMBER)
	      state = curline->token;
	    else
	      if (state == LINE) {
		printf("%s%c",curline->data,((++coordno)%3)?' ':'\n');
	      }
	  }
	}
	{
	  struct line *curline;
	  Color c;
	  char hascolor=0;
	  char cnum=3;
	  
	  c[0] = c[1] = c[2] = 0;
	  
	  for (curline=globline; curline && !ismajor(curline->token);
	       curline=curline->next) {
	    if (curline->token == NUMBER) {
	      if (cnum<3) {
		c[(int)cnum++]=atof(curline->data);
		hascolor = 1;
	      }
	    } else if (curline->token == LINE) {
	      /* note VECTs need RGBA colors */
	      if(hascolor)
		printf("%f %f %f 1\n",c[0],c[1],c[2]);
	      hascolor = 0;
	    } else if (curline->token == COLOR)
	      cnum = 0;
	    
	  }
	}
	printf("\n} #end of VECT\n");
      }
      globline = curline;
    break;
    default:
	    fprintf(stderr, "math2oogl: unexpected data: %s\n", globline->data);
	    return 1;
    } /* end switch (globline->token) */
 }
  printf ("\n} #end of LIST\n");
  if (togv) {		/* end our (geometry */
    printf("\n)\n");
  }
  return 0;
}
