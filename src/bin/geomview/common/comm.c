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


/* Authors: Stuart Levy, Tamara Munzner, Mark Phillips */

#include <stdio.h>
#include <stdlib.h>
#ifdef NeXT
#include <bsd/libc.h>
#else /* any other reasonable UNIX */
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include "ooglutil.h"
#include "drawer.h"
#include "ui.h"
#include "handle.h"
#include "handleP.h"
#include "streampool.h"
#include "main.h"
#include "event.h"
#include "comm.h"
#include "lang.h"
#include "lisp.h"
#include "transobj.h"

extern HandleOps CamOps, GeomOps, TransOps, CommandOps, WindowOps;

int gv_debug = 0;

int commandimport(Pool *, Handle **, Ref **);
int commandclose(Pool *);
int emodule_commandclose(Pool *);
int listenimport(Pool *, Handle **, Ref **);
void useconnection( char *name, HandleOps *ops, Handle *h, Ref *obj, int unique );

static void MyPoolDelete(Pool *p);


HandleOps CommandOps = {
	"command_language",
	commandimport,
	NULL,
	NULL,
	NULL,
	commandclose
};

HandleOps emoduleCommandOps = {
	"command_language",
	commandimport,
	NULL,
	NULL,
	NULL,
	emodule_commandclose,
};

HandleOps listenOps = {	/* "Ops" structure for listening sockets */
	"socket_listener",
	listenimport,	/* "read" routine really spawns a new connection's data socket */
	NULL,
	NULL,
	NULL,
	NULL
};

int 
commandimport(Pool *p, Handle **unused, Ref **unused_too )
{
    char *w, *raww;
    int c;
    int ok = 0;
    IOBFILE *inf;
    Lake *lake;

    if((inf = PoolInputFile(p)) == NULL)
      goto done;

    if((c = async_iobfnextc(inf,0)) == NODATA)
      return 1;		/* pretend we got something. */

    if ((lake=(Lake*)PoolClientData(p)) == NULL) {
	lake = LakeDefine(p->inf, p->outf, p);
	PoolSetClientData(p, (void*)lake);
    } else if(lake->streamin != inf) {
	lake->streamin = inf;
	lake->streamout = PoolOutputFile(p);
    }

    switch(c) {
    case '<':
	iobfgetc(inf);
	w = iobfdelimtok("()", inf, 0);
	if(w == NULL)
	  goto done;
	if(strcmp(w, "-") && (w = findfile(PoolName(p), raww = w)) == NULL) {
	    OOGLSyntax(inf, "Reading commands from \"%s\": can't find command file %s",
		PoolName(p), raww);
	    goto done;
	}
	p = PoolStreamOpen(w, NULL, 0, &CommandOps);
	if (iobfile(PoolInputFile(p)) == stdin && !PoolOutputFile(p))
	  p = PoolStreamOpen(PoolName(p), stdout, 1, &CommandOps);
	if(p != NULL && inf != NULL)
	  ok = comm_object(w, &CommandOps, NULL, NULL, COMM_LATER);
	break;
    case '(':
	{
	  LObject *val;
	  val = LEvalSexpr(lake);
	  ok = (val != Lnil) ? 1 : -1;
	  LFree(val);
	}
	break;
    default:
	{
	  LFree( LEvalSexpr(lake) );
	}
    }
  done:
    return ok;
}

/*
 * What is this file?  Returns
 *  -1 : nonexistent
 *   0 : is a plain file
 *   1 : is something else, probably a named pipe or tty
 */
int
ispipe(char *fname)
{
    struct stat st;

    if(stat(fname, &st) < 0)
	return -1;
    return (st.st_mode & S_IFMT) != S_IFREG;
}

int
comm_object(char *str, HandleOps *ops, Handle **hp, Ref **rp, int now)
{
    int c, ok = 0;
    Pool *p;

    if(str == NULL)
	return 0;
    if(strcmp(str, "-") == 0 || access(str, 0) == 0) {
	Handle *h = HandleReferringTo('<', str, ops, hp);
	/*
	 * If we haven't read something from this file yet,
	 * forget it.
	 */
	if(h) {
	    if(HandleObject(h)) {
		ok = 1;
		if(rp) HandleUpdRef(&h, NULL, rp);
	    } else if(((p = PoolByName(HandleName(h)))) == NULL ||
			(p->flags & PF_ANY) || (!p->seekable && !now)) {
		
		/* When reading plain files, always demand an object.
		 * When reading others (pipe, tty), demand one if 'now' set.
		 * Use PF_ANY flag as an alternate hint of reading an object,
		 *  since reading commands leaves no object attached to h.
		 */
		ok = 1;
	    } else {
		/* Failed */
		HandleDelete(h);
	    }
	}
	/* If not ok, close the file.
	 * If 'now' and not some sort of pipe, also close the file.
	 */
	if((p = PoolByName(str)) != NULL && (!ok || (now && p->seekable))) {
	    if(now && ok) {
		/* Read as much as possible if we need it right now. */
		while(PoolInputFile(p) != NULL &&
		      (c = async_iobfnextc(PoolInputFile(p), 0)) != NODATA &&
		      c != EOF && (*ops->strmin)(p, hp, rp))
		  ;
	    }
	    PoolClose(p);
	    MyPoolDelete(p);
	} else if (iobfile(PoolInputFile(p)) == stdin
		   && PoolOutputFile(p) == NULL)
	  p = PoolStreamOpen(PoolName(p), stdout, 1, ops);
	return ok;
    } else if(strpbrk(str, "({ \t\n")) {
	static Pool *pcache;	/* Cache a pool for handling strings */
	static int inuse = 0;	/* Use cached pool unless already in use */
	IOBFILE *inf = iobfileopen(fmemopen(str, strlen(str), "rb"));
		    /* Caching implies this first pool has a long lifetime;
		     * suitable for expressing (interest (...)) 
		     */
	if(!inuse) {
	    if((p = pcache) == NULL)
		p = pcache = PoolStreamTemp(str, inf, NULL, 0, ops);
	    inuse = 1;
	} else {
	    p = PoolStreamTemp(str, inf, NULL, 0, ops);
	}
	if(p == NULL)
	    return 0;		/* Failed */
	p->inf = inf;
	p->outf = stdout;	/* Attach default output stream */
	while(iobfnextc(inf, 0) != EOF)
	    ok = (*ops->strmin)(p, hp, rp);
	PoolClose(p);
	if(p == pcache)
	    inuse = 0;
	else
	    MyPoolDelete(p); /* Delete temp pool unless it's our cached one */
    } else {
	/* Print the "No such file..." error left by access() */
	fprintf(stderr, "%s: %s\n", str, sperror());
    }
    return ok;
}


