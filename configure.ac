AC_INIT(geomview, [1.8.2-rc1], Claus-Justus Heine <Claus.Heine@Mathematik.Uni-Freiburg.DE>)
AC_CANONICAL_TARGET([])
AC_CONFIG_SRCDIR([src/bin/geomview/x11/gvmain.c])
AC_CONFIG_HEADERS([config.h])
AC_PREFIX_DEFAULT([/usr/local/Geomview])

AM_INIT_AUTOMAKE([dist-bzip2])

AC_MSG_RESULT([configuring $PACKAGE $VERSION])

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

## Autoheader stuff.
AH_TOP([#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED])
AH_BOTTOM([#if D1_DEBUG
#define D1PRINT(args) printf args
#else
#define D1PRINT(args)
#endif

#endif /* CONFIG_H_INCLUDED */
])
AC_DEFINE(USE_FSTROPEN_1, 0,
[USE_FSTROPEN_1 determines whether we forceably use one of our	
manual implementations of fstropen (in src/lib/oogl/util/futil.c)	
rather than calling fmemopen or CC_fmemopen__FPci.  This is		
relevant only on Linux systems.  Normally you shouldn't have to set	
this at all; futil.c will use HAVE_FMEMOPEN, defined above, to	
decide what to do.  If this doesn't work for you (the symptom is	
that Geomview crashes upon startup while trying to read the initial	
appearance string), then try configuring with			
"--enable-fstropen-1", which sets USE_FSTROPEN_1.			
See the comments in in src/lib/oogl/util/futil.c for details.])

dnl Add extra include or link directories
AC_ARG_WITH(build-path,[
  --with-build-path[=DIR] use DIR/include DIR/lib
],[
for d in `echo $withval | tr : ' '`; do
    test -d $d/include && CPPFLAGS="$CPPFLAGS -I$d/include"
    test -d $d/lib && LDFLAGS="$LDFLAGS -L$d/lib"
done
])

# Search for necessary programs.

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

AC_PROG_CC
#GEOM_AC_PROG_CXX
AC_PROG_CXX
if test "$CXX" = "" ; then
  HAVE_CXX=false
  case "$target" in
    *linux*)
  AC_MSG_ERROR([
    Cannot find a working C++ compiler.  Geomview requires C++
    on Linux systems.
  ])
    ;;
    *)
  AC_MSG_WARN([

    Configuring without a C++ compiler (couldn't find one on
    your system).  C++ is only used only to compile the vrml->OOGL
    converter, so your distribution will be compiled without it.
  ])
    ;;
  esac
else
  HAVE_CXX=true
fi

AM_PROG_LEX
AC_PROG_YACC
AC_PROG_RANLIB
AC_PROG_INSTALL

# Check for ways to generate a PDF file.
# GNU Texinfo has a texi2pdf, otherwise use a DVI->PDF converter.
#   For now don't bother checking for texi2pdf, because it doesn't
#   include the figures.  Anyone know how to get figures into a pdf
#   file generated with texi2pdf? [mbp Sun Nov 19 15:33:19 2000]
#AC_CHECK_PROGS(TEXI2PDF, texi2pdf)
AM_CONDITIONAL(HAVE_TEXI2PDF, test x$TEXI2PDF != x)

AC_CHECK_PROGS(DVIPDF, dvipdfm dvipdf)
AM_CONDITIONAL(HAVE_DVIPDF, ! test x${DVIPDF} = x)

AC_CHECK_PROGS(TEX, tex)
AM_CONDITIONAL(HAVE_TEX, ! test x${TEX} = x)

AC_CHECK_PROGS(PDFTEX, pdftex)
AM_CONDITIONAL(HAVE_PDFTEX, ! test x${PDFTEX} = x)

AC_CHECK_PROGS(PS2EPSI, ps2epsi)
AC_CHECK_PROGS(EPSTOPDF, epstopdf)
AC_CHECK_PROGS(PDFIMAGESPROG, pdfimages)
AC_CHECK_PROGS(CONVERT, convert)

########################################################################
########################################################################
########################################################################

# If --with-motif=DIR was specified, set MOTIF_DIR to DIR
AC_ARG_WITH(motif,
	    [  --with-motif=DIR        set DIR to motif installation prefix],
	    MOTIF_DIR=$withval
           )

AC_ARG_WITH(opengl,
	    [  --with-opengl=DIR       set DIR to opengl installation prefix],
	    OPENGL_DIR=$withval
           )



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

