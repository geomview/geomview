dnl
dnl
dnl GEOM_LOG_MSG(MESSAGE)
define(GEOM_LOG_MSG,
[echo "configure:__oline__: $1" >&AC_FD_CC])
dnl 
dnl GEOM_FIND_L_OPTION(LIB, FUNC, DIRS, OTHERLIBS)
dnl 
dnl Figure out what -L option might be needed to link with the library
dnl LIB. FUNC should be some function from that library.  DIRS should be a
dnl list of directories to try, one of them can be "" which means use no
dnl -L option.  OTHERLIBS, if present, can be a list of other libraries to
dnl link with.  Upon return:
dnl 
dnl    * If linking was successful, then the shell variable GEOM_L_OPTION
dnl      is set to the first -L option (including the "-L") that worked,
dnl      unless what worked was linking with no -L option, in which case
dnl      GEOM_L_OPTION is the empty string.
dnl 
dnl    * If linking didn't work with any of the dirs in DIRS, then
dnl      GEOM_L_OPTION is 0.
dnl 
AC_DEFUN([GEOM_FIND_L_OPTION],[
geom_lib=$1
geom_func=$2
geom_dirs='$3'
geom_otherlibs=$4
geom_saved_LIBS=$LIBS
#AC_MSG_CHECKING([how to link with $geom_lib])

GEOM_L_OPTION=0
for geom_z in $geom_dirs ; do
  geom_z=`eval echo $geom_z`
  geom_l_option=
  test -n "$geom_z" && geom_l_option="-L$geom_z"
  LIBS="$geom_l_option $geom_lib $geom_otherlibs"
  GEOM_LOG_MSG([checking for $geom_func with $geom_l_option])
  AC_TRY_LINK_FUNC($geom_func,
	           [ GEOM_L_OPTION="$geom_l_option"
                     break ]
		   )
done
LIBS=$geom_saved_LIBS
#if test "$GEOM_L_OPTION" != "0" ; then
#  AC_MSG_RESULT([$geom_l_option $geom_lib])
#else
#  AC_MSG_RESULT([not found])
#fi
])
dnl 
dnl 
dnl 
dnl 
dnl 
dnl 
dnl 
dnl GEOM_FIND_I_OPTION(HEADER, DIRS)
dnl 
# Find out what -I argument we need in order to get the header file HEADER.
# DIRS should be a list of strings to use as -I arguments; the empty
# string corresponds to no -I option at all.  If we're successful, return
# with GEOM_I_OPTION set to the relevant -I option (including the "-I" itself).
# Note that this might be the empty string, which corresponds to no -I
# option at all.  If we don't find HEADER in any of the dirs listed in DIRS,
# return with GEOM_I_OPTION = "0".
AC_DEFUN([GEOM_FIND_I_OPTION],[
geom_header=$1
geom_dirs='$2'
geom_saved_CPPFLAGS=$CPPFLAGS
GEOM_I_OPTION="0"
#AC_MSG_CHECKING([for $geom_header])
for geom_z in $geom_dirs ; do
  geom_z=`eval echo $geom_z`
  geom_i_option=
  test -n "$geom_z" && geom_i_option="-I$geom_z"
  CPPFLAGS="$geom_i_option $geom_saved_CPPFLAGS"
  GEOM_LOG_MSG([checking for $geom_header with $geom_i_option])
  AC_TRY_CPP([ #include <$geom_header> ],
	     [ GEOM_I_OPTION="$geom_i_option"
               break ]
	     )
done
CPPFLAGS=$geom_saved_CPPFLAGS
#if test "$GEOM_I_OPTION" = "0" ; then
#  AC_MSG_RESULT([not found])
#else
#  if test "$GEOM_I_OPTION" != "" ; then
#   AC_MSG_RESULT($GEOM_I_OPTION)
#  else
#   AC_MSG_RESULT([(found with no -I required)])
#  fi
#fi
])
dnl 
dnl 
dnl GEOM_FIND_LIBC_VERSION
dnl 
dnl Try to figure out which version of the libc library is installed on
dnl a linux system.  Return with GEOM_LIBC_VERSION set to:
dnl
dnl       5	for the old libc (libc.so.5)
dnl       6	for glibc (i.e., libc.so.6) (RedHat 5.0 and later)
dnl
dnl default to 6 if we can't figure it out.  Note that this test is
dnl not really correct; I think there is yet another version of libc
dnl that we probably need to distinguish, but I'm not sure and I don't
dnl know how to detect it, so this'll have to do for now.  If you know
dnl more about this, please email me at mbp@geomtech.com.
dnl
AC_DEFUN([GEOM_FIND_LIBC_VERSION],[
if test -e "/lib/libc.so.6" ; then
  GEOM_LIBC_VERSION="6"
else
  if test -e "/lib/libc.so.5" ; then
    GEOM_LIBC_VERSION="5"
  else
    GEOM_LIBC_VERSION="6"
  fi
fi
])
dnl
dnl GEOM_AC_PROG_CXX (and its internally used GEOM_AC_PROG_CXX_WORKS) is a
dnl replacement that I wrote for the standard AC_PROG_CXX (and
dnl AC_PROG_CXX_WORKS).  It differs from the standard one only in that it
dnl does not abort if a C++ compiler isn't found --- it just sets CXX to
dnl the empty string.  If anyone knows a better way to test for the
dnl presense of a C++ compiler without exiting if it isn't found, let me
dnl know! [mbp@geomtech.com, Sat Oct 14 00:33:38 2000]
dnl 
AC_DEFUN([GEOM_AC_PROG_CXX_WORKS],
[AC_MSG_CHECKING([whether the C++ compiler ($CXX $CXXFLAGS $LDFLAGS) works])
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
AC_TRY_COMPILER([int main(){return(0);}], ac_cv_prog_cxx_works, ac_cv_prog_cxx_cross)
AC_LANG_RESTORE
AC_MSG_RESULT($ac_cv_prog_cxx_works)
AC_MSG_CHECKING([whether the C++ compiler ($CXX $CXXFLAGS $LDFLAGS) is a cross-compiler])
AC_MSG_RESULT($ac_cv_prog_cxx_cross)
cross_compiling=$ac_cv_prog_cxx_cross
])

AC_DEFUN([GEOM_AC_PROG_CXX],
[AC_BEFORE([$0], [AC_PROG_CXXCPP])dnl
AC_CHECK_PROGS(CXX, $CCC c++ g++ gcc CC cxx cc++ cl, gcc)

GEOM_AC_PROG_CXX_WORKS

if test $ac_cv_prog_cxx_works = no; then
  CXX=
else

  AC_PROG_CXX_GNU

  if test $ac_cv_prog_gxx = yes; then
    GXX=yes
  else
    GXX=
  fi

  dnl Check whether -g works, even if CXXFLAGS is set, in case the package
  dnl plays around with CXXFLAGS (such as to build both debugging and
  dnl normal versions of a library), tasteless as that idea is.
  ac_test_CXXFLAGS="${CXXFLAGS+set}"
  ac_save_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS=
  AC_PROG_CXX_G
  if test "$ac_test_CXXFLAGS" = set; then
    CXXFLAGS="$ac_save_CXXFLAGS"
  elif test $ac_cv_prog_cxx_g = yes; then
    if test "$GXX" = yes; then
      CXXFLAGS="-g -O2"
    else
      CXXFLAGS="-g"
    fi
  else
    if test "$GXX" = yes; then
      CXXFLAGS="-O2"
    else
      CXXFLAGS=
    fi
  fi

fi
])
dnl
dnl
AC_DEFUN([GEOM_GET_CACHED_VALUE],[
AC_MSG_CHECKING([for $1])
AC_CACHE_VAL(ac_cv_$1,[
AC_MSG_ERROR([

Can't find cached value for $1.  You're not trying to run 'configure'
from $PACKAGE's source directory, are you?  You should only run
Geomview's top-level 'configure' script, not this one directly.  See
the file INSTALL in $PACKAGE's source directory for details.

])])
eval $ac_cv_$1
AC_MSG_RESULT([done.])
])
dnl
dnl
AC_DEFUN([GEOM_REQUIRE_XFORMS],[
AC_MSG_CHECKING([for xforms])
AC_CACHE_VAL(ac_cv_have_xforms,[:])
eval $ac_cv_have_xforms

if test "$have_xforms" != "yes" ; then
  AC_MSG_RESULT([no])
  AC_MSG_ERROR([

$PACKAGE requires the xforms library, but I can't find it in
Geomview's top-level config.cache.  Please install an xforms
distribution appropriate to your machine in the 'xforms' directory
under the top-level Geomview directory, delete any existing
'config.cache' file in that directory, and re-run 'configure' in that
directory.  You should only run Geomview's top-level 'configure'
script, not this one directly.  See the file INSTALL in $PACKAGE's
source directory for details.

])
else
  XFORMSINCLUDE=$ac_xforms_include
  XFORMSLIBS=$ac_xforms_libs
  AC_MSG_RESULT([headers '$ac_xforms_include', libraries '$ac_xforms_libs'])
fi
])

dnl Less severe version of the above: just check and WARN
dnl
AC_DEFUN([GEOM_CHECK_XFORMS],[
AC_MSG_CHECKING([for xforms])
AC_CACHE_VAL(ac_cv_have_xforms,[:])
eval $ac_cv_have_xforms

if test "$have_xforms" != "yes" ; then
  AC_MSG_RESULT([no])
  AC_MSG_WARN([

$PACKAGE requires the xforms library, but I can't find it in
Geomview's top-level config.cache.  Please install an xforms
distribution appropriate to your machine in the 'xforms' directory
under the top-level Geomview directory, delete any existing
'config.cache' file in that directory, and re-run 'configure' in that
directory.  You should only run Geomview's top-level 'configure'
script, not this one directly.  See the file INSTALL in $PACKAGE's
source directory for details.

])
else
  XFORMSINCLUDE=$ac_xforms_include
  XFORMSLIBS=$ac_xforms_libs
  AC_MSG_RESULT([headers '$ac_xforms_include', libraries '$ac_xforms_libs'])
fi
])
dnl
dnl GEOM_REQUIRE_GEOMVIEW_VERSION(TOPDIR,MAJOR_OP,MAJOR,MINOR_OP,MINOR_COND)
dnl                               $1     $2       $3,   $4,      $5
dnl
AC_DEFUN([GEOM_REQUIRE_GEOMVIEW_VERSION],[
if test -f "$1/configure.in"; then
	GVCFGFILE="$1/configure.in"
else test -f "$1/configure.ac"
	GVCFGFILE="$1/configure.ac"
fi
if test "z${GVCFGFILE}" = "z"; then
	AC_MSG_ERROR([Geomview configure file not found ("$1/configure.in/ac")])
fi
changequote(<<, >>)dnl
if test "z${GVCFGFILE}" = "zconfigure.in"; then
	gv_version=`sed -e 's/#.*$//' ${GVCFGFILE} | grep AM_INIT_AUTOMAKE | sed -e 's/^.*geomview,//' | sed -e 's/)//'`
else
	gv_version=`grep <<AC_INIT>> ${GVCFGFILE}|cut -d ',' -f 2| sed -e 's/[][]//g'`
changequote([,])dnl
AC_MSG_NOTICE(version: $gv_version)
changequote(<<, >>)dnl
fi
gv_major=`echo $gv_version | sed -e 's/\..*//'`
gv_rest=`echo $gv_version | sed -e 's/^[^\.]\.//'`
gv_minor=`echo $gv_rest | sed -e 's/\..*//'`
gv_rev=`echo $gv_rest | sed -e 's/^[^\.]\.//'`
changequote([,])dnl
gv_major_ok=`expr $gv_major '$2' $3`
gv_minor_ok=`expr $gv_minor '$4' $5`
if test "$gv_major_ok" != "1" -o "$gv_minor_ok" != "1" ; then
  AC_MSG_ERROR([

$PACKAGE requires Geomview version N.M, where N$2$3 and M$4$5.
Your version of Geomview seems to be $gv_major.$gv_minor.$gv_rev.

])
fi
])