/*
 * Arrange that later attempts to read the same file will actually re-read it.
 */
int
commandclose(Pool *p)
{
    PoolDoReread(p);
    PoolClose(p);
    return 0;
}


LDEFINE(setenv, LVOID,
"(setenv  name string)  sets the environment variable ``name'' to the value\n\
``string''; the name is visible to geomview (as in pathnames containing $name)\n\
and to processes it creates, e.g. external modules.")
{
    char *name, *string = NULL;
    char buf[10240];

    LDECLARE(("setenv", LBEGIN,
	LSTRING, &name,
	LSTRING, &string,
	LEND));
    sprintf(buf, "%s=%s", name, string);
    envexpand(buf+strlen(name)+1);
    putenv(strdup(buf));
    return Lt;
}


LDEFINE(input_translator, LVOID,
"(input-translator  \"#prefix_string\"  \"Bourne-shell-command\")\n\
Defines an external translation program for special input types.\n\
When asked to read a file which begins with the specified string,\n\
geomview invokes that program with standard input coming from the given file.\n\
The program is expected to emit OOGL geometric data to its standard output.\n\
In this implementation, only prefixes beginning with # are recognized.")
{
    char *prefix, *cmd;
    LDECLARE(("input-translator", LBEGIN,
	LSTRING, &prefix,
	LSTRING, &cmd,
	LEND));
    GeomAddTranslator(prefix, cmd);
    return Lt;
}

/************************************************************************/

/*
 * Signal handling routines.
 * Any code here must be careful not to do anything unsafe at signal time,
 * e.g. calls to stdio routines or malloc.  These routines just examine and
 * change fields in existing data structures.
 */

/*
 * Handle death of a child process -- possibly an emodule.
 * If it is, mark it dead.
 */
static void
comm_sigchld(int sig)
{
#if HAVE_WAITPID
  int status;
  int pid = waitpid(-1, &status, WNOHANG);
#elif HAVE_WAIT3
  union wait status;
  int pid = wait3(&status, WNOHANG|WUNTRACED, NULL);
#else
# error FIXME
#endif

  if(WIFEXITED(status) || WIFSIGNALED(status)) {
    emodule *em = VVEC(uistate.emod, emodule);
    int i;
    for(i = 0; i < VVCOUNT(uistate.emod); i++, em++) {
	if(em->pid == pid) {
	    em->pid = -pid;
	    uistate.emod_check = 1;
	}
    }
  }
  signal(SIGCHLD, comm_sigchld);
}

/*
 * SIGTTIN indicates we're reading from stdin, but we've been placed in the
 * background.  Suspend reading from stdin for two seconds.
 */
static void
comm_sigttin(int sig)
{
  Pool *p = PoolByName("-");
  if(p != NULL)
    PoolSleepFor(p, 2.);

  signal(SIGTTIN, comm_sigttin);

#ifdef SA_RESTART	/* Do we have sigaction()? If so, be sure
			 * SIGTTIN won't restart the read it interrupts!
			 */
  {
    struct sigaction sa;
    if(sigaction(SIGTTIN, NULL, &sa) >= 0) {
	sa.sa_flags &= ~SA_RESTART;
	sigaction(SIGTTIN, &sa, NULL);
    }
  }
#endif
}

static void
fatalsig(int sig)
{
  char *msg;
  static char msg0[] = "Geomview: internal error: ";
  static char msg1[] = "; dump core now (y/n) [n] ? ";
  char die = 'y';

  switch(sig) {
  case SIGSEGV: msg = "Segmentation violation"; break;
  case SIGBUS: msg = "Bus error"; break;
  case SIGILL: msg = "Illegal instruction"; break;
  default:     msg = "unknown signal?"; break;
  }
  write(2, msg0, sizeof(msg0)-1);
  write(2, msg, strlen(msg));
  write(2, msg1, sizeof(msg1)-1);
  read(2, &die, 1);
  if(die != 'y' && die != 'Y')
	gv_exit();
  /* else return, and hope the OS gives us a core dump. */
  signal(sig, SIG_DFL);
}

typedef void (*mysigfunc_t)();

static void
catchsig(int sig, mysigfunc_t func)
{
   mysigfunc_t oldfunc = (mysigfunc_t)signal(sig, func);
   if(oldfunc == (mysigfunc_t)SIG_IGN)
	signal(sig, oldfunc);
}