AC_MSG_CHECKING([for opengl])
if test "$OPENGL_DIR" != "" ; then
  USER_OPENGL_DIR="$OPENGL_DIR/include"
else
  USER_OPENGL_DIR=""
fi
GEOM_FIND_I_OPTION(GL/gl.h, ["$USER_OPENGL_DIR" "" "/usr/local/include"])
if test "$GEOM_I_OPTION" = "0" ; then
    AC_MSG_ERROR([

Can't find OpenGL header file GL/gl.h.  Geomview requires OpenGL
(or Mesa).  See the file INSTALL.Geomview for details.

])
fi
OPENGLINCLUDE=$GEOM_I_OPTION
AC_SUBST(OPENGLINCLUDE)

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
    AC_MSG_ERROR([

Can't find OpenGL libraries.  Geomview requires OpenGL (or Mesa).
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
AC_SUBST(OPENGLLIBS)

AC_MSG_RESULT([headers $OPENGLINCLUDE, libraries $OPENGLLIBS])

########################################################################

AC_ARG_WITH(xforms,
	    [  --with-xforms=DIR       set DIR to xforms installation dir],
	    [ if test "$withval" = "no" ; then
                TRY_XFORMS=false
              else
                TRY_XFORMS=true
                XFORMS_DIR=$withval
              fi ;
            ],
            [ TRY_XFORMS=true ]
           )

########################################################################
########################################################################
########################################################################
AC_MSG_CHECKING([for xforms])

AC_CACHE_VAL(ac_cv_have_xforms,
[if $TRY_XFORMS ; then
    if test "$XFORMS_DIR" != "" ; then
      USER_XFORMS_DIR="$XFORMS_DIR"
    else
	# If the 'xforms' directory is present, look for a
	# subdirectory of it called XFORMS (perhaps more than one level
	# down).  If found, use that as the XFORMS_DIR directory.  If
	# there's more than one XFORMS subdir, use the first one found.
	# (There should never be more than one, but in case there is,
	# we need a definite behavior to resolve the ambiguity.)
	if test -d $srcdir/xforms ; then
	  USER_XFORMS_SUBDIR=`$FIND $srcdir/xforms -name FORMS -print | sed -e '2,$d'`
	  USER_XFORMS_DIR="`pwd`/$USER_XFORMS_SUBDIR"
	fi
    fi
    GEOM_FIND_I_OPTION(forms.h, ["$USER_XFORMS_DIR" "" "$x_includes" "$x_includes/X11" "/usr/local/include"])
    ac_xforms_include=$GEOM_I_OPTION
    GEOM_FIND_L_OPTION(-lforms,
    		   fl_initialize,
    		   ["$USER_XFORMS_DIR" "" "$x_libraries" "/usr/local/lib"],
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

AC_ARG_ENABLE(seekpipe,
[  --disable-seekpipe      don't try to use GLIBC's open-cookie extension
                          to implement seekable pipes (default: autodetect).],
[case "$enableval" in
	yes) USE_SEEKPIPE=1 ;;
	no)  USE_SEEKPIPE=0 ;;
esac],
[USE_SEEKPIPE=true])
if test "$USE_SEEKPIPE" = "true"; then
	AC_CHECK_FUNCS([fopencookie],[USE_SEEKPIPE=1],[USE_SEEKPIPE=0])
fi
AM_CONDITIONAL(SEEKPIPE, test "$USE_SEEKPIPE" = "1")
AC_DEFINE_UNQUOTED(USE_SEEKPIPE, $USE_SEEKPIPE,
		[define set/getpos() with pipes, using fopencookie()])

AC_ARG_ENABLE(iomarker,
[  --enable-iomarker       try to use GLIBC's internal IO-marker stuff.
                          This is generally a bad idea, but works nicely.],
[case "$enableval" in
	yes) USE_IO_MARKER=1 ;;
	no)  USE_IO_MARKER=0 ;;
esac],
[USE_IO_MARKER=false])
if test "$USE_IO_MARKER" = "1" -a "$USE_SEEKPIPE" = "0"; then
	AC_CHECK_FUNCS([_IO_init_marker _IO_remove_marker _IO_seekmark],
			[USE_IO_MARKER=1],[USE_IO_MARKER=0])
fi
AM_CONDITIONAL(IO_MARKER, test "$USE_IO_MARKER" = "1")
AC_DEFINE_UNQUOTED(USE_IO_MARKER, $USE_IO_MARKER,
		[twiddle with GLIBC internal I/O-markers.])

AC_ARG_ENABLE(fstropen-1,
	      [  --enable-fstropen-1     use manual fstropen rather than fmemopen],
	      [ if test "$enableval" != "no" ; then
                  AC_DEFINE(USE_FSTROPEN_1, 1)
                fi ]
              )

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

case "$target" in
 *linux*) MACHTYPE="linux"
	  AC_DEFINE_UNQUOTED(MACHTYPE, "$MACHTYPE",
                             [Define to the machine type])
          LINUX=true
	  AC_CHECK_HEADERS([sys/socket.h])
          AC_CHECK_TYPES([socklen_t],[
AC_DEFINE(ACCEPT_ARG3_TYPE, [socklen_t],
                    [ACCEPT_ARG3_TYPE should be the datatype of the
3rd arg to the 'accept' system call.  It's 'int' on some systems,
'unsigned long' on some, and could be something else on others.
Default to int])],[
AC_DEFINE(ACCEPT_ARG3_TYPE, [unsigned int],
                    [ACCEPT_ARG3_TYPE should be the datatype of the
3rd arg to the 'accept' system call.  It's 'int' on some systems,
'unsigned long' on some, and could be something else on others.
Default to int])],[#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
])
          ;;
  *irix*) MACHTYPE="sgi"
	  AC_DEFINE_UNQUOTED(MACHTYPE, "$MACHTYPE",
                             [Define to the machine type])
	  AC_DEFINE(sgi, 1, [Define to 1 if we are compiling for sgi])
          if test "$CC" = "cc" ; then
            CFLAGS="$CFLAGS -float"
            LIBS="$LIBS -lmalloc"
          fi
          ;;
