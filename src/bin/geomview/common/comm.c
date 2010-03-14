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
#include "ntransobj.h"

extern HandleOps CamOps, GeomOps, TransOps, NTransOps, CommandOps, WindowOps;
extern HandleOps ImageOps, AppearanceOps;

int gv_debug = 0;

static int commandimport(Pool *, Handle **, Ref **);
static int commandclose(Pool *);
static int emodule_commandclose(Pool *);
static int listenimport(Pool *, Handle **, Ref **);
static void useconnection( char *name, HandleOps *ops, Handle *h, Ref *obj, int unique );

static void MyPoolDelete(Pool *p);


HandleOps CommandOps = {
  "command_language",
  commandimport,
  NULL,
  NULL,
  NULL,
  commandclose
};

static HandleOps emoduleCommandOps = {
  "command_language",
  commandimport,
  NULL,
  NULL,
  NULL,
  emodule_commandclose,
};

#if HAVE_UNIX_SOCKETS
static HandleOps listenOps = {	/* "Ops" structure for listening sockets */
  "socket_listener",
  listenimport,	/* "read" routine really spawns a new connection's
		 * data socket
		 */
  NULL,
  NULL,
  NULL,
  NULL
};
#endif

static int ign_read(int fd, void *buffer, size_t size)
{
  int result;
  
  result = read(fd, buffer, size);
  
  return result;
}