void 
comm_init()
{
  signal(SIGCHLD, comm_sigchld);
  signal(SIGTTIN, comm_sigttin);
  signal(SIGPIPE, SIG_IGN);	/* Write on broken pipe -> I/O error */
  if(!gv_debug) {
    catchsig(SIGINT, (mysigfunc_t)gv_exit);
    catchsig(SIGSEGV, (mysigfunc_t)fatalsig);
    catchsig(SIGBUS, (mysigfunc_t)fatalsig);
    catchsig(SIGILL, (mysigfunc_t)fatalsig);
  }
  catchsig(SIGHUP, (mysigfunc_t)gv_exit);
  catchsig(SIGTERM, (mysigfunc_t)gv_exit);


  lang_init();
}

int 
comm_route( char *str )
{
    comm_object( str, &CommandOps, NULL, NULL, COMM_LATER );
    return 0;
}


int
emodule_commandclose(Pool *p)
{
  int i;
  emodule *em;

  for(i = 0, em = VVEC(uistate.emod, emodule); i < VVCOUNT(uistate.emod); i++, em++) {
    if(em->link == p && em->pid <= 0) {
	emodule_reap(em);
	return 1;
    }
  }
  return 0;
}

void
emodule_reap(emodule *em)
{
  Lake *lake;
  Pool *p = em->link;

  if(p != NULL) {
    em->link = NULL;
    if ((lake=(Lake*)PoolClientData(p)) != NULL) {
      RemoveLakeInterests(lake);
    }
    PoolClose(p);
    MyPoolDelete(p);
  }
  ui_emodule_uninstall(em - VVEC(uistate.emod, emodule));
  return; 
}

int
emodule_kill(emodule *em)
{
  kill(-em->pid, SIGHUP);   /* Kill child's process group in case it has one */
  /* Kill child process itself.  comm_sigchld() ought to be invoked if it dies,
   * but put this check here, just in case, since we're having trouble on the
   * SGI.
   */
  if(kill(em->pid, SIGHUP) < 0 && errno == ESRCH) {
    em->pid = -abs(em->pid);
    uistate.emod_check = 1;
  }
  return 1;
}

LDEFINE(emodule_run, LVOID,
       "(emodule-run  SHELL-COMMAND ARGS...)\n\
	Runs the given SHELL-COMMAND (a string containing a UNIX shell\n\
	command) as an external module.  The module's standard output\n\
	is taken as geomview commands; responses (written to filename\n\
	\"-\") are sent to the module's standard input.  The shell\n\
	command is interpreted by /bin/sh, so e.g. I/O redirection may\n\
	be used; a program which prompts the user for input from the\n\
	terminal could be run with:\n\
	  (emodule-run  yourprogram  <&2)\n\
	If not already set, the environment variable $MACHTYPE is set\n\
	to the name of the machine type.  Input and output\n\
	connections to geomview are dropped when the shell command\n\
	terminates.  Clicking on a running program's module-browser entry\n\
	sends the signal SIGHUP to the program.  For this to work, programs\n\
	should avoid running in the background; those using FORMS or GL\n\
	should call foreground() before the first FORMS or winopen() call.\n\
	See also emodule-define, emodule-start.")
{
  char *cmd;
  emodule em;

  LDECLARE(("emodule-run", LBEGIN,
	    LSTRINGS, &cmd,
	    LEND));
  em.text = em.name = cmd;
  em.data = NULL;
  em.dir = NULL;
  emodule_run(&em);
  return Lt;
}

/* Does str appear to contain template?
 * Check if str is either equal to the template, or contains template as a
 * prefix followed by a blank.
 */
static int matches(char *template, int len, char *str)
{
   if(str == NULL) return 0;
   return (strncasecmp(template, str, len) == 0 &&
		(str[len] == '\0' || isspace(str[len])));
}

/* This routine searches through the running emodules and returns the index
 * of the first on matching modname.  Case sensitive.  modname is 
 * looked for first in the names as they appear in the browser 
 * (minus the number in []'s) then as they appear in the command used to 
 * run the module, minus the arguments.  Returns -1 if the module is not
 * found.
 * -cf */
int emodule_running_index(char *modname) 
{
  char *name;
  emodule *em;
  int i, len;

  if (modname == NULL) return -1;
  len = strlen(modname);
  for (i = 0, em = VVEC(uistate.emod, emodule); i < VVCOUNT(uistate.emod);
       i++, em++) 
    if (em->pid > 0) {
      name = strchr(em->name, ']');
      if (name == NULL) name = em->name;
      else name++;
      if (matches(modname, len, name) || matches(modname, len, em->name)
		|| matches(modname, len, em->text))
	return i;
    }
  return -1;
}


LDEFINE(emodule_isrunning, LVOID,
	"(emodule-isrunning NAME)\n\
	Returns Lt if the emodule NAME is running, or Lnil\n\
	if it is not running.  NAME is searched for in the\n\
	names as they appear in the browser and in the shell commands\n\
	used to execute the external modules (not including arguments).")
{
  char *modname;

  LDECLARE(("emodule-isrunning", LBEGIN,
	    LSTRING, &modname,
	    LEND));
  if (emodule_running_index(modname) != -1) return Lt;
  else return Lnil;
}

/*
 * NeXT applications need to know the absolute paths of their executables.
 * If we got an emodule definition from a ``.geomview-X'' file in directory Y,
 * and the definition begins with the name of a file living in Y,
 * then prepend the directory name to the shell command.
 * The strcspn() is an attempt to grab the first word of a shell command.
 */