*rs6000*) MACHTYPE="rs6000"
	  AC_DEFINE_UNQUOTED(MACHTYPE, "$MACHTYPE",
                             [Define to the machine type])
          AC_DEFINE(AIX, 1, [Define to 1if we are compiling for AIX])
	  AC_DEFINE(ACCEPT_ARG3_TYPE, [unsigned long],
                    [ACCEPT_ARG3_TYPE should be the datatype of the
3rd arg to the 'accept' system call.  It's 'int' on some systems,
'unsigned long' on some, and could be something else on others.
Default to int])
          AC_DEFINE(unix, 1, [Define to 1 if we are compiling for unix???])
          AC_DEFINE(alloca, __alloca,
                    [Define to the name of alloca (if alloca is not alloca)])
	  ;;
       *) MACHTYPE=$target_os
	  AC_DEFINE_UNQUOTED(MACHTYPE, "$MACHTYPE",
                             [Define to the machine type])
	  AC_DEFINE(ACCEPT_ARG3_TYPE, [int],
	          [ACCEPT_ARG3_TYPE should be the datatype of the
3rd arg to the 'accept' system call.  It's 'int' on some systems,
'unsigned long' on some, and could be something else on others.
Default to int])
          ;;
esac

AC_DEFINE(BINARY_POPEN_REQUIRES_B, 0,
[BINARY_POPEN_REQUIRES_B says whether the system's popen() call needs 
a 'b' in the 2nd argument when the data coming through its pipe is	
binary.  This comes up in mgtexture.c when popen is called to pipe	
a compressed image through gzip.  Initially Stuart wrote the call	
with the 2nd arg being "rb", but this didn't work on my Linux	
system so I added this switch to allow it to be compiled with just	
"r", which does work.  For now, I'm just setting this to 0 to force	
"r" on all systems, but it might be necessary to use "rb" on some	
systems which treat binary files differently (Windows?), in which	
case the configure script should be modified to set			
BINARY_POPEN_REQUIRES_B to 1 for those systems.			
mbp Tue Sep 12 12:33:38 2000])

AC_SUBST(MACHTYPE)

AM_CONDITIONAL(HAVE_CXX,    test x$HAVE_CXX = xtrue)
AM_CONDITIONAL(LINUX,       test x$LINUX = xtrue)
AM_CONDITIONAL(HAVE_XFORMS, test x$have_xforms = xyes)

