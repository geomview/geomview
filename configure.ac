AC_PREREQ(2.59)
AC_INIT(geomview, [1.8.2-rc8], Claus-Justus Heine <Claus.Heine@Mathematik.Uni-Freiburg.DE>)
AC_MSG_RESULT([configuring $PACKAGE_NAME $PACKAGE_VERSION])
if test "z${cache_file}" = "z/dev/null"; then
  cache_file=config.cache
  AC_CACHE_LOAD
fi
AC_CANONICAL_TARGET([])
AM_INIT_AUTOMAKE([dist-bzip2 1.9])
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([src/bin/geomview/x11/gvmain.c])
AC_CONFIG_HEADER([config.h])
AC_PREFIX_DEFAULT([/usr/local/Geomview])

dnl Add extra include or link directories
AC_ARG_WITH(build-path,[
  --with-build-path[=DIR] use DIR/include DIR/lib
],[
for d in `echo $withval | tr : ' '`; do
    test -d $d/include && CPPFLAGS="$CPPFLAGS -I$d/include"
    test -d $d/lib && LDFLAGS="$LDFLAGS -L$d/lib"
done
])

## Autoheader stuff.
AH_TOP([#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED])
AH_BOTTOM([#if D1_DEBUG
#define D1PRINT(args) printf args
#else
#define D1PRINT(args)
#endif
#if !HAVE_WORKING_FORK
# error FIXME! We need fork()
#endif

/* AIX requires this to be the first thing in the file.  */
#ifndef __GNUC__
# if HAVE_ALLOCA_H
#  include <alloca.h>
# else
#  ifdef _AIX
#pragma alloca
#  else
#   ifndef alloca /* predefined by HP cc +Olibcalls */
char *alloca ();
#   endif
#  endif
# endif
#endif

#if !HAVE_LSTAT || \
    !LSTAT_FOLLOWS_SLASHED_SYMLINK || \
    HAVE_LSTAT_EMPTY_STRING_BUG 
# define lstat(path, buf) gv_lstat(path, buf)
#endif

#if !HAVE_STAT || HAVE_STAT_EMPTY_STRING_BUG 
# define stat(path, buf) gv_stat(path, buf)
# define fstat(fd, buf) gv_fstat(fd, buf)
#endif

#if 0
#undef malloc
#undef free
#undef realloc
#undef calloc

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
extern void *malloc_record(size_t size,
                           const char *file, const char *func, int line);
extern void *realloc_record(void *ptr, size_t size,
			    const char *file, const char *func, int line);
extern void *calloc_record(size_t nmemb, size_t size,
	 		   const char *file, const char *func, int line);
extern void free_record(void *ptr);

#define malloc(size) malloc_record(size, __FILE__, __FUNCTION__, __LINE__)
#define realloc(ptr, size) realloc_record(ptr, size, __FILE__, __FUNCTION__, __LINE__)
#define calloc(nmemb, size) calloc_record(nmemb, size, __FILE__, __FUNCTION__, __LINE__)
#define free(ptr) free_record(ptr)

#define OOG_NewE(size, blah)      malloc(size)
#define OOG_RenewE(p, size, blah) realloc(p, size)
#define OOGLFree(p)               free(p)
#endif

#endif /* CONFIG_H_INCLUDED */
])

################################################################################
#
# What follows was originally output from autoscan. We do some generic
# checking before overiding specific stuff (ACCEPT_ARG3_TYPE) for
# certain known systems.
#

# Checks for programs.
AC_PROG_CXX
if test "$CXX" = "" ; then
  HAVE_CXX=false
  AC_MSG_WARN([
    Configuring without a C++ compiler (couldn't find one on
    your system).  C++ is only used only to compile the vrml->OOGL
    converter, so your distribution will be compiled without it.
  ])
else
  HAVE_CXX=true
fi
AC_PROG_CC
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_CPP
AC_PROG_LEX
#AC_PROG_RANLIB
AC_PROG_YACC
AC_DISABLE_SHARED
AM_CONDITIONAL([INSTALL_LIBGEOMVIEW],[true || test  "z${enable_shared}" = "zyes"])
AC_PROG_LIBTOOL
# Search path for "find" executable.  Use "find" unless
# the bogus MS Windows version is first on the path, in 
# which case we use the full pathname.  This AC_CHECK_PROG
# macro is very strange.
AC_CHECK_PROG([FIND],[find],[find],[no],,[/cygdrive/c/WINNT/system32/find])
if test "$FIND" = "no"; then
    AC_MSG_ERROR([No find found.  Cannot continue.])
fi
# Check for availability of /bin/csh for file-name globbing. Try /bin/sh
# if not found. 
AC_PATH_PROGS(GLOB_SHELL_PATH, [csh sh], [/bin/csh])
if echo ${GLOB_SHELL_PATH} | grep csh > /dev/null 2>&1; then
	GLOB_SHELL="${GLOB_SHELL_PATH} -f -c "
else
	GLOB_SHELL="${GLOB_SHELL_PATH} -c "
fi
AC_DEFINE_UNQUOTED([GLOB_SHELL], ["${GLOB_SHELL}"],
	[Define to path and arguments of shell used for file name globbing])
AC_SUBST([GLOB_SHELL])
#
if test $USE_MAINTAINER_MODE = yes; then
AC_CHECK_PROGS(PS2EPSI, ps2epsi)
AC_CHECK_PROGS(EPSTOPDF, epstopdf)
AC_CHECK_PROGS(PDFIMAGESPROG, pdfimages)
AC_CHECK_PROGS(PNMTOPNG, pnmtopng)
fi

# The stereo module needs to fiddle with monitor modes.
# Currently, this only works on IRIX using an external command.
#
AC_PATH_PROG([SETMON_PATH],[setmon],,[$PATH:/usr/gfx])
if test -n "$SETMON_PATH"; then
  AC_DEFINE_UNQUOTED([SETMON_PATH], "$SETMON_PATH", [Path to setmon program])
fi

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [main])

# Checks for header files.
AC_PATH_XTRA
AC_FUNC_ALLOCA
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT

AC_CHECK_HEADERS([errno.h fcntl.h inttypes.h libintl.h limits.h malloc.h \
 netinet/in.h stddef.h stdlib.h string.h sys/file.h sys/param.h sys/select.h \
 sys/socket.h sys/systeminfo.h sys/time.h sys/types.h unistd.h io.h math.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_BIGENDIAN
AC_C_CONST
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_LANG_PUSH([C])
AC_MSG_CHECKING([for M_PI])
AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM(
[#if HAVE_MATH_H
# include <math.h>
#endif
],
[double blah = M_PI;])],
[AC_MSG_RESULT([yes])
AC_DEFINE(HAVE_M_PI, 1, [Define to 1 if M_PI is defined])],
[AC_MSG_RESULT([no])])
AC_LANG_POP([C])

# Checks for library functions
AC_CONFIG_LIBOBJ_DIR([src/lib/aclib])
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_FORK
AC_FUNC_MEMCMP
#AC_FUNC_MALLOC  It is ok when malloc(0) returns NULL. Why not?
#AC_FUNC_REALLOC It is ok when realloc(0, 0) returns NULL. Why not?
AC_FUNC_SETPGRP
AC_FUNC_SETVBUF_REVERSED
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_LSTAT
#AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_FUNC_WAIT3
AC_CHECK_FUNCS([fmemopen],
  [AC_CHECK_DECLS([fmemopen])],
  [AC_LIBOBJ([fmemopen])])
AC_REPLACE_FUNCS([acosh popen putenv strcasecmp strdup strncasecmp lstat stat])
AC_CHECK_FUNCS([fstat])
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNC([select],
  [AC_DEFINE(HAVE_SELECT, 1, [Define to 1 if you have the `select' function.])
AC_CHECK_DECLS([select],[],[],[
#if HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#if HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif])])
AC_CHECK_DECLS([errno],[],[],[
#if HAVE_ERRNO_H
# include <errno.h>
#endif])
AC_CHECK_FUNCS([alarm bzero bcopy dup2 floor fcntl finite gethostname \
  gettimeofday isascii memset mkdir pow rint select sqrt strcasecmp strchr \
  strcspn strdup strerror strncasecmp strpbrk strrchr strstr strtol sysinfo \
  waitpid wait3])

#
# we can use AC_COMPILE, because we check later for the existence of
# socket()
#
AC_MSG_CHECKING([for Unix domain sockets])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
  ],[int s = socket(AF_UNIX, SOCK_STREAM, 0); ])
],[
  HAVE_UNIX_SOCKETS=yes
  AC_MSG_RESULT([yes])
],[
  HAVE_UNIX_SOCKETS=no
  AC_MSG_RESULT([no])])

if test "z${HAVE_UNIX_SOCKETS}" != "zno"; then

#
# Try to figure out how to get at the "socket()" function
#
  AC_MSG_NOTICE([Checking whether we need to link -lsocket to get socket() ...])
  AC_CHECK_FUNCS([socket],[
    AC_DEFINE([HAVE_SOCKET],1,[Define to 1 if you have socket()])
    SOCKETLIBS=
    AC_MSG_RESULT([-lsocket not needed])
  ],[
    AC_CHECK_LIB([socket],[socket],[
      AC_DEFINE([HAVE_SOCKET],1,[Define to 1 if you have socket()])
      SOCKETLIBS=-lsocket
      AC_MSG_RESULT([-lsocket is needed])
    ],[
      SOCKETLIBS=
      HAVE_UNIX_SOCKETS=no
      AC_MSG_RESULT([socket() not available])
    ])
  ])
fi

ac_save_LIBS="${LIBS}"
LIBS="${SOCKETLIBS} ${LIBS}"
if test "z${HAVE_UNIX_SOCKETS}" != "zno"; then

  AC_CHECK_FUNCS([accept bind connect listen socket],[],[
    SOCKETLIBS=
    HAVE_UNIX_SOCKETS=no])
fi
LIBS="${ac_save_LIBS}"


if test "z${HAVE_UNIX_SOCKETS}" != "zno"; then
#
# Try to figure out what the 3rd argument of accept should be. The
# third argument is an integer type, ideally it is "socklent_t". As
# the third argument is a pointer, we cannot just check for a
# declaration of accept() and leave the rest to the comiler: the size
# of the type should match. On little-endian machines we could just
# choose the largest integer type available, on big-endian machines
# this would not work. Gnah.
#
  AC_CHECK_DECLS([accept],[],[],[
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
  ])
  AC_CHECK_TYPES([socklen_t],[
    ACCEPT_ARG3_TYPE=socklen_t
  ],[
    ACCEPT_ARG3_TYPE=int
  ],[
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
  ])
fi # HAVE_UNIX_SOCKETS=yes

AC_SUBST(SOCKETLIBS)
if test "z${HAVE_UNIX_SOCKETS}" != "zno"; then
 AC_DEFINE([HAVE_UNIX_SOCKETS],1,
  [Define to 1 if Unix domain sockets are available])
fi

#
# End autoscan stuff.
#
################################################################################


# The following vars contain info about the local system:
#   (AC_CANONICAL_TARGET([]) sets them)
#       var		value on RedHat 6.1
#       ---		-------------------
#       build           i686-pc-linux-gnu
#       host            i686-pc-linux-gnu
#       target          i686-pc-linux-gnu
#       build_alias     i686-pc-linux-gnu
#       host_alias      i686-pc-linux-gnu
#       target_alias    i686-pc-linux-gnu
#       build_cpu       i686
#       build_vendor    pc
#       build_os        linux-gnu
#       host_cpu        i686
#       host_vendor     pc
#       host_os         linux-gnu
#       target_cpu      i686
#       target_vendor   pc
#       target_os       linux-gnu

case "$target" in
  *linux*) MACHTYPE="linux"
          ;;
  *irix*) MACHTYPE="sgi"
	  AC_DEFINE(sgi, 1, [Define to 1 if we are compiling for sgi])
          if test "$CC" = "cc" ; then
            CFLAGS="$CFLAGS -float"
            LIBS="$LIBS -lmalloc"
          fi
          ;;
  *darwin*) MACHTYPE=$target_os
          AC_DEFINE(unix, 1, [Define to 1 since we are compiling for unix])
          ;;
  *rs6000*) MACHTYPE="rs6000"
          AC_DEFINE(AIX, 1, [Define to 1if we are compiling for AIX])
	  ACCEPT_ARG3_TYPE="unsigned long"
          AC_DEFINE(unix, 1, [Define to 1 if we are compiling for unix???])
	  ;;
       *) MACHTYPE=$target_os
          ;;
esac

AC_DEFINE_UNQUOTED(MACHTYPE, "$MACHTYPE",[Define to the machine type])
AC_SUBST(MACHTYPE)

AC_DEFINE_UNQUOTED(ACCEPT_ARG3_TYPE, $ACCEPT_ARG3_TYPE,[
ACCEPT_ARG3_TYPE should be the datatype of the
3rd arg to the 'accept' system call.  It's 'int' on some systems,
'unsigned long' on some, and could be something else on others.
Default to int])

# Search for necessary programs.

########################################################################
########################################################################
########################################################################

# If --with-motif=DIR was specified, set MOTIF_DIR to DIR
AC_ARG_WITH(motif,
  AC_HELP_STRING([--with-motif=DIR], [set DIR to motif installation prefix]),
  [MOTIF_DIR=$withval])

AC_ARG_WITH(opengl,
  AC_HELP_STRING([--with-opengl=DIR], [set DIR to opengl installation prefix]),
  [OPENGL_DIR=$withval])

AC_ARG_WITH(xforms,
  AC_HELP_STRING([--with-xforms=DIR], [set DIR to xforms installation dir]),
  [
if test "$withval" = "no" ; then
    TRY_XFORMS=false
else
    TRY_XFORMS=true
    XFORMS_DIR=$withval
fi
  ],
  [TRY_XFORMS=true])

########################################################################

#
# Store X library link line in XLIBS
#
AC_PATH_XTRA
XLIBS="$X_LIBS $X_PRE_LIBS -lXt -lXext -lX11 $X_EXTRA_LIBS"
AC_SUBST(XLIBS)

AC_CACHE_VAL(ac_cv_XLIBS,[ac_cv_XLIBS="XLIBS='$XLIBS'"])

########################################################################

AC_MSG_CHECKING([for Xmu])

result="no"
GEOM_FIND_L_OPTION(-lXmu,
		   XmuLookupStandardColormap,
                   ["" "$x_libraries" "$MOTIF_DIR/lib"],
		   $XLIBS)
if test "$GEOM_L_OPTION" != "0" ; then
  result="yes"
  AC_DEFINE(HAVE_XMU, 1, [Does -lXmu exist on this system?])
  if test "$GEOM_L_OPTION" != "" ; then
    XMULIBS="$GEOM_L_OPTION -lXmu"
  else 
    XMULIBS="-lXmu"
  fi
fi

AC_SUBST(XMULIBS)
AC_MSG_RESULT([$result])

########################################################################

AC_MSG_CHECKING([for motif])

if test "$MOTIF_DIR" != "" ; then
  USER_MOTIF_DIR="$MOTIF_DIR/include"
else
  USER_MOTIF_DIR=""
fi
GEOM_FIND_I_OPTION(Xm/Xm.h, ["$USER_MOTIF_DIR" "" "$x_includes" "/usr/local/include"])
if test "$GEOM_I_OPTION" = "0" ; then
    AC_MSG_ERROR([

Can't find Motif header file Xm/Xm.h.  Geomview requires Motif
(or Lesstif).  See the file INSTALL.Geomview for details.

])
fi

MOTIFINCLUDE=$GEOM_I_OPTION
AC_SUBST(MOTIFINCLUDE)

########################################################################

if test "$MOTIFLIBS" = "" ; then
  MOTIFLIBS="-lXm"
fi

if test "$MOTIF_DIR" != "" ; then
  USER_MOTIF_DIR="$MOTIF_DIR/lib"
else
  USER_MOTIF_DIR=""
fi
GEOM_FIND_L_OPTION($MOTIFLIBS,
		   XmCreateForm,
		   ["$USER_MOTIF_DIR" "" "$x_libraries" "/usr/local/lib"],
		   $XLIBS
		  )
if test "$GEOM_L_OPTION" = "0" ; then
    AC_MSG_ERROR([

Can't find Motif libraries.  Geomview requires Motif
(or Lesstif).  See the file INSTALL.Geomview for details.

])
fi
MOTIF_L_OPTION=$GEOM_L_OPTION
MOTIFLIBS="$GEOM_L_OPTION $MOTIFLIBS"
AC_SUBST(MOTIFLIBS)

AC_MSG_RESULT([headers $MOTIFINCLUDE, libraries $MOTIFLIBS])

########################################################################

if test "z${OPENGL_DIR}" != "zno"; then

  AC_MSG_CHECKING([for opengl])
  if test "$OPENGL_DIR" != "" ; then
    USER_OPENGL_DIR="$OPENGL_DIR/include"
  else
    USER_OPENGL_DIR=""
  fi
  GEOM_FIND_I_OPTION(GL/gl.h, ["$USER_OPENGL_DIR" "" "/usr/local/include"])
  if test "$GEOM_I_OPTION" = "0" ; then
    AC_MSG_WARN([
Can't find OpenGL header file GL/gl.h.  Geomview does not require OpenGL
but you will have to live without hardware accellerated rendering.
See the file INSTALL.Geomview for details.
])
    OPENGLINCLUDE=$GEOM_I_OPTION
  fi
  GL_I_OPTION=$GEOM_I_OPTION

  # Geomview does not require OpenGL, _BUT_ an OpenGL implementation
  # through the X11 GLX extension; so check for glx.h as well.
  #
  #
  GEOM_FIND_I_OPTION(GL/glx.h, ["$USER_OPENGL_DIR" "" "/usr/local/include"])
  if test "$GEOM_I_OPTION" = "0" ; then
    AC_MSG_WARN([
Can't find OpenGL header file GL/glx.h.  Geomview does not require OpenGL
but you will have to live without hardware accellerated rendering.
See the file INSTALL.Geomview for details.
])
    GL_I_OPTION=0
  else
    GL_I_OPTION="${GL_I_OPTION} ${GEOM_I_OPTION}"
    OPENGLINCLUDE="${OPENGLINCLUDE} ${GEOM_I_OPTION}"
  fi

########################################################################

  if test "$OPENGL_DIR" != "" ; then
    USER_OPENGL_DIR="$OPENGL_DIR/lib"
  else
    USER_OPENGL_DIR=""
  fi
  GEOM_FIND_L_OPTION(-lGL,
	  	     glBegin,
		     ["$USER_OPENGL_DIR" "" "$x_libraries" "/usr/local/lib"],
		     "$XLIBS -lm")
  if test "$GEOM_L_OPTION" = "0" ; then
    AC_MSG_WARN([
Can't find OpenGL header file GL/gl.h.  Geomview does not require OpenGL
but you will have to live without hardware accellerated rendering.
See the file INSTALL.Geomview for details.
])
  fi
  GL_L_OPTION=$GEOM_L_OPTION
  GEOM_FIND_L_OPTION(-lGLU,
	 	     gluBeginPolygon,
		     ["$USER_OPENGL_DIR" "" "$x_libraries" "/usr/local/lib"],
		     "$GL_L_OPTION -lGL $XLIBS -lm")
  if test "$GEOM_L_OPTION" != "$GL_L_OPTION" ; then
    GLU_L_OPTION=$GEOM_L_OPTION
  else
    GLU_L_OPTION=""
  fi
  OPENGLLIBS="$GL_L_OPTION -lGL $GLU_L_OPTION -lGLU"

  AC_MSG_RESULT([headers $OPENGLINCLUDE, libraries $OPENGLLIBS])

fi # without-opengl

#
# substitute, even if empty
#
AC_SUBST(OPENGLLIBS)
AC_SUBST(OPENGLINCLUDE)

#
# MGX11 is always defined, does not hurt to include it in the library.
#
if test "z${GL_I_OPTION}" != "z0" && \
   test "z${GL_L_OPTION}" != "z0" && \
   test "z${OPENGL_DIR}" != "zno"; then
  MGOPENGL=1
  MGX11=1
else
  MGOPENGL=0
  MGX11=1
fi

AM_CONDITIONAL([MGOPENGL],[test "${MGOPENGL}" = 1])
AC_DEFINE_UNQUOTED([MGOPENGL],${MGOPENGL},
  [Define to 1 if OpenGL (or Mesa or anything equivalent) is available and should be used for rendering])

AM_CONDITIONAL([MGX11],[test "${MGX11}" = 1])
AC_DEFINE_UNQUOTED([MGX11],${MGX11},
  [Define to 1 if all rendering should be performed through the X library (i.e. no accelerated 3D rendering)])

########################################################################

AC_MSG_CHECKING([for xforms])

AC_CACHE_VAL(ac_cv_have_xforms,
[if $TRY_XFORMS ; then
    if test "$XFORMS_DIR" != "" ; then
      USER_XFORMS_DIR="$XFORMS_DIR"
    fi
    GEOM_FIND_I_OPTION(forms.h, ["${USER_XFORMS_DIR}/include" "" "$x_includes" "$x_includes/X11" "/usr/local/include"])
    ac_xforms_include=$GEOM_I_OPTION
    GEOM_FIND_L_OPTION(-lforms,
    		   fl_initialize,
    		   ["${USER_XFORMS_DIR}/lib" "" "$x_libraries" "/usr/local/lib"],
    		   "$XLIBS -lXpm -lm"
    		  )
    if test "$ac_xforms_include" != 0 -a "$GEOM_L_OPTION" != "0" ; then
      XFORMS_L_OPTION=$GEOM_L_OPTION
      ac_xforms_libs="$XFORMS_L_OPTION -lforms -lXpm"
      ac_have_xforms=yes
    else
      ac_xforms_include=
      ac_xforms_libs=""
      ac_have_xforms=no
    fi
else
  ac_xforms_include=
  ac_xforms_libs=""
  ac_have_xforms=no
fi
if test "$ac_have_xforms" = "no" ; then
  ac_cv_have_xforms="have_xforms=no"
else
  ac_cv_have_xforms="have_xforms=yes \
	ac_xforms_include='$ac_xforms_include' ac_xforms_libs='$ac_xforms_libs'"
fi
])
eval $ac_cv_have_xforms



########################################################################
########################################################################
########################################################################

if test "$have_xforms" = "no" ; then
  AC_MSG_RESULT([no])
  AC_MSG_WARN([

    Configuring without the XFORMS library.  Several of Geomview's
    external modules use the XFORMS library, and they will not be
    compiled unless you reconfigure Geomview with XFORMS.  See the
    file INSTALL.Geomview for instructions on doing that (it's easy
    --- just get a binary XFORMS distribution and unpack it into your
    Geomview source tree.)  Geomview itself does not use XFORMS, so
    it's OK to skip XFORMS if all you want is Geomview.

])
else
  AC_MSG_RESULT([headers $ac_xforms_include, libraries $ac_xforms_libs])
fi

XFORMSINCLUDE=$ac_xforms_include
XFORMSLIBS=$ac_xforms_libs

AC_SUBST(XFORMSINCLUDE)
AC_SUBST(XFORMSLIBS)

########################################################################

# Check for Tcl/Tk with new CHECK_PACKAGE macro:

GEOMVIEW_CHECK_PACKAGE(tcl, tcl, ${x_libraries}, ${XLIBS}, tcl.h, , ,optional)
GEOMVIEW_CHECK_PACKAGE(tk, tk, ${x_libraries}, ${XLIBS}, tk.h, , ,optional)

AM_CONDITIONAL(HAVE_TCLTK, [test -n "${TCL_LIB}" -a -n "${TK_LIB}"])

#
# Cygwin does not seem to have the Tcl_CreateFileHandler() function,
# so test for it and disable the crayola module in this case.
#
if test -n "${TCL_LIB}" -a -n "${TK_LIB}"; then
    ac_save_LIBS="${LIBS}"
    LIBS="-ltcl ${LIBS}"
    AC_CHECK_FUNCS([Tcl_CreateFileHandler],
	[TCLFILEHANDLER=yes],
	[TCLFILEHANDLER=no])
    LIBS="${ac_save_LIBS}"
fi
AM_CONDITIONAL([TCLFILEHANDLER], [test "z${TCLFILEHANDLER}" = "zyes"])

########################################################################

OPENGL_LD_LIBRARY_PATH=`expr "$GL_L_OPTION" : '-L\(.*\)'`
MOTIF_LD_LIBRARY_PATH=`expr "$MOTIF_L_OPTION" : '-L\(.*\)'`
XFORMS_LD_LIBRARY_PATH=`expr "$XFORMS_L_OPTION" : '-L\(.*\)'`

if test "$OPENGL_LD_LIBRARY_PATH" != "0" ; then
  LOCAL_LD_LIBRARY_PATH=$OPENGL_LD_LIBRARY_PATH
fi
if test "$MOTIF_LD_LIBRARY_PATH" != "0" ; then
  if test "$LOCAL_LD_LIBRARY_PATH" != "" ; then
    LOCAL_LD_LIBRARY_PATH="$LOCAL_LD_LIBRARY_PATH:$MOTIF_LD_LIBRARY_PATH"
  else
    LOCAL_LD_LIBRARY_PATH="$MOTIF_LD_LIBRARY_PATH"
  fi
fi
if test "$XFORMS_LD_LIBRARY_PATH" != "0" ; then
  if test "$LOCAL_LD_LIBRARY_PATH" != "" ; then
    LOCAL_LD_LIBRARY_PATH="$LOCAL_LD_LIBRARY_PATH:$XFORMS_LD_LIBRARY_PATH"
  else
    LOCAL_LD_LIBRARY_PATH="$XFORMS_LD_LIBRARY_PATH"
  fi
fi
if test "$LOCAL_LD_LIBRARY_PATH" != "" ; then
  LD_LIBRARY_PATH_SETTINGS="LD_LIBRARY_PATH=$LOCAL_LD_LIBRARY_PATH"
  AC_SUBST(LD_LIBRARY_PATH_SETTINGS)
fi

########################################################################
########################################################################
########################################################################

AC_ARG_ENABLE(d1debug,
	      [  --enable-d1debug        turn on D1 level debugging output],
	      [ if test "$enableval" != "no" ; then
                  AC_DEFINE(D1_DEBUG, 1)
                fi ],
              [AC_DEFINE(D1_DEBUG, 0, [D1 debugging?])]
              )

AC_ARG_ENABLE(motion-averaging,
[  --enable-motion-averaging      
                          turn on experimental motion averaging (default is off)],
	      [ if test "$enableval" = "no" ; then
                  AC_DEFINE(EXPERIMENTAL_MOTION_AVERAGING, 0,
			    [Use experimental motion averaging code?])
                else
                  AC_DEFINE(EXPERIMENTAL_MOTION_AVERAGING, 1)
                fi ],
              [ AC_DEFINE(EXPERIMENTAL_MOTION_AVERAGING, 0) ]
              )

########################################################################
########################################################################
########################################################################

# Check for some additional functions.

AC_CHECK_FUNCS([strerror])

########################################################################
########################################################################
########################################################################

AC_LANG_PUSH([C])
AC_MSG_CHECKING(
[whether popen(CMD, MODE) needs/accepts MODE = "rb" for binary files])
AC_RUN_IFELSE(
  [AC_LANG_PROGRAM(
[#include <stdio.h>
],
[FILE *pipe = popen("${SHELL}", "rb");
if (pipe == NULL)
  return 1;
else
  return 0;])],[AC_MSG_RESULT([yes])
AC_DEFINE(POPEN_ACCEPTS_RB, 1,
[Define to 1 if popen(CMD, MODE) accepts "rb" for MODE])],
[AC_MSG_RESULT([no])])
AC_LANG_POP([C])

AM_CONDITIONAL(HAVE_CXX,    test x$HAVE_CXX = xtrue)
AM_CONDITIONAL(HAVE_XFORMS, test x$have_xforms = xyes)

########################################################################

# cache 'moduledir' and 'geomdatadir' so individual module configure scripts
# can get them from the cache

moduledir="\$(libexecdir)/geomview"
AC_SUBST(moduledir)

geomdatadir="\$(datadir)/geomview"
AC_SUBST(geomdatadir)

# cache 'moduledir' and 'geomdatadir' so individual module configure scripts
# can get them from the cache
AC_CACHE_VAL(ac_cv_moduledir,[ac_cv_moduledir="moduledir='$moduledir'"])
AC_CACHE_VAL(ac_cv_geomdatadir,[ac_cv_geomdatadir="geomdatadir='$geomdatadir'"])

########################################################################

top_geom_builddir='$(top_builddir)'
AC_SUBST(top_geom_builddir)

OOGLLIBS="\$(top_geom_builddir)/src/lib/oogl/wa/libwa.la                     \
          \$(top_geom_builddir)/src/lib/camera/libcamera.la                  \
          \$(top_geom_builddir)/src/lib/color/libcolor.la                    \
          \$(top_geom_builddir)/src/lib/fexpr/libfexpr.la                    \
          \$(top_geom_builddir)/src/lib/geomutil/bdy/libbdy.la               \
          \$(top_geom_builddir)/src/lib/geomutil/crayplutil/libcrayplutil.la \
          \$(top_geom_builddir)/src/lib/geomutil/dim/libdim.la               \
          \$(top_geom_builddir)/src/lib/geomutil/plutil/libplutil.la         \
          \$(top_geom_builddir)/src/lib/gprim/bbox/libbbox.la                \
          \$(top_geom_builddir)/src/lib/gprim/bezier/libbezier.la            \
          \$(top_geom_builddir)/src/lib/gprim/comment/libcomment.la          \
          \$(top_geom_builddir)/src/lib/gprim/discgrp/libdg.la               \
          \$(top_geom_builddir)/src/lib/gprim/geom/libgeom.la                \
          \$(top_geom_builddir)/src/lib/gprim/inst/libinst.la                \
          \$(top_geom_builddir)/src/lib/gprim/lincoln/liblincoln.la          \
          \$(top_geom_builddir)/src/lib/gprim/list/liblist.la                \
          \$(top_geom_builddir)/src/lib/gprim/mesh/libmesh.la                \
          \$(top_geom_builddir)/src/lib/gprim/ndmesh/libndmesh.la            \
          \$(top_geom_builddir)/src/lib/gprim/npolylist/libnpolylist.la      \
          \$(top_geom_builddir)/src/lib/gprim/polylist/libpolylist.la        \
          \$(top_geom_builddir)/src/lib/gprim/quad/libquad.la                \
          \$(top_geom_builddir)/src/lib/gprim/skel/libskel.la                \
          \$(top_geom_builddir)/src/lib/gprim/sphere/libsphere.la            \
          \$(top_geom_builddir)/src/lib/gprim/tlist/libtlist.la              \
          \$(top_geom_builddir)/src/lib/gprim/vect/libvect.la                \
          \$(top_geom_builddir)/src/lib/mg/buf/liblibmgbuf.la                \
          \$(top_geom_builddir)/src/lib/mg/common/libmgcommon.la             \
          \$(top_geom_builddir)/src/lib/mg/ps/libmgps.la                     \
          \$(top_geom_builddir)/src/lib/mg/rib/libmgrib.la                   \
          \$(top_geom_builddir)/src/lib/pointlist/libpointlist.la            \
          \$(top_geom_builddir)/src/lib/shade/libshade.la                    \
          \$(top_geom_builddir)/src/lib/window/libwindow.la                  \
          \$(top_geom_builddir)/src/lib/geometry/cmodel/libcmodel.la         \
          \$(top_geom_builddir)/src/lib/geometry/hpoint3/libhpoint3.la       \
          \$(top_geom_builddir)/src/lib/geometry/hpointn/libhpointn.la       \
          \$(top_geom_builddir)/src/lib/geometry/point3/libpoint3.la         \
          \$(top_geom_builddir)/src/lib/geometry/transform3/libtransform3.la \
          \$(top_geom_builddir)/src/lib/geometry/transformn/libtransformn.la \
          \$(top_geom_builddir)/src/lib/geometry/transobj/libtransobj.la     \
          \$(top_geom_builddir)/src/lib/oogl/lisp/liblisp.la                 \
          \$(top_geom_builddir)/src/lib/oogl/refcomm/librefcomm.la           \
          \$(top_geom_builddir)/src/lib/oogl/util/libooglutil.la             \
          \$(top_geom_builddir)/src/lib/aclib/libacgeomview.la"

if test "z${MGOPENGL}" = "z1"; then
  OOGLLIBS="${OOGLLIBS} \$(top_geom_builddir)/src/lib/mg/opengl/libmgopengl.la"
fi
if test "z${MGX11}" = "z1"; then
  OOGLLIBS="${OOGLLIBS} \$(top_geom_builddir)/src/lib/mg/x11/libmgx11.la"
fi

AC_CACHE_VAL(ac_cv_OOGLLIBS,[ac_cv_OOGLLIBS="OOGLLIBS='$OOGLLIBS'"])
AC_SUBST(OOGLLIBS)

OOGLLIB='$(top_geom_builddir)/src/lib/libgeomview.la'
AC_CACHE_VAL(ac_cv_OOGLLIB, [ac_cv_OOGLLIB="OOGLLIB='$OOGLLIB'"])
AC_SUBST(OOGLLIB)

MOTIFUILIB='$(top_geom_builddir)/src/lib/mib/libmib.a'
AC_CACHE_VAL(ac_cv_MOTIFUILIB, [ac_cv_MOTIFUILIB="MOTIFUILIB='$MOTIFUILIB'"])
AC_SUBST(MOTIFUILIB)

STUBLIB="\$(top_builddir)/src/lib/gprim/stub/libstub.la"
AC_SUBST(STUBLIB)

########################################################################

LF_LINK_HEADERS(src/lib/oogl/lisp		\
		src/lib/oogl/refcomm		\
		src/lib/oogl/util		\
		src/lib/oogl/wa			\
		src/lib/geometry/cmodel		\
		src/lib/geometry/hpoint3	\
		src/lib/geometry/hpointn	\
		src/lib/geometry/point3		\
		src/lib/geometry/transform3	\
		src/lib/geometry/transformn	\
		src/lib/geometry/transobj	\
		src/lib/camera			\
		src/lib/color			\
		src/lib/fexpr			\
		src/lib/geomutil/bdy		\
		src/lib/geomutil/crayplutil	\
		src/lib/geomutil/dim		\
		src/lib/geomutil/plutil		\
		src/lib/gprim/bbox		\
		src/lib/gprim/bezier		\
		src/lib/gprim/comment		\
		src/lib/gprim/discgrp		\
		src/lib/gprim/geom		\
		src/lib/gprim/inst		\
		src/lib/gprim/lincoln		\
		src/lib/gprim/list		\
		src/lib/gprim/mesh		\
		src/lib/gprim/polylist		\
		src/lib/gprim/ndmesh		\
		src/lib/gprim/npolylist		\
		src/lib/gprim/quad		\
		src/lib/gprim/skel		\
		src/lib/gprim/sphere		\
		src/lib/gprim/stub		\
		src/lib/gprim/tlist		\
		src/lib/gprim/vect		\
		src/lib/mg/buf			\
		src/lib/mg/common		\
		src/lib/mg/opengl		\
		src/lib/mg/x11			\
		src/lib/mg/ps			\
		src/lib/mg/rib			\
		src/lib/mib			\
		src/lib/pointlist		\
		src/lib/shade			\
		src/lib/window			\
		src/lib/forms			\
	 	src/bin/geomview/common		\
		)

LF_SET_INCLUDES


module_srcdirs=`( cd $srcdir ; $FIND src/bin \( -name 'configure.in' -o -name 'configure.ac' \) -a -print ) | sed -e 's/\/configure.in$//g' -e 's/\/configure.ac$//g'`
module_srcdirs=`echo $module_srcdirs`
src_bin_module_subdirs=`echo $module_srcdirs | sed -e 's/src\/bin\///g'`

AC_SUBST(src_bin_module_subdirs)

AC_CONFIG_SUBDIRS($module_srcdirs)


AC_CONFIG_FILES([Makefile
	  data/Makefile
	  doc/Makefile
	  src/Makefile
	  src/lib/Makefile
	  src/lib/aclib/Makefile
	  src/lib/camera/Makefile
	  src/lib/color/Makefile
	  src/lib/fexpr/Makefile
	  src/lib/oogl/Makefile
	  src/lib/oogl/lisp/Makefile
	  src/lib/oogl/refcomm/Makefile
	  src/lib/oogl/util/Makefile
	  src/lib/oogl/wa/Makefile
	  src/lib/geometry/Makefile
	  src/lib/geometry/cmodel/Makefile
	  src/lib/geometry/hpoint3/Makefile
	  src/lib/geometry/hpointn/Makefile
	  src/lib/geometry/point3/Makefile
	  src/lib/geometry/transform3/Makefile
	  src/lib/geometry/transformn/Makefile
	  src/lib/geometry/transobj/Makefile
	  src/lib/geomutil/Makefile
	  src/lib/geomutil/bdy/Makefile
	  src/lib/geomutil/crayplutil/Makefile
	  src/lib/geomutil/dim/Makefile
	  src/lib/geomutil/plutil/Makefile
	  src/lib/gprim/Makefile
	  src/lib/gprim/bbox/Makefile
	  src/lib/gprim/bezier/Makefile
	  src/lib/gprim/comment/Makefile
	  src/lib/gprim/discgrp/Makefile
	  src/lib/gprim/geom/Makefile
	  src/lib/gprim/inst/Makefile
	  src/lib/gprim/lincoln/Makefile
	  src/lib/gprim/list/Makefile
	  src/lib/gprim/mesh/Makefile
	  src/lib/gprim/polylist/Makefile
	  src/lib/gprim/ndmesh/Makefile
	  src/lib/gprim/npolylist/Makefile
	  src/lib/gprim/quad/Makefile
	  src/lib/gprim/skel/Makefile
	  src/lib/gprim/sphere/Makefile
	  src/lib/gprim/stub/Makefile
	  src/lib/gprim/tlist/Makefile
	  src/lib/gprim/vect/Makefile
	  src/lib/mg/Makefile
	  src/lib/mg/buf/Makefile
	  src/lib/mg/common/Makefile
	  src/lib/mg/opengl/Makefile
	  src/lib/mg/x11/Makefile
	  src/lib/mg/ps/Makefile
	  src/lib/mg/rib/Makefile
	  src/lib/mib/Makefile
	  src/lib/pointlist/Makefile
	  src/lib/shade/Makefile
	  src/lib/window/Makefile
	  src/lib/forms/Makefile
	  src/bin/Makefile
	  src/bin/geomview/Makefile
	  src/bin/geomview/common/Makefile
	  src/bin/geomview/x11/Makefile
	  src/bin/geomview/x11/bitmaps/Makefile
	  src/bin/geomview/x11/interface/Makefile
	  src/bin/NDview/Makefile
	  src/bin/ndutils/Makefile
	  src/bin/clipboard/Makefile
	  src/bin/crayola/Makefile
	  src/bin/example/Makefile
	  src/bin/ginsu/Makefile
	  src/bin/gvclock/gvclock
	  src/bin/gvclock/Makefile
	  src/bin/labeler/Makefile
	  src/bin/tackdown/Makefile
	  src/bin/animate/Makefile
	  src/bin/animate/interface/Makefile
	  src/bin/graffiti/Makefile
	  src/bin/transformer/Makefile
	  src/bin/stereo/Makefile
	  src/bin/geomutil/Makefile
	  src/bin/geomutil/bdy/Makefile
	  src/bin/geomutil/clip/Makefile
	  src/bin/geomutil/hvectext/Makefile
	  src/bin/geomutil/hvectext/hvectext
	  src/bin/geomutil/maple2oogl/Makefile
	  src/bin/geomutil/math2oogl/Makefile
	  src/bin/geomutil/oogl2rib/Makefile
	  src/bin/geomutil/oogl2vrml/Makefile
	  src/bin/geomutil/plutil/Makefile
	  src/bin/geomutil/ucd/Makefile
	  src/bin/geomutil/vrml2oogl/Makefile
	  src/bin/geomutil/vrml2oogl/lib/Makefile
	  src/bin/sweep/Makefile
	  src/bin/hinge/Makefile
	  src/bin/flythrough/Makefile
	  src/bin/cplxview/Makefile
	  src/bin/togeomview/Makefile
])
AC_OUTPUT