static char *
truepath(char *program, char *dir)
{
    static char *buf = NULL;
    int len;
    char path[10240];

    len = strcspn(program, "()<> \t;");
    if(len == 0) return program;
    sprintf(path, "%s/%.*s", dir, len, program);
    envexpand(path);
    if(access(path, X_OK) == 0) {
	if(buf) free(buf);
	strcat(path, program + len);
	program = buf = strdup(path);
    }
    return program;
}
	

    
emodule *
emodule_run(emodule *em)
{
  struct pipe { int r, w; } pfrom, pto;
  emodule *newem;
  int i, pid;
  char *program;
  int otherpgrp = 1;
  char seqname[128];

  program = em->text;
  if(program[0] == '!') {
    program++;
    otherpgrp = 0;
  }

  /* create the communication pipes */
  pfrom.r = pfrom.w = -1;
  if(pipe((int *)&pfrom) < 0 || pipe((int *)&pto) < 0) {
    OOGLError(1, "Can't create pipe to external module: %s", sperror());
    close(pfrom.r); close(pfrom.w);
    return NULL;
  }

  signal(SIGCHLD, SIG_DFL);
  /* invoke external module */
  switch(pid = fork()) {
  case -1:
	OOGLError(1, "Can't fork external module: %s", sperror());
	return NULL;

  case 0: {
	static char rats[] = "Can't exec external module: ";
	char envbuf[10240];

	if(otherpgrp) {
#if SETPGRP_VOID
	    setpgrp();
#else
	    setpgrp(0,getpid());
#endif
	}
	if (em->dir) {
	  program = truepath(program, em->dir);
	  sprintf(envbuf, "PATH=%s:%s", em->dir, getenv("PATH"));
	  envexpand(envbuf);
	} else {
	  /* Append known module directories to the subprocess' PATH.
	   * This lets us emodule-run an existing module program with
	   * special arguments, without having to specify its full path.
	   */
	  char *p = envbuf;
	  sprintf(envbuf, "PATH=%s", getenv("PATH"));
	  for(i = 0; i < emodule_path_count; i++) {
	    p += strlen(p);
	    *p++ = ':';
	    strcpy(p, VVEC(vv_emodule_path, char *)[i]);
	  }
	}
	putenv(envbuf);

	close(pfrom.r);
	close(pto.w);
	dup2(pto.r, 0);
	close(pto.r);
	dup2(pfrom.w, 1);
	close(pfrom.w);
	signal(SIGPIPE, SIG_DFL);
	signal(SIGCHLD, SIG_DFL);
	execl("/bin/sh", "sh", "-c", program, NULL);

	write(2, rats, sizeof(rats)-1);
	perror(em->text);
	exit(1);
      }
  }

  close(pto.r);
  close(pfrom.w);

  for(i=1; ; i++) {
    sprintf(seqname, "[%d]%.100s", i, em->name);
    if(ui_emodule_index(seqname,NULL) < 0)
	break;
  }

  /*
   * Register new module in the UI's table.
   * Setting its callback to emodule_kill() means that clicking on that
   * browser entry will kill the module.
   * We insert it before the entry that created it, if any;
   * otherwise at the beginning of the table.
   */
  i = em - VVEC(uistate.emod, emodule);
  if(i < 0 || i > VVCOUNT(uistate.emod))
    i = 0;
  newem = ui_emodule_install(i, seqname, emodule_kill);

  newem->link = PoolStreamOpen( seqname, fdopen(pfrom.r, "rb"), 0, &emoduleCommandOps );
  if(newem->link) {
	/* Attach output stream, too. */
    PoolStreamOpen(seqname, fdopen(pto.w, "w"), 1, &emoduleCommandOps);

	/* Kludge.  We want to ensure that EOF indications are "hard", i.e.
	 * that we cease reading and drop the emodule as soon as we see EOF.
	 * Unfortunately I can't find any way in the refcomm library
	 * to distinguish nameless pipes (-> hard eof) from named ones
	 * (where eof may be a temporary condition).  So the library
	 * guesses they're soft.  We need to tell it otherwise. -slevy 921005
	 */
    newem->link->softEOF = 0;
  }
  newem->pid = pid;
  signal(SIGCHLD, comm_sigchld);
  return newem;
}

LDEFINE(command, LVOID,
       "(command        INFILE [OUTFILE])\n\
	Read commands from INFILE; send corresponding responses\n\
	(e.g. anything written to filename \"-\") to OUTFILE, stdout\n\
	by default.")
{
  char *file, *ofile = NULL;
  LDECLARE(("command", LBEGIN,
	    LSTRING, &file,
	    LOPTIONAL,
	    LSTRING, &ofile,
	    LEND));
  if(PoolStreamOpen(file, NULL, 0, &CommandOps) == NULL) {
    OOGLError(0,"command: cannot open input %s: %s\n", file, sperror());
    return Lnil;
    }
  if (ofile) {
    FILE *outf = (strcmp(ofile, "-")) ? fopen(ofile, "w") : stdout;
    if(outf == NULL) {
      OOGLError(0,"command: cannot open output %s: %s\n", ofile, sperror());
      return Lnil;
    }
    else
      PoolStreamOpen(file, outf, 1, &CommandOps);
  }
  return Lt;
}

LDEFINE(sleep_for, LVOID,
	"(sleep-for  TIME)\n\
	Suspend reading commands from this stream for TIME seconds.\n\
	Commands already read will still be executed; ``sleep-for'' inside\n\
	``progn'' won't delay execution of the rest of the progn's contents.")
{
  Lake *sweenie;
  float time;

  LDECLARE(("sleep-for", LBEGIN,
	LLAKE, &sweenie,
	LFLOAT, &time,
	LEND));
  PoolSleepFor(POOL(sweenie), time);
  return Lt;
}