# What this actually checks is whether a program that includes
# <math.h> and calls 'sin' needs -lm to link properly.  I put this
# test here because on my linux system, -lm seems unnecessary
# (<math.h> seems to define everything with inline code).  Perhaps it
# would be harmless to just give -lm in all cases, but what the heck;
# might as well check for it since we can.  It's possible that this
# could cause problems if on some system <math.h> deals with sin() but
# not with some other function that we call, but we'll deal with that
# if/when it comes up (by adding that function to this test, for
# example).
# mbp Mon May  8 18:47:33 2000
ac_save_CFLAGS="${CFLAGS}"
CFLAGS="-O0"
AC_MSG_CHECKING([whether we need to link -lm to get math functions])
AC_LINK_IFELSE([AC_LANG_PROGRAM([ #include <math.h> ], [ double s = sin(1.5); ])],[ MATHLIB=""
	      AC_MSG_RESULT([no]) ],[ MATHLIB=-lm
	      AC_MSG_RESULT([yes]) 
	   ])
CFLAGS="${ac_save_CFLAGS}"
AC_SUBST(MATHLIB)
# MATHLIB is now "-lm" on systems that need it, or empty on those that don't.
# So it's safe to put $(MATHLIB) in link commands.


AC_MSG_CHECKING([whether we need to link -lsocket to get socket()])
AC_LINK_IFELSE([AC_LANG_PROGRAM([ #include <sys/socket.h> ], [ int s = socket(AF_UNIX, SOCK_STREAM, 0); ])],[ SOCKETLIBS=""
              AC_MSG_RESULT([no]) ],[ SOCKETLIBS=-lsocket
              AC_MSG_RESULT([yes]) 
           ])
AC_SUBST(SOCKETLIBS)


AC_FUNC_SETPGRP
AC_CHECK_FUNCS([__setfpucw fmemopen])

########################################################################

AC_TYPE_SIZE_T
AC_C_BIGENDIAN

AC_CHECK_HEADERS(fpu_control.h i386/fpu_control.h netinet/in.h)

moduledir="\$(exec_prefix)/bin"
AC_SUBST(moduledir)

geomdatadir="\$(prefix)/data"
AC_SUBST(geomdatadir)

# cache 'moduledir' and 'geomdatadir' so individual module configure scripts
# can get them from the cache
AC_CACHE_VAL(ac_cv_moduledir,[ac_cv_moduledir="moduledir='$moduledir'"])
AC_CACHE_VAL(ac_cv_geomdatadir,[ac_cv_geomdatadir="geomdatadir='$geomdatadir'"])

########################################################################

top_geom_builddir='$(top_builddir)'
AC_SUBST(top_geom_builddir)

OOGLLIBS_ONCE="\$(top_geom_builddir)/src/lib/oogl/wa/libwa.a                \
          \$(top_geom_builddir)/src/lib/camera/libcamera.a                  \
          \$(top_geom_builddir)/src/lib/color/libcolor.a                    \
          \$(top_geom_builddir)/src/lib/fexpr/libfexpr.a                    \
          \$(top_geom_builddir)/src/lib/geomutil/bdy/libbdy.a               \
          \$(top_geom_builddir)/src/lib/geomutil/crayplutil/libcrayplutil.a \
          \$(top_geom_builddir)/src/lib/geomutil/dim/libdim.a               \
          \$(top_geom_builddir)/src/lib/geomutil/plutil/libplutil.a         \
          \$(top_geom_builddir)/src/lib/gprim/bbox/libbbox.a                \
          \$(top_geom_builddir)/src/lib/gprim/bezier/libbezier.a            \
          \$(top_geom_builddir)/src/lib/gprim/comment/libcomment.a          \
          \$(top_geom_builddir)/src/lib/gprim/discgrp/libdg.a               \
          \$(top_geom_builddir)/src/lib/gprim/geom/libgeom.a                \
          \$(top_geom_builddir)/src/lib/gprim/inst/libinst.a                \
          \$(top_geom_builddir)/src/lib/gprim/lincoln/liblincoln.a          \
          \$(top_geom_builddir)/src/lib/gprim/list/liblist.a                \
          \$(top_geom_builddir)/src/lib/gprim/mesh/libmesh.a                \
          \$(top_geom_builddir)/src/lib/gprim/ndmesh/libndmesh.a            \
          \$(top_geom_builddir)/src/lib/gprim/npolylist/libnpolylist.a      \
          \$(top_geom_builddir)/src/lib/gprim/polylist/libpolylist.a        \
          \$(top_geom_builddir)/src/lib/gprim/quad/libquad.a                \
          \$(top_geom_builddir)/src/lib/gprim/skel/libskel.a                \
          \$(top_geom_builddir)/src/lib/gprim/sphere/libsphere.a            \
          \$(top_geom_builddir)/src/lib/gprim/tlist/libtlist.a              \
          \$(top_geom_builddir)/src/lib/gprim/vect/libvect.a                \
          \$(top_geom_builddir)/src/lib/mg/buf/liblibmgbuf.a                \
          \$(top_geom_builddir)/src/lib/mg/common/libmgcommon.a             \
          \$(top_geom_builddir)/src/lib/mg/opengl/libmgopengl.a             \
          \$(top_geom_builddir)/src/lib/mg/ps/libmgps.a                     \
          \$(top_geom_builddir)/src/lib/mg/rib/libmgrib.a                   \
          \$(top_geom_builddir)/src/lib/mib/libmib.a                        \
          \$(top_geom_builddir)/src/lib/pointlist/libpointlist.a            \
          \$(top_geom_builddir)/src/lib/shade/libshade.a                    \
          \$(top_geom_builddir)/src/lib/window/libwindow.a                  \
          \$(top_geom_builddir)/src/lib/geometry/cmodel/libcmodel.a         \
          \$(top_geom_builddir)/src/lib/geometry/hpoint3/libhpoint3.a       \
          \$(top_geom_builddir)/src/lib/geometry/hpointn/libhpointn.a       \
          \$(top_geom_builddir)/src/lib/geometry/point3/libpoint3.a         \
          \$(top_geom_builddir)/src/lib/geometry/transform3/libtransform3.a \
          \$(top_geom_builddir)/src/lib/geometry/transformn/libtransformn.a \
          \$(top_geom_builddir)/src/lib/geometry/transobj/libtransobj.a     \
          \$(top_geom_builddir)/src/lib/oogl/lisp/liblisp.a                 \
          \$(top_geom_builddir)/src/lib/oogl/refcomm/librefcomm.a           \
          \$(top_geom_builddir)/src/lib/oogl/util/libooglutil.a"

AC_SUBST(OOGLLIBS_ONCE)
AC_CACHE_VAL(ac_cv_OOGLLIBS_ONCE,[ac_cv_OOGLLIBS_ONCE="OOGLLIBS_ONCE='$OOGLLIBS_ONCE'"])

# STRSTREAMLIB was (potentially) set above
AC_SUBST(STRSTREAMLIB)
AC_CACHE_VAL(ac_cv_STRSTREAMLIB,[ac_cv_STRSTREAMLIB="STRSTREAMLIB='$STRSTREAMLIB'"])

AC_CACHE_VAL(ac_cv_OOGLLIBS,[ac_cv_OOGLLIBS="OOGLLIBS='\$(OOGLLIBS_ONCE) \$(OOGLLIBS_ONCE) \$(OOGLLIBS_ONCE) \$(STRSTREAMLIB)'"])
eval $ac_cv_OOGLLIBS
AC_SUBST(OOGLLIBS)

STUBLIB="\$(top_builddir)/src/lib/gprim/stub/libstub.a"
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


module_srcdirs=`( cd $srcdir ; $FIND src/bin -name 'configure.in' -o -name 'configure.ac' -print ) | sed -e 's/\/configure.in$//g' | sed -e 's/\/configure.ac$//g'`
module_srcdirs=`echo $module_srcdirs`
src_bin_module_subdirs=`echo $module_srcdirs | sed -e 's/src\/bin\///g'`

AC_SUBST(src_bin_module_subdirs)

AC_CONFIG_SUBDIRS($module_srcdirs)


AC_CONFIG_FILES([Makefile
	  data/Makefile
	  doc/Makefile
	  src/Makefile
	  src/lib/Makefile
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
	  src/bin/example/Makefile
	  src/bin/ginsu/Makefile
	  src/bin/tackdown/Makefile
	  src/bin/animate/Makefile
	  src/bin/animate/interface/Makefile
	  src/bin/graffiti/Makefile
	  src/bin/transformer/Makefile
	  src/bin/stereo/Makefile
	  src/bin/geomutil/Makefile
	  src/bin/geomutil/bdy/Makefile
	  src/bin/geomutil/clip/Makefile
	  src/bin/geomutil/maple2oogl/Makefile
	  src/bin/geomutil/math2oogl/Makefile
	  src/bin/geomutil/oogl2rib/Makefile
	  src/bin/geomutil/oogl2vrml/Makefile
	  src/bin/geomutil/plutil/Makefile
	  src/bin/geomutil/ucd/Makefile
	  src/bin/geomutil/vrml2oogl/Makefile
	  src/bin/geomutil/vrml2oogl/lib/Makefile
	  src/bin/sweep/Makefile
	  src/bin/togeomview/Makefile
])
AC_OUTPUT

#	  src/bin/cplxview/Makefile
#	  src/bin/maniview/Makefile
#	  src/bin/flythrough/Makefile
#	  src/bin/hinge/Makefile