static int 
commandimport(Pool *p, Handle **unused, Ref **unused_too )
{
  char *w, *raww;
  int c;
  int ok = 0;
  IOBFILE *inf;
  Lake *lake;

  if ((inf = PoolInputFile(p)) == NULL)
    goto done;

  if ((c = async_iobfnextc(inf,0)) == NODATA)
    return 1;		/* pretend we got something. */

  if ((lake=(Lake*)PoolClientData(p)) == NULL) {
    lake = LakeDefine(p->inf, p->outf, p);
    PoolSetClientData(p, (void*)lake);
  } else if (lake->streamin != inf) {
    lake->streamin = inf;
    lake->streamout = PoolOutputFile(p);
  }

  switch(c) {
  case '<':
    iobfgetc(inf);
    w = iobfdelimtok("()", inf, 0);
    if (w == NULL)
      goto done;
    if (strcmp(w, "-") && (w = findfile(PoolName(p), raww = w)) == NULL) {
      OOGLSyntax(inf, "Reading commands from \"%s\": can't find command file %s",
		 PoolName(p), raww);
      goto done;
    }
    p = PoolStreamOpen(w, NULL, 0, &CommandOps);
    if (iobfile(PoolInputFile(p)) == stdin && !PoolOutputFile(p))
      p = PoolStreamOpen(PoolName(p), stdout, 1, &CommandOps);
    if (p != NULL && inf != NULL)
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
static int
ispipe(char *fname)
{
  struct stat st;

  if (stat(fname, &st) < 0)
    return -1;
  return (st.st_mode & S_IFMT) != S_IFREG;
}

int
comm_object(char *str, HandleOps *ops, Handle **hp, Ref **rp, int now)
{
  int c, ok = 0;
  Pool *p;

  if (str == NULL)
    return 0;
  if (strcmp(str, "-") == 0 || access(str, 0) == 0) {
    Handle *h = HandleReferringTo('<', str, ops, hp);
    /*
     * If we haven't read something from this file yet,
     * forget it.
     */
    if (h) {
      if (HandleObject(h)) {
	ok = 1;
	if (rp) {
	  HandleUpdRef(&h, NULL, rp);
	}
      } else if (((p = PoolByName(HandleName(h), ops))) == NULL ||
		 (p->flags & PF_ANY) || (!p->seekable && !now)) {
		
	/* When reading plain files, always demand an object. When
	 * reading others (pipe, tty), demand one if 'now' set. Use
	 * PF_ANY flag as an alternate hint of reading an object,
	 * since reading commands leaves no object attached to h.
	 */
	ok = 1;
      } else {
	/* Failed */
	HandleDelete(h);
	if (hp) {
	  *hp = NULL;
	}
      }
    }
    /* If not ok, close the file.
     * If 'now' and not some sort of pipe, also close the file.
     */
    if ((p = PoolByName(str, ops)) != NULL && (!ok || (now && p->seekable))) {
      if (now && ok) {
	/* Read as much as possible if we need it right now. */
	while(PoolInputFile(p) != NULL &&
	      (c = async_iobfnextc(PoolInputFile(p), 0)) != NODATA &&
	      c != EOF && (*ops->strmin)(p, hp, rp))
	  ;
      }
      PoolClose(p);
      MyPoolDelete(p);
    } else if (iobfile(PoolInputFile(p)) == stdin 
	       && PoolOutputFile(p) == NULL) {
      p = PoolStreamOpen(PoolName(p), stdout, 1, ops);
    }
    return ok;
  } else if (strpbrk(str, "({ \t\n")) {
    static Pool *pcache;	/* Cache a pool for handling strings */
    static bool inuse = false;	/* Use cached pool unless already in use */
    IOBFILE *inf = iobfileopen(fmemopen(str, strlen(str), "rb"));
    /* Caching implies this first pool has a long lifetime;
     * suitable for expressing (interest (...)) 
     */
    if (!inuse) {
      if ((p = pcache) == NULL) {
	p = pcache = PoolStreamTemp(str, inf, stdout, 2, ops);
      } else {
	p->inf  = inf; /* hack */
	p->outf = stdout; /* hack */
      }
      inuse = true;
    } else {
      p = PoolStreamTemp(str, inf, stdout, 2, ops);
    }
    if (p == NULL) {
      return 0;		/* Failed */
    }
    while(iobfnextc(inf, 0) != EOF) {
      ok = (*ops->strmin)(p, hp, rp);
    }
    PoolClose(p);
    if (p == pcache) {
      inuse = false;
    } else {
      MyPoolDelete(p); /* Delete temp pool unless it's our cached one */
    }
  } else {
    /* Print the "No such file..." error left by access() */
    fprintf(stderr, "%s: %s\n", str, sperror());
  }
  return ok;
}


/*
 * Arrange that later attempts to read the same file will actually re-read it.
 *
 * cH: above comment is crap: at least since v1.6 a PoolClose() would
 * result in actually closing the file, so a new PoolStreamOpen()
 * would position right at the start of the data. We just delete the
 * pool.
 */
static int
commandclose(Pool *p)
{
  PoolClose(p);
  MyPoolDelete(p);
  return 0;
}


LDEFINE(setenv, LVOID,
	"(setenv  name string)\n"
	"Sets the environment variable ``name'' to the value ``string''; the "
	"name is visible to geomview (as in pathnames containing $name) "
	"and to processes it creates, e.g. external modules.")
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
	"(input-translator  \"#prefix_string\"  \"Bourne-shell-command\")\n"
	"Defines an external translation program for special input types. "
	"When asked to read a file which begins with the specified string, "
	"Geomview invokes that program with standard input coming from the "
	"given file. The program is expected to emit OOGL geometric data to "
	"its standard output. In this implementation, only prefixes beginning "
	"with # are recognized.")
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

  if (WIFEXITED(status) || WIFSIGNALED(status)) {
    emodule *em = VVEC(uistate.emod, emodule);
    int i;
    for(i = 0; i < VVCOUNT(uistate.emod); i++, em++) {
      if (em->pid == pid) {
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
  Pool *p = PoolByName("-", NULL);
  if (p != NULL)
    PoolSleepFor(p, 2.);

  signal(SIGTTIN, comm_sigttin);

#ifdef SA_RESTART	/* Do we have sigaction()? If so, be sure
			 * SIGTTIN won't restart the read it interrupts!
			 */
  {
    struct sigaction sa;
    if (sigaction(SIGTTIN, NULL, &sa) >= 0) {
      sa.sa_flags &= ~SA_RESTART;
      sigaction(SIGTTIN, &sa, NULL);
    }
  }
#endif
}

static const char *signal_name(int sig)
{
  static char buffer[256];

  switch(sig) {
  case SIGSEGV:
    return "Segmentation violation";
  case SIGBUS:
    return "Bus error";
  case SIGILL:
    return "Illegal instruction";
  default:
    snprintf(buffer, sizeof(buffer), "Signal number %d\n", sig);
    return buffer;
  }
}

static void
fatalsig(int sig)
{
  char die = 'y';

  fprintf(stderr,
	  "Geomview(%d): internal error: \"%s\"; "
	  "dump core now (y/n) [n] ? ",
	  getpid(), signal_name(sig));
  fflush(stderr);
  ign_read(2, &die, 1);
  fprintf(stderr, "got answer %c\n", (int)die % 0xff);
  fflush(stderr);
  if (die != 'y' && die != 'Y')
    gv_exit();
  /* else return, and hope the OS gives us a core dump. */
  signal(sig, SIG_DFL);
}

typedef void (*mysigfunc_t)();

static void
catchsig(int sig, mysigfunc_t func)
{
  mysigfunc_t oldfunc = (mysigfunc_t)signal(sig, func);
  if (oldfunc == (mysigfunc_t)SIG_IGN)
    signal(sig, oldfunc);
}


void 
comm_init()
{
  signal(SIGCHLD, comm_sigchld);
  signal(SIGTTIN, comm_sigttin);
  signal(SIGPIPE, SIG_IGN);	/* Write on broken pipe -> I/O error */
  if (!gv_debug) {
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


static int
emodule_commandclose(Pool *p)
{
  int i;
  emodule *em;

  for(i = 0, em = VVEC(uistate.emod, emodule); i < VVCOUNT(uistate.emod); i++, em++) {
    if (em->link == p && em->pid <= 0) {
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

  if (p != NULL) {
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
  if (kill(em->pid, SIGHUP) < 0 && errno == ESRCH) {
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
  if (str == NULL) return 0;
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
  if (len == 0) return program;
  sprintf(path, "%s/%.*s", dir, len, program);
  envexpand(path);
  if (access(path, X_OK) == 0) {
    if (buf) free(buf);
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
  if (program[0] == '!') {
    program++;
    otherpgrp = 0;
  }

  /* create the communication pipes */
  pfrom.r = pfrom.w = -1;
  if (pipe((int *)&pfrom) < 0 || pipe((int *)&pto) < 0) {
    OOGLError(1, "Can't create pipe to external module: %s", sperror());
    if (pfrom.r >= 0) {
      close(pfrom.r); close(pfrom.w);
    }
    return NULL;
  }

  signal(SIGCHLD, SIG_DFL);
  /* invoke external module */
  switch(pid = fork()) {
  case -1:
    OOGLError(1, "Can't fork external module: %s", sperror());
    return NULL;

  case 0: {
    char envbuf[10240];

    if (otherpgrp) {
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
    dup2(pto.r, STDIN_FILENO);
    close(pto.r);
    dup2(pfrom.w, STDOUT_FILENO);
    close(pfrom.w);
    signal(SIGPIPE, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
    execl("/bin/sh", "sh", "-c", program, NULL);

    fprintf(stderr, "Can't exec external module: ");
    perror(em->text);
    exit(1);
  }
  default: /* parent */
    break;
  }

  close(pto.r);
  close(pfrom.w);

  for(i=1; ; i++) {
    sprintf(seqname, "[%d]%.100s", i, em->name);
    if (ui_emodule_index(seqname,NULL) < 0)
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
  if (i < 0 || i > VVCOUNT(uistate.emod))
    i = 0;
  newem = ui_emodule_install(i, seqname, emodule_kill);

  newem->link = PoolStreamOpen( seqname, fdopen(pfrom.r, "rb"), 0, &emoduleCommandOps );
  if (newem->link) {
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

  if (PoolStreamOpen(file, NULL, 0, &CommandOps) == NULL) {
    OOGLError(0,"command: cannot open input %s: %s\n", file, sperror());
    return Lnil;
  }
  if (ofile) {
    FILE *outf = (strcmp(ofile, "-")) ? fopen(ofile, "w") : stdout;
    if (outf == NULL) {
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
	"(clock)\n"
	"Returns the current time, in seconds, as shown by this stream's "
	"clock.	See also set-clock and sleep-until.")
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

  if (arglist == NULL) {
    fputs("\n", fp);
  } else {
    for(;;) {
      arg = arglist->car;
      /* Note that -- for compatibility -- we distinguish the case of
       * a function which evaluates to a string and a literal
       * string. A literal string is written without quotes, a
       * function evaluating to a string results in printing a quoted
       * string. An example is (echo (read-id focus)) which used to
       * result in printing the id of the active camera as quoted
       * string.
       */
      if (arg->type == LSTRING) {
	/* omit the quotes */
	fputs(LSTRINGVAL(arg), fp);
      } else {
	val = LEval(arg);
	LWrite(fp, val);
	LFree(val);
      }
      if ((arglist = arglist->cdr) == NULL) {
	break;
      }
      fputs(" ", fp);
    }
  }
  fflush(fp);
}

LDEFINE(echo, LVOID,
	"(echo ...)\n"
	"Write the given data to the special file \"-\".  Strings are written "
	"literally; lisp expressions are evaluated and their values written. "
	"If received from an external program, \"echo\" sends to the "
	"program's input. Otherwise writes to geomview's own standard output "
	"(typically the terminal).")
{
  Lake *powderhorn;
  LList *arglist = NULL;
  FILE *fp;

  LDECLARE(("echo", LBEGIN,
	    LLAKE, &powderhorn,
	    LHOLD,
	    LREST, &arglist,
	    LEND));
  
  if ((fp = PoolOutputFile(POOL(powderhorn))) == NULL) {
    fp = stdout;
  }
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
	    LREST, &message,
	    LEND));

  i = emodule_running_index(modname);
  if (i == -1) {
    return Lnil;
  }
  em = VVINDEX(uistate.emod, emodule, i);
  if (em->link && em->link->outf) {
    echo_to_fp(message, em->link->outf);
    return Lt;
  }
  return Lnil;
}



LDEFINE(read, LVOID,
	"(read {geometry|camera|image|appearance|transform|ntransform|command} "
	"{GEOMETRY or CAMERA or ...})\n"
	"Read and interpret the text in ... as containing the "
	"given type of data.  Useful for defining objects using OOGL "
	"reference syntax, e.g. "
	"\n\n\n\n"
	"(geometry  thing { INST  transform : T    geom : fred })\n\n"
	"(read  geometry  { define fred QUAD 1 0 0  0 1 0  0 0 1  1 0 0 })\n\n"
	"(read  transform { define T <myfile})")
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

  if (LPARSEMODE) {
    /* parse first arg [ops]: */
    if (!LakeMore(lake) ||
	(kw = LSexpr(lake)) == Lnil ||
	!LSTRINGFROMOBJ(kw, &opsname) ||
	!(ops = str2ops(opsname))) {
      OOGLSyntax(lake->streamin,
		 "\"read\" in \"%s\": keyword "
		 "{command|geometry|camera|window|transform|"
		 "ntransform|image|appearance} expected, "
		 "got \"%s\"",
		 LakeName(lake), opsname);
      goto fail;
    }

    /* parse 2nd arg, using ops determined by 1st arg.
       Note: we don't actually store the 1st arg because this function's
       work is all done during parsing.  */
    if (!LakeMore(lake) || (*ops->strmin)(POOL(lake), NULL, NULL) == 0) {
      OOGLSyntax(lake->streamin, "\"read %s\" in \"%s\": error reading %s's",
		 opsname, PoolName(POOL(lake)), opsname);
      goto fail;
    }
  }
  /* if lake == NULL then we're evaluating, but this function does
     no evaluation work; it's all in the parsing */
  LFree(kw);
  return Lt;

 fail:
  LFree(kw);
  return Lnil;
}

void gv_merge(HandleOps *ops, int camid, Ref *object)
{
  CameraStruct cs;		/* Might be either camera or window, really */

  cs.h = NULL;
#if 0
  cs.cam = REFGET(Camera, object);     /* Since (merge ...) will delete it */
#endif

  LFree(LEvalFunc("merge",
		  LSTRING, ops == &CamOps ? "camera" : "window",
		  LID, camid,
		  ops == &CamOps ? LCAMERA : LWINDOW, &cs,
		  LEND) );
}

LDEFINE(merge, LVOID,
	"(merge {window|camera} CAM-ID  { WINDOW or CAMERA ... } )\n"
	"Modify the given window or camera, changing just those properties "
	"specified in the last argument. E.g.\n\n"
	"               (merge camera \"Camera\" { far 20 })\n\n"
	"sets Camera's far clipping plane to 20 while leaving "
	"other attributes untouched.")
/*
  Since we don't use LDECLARE we must remember to LDefun()
  this function manually in lang_init() in file lang.c.
*/
{
  char *opsname = NULL;
  HandleOps *ops;
  int id;
  LObject *kw = NULL, *idarg = NULL, *item = NULL;

  if (LPARSEMODE) {
    /* parse first arg [ops]: */
    if (!LakeMore(lake) || (kw = LSexpr(lake)) == Lnil ||
	!LSTRINGFROMOBJ(kw, &opsname) ||
	((ops = str2ops(opsname)) != &CamOps && ops != &WindowOps)) {
      OOGLSyntax(lake->streamin,
		 "\"merge\" in \"%s\": expected \"camera\" or \"window\", "
		 "got \"%s\"", LakeName(lake), opsname);
      goto parsefail;
    }

    /* parse 2nd arg; it's a string (id) */
    if (!LakeMore(lake) || (idarg = LEvalSexpr(lake)) == Lnil) {
      OOGLSyntax(lake->streamin,"\"merge\" in \"%s\": expected CAM-ID",
		 LakeName(lake));
      goto parsefail;
    }

    item = LPARSE(((ops == &CamOps) ? LCAMERA : LWINDOW))(lake);
    if (item == Lnil) {
      OOGLSyntax(lake->streamin, "\"merge\" in \"%s\": error reading %s",
		 LakeName(lake), LSTRINGVAL(kw));
      goto parsefail;
    }
    LListAppend(args, kw);
    LListAppend(args, idarg);
    LListAppend(args, item);
    return Lt;
  } else {
    LParseArgs("merge", LBEGIN, LREST, &args, LEND);
  }

  kw = LListEntry(args, 1);
  idarg = LListEntry(args, 2);
  item = LListEntry(args, 3);
  if (!LSTRINGFROMOBJ(kw, &opsname) ||
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
    /* CamDelete(cs->cam); */
  } else {
    WindowStruct *ws;
    if (!LFROMOBJ(LWINDOW)(item, &ws)) {
      OOGLError(0,"\"merge\": expected window in arg position 3");
      return Lnil;
    }
    drawer_merge_window(id, ws->wn);
    /* WnDelete(ws->wn); */
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
  if (str == NULL) return NULL;
  else if (!strncmp(str, "cam", 3)) return &CamOps;
  else if (!strncmp(str, "geom", 4)) return &GeomOps;
  else if (!strncmp(str, "comm", 4)) return &CommandOps;
  else if (!strncmp(str, "trans", 5)) return &TransOps;
  else if (!strncmp(str, "ntrans", 6)) return &NTransOps;
  else if (!strncmp(str, "win", 3)) return &WindowOps;
  else if (!strncmp(str, "image", sizeof("image")-1)) return &ImageOps;
  else if (!strncmp(str, "appearance",
		    sizeof("appearance")-1)) return &AppearanceOps;
  else return NULL;
}

LType *
ops2ltype(HandleOps *ops)
{
  if (ops == &CamOps) return LCAMERA;
  else if (ops == &GeomOps) return LGEOM;
  else if (ops == &WindowOps) return LWINDOW;
  else if (ops == &TransOps) return LTRANSFORM;
  else if (ops == &NTransOps) return LTRANSFORMN;
  else if (ops == &CommandOps) return LLOBJECT;
  else if (ops == &ImageOps) return LIMAGE;
  else if (ops == &AppearanceOps) return LAP;
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

  if (opsname != NULL) {
    guess = 0;
    ops = str2ops(opsname);
    if (ops != &CommandOps && ops != &GeomOps && ops != &CamOps) {
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
  
  if (strcmp(name, "-") == 0) {
    guess = 0;
  } else if ((pathname=findfile(NULL, name))) {
    name = strdup(pathname);
    freename = 1;
  } else if (strchr(name, ' ') == NULL && strchr(name, '(') == NULL
	    && strchr(name, '<') == NULL) {
    OOGLError(0, "Can't find file %s", name);
    return;
  }
  
  if (ispipe(name)) {
    guess = 0;
  }
  if (!guess) {
    if (comm_object(name, defops, &h, &obj, COMM_LATER)) {
      ops = defops;
    } else {
      OOGLError(0, "Can't load %s's from %s", defops->prefix, name);
      return;
    }
  } else if (comm_object(name, &GeomOps, &h, &obj, COMM_LATER)) {
    ops = &GeomOps;
  } else if (comm_object(name, &CamOps, &h, &obj, COMM_LATER)) {
    ops = &CamOps;
  } else if (!(comm_object(name, &CommandOps, &h, NULL, COMM_LATER))) {
    OOGLError(0,"Can't load %s",name);
    return;
  }
  useconnection( name, ops, h, obj, 1 );
  if (freename) {
    free(name);
  }
  return;
}

LDEFINE(hdelete, LVOID,
	"(hdelete [geometry|camera|window|image|appearance|transform|ntransform] name)\n"
	"Deletes the given handle. Note that the handle will not actually be "
	"deleted in case there are still other objects referring to the "
	"handle, but once those objects are gone, the handle will also "
	"automatically go away. The object the handle refers to (if any) "
	"will only be deleted if there are no other references to that object."
	"\n\n\n\n"
	"If the optional first argument is omitted, then the first handle "
	"matching \"name\" will be deleted, regardless of the type of the "
	"object it is attached to. It is not an error to call this function "
	"with a non-existent handle, but it is an error to call this funcion "
	"with the name of a non-global handle, i.e. one that was not "
	"created by (hdefine ...) or (read ... { define ...}).")
{
  Handle *h = NULL;
  HandleOps *ops = NULL;
  char *hname = NULL, *opsname = NULL, *horo;

  LDECLARE(("hdelete", LBEGIN,
	    LSTRING, &horo,
	    LOPTIONAL,
	    LSTRING, &hname,
	    LEND));

  if (hname == NULL) {
    hname = horo;
  } else {
    opsname = horo;
  }

  if (opsname && ((ops = str2ops(opsname)) == NULL || ops2ltype(ops) == NULL)) {
    OOGLError(0, "\"hdelete\": "
	      "expected \"camera\" or \"window\" or \"geometry\" or "
	      "\"transform\" or \"ntransform\" or "
	      "\"image\" or \"appearance\", got \"%s\"",
	      opsname);
    return Lnil;
  }
  
  if ((h = HandleByName(hname, ops)) != NULL) {
    REFPUT(h); /* undo HandleByName() REFGET */
    if (!h->permanent) {
      OOGLError(0, "\"hdelete\": "
		"attempt to delete the non-global handle \"%s[%s]\"",
		opsname ? opsname : "", hname);
      return Lnil;
    }
    h->permanent = false;
    HandleDelete(h);
  }
  return Lt;
}


LDEFINE(hdefine, LVOID,
	"(hdefine "
	"{geometry|camera|window|image|appearance|transform|ntransform} "
	"name value)\n"
	"Sets the value of a handle of a given type. "
	"(hdefine  <type>  <name>  <value>)  is generally equivalent to "
	"(read <type>  { define <name> <value> }) "
	"except that the assignment is done when hdefine is executed, "
	"(possibly not at all if inside a conditional statement), "
	"while the ``read ... define'' performs assignment as soon as the "
	"text is read.")
{
  Handle *h = NULL;
  HandleOps *ops = NULL;
  LType *ltype;
  char *hname;
  char *opsname = NULL;
  Ref *obj;
  LObject *kw = NULL, *name = NULL, *item = NULL;
  union {
    GeomStruct gs;
    CameraStruct cs;
    TransformStruct ts;
    TmNStruct tns;
    WindowStruct ws;
    ApStruct as;
    ImgStruct img;
    LObject lobj;
  } *s;

  if (LPARSEMODE) {
    /* parse first arg [ops]: */
    if (! LakeMore(lake) || (kw = LSexpr(lake)) == Lnil ||
	!LSTRINGFROMOBJ(kw, &opsname) ||
	(ops = str2ops(opsname)) == NULL ||
	(ltype = ops2ltype(ops)) == NULL) {
      OOGLSyntax(lake->streamin,
		 "\"hdefine\" in \"%s\": "
		 "expected \"camera\" or \"window\" or \"geometry\" or "
		 "\"transform\" or \"ntransform\" or "
		 "\"image\" or \"appearance\", got \"%s\"",
		 LakeName(lake), opsname);
      goto parsefail;
    }

    /* parse 2nd arg; it's a string (id) */
    if (!LakeMore(lake) || (name = LEvalSexpr(lake)) == Lnil) {
      OOGLSyntax(lake->streamin,
		 "\"hdefine %s\" in \"%s\": expected handle name",
		 LakeName(lake), opsname);
      goto parsefail;
    }

    item = LPARSE(ltype)(lake);
    if (item == Lnil) {
      OOGLSyntax(lake->streamin,
		 "\"hdefine\" in \"%s\": error reading %s",
		 LakeName(lake), LSTRINGVAL(kw));
      goto parsefail;
    }
    LListAppend(args, kw);
    LListAppend(args, name);
    LListAppend(args, item);

    return Lt;
  }

  LParseArgs("hdefine", LBEGIN, LREST, &args, LEND);

  kw = LListEntry(args, 1);
  name = LListEntry(args, 2);
  item = LListEntry(args, 3);
  if (!LSTRINGFROMOBJ(kw, &opsname) ||
      (ops = str2ops(opsname)) == NULL ||
      (ltype = ops2ltype(ops)) == NULL) {
    OOGLError(0, "\"hdefine\": expected data type, got %s",
	      LSummarize(kw));
    return Lnil;
  }
  if (!LSTRINGFROMOBJ(name, &hname)) {
    OOGLError(0, "\"hdefine\": expected handle name, got %s", LSummarize(name));
    return Lnil;
  }
  if (!LFROMOBJ(ltype)(item, &s)) {
    OOGLError(0, "\"hdefine\": Can't extract %s from %s",
	      ltype->name, LSummarize(item));
    return Lnil;
  }
  h = HandleCreateGlobal(hname, ops);
  REFPUT(h);
  if (ops == &CommandOps) {
    obj = NULL; /* (Ref *)LispCreate( s->lobj ) */
  } else if (ops == &TransOps) {
    obj = (Ref *)TransCreate(s->ts.tm);
  } else {
    obj = (Ref *)s->gs.geom; /* All other types resemble geoms */
  }
  HandleSetObject(h, obj);
  if (ops == &TransOps) {
    REFPUT(obj); /* otherwise obj will never be deleted */
  }
  return Lt;

 parsefail:
  LFree(kw);
  LFree(name);
  LFree(item);
  return Lnil;
}


/*****************************************************************************/

#if HAVE_UNIX_SOCKETS || HAVE_INET_SOCKETS || HAVE_INET6_SOCKETS

#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#if !HAVE_DECL_ACCEPT
int accept(int sockfd, struct sockaddr *addr, ACCEPT_ARG3_TYPE *addrlen);
#endif

#if HAVE_UNIX_SOCKETS
/*
 * makeunixsocket(name) makes a UNIX-domain listening socket and returns its fd.
 */
static int
makeunixsocket(char *name)
{
  struct sockaddr_un un;
  int s;

  unlink(name);
  strcpy(un.sun_path, name);
  un.sun_family = AF_UNIX;
  if ((s = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
    OOGLError(0, "geomview: can't make UNIX domain socket: %s", sperror());
    return -1;
  }
  if (bind(s, (struct sockaddr *)&un, sizeof(un)) < 0 || listen(s, 4) < 0) {
    OOGLError(0, "geomview: can't listen on socket %s: %s",name,sperror());
    return -1;
  }
  return s;
}
#endif /* HAVE_UNIX_SOCKKETS */

#if HAVE_INET_SOCKETS
/*
 * makeinetsocket(name) makes a IPv4 listening socket and returns its fd.
 */
static int
makeinetsocket(int port)
{
  struct sockaddr_in in;
  int s;

  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    OOGLError(0, "geomview: can't make IPv4 socket: %s", sperror());
    return -1;
  }
  in.sin_family = AF_INET;
  in.sin_port = htons(port);
  in.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(s, (struct sockaddr *)&in, sizeof(in)) < 0 || listen(s, 4) < 0) {
    OOGLError(0, "geomview: can't listen on IPv4 port %d: %s", port, sperror());
    return -1;
  }
  return s;
}
#endif /* HAVE_INET_SOCKETS */

#if HAVE_INET6_SOCKETS
/*
 * makeinet6socket(name) makes a IPv6 listening socket and returns its fd.
 */
static int
makeinet6socket(int port)
{
  struct sockaddr_in6 in;
  struct in6_addr addr = IN6ADDR_ANY_INIT;
  int s;

  if ((s = socket(PF_INET6, SOCK_STREAM, 0)) < 0) {
    OOGLError(0, "geomview: can't make IPv6 socket: %s", sperror());
    return -1;
  }
  in.sin6_family = AF_INET6;
  in.sin6_port = htons(port);
  in.sin6_addr = addr;
  if (bind(s, (struct sockaddr *)&in, sizeof(in)) < 0 || listen(s, 4) < 0) {
    OOGLError(0, "geomview: can't listen on IPv6 port %d: %s", port, sperror());
    return -1;
  }
  return s;
}
#endif /* HAVE_INET6_SOCKETS */

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
  HandleOps *ops = (HandleOps *)PoolClientData(listenp);

  if ((ds =
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
    if (PoolByName(conname, ops) == NULL)
      break;
  }
  PoolStreamOpen(conname, fdopen(ds, "rb"), 0, ops);
  /*
   * Reply on the same pipe's return stream.
   * Don't do this yet; only user so far is "togeomview",
   * which isn't prepared to receive data back from us.
   * PoolStreamOpen(conname, fdopen(ds, "w"), 1, (HandleOps*)PoolClientData(listenp));
   */
  PoolStreamOpen(conname, fdopen(ds, "w"), 1, ops);
  useconnection( listenp->poolname, ops, HandleCreate(conname, ops), NULL, 0 );
  return 1;
}

#endif

/*
 * usepipe(dir, suffix, type)
 * where the characters in "type" specify:
 * kind of connection (named-pipe 'p' or socket 's') and
 * type of data expected (command 'c' or geometry 'g').
 * 
 * For sockets 's' may be followed by eigher "un" (Unix domain
 * socket), "in" (IPv4 socket) or "in6" (IPv6 socket). For IP sockets
 * "suffix" actually specifies the port-number to use. pipedir is
 * ignored in that case.
 */
void
usepipe(char *pipedir, char *suffix, char *pipetype)
{
  HandleOps *ops = &GeomOps;
  int s;
  char *tail;
  char pipename[PATH_MAX];
  char pdir[PATH_MAX];
  enum {
    nopipe = -1,
    namedpipe = 1,
    unixsocket = 2,
    inetsocket = 3,
    inet6socket = 4
  } usepipe = nopipe;
  int port = -1;

  while (*pipetype) switch(*pipetype++) {
  case 's':
    usepipe = unixsocket;
    /* Check for sin:PORT and sin6:PORT sockets, allow sun socket type */
    if (strncmp(pipetype, "un", 2) == 0) {
      pipetype += 2;
    } else if (strncmp(pipetype, "in6", 3) == 0) {
      usepipe = inet6socket;
      pipetype += 3;
    } else if (strncmp(pipetype, "in", 2) == 0) {
      usepipe = inetsocket;
      pipetype += 2;
    }
    break;
  case 'p': usepipe = namedpipe; break;
  case 'c': ops = &CommandOps; break;
  case 'g': ops = &GeomOps; break;
  default:
    OOGLError(0, "Unknown character '%c' in pipe type string: "
	      "expected s, p, c, g", pipetype[-1]);
  }
  if (usepipe == nopipe) {
#ifdef NeXT
    usepipe = unixsocket;
#else
    usepipe = namedpipe;
#endif
  }

  if (usepipe == unixsocket || usepipe == namedpipe) {
    if (suffix[0] == '/') {
      strcpy(pdir, suffix);
      tail = strrchr(pdir, '/');
      *tail = '\0';
      pipedir = pdir;
      suffix = tail + 1;
    }
    sprintf(pipename, "%s/%s", pipedir, suffix);
    mkdir(pipedir, 0777);
    chmod(pipedir, 0777);
  } else {
    port = atoi(suffix);
  }

  switch (usepipe) {
  case namedpipe: {
    /* Establish System-V style named pipe.
     * Expect data on it of type 'ops'.
     * If there's a non-pipe with that name, trash it.
     */
    struct stat st;

    if (stat(pipename, &st) == 0 && (st.st_mode & S_IFMT) != S_IFIFO)
      unlink(pipename);
    if (access(pipename, 0) < 0) {
      if (mknod(pipename, S_IFIFO, 0) < 0)
	OOGLError(1, "Can't make pipe: %s: %s", suffix, sperror());
      chmod(pipename, 0666);
    }
    loadfile(pipename, ops, 0);
    break;
  }
#if HAVE_UNIX_SOCKETS
  case unixsocket:
    /* Establish UNIX-domain listener socket.
     * When we get connections to it, expect data of type 'ops'.
     */
    s = makeunixsocket(pipename);
    if (s >= 0) {
      Pool *p = PoolStreamOpen(pipename, fdopen(s, "rb"), 0, &listenOps);
      if (p) PoolSetClientData(p, ops);
      p->flags |= PF_NOPREFETCH;
    }
    break;
#endif
#if HAVE_INET_SOCKETS
  case inetsocket:
    /* Establish UNIX-domain listener socket.
     * When we get connections to it, expect data of type 'ops'.
     */
    s = makeinetsocket(port);
    if (s >= 0) {
      Pool *p = PoolStreamOpen(pipename, fdopen(s, "rb"), 0, &listenOps);
      if (p) PoolSetClientData(p, ops);
      p->flags |= PF_NOPREFETCH;
    }
    break;
#endif
#if HAVE_INET6_SOCKETS
  case inet6socket:
    /* Establish UNIX-domain listener socket.
     * When we get connections to it, expect data of type 'ops'.
     */
    s = makeinet6socket(port);
    if (s >= 0) {
      Pool *p = PoolStreamOpen(pipename, fdopen(s, "rb"), 0, &listenOps);
      if (p) PoolSetClientData(p, ops);
      p->flags |= PF_NOPREFETCH;
    }
    break;
#endif
  default:
    break;
  }
}

/*
 * Assume we have a connection open named "name", expecting data of
 * type "ops".  Wire it to an appropriate geomview object, if any.
 * (Data received on that connection will set the value of that object.)
 * Since (new_)geometry and (new_)camera take ownership of the object,
 * increasing its reference count, we decr the ref count after handing it over.
 */
static void
useconnection(char *name, HandleOps *ops, Handle *h, Ref *obj, int unique)
{
  char *tail;
  tail = strrchr(name, '/');
  if (tail)
    tail++;
  else
    tail = name;

  if (ops == &GeomOps) {
    GeomStruct gs;
    if(h && !h->permanent) {
#if 0
      /* same logic as in geomparse() */
      HandleDelete(h);
      h = NULL;
#endif
    }
    gs.h = h;
    gs.geom = (Geom *)obj;
    if (unique)
      gv_new_geometry( tail, &gs );
    else
      gv_geometry( tail, &gs );
    GeomDelete((Geom *)obj);	/* Maintain ref count */
    HandleDelete(h); /* Maintain ref count */
  } else if (ops == &CamOps) {
    CameraStruct cs;
    if(h && !h->permanent) {
#if 0
      /* same logic as in geomparse() */
      HandleDelete(h);
      h = NULL;
#endif
    }
    cs.h = h;
    cs.cam = (Camera *)obj;
    if (unique)
      gv_new_camera( tail, &cs );
    else
      gv_camera( tail, &cs );
    CamDelete((Camera *)obj);	/* Maintain ref count */
    HandleDelete(h); /* Maintain ref count */
  }
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