LDEFINE(sleep_until, LFLOAT,
	"(sleep-until TIME)\n\
	Suspend reading commands from this stream until TIME (in seconds).\n\
	Commands already read will still be executed; ``sleep-until'' inside\n\
	``progn'' won't delay execution of the rest of the progn's contents.\n\
	Time is measured according to this stream's clock, as set by\n\
	``set-clock''; if never set, the first sleep-until sets it to 0\n\
	(so initially (sleep-until TIME) is the same as (sleep-for TIME)).\n\
	Returns the number of seconds until TIME.")
{
  Lake *bass;
  float time;

  LDECLARE(("sleep-until", LBEGIN,
	LLAKE, &bass,
	LFLOAT, &time,
	LEND));
  PoolSleepUntil(POOL(bass), time);
  time -= PoolTimeAt(POOL(bass), NULL);	/* NULL => now. */
  return LNew(LFLOAT, &time);
}

LDEFINE(set_clock, LVOID,
	"(set-clock TIME)\n\
	Adjusts the clock for this command stream to read TIME (in seconds)\n\
	as of the moment the command is received.  See also sleep-until, clock.")
{
  Lake *bass;
  float time;
  LDECLARE(("set-clock", LBEGIN,
	LLAKE, &bass,
	LFLOAT, &time,
	LEND));
  PoolSetTime(POOL(bass), NULL, time);
  return Lt;
}

LDEFINE(clock, LVOID,
	"(clock)\n\
	Returns the current time, in seconds, as shown by this stream's clock.\n\
	See also set-clock and sleep-until.")
{
  Lake *rainy;
  float time;
  LDECLARE(("clock", LBEGIN,
	LLAKE, &rainy,
	LEND));
  time = PoolTimeAt(POOL(rainy), NULL);
  return LNew(LFLOAT, &time);
}

/* This used to be the guts of echo - it's now in a seperate procedure
 * for use by emodule-transmit.
 * -cf */
void echo_to_fp(LList *arglist, FILE *fp)
{
  LObject *arg, *val;

  if(arglist == NULL) {
    fputs("\n", fp);
  } else {
    for(;;) {
	arg = arglist->car;
	if(arg->type == LSTRING)
	    fputs(LSTRINGVAL(arg), fp);
	else if(arg->type == LLIST) {
	    val = LEval(arg);
	    LWrite(fp, val);
	    LFree(val);
	} else
	    LWrite(fp, arg);
	if((arglist = arglist->cdr) == NULL)
	    break;
	fputs(" ", fp);
    }
  }
  fflush(fp);
}

LDEFINE(echo, LVOID,
       "(echo          ...)\n\
	Write the given data to the special file \"-\".  Strings are written\n\
	literally; lisp expressions are evaluated and their values written.\n\
	If received from an external program, \"echo\" sends to the program's\n\
	input.  Otherwise writes to geomview's own standard output\n\
	(typically the terminal).")
{
  Lake *powderhorn;
  LList *arglist = NULL;
  FILE *fp;
  LDECLARE(("echo", LBEGIN,
	    LHOLD,
	    LLAKE, &powderhorn,
	    LREST, &arglist));
  
  if ( (fp = PoolOutputFile(POOL(powderhorn))) == NULL)
    fp = stdout;
  echo_to_fp(arglist, fp);

  return Lt;
}


LDEFINE(emodule_transmit, LVOID,
	"(emodule-transmit NAME LIST)\n\
	Places LIST into external module NAME's standard input.  NAME is\n\
	searched for in the names of the modules as they appear in the\n\
	External Modules browser and then in the shell commands used to\n\
	execute the external modules.  Does nothing if modname is not\n\
	running.")
{
  char *modname;
  emodule *em;
  LList *message = NULL;
  int i;
  LDECLARE(("emodule-transmit", LBEGIN,
	    LSTRING, &modname,
	    LHOLD,
	    LREST, &message));

  i = emodule_running_index(modname);
  if (i == -1) return Lnil;
  em = VVINDEX(uistate.emod, emodule, i);
  if(em->link && em->link->outf) {
    echo_to_fp(message, em->link->outf);
    return Lt;
  }
  return Lnil;
}



LDEFINE(read, LVOID,
       "(read {geometry|camera|transform|command} {GEOMETRY or CAMERA or ...})\n\
	Read and interpret the text in ... as containing the\n\
	given type of data.  Useful for defining objects using OOGL\n\
	reference syntax, e.g.\n\
\n\
	  (geometry  thing { INST  transform : T    geom : fred })\n\
	  (read  geometry  { define fred QUAD 1 0 0  0 1 0  0 0 1  1 0 0 })\n\
	  (read  transform { define T <myfile})")
/*
  NO LDECLARE !
  There is currently no C interface to this function.  Perhaps we
  don't really want one since "read" is an inherently lisp thing.
  Or perhaps one that takes a string and reads it?

  Since we don't use LDECLARE we must remember to LDefun()
  this function manually in lang_init() in file lang.c.
*/
{
  char *opsname = NULL;
  HandleOps *ops;
  LObject *kw = NULL;
  int c;

  if (lake != NULL) {
    /* parse first arg [ops]: */
    if (!LakeMore(lake, c) ||
	(kw = LSexpr(lake)) == Lnil ||
	!LFROMOBJ(LSTRING)(kw, &opsname) ||
	!(ops = str2ops(opsname))) {
      OOGLSyntax(lake->streamin, "\"read\" in \"%s\": keyword expected (command|geometry|camera|window|transform), got \"%s\"",
	  LakeName(lake), opsname);
      goto fail;
    }

    /* parse 2nd arg, using ops determined by 1st arg.
       Note: we don't actually store the 1st arg because this function's
       work is all done during parsing.  */
    if(!LakeMore(lake,c) || (*ops->strmin)(POOL(lake), NULL, NULL) == 0) {
      OOGLSyntax(lake->streamin, "\"read %s\" in \"%s\": error reading %s's",
		opsname, PoolName(POOL(lake)), opsname);
      goto fail;
    }
    return Lt;
  }
  /* if lake == NULL then we're evaluating, but this function does
     no evaluation work; it's all in the parsing */
  return Lt;
 fail:
  LFree(kw);
  return Lnil;
}

void gv_merge(HandleOps *ops, int camid, Ref *object)
{
  CameraStruct cs;		/* Might be either camera or window, really */
  cs.h = NULL;
  cs.cam = REFINCR(Camera, object);     /* Since (merge ...) will delete it */

  LFree( LEvalFunc("merge",
		LSTRING, ops == &CamOps ? "camera" : "window",
		LID, camid,
		ops == &CamOps ? LCAMERA : LWINDOW, &cs,
		LEND) );
}

LDEFINE(merge, LVOID,
       "(merge          {window|camera} CAM-ID  { WINDOW or CAMERA ... } )\n\
	Modify the given window or camera, changing just those properties\n\
	specified in the last argument.  E.g.\n\
		(merge camera \"Camera\" { far 20 })\n\
	sets Camera's far clipping plane to 20 while leaving\n\
	other attributes untouched.")
/*
  Since we don't use LDECLARE we must remember to LDefun()
  this function manually in lang_init() in file lang.c.
*/
{
  char *opsname = NULL;
  HandleOps *ops;
  int c, id;
  LObject *kw = NULL, *idarg = NULL, *item = NULL;

  if (lake != NULL) {
    /* parse first arg [ops]: */
    if (! LakeMore(lake,c) || (kw = LSexpr(lake)) == Lnil ||
	!LFROMOBJ(LSTRING)(kw, &opsname) ||
	  ((ops = str2ops(opsname)) != &CamOps && ops != &WindowOps)) {
      OOGLSyntax(lake->streamin,
	"\"merge\" in \"%s\": expected \"camera\" or \"window\", got \"%s\"", LakeName(lake), opsname);
      goto parsefail;
    }

    /* parse 2nd arg; it's a string (id) */
    if (! LakeMore(lake,c) || (idarg = LEvalSexpr(lake)) == Lnil) {
      OOGLSyntax(lake->streamin,"\"merge\" in \"%s\": expected CAM-ID",
	LakeName(lake));
      goto parsefail;
    }

    item = LPARSE(((ops == &CamOps) ? LCAMERA : LWINDOW))(lake);
    if(item == Lnil) {
	OOGLSyntax(lake->streamin, "\"merge\" in \"%s\": error reading %s",
	    LakeName(lake), LSTRINGVAL(kw));
	goto parsefail;
    }
    LListAppend(args, kw);
    LListAppend(args, idarg);
    LListAppend(args, item);
    return Lt;
  }

  kw = LListEntry(args, 1);
  idarg = LListEntry(args, 2);
  item = LListEntry(args, 3);
  if(!LFROMOBJ(LSTRING)(kw, &opsname) ||
	((ops = str2ops(opsname)) != &CamOps && ops != &WindowOps)) {
    OOGLError(0, "\"merge\": expected \"camera\" or \"window\", got %s",
	LSummarize(idarg));
    return Lnil;
  }
  if (!LFROMOBJ(LID)(idarg, &id) || !ISCAM(id)) {
    OOGLError(0, "\"merge\": expected CAM-ID in arg position 2, got %s",
	LSummarize(idarg));
    return Lnil;
  }
  if (ops == &CamOps) {
    CameraStruct *cs;
    if (!LFROMOBJ(LCAMERA)(item, &cs)) {
      OOGLError(0,"\"merge\": expected camera in arg position 3");
      return Lnil;
    }
    drawer_merge_camera(id, cs->cam);
    CamDelete(cs->cam);
  }
  else {
    WindowStruct *ws;
    if (!LFROMOBJ(LWINDOW)(item, &ws)) {
      OOGLError(0,"\"merge\": expected window in arg position 3");
      return Lnil;
    }
    drawer_merge_window(id, ws->wn);
    WnDelete(ws->wn);
  }
  return Lt;

 parsefail:
  LFree(kw);
  LFree(idarg);
  LFree(item);
  return Lnil;
}

static void
MyPoolDelete(Pool *p)
{
  Lake *lake = (Lake*)PoolClientData(p);
  if (lake) {
    RemoveLakeInterests(lake);
    LakeFree(lake);
  }
  PoolDelete(p);
}

HandleOps *
str2ops(char *str)
{
  if(str == NULL) return NULL;
  else if(!strncmp(str, "cam", 3)) return &CamOps;
  else if(!strncmp(str, "geom", 4)) return &GeomOps;
  else if(!strncmp(str, "comm", 4)) return &CommandOps;
  else if(!strncmp(str, "trans", 5)) return &TransOps;
  else if(!strncmp(str, "win", 3)) return &WindowOps;
  else return NULL;
}

LType *
ops2ltype(HandleOps *ops)
{
  if(ops == &CamOps) return LCAMERA;
  else if(ops == &GeomOps) return LGEOM;
  else if(ops == &WindowOps) return LWINDOW;
  else if(ops == &TransOps) return LTRANSFORM;
  else if(ops == &CommandOps) return LLOBJECT;
  else return NULL;
}

/************************************************************************/

LDEFINE(load, LINT,
	"(load  filename  [command|geometry|camera])\n\
	Loads the given file into geomview.  The optional second argument\n\
	specifies the type of data it contains, which may be \"command\"\n\
	(geomview commands), \"geometry\" (OOGL geometric data), or\n\
	\"camera\" (OOGL camera definition).  If omitted, attempts to guess\n\
	about the file's contents.\n\
	Loading geometric data creates a new visible object; loading a camera\n\
	opens a new window; loading a command file executes those commands.\n")
{
  char *file, *opsname = NULL;
  HandleOps *ops = &CommandOps;
  int guess = 1;

  LDECLARE(("load", LBEGIN,
	    LSTRING, &file,
	    LOPTIONAL,
	    LSTRING, &opsname,
	    LEND));

  if(opsname != NULL) {
    guess = 0;
    ops = str2ops(opsname);
    if(ops != &CommandOps && ops != &GeomOps && ops != &CamOps) {
	OOGLError(0, "load: expected \"command\" or \"geometry\" or \"camera\", got \"%s\"", opsname);
	return Lnil;
    }
  }
  loadfile(file, ops, guess);
  return Lt;
}

void
loadfile(char *name, HandleOps *defops, int guess)
{
  Handle *h = NULL;
  Ref *obj = NULL;
  HandleOps *ops = NULL;
  char *pathname;
  int freename = 0;
  
  if(strcmp(name, "-") == 0) {
    guess = 0;
  } else if ((pathname=findfile(NULL, name))) {
    name = strdup(pathname);
    freename = 1;
  } else if(strchr(name, ' ') == NULL && strchr(name, '(') == NULL
	    && strchr(name, '<') == NULL) {
    OOGLError(0, "Can't find file %s", name);
    return;
  }
  
  if (ispipe(name)) {
    guess = 0;
  }
  if(!guess) {
    if(comm_object(name, defops, &h, &obj, COMM_LATER)) {
	ops = defops;
    } else {
	OOGLError(0, "Can't load %s's from %s", defops->prefix, name);
	return;
    }
  } else if (comm_object(name, &GeomOps, &h, &obj, COMM_LATER)) {
    ops = &GeomOps;
  } else if (comm_object(name, &CamOps, &h, &obj, COMM_LATER)) {
    ops = &CamOps;
  } else if (!(comm_object(name, &CommandOps, NULL, NULL, COMM_LATER))) {
    OOGLError(0,"Can't load %s",name);
    return;
  }
  useconnection( name, ops, h, obj, 1 );
  if(freename) free(name);
  return;
}

LDEFINE(hdefine, LVOID,
	"(hdefine  \"geometry\"|\"camera\"|\"transform\"|\"window\"  name  value)\n\
	Sets the value of a handle of a given type.\n\
	  (hdefine  <type>  <name>  <value>)  is generally equivalent to\n\
	  (read <type>  { define <name> <value> })\n\
	except that the assignment is done when hdefine is executed,\n\
	(possibly not at all if inside a conditional statement),\n\
	while the ``read ... define'' performs assignment as soon as the\n\
	text is read.\n")
{
  HandleOps *ops = NULL;
  LType *ltype;
  char *hname;
  char *opsname = NULL;
  Ref *obj;
  int c;
  LObject *kw = NULL, *name = NULL, *item = NULL;
  union {
    GeomStruct gs;
    CameraStruct cs;
    TransformStruct ts;
    WindowStruct ws;
    ApStruct as;
    LObject lobj;
  } *s;

  if (lake != NULL) {
    /* parse first arg [ops]: */
    if (! LakeMore(lake,c) || (kw = LSexpr(lake)) == Lnil ||
	!LFROMOBJ(LSTRING)(kw, &opsname) ||
	  (ops = str2ops(opsname)) == NULL ||
	  (ltype = ops2ltype(ops)) == NULL) {
      OOGLSyntax(lake->streamin,
	"\"hdefine\" in \"%s\": expected \"camera\" or \"window\" or \"transform\" or \"geometry\", got \"%s\"", LakeName(lake), opsname);
      goto parsefail;
    }

    /* parse 2nd arg; it's a string (id) */
    if (! LakeMore(lake,c) || (name = LEvalSexpr(lake)) == Lnil) {
      OOGLSyntax(lake->streamin,"\"hdefine %s\" in \"%s\": expected handle name",
	LakeName(lake), opsname);
      goto parsefail;
    }

    item = LPARSE(ltype)(lake);
    if(item == Lnil) {
	OOGLSyntax(lake->streamin, "\"hdefine\" in \"%s\": error reading %s",
	    LakeName(lake), LSTRINGVAL(kw));
	goto parsefail;
    }
    LListAppend(args, kw);
    LListAppend(args, name);
    LListAppend(args, item);
    return Lt;
  }

  kw = LListEntry(args, 1);
  name = LListEntry(args, 2);
  item = LListEntry(args, 3);
  if(!LFROMOBJ(LSTRING)(kw, &opsname) ||
	(ops = str2ops(opsname)) == NULL ||
	(ltype = ops2ltype(ops)) == NULL) {
    OOGLError(0, "\"hdefine\": expected data type, got %s",
	LSummarize(kw));
    return Lnil;
  }
  if (!LFROMOBJ(LSTRING)(name, &hname)) {
    OOGLError(0, "\"hdefine\": expected handle name, got %s", LSummarize(name));
    return Lnil;
  }
  if(!LFROMOBJ(ltype)(item, &s)) {
    OOGLError(0, "\"hdefine\": Can't extract %s from %s",
	ltype->name, LSummarize(item));
    return Lnil;
  }
  if(ops == &TransOps) obj = (Ref *)TransCreate( s->ts.tm );
  else if(ops == &CommandOps) obj = NULL; /* (Ref *)LispCreate( s->lobj ) */
  else obj = (Ref *)s->gs.geom;	/* All other types resemble geoms */
  HandleAssign(hname, ops, obj);
  return Lt;

 parsefail:
  LFree(kw);
  LFree(name);
  LFree(item);
  return Lnil;
}


/*****************************************************************************/

/*
 * makesocket(name) makes a UNIX-domain listening socket and returns its fd.
 */
int
makesocket(char *name)
{
    struct sockaddr_un un;
    int s;

    unlink(name);
    strcpy(un.sun_path, name);
    un.sun_family = AF_UNIX;
    if((s = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
	OOGLError(0, "geomview: can't make UNIX domain socket: %s", sperror());
	return -1;
    }
    if(bind(s, (struct sockaddr *)&un, sizeof(un)) < 0 || listen(s, 4) < 0) {
	OOGLError(0, "geomview: can't listen on socket %s: %s",name,sperror());
	return -1;
    }
    return s;
}


/*
 * This pretends to be a stream-import routine, but it really handles
 * new connections on UNIX-domain sockets!
 */
int
listenimport(Pool *listenp, Handle **hp, Ref **rp)
{
    struct sockaddr_un un;
    ACCEPT_ARG3_TYPE len = sizeof(un);
    char conname[10240];
    int i, ds;
    Pool *p;
    HandleOps *ops = (HandleOps *)PoolClientData(listenp);

    if((ds =
	accept(
	       iobfileno(listenp->inf),
	       (struct sockaddr *)&un,
	       &len)
	) < 0) {
	OOGLError(0, "geomview: couldn't accept connection on %s: %s",
		listenp->poolname, sperror());
	return 0;
    }
    for(i=1; ; i++) {	/* Construct unique connection name */
	sprintf(conname, "[%d]%.500s", i, listenp->poolname);
	if(PoolByName(conname) == NULL)
	    break;
    }
    p = PoolStreamOpen(conname, fdopen(ds, "rb"), 0, ops);
    /*
     * Reply on the same pipe's return stream.
     * Don't do this yet; only user so far is "togeomview",
     * which isn't prepared to receive data back from us.
     * PoolStreamOpen(conname, fdopen(ds, "w"), 1, (HandleOps*)PoolClientData(listenp));
     */
    useconnection( listenp->poolname, ops, HandleCreate(conname, ops), NULL, 0 );
    return 1;
}


/*
 * usepipe(dir, suffix, type)
 * where the characters in "type" specify:
 * kind of connection (named-pipe 'p' or UNIX-domain socket 's') and
 * type of data expected (command 'c' or geometry 'g').
 */
void
usepipe(char *pipedir, char *suffix, char *pipetype)
{
    HandleOps *ops = &GeomOps;
    int s, usepipe = -1;
    char *tail;
    char pipename[10240];
    char pdir[10240];

    if(suffix[0] == '/') {
	strcpy(pdir, suffix);
	tail = strrchr(pdir, '/');
	*tail = '\0';
	pipedir = pdir;
	suffix = tail + 1;
    }
    sprintf(pipename, "%s/%s", pipedir, suffix);
    mkdir(pipedir, 0777);
    chmod(pipedir, 0777);

    while(*pipetype) switch(*pipetype++) {
	case 's': usepipe = 0; break;
	case 'p': usepipe = 1; break;
	case 'c': ops = &CommandOps; break;
	case 'g': ops = &GeomOps; break;
	default: OOGLError(0, "Unknown character '%c' in pipe type string; expected s, p, c, g", pipetype[-1]);
    }
    if(usepipe < 0) {
#ifdef NeXT
	usepipe = 0;
#else
	usepipe = 1;
#endif
    }

    if(usepipe) {
	/* Establish System-V style named pipe.
	 * Expect data on it of type 'ops'.
	 * If there's a non-pipe with that name, trash it.
	 */
	struct stat st;
	if(stat(pipename, &st) == 0 && (st.st_mode & S_IFMT) != S_IFIFO)
	    unlink(pipename);
	if(access(pipename, 0) < 0) {
	    if(mknod(pipename, S_IFIFO, 0) < 0)
		OOGLError(1, "Can't make pipe: %s: %s", suffix, sperror());
	    chmod(pipename, 0666);
	}
	loadfile(pipename, ops, 0);
    } else {
	/* Establish UNIX-domain listener socket.
	 * When we get connections to it, expect data of type 'ops'.
	 */
	s = makesocket(pipename);
	if(s >= 0) {
	    Pool *p = PoolStreamOpen(pipename, fdopen(s, "rb"), 0, &listenOps);
	    if(p) PoolSetClientData(p, ops);
	    p->flags |= PF_NOPREFETCH;
	}
    }
}

/*
 * Assume we have a connection open named "name", expecting data of
 * type "ops".  Wire it to an appropriate geomview object, if any.
 * (Data received on that connection will set the value of that object.)
 * Since (new_)geometry and (new_)camera take ownership of the object,
 * increasing its reference count, we decr the ref count after handing it over.
 */
void
useconnection( char *name, HandleOps *ops, Handle *h, Ref *obj, int unique )
{
    char *tail;
    tail = strrchr(name, '/');
    if(tail) tail++; else tail = name;

    if(ops == &GeomOps) {
	GeomStruct gs;
	gs.h = h;
	gs.geom = (Geom *)obj;
	if(unique) gv_new_geometry( tail, &gs );
	else gv_geometry( tail, &gs );
	GeomDelete((Geom *)obj);	/* Maintain ref count */
    } else if(ops == &CamOps) {
	CameraStruct cs;
	cs.h = h;
	cs.cam = (Camera *)obj;
	if(unique) gv_new_camera( tail, &cs );
	else gv_camera( tail, &cs );
	CamDelete((Camera *)obj);	/* Maintain ref count */
    }
    HandleDelete(h);			/* Maintain ref count */
}
