dnl aclocal.m4 generated automatically by aclocal 1.4

dnl Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.

# Do all the work for Automake.  This macro actually does too much --
# some checks are only needed if your package does certain things.
# But this isn't really a big deal.

# serial 1

dnl Usage:
dnl AM_INIT_AUTOMAKE(package,version, [no-define])

AC_DEFUN(AM_INIT_AUTOMAKE,
[AC_REQUIRE([AC_PROG_INSTALL])
PACKAGE=[$1]
AC_SUBST(PACKAGE)
VERSION=[$2]
AC_SUBST(VERSION)
dnl test to see if srcdir already configured
if test "`cd $srcdir && pwd`" != "`pwd`" && test -f $srcdir/config.status; then
  AC_MSG_ERROR([source directory already configured; run "make distclean" there first])
fi
ifelse([$3],,
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Name of package])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Version number of package]))
AC_REQUIRE([AM_SANITY_CHECK])
AC_REQUIRE([AC_ARG_PROGRAM])
dnl FIXME This is truly gross.
missing_dir=`cd $ac_aux_dir && pwd`
AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
AM_MISSING_PROG(MAKEINFO, makeinfo, $missing_dir)
AC_REQUIRE([AC_PROG_MAKE_SET])])

#
# Check to make sure that the build environment is sane.
#

AC_DEFUN(AM_SANITY_CHECK,
[AC_MSG_CHECKING([whether build environment is sane])
# Just in case
sleep 1
echo timestamp > conftestfile
# Do `set' in a subshell so we don't clobber the current shell's
# arguments.  Must try -L first in case configure is actually a
# symlink; some systems play weird games with the mod time of symlinks
# (eg FreeBSD returns the mod time of the symlink's containing
# directory).
if (
   set X `ls -Lt $srcdir/configure conftestfile 2> /dev/null`
   if test "[$]*" = "X"; then
      # -L didn't work.
      set X `ls -t $srcdir/configure conftestfile`
   fi
   if test "[$]*" != "X $srcdir/configure conftestfile" \
      && test "[$]*" != "X conftestfile $srcdir/configure"; then

      # If neither matched, then we have a broken ls.  This can happen
      # if, for instance, CONFIG_SHELL is bash and it inherits a
      # broken ls alias from the environment.  This has actually
      # happened.  Such a system could not be considered "sane".
      AC_MSG_ERROR([ls -t appears to fail.  Make sure there is not a broken
alias in your environment])
   fi

   test "[$]2" = conftestfile
   )
then
   # Ok.
   :
else
   AC_MSG_ERROR([newly created file is older than distributed files!
Check your system clock])
fi
rm -f conftest*
AC_MSG_RESULT(yes)])

dnl AM_MISSING_PROG(NAME, PROGRAM, DIRECTORY)
dnl The program must properly implement --version.
AC_DEFUN(AM_MISSING_PROG,
[AC_MSG_CHECKING(for working $2)
# Run test in a subshell; some versions of sh will print an error if
# an executable is not found, even if stderr is redirected.
# Redirect stdin to placate older versions of autoconf.  Sigh.
if ($2 --version) < /dev/null > /dev/null 2>&1; then
   $1=$2
   AC_MSG_RESULT(found)
else
   $1="$3/missing $2"
   AC_MSG_RESULT(missing)
fi
AC_SUBST($1)])

# Like AC_CONFIG_HEADER, but automatically create stamp file.

AC_DEFUN(AM_CONFIG_HEADER,
[AC_PREREQ([2.12])
AC_CONFIG_HEADER([$1])
dnl When config.status generates a header, we must update the stamp-h file.
dnl This file resides in the same directory as the config header
dnl that is generated.  We must strip everything past the first ":",
dnl and everything past the last "/".
AC_OUTPUT_COMMANDS(changequote(<<,>>)dnl
ifelse(patsubst(<<$1>>, <<[^ ]>>, <<>>), <<>>,
<<test -z "<<$>>CONFIG_HEADERS" || echo timestamp > patsubst(<<$1>>, <<^\([^:]*/\)?.*>>, <<\1>>)stamp-h<<>>dnl>>,
<<am_indx=1
for am_file in <<$1>>; do
  case " <<$>>CONFIG_HEADERS " in
  *" <<$>>am_file "*<<)>>
    echo timestamp > `echo <<$>>am_file | sed -e 's%:.*%%' -e 's%[^/]*$%%'`stamp-h$am_indx
    ;;
  esac
  am_indx=`expr "<<$>>am_indx" + 1`
done<<>>dnl>>)
changequote([,]))])

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
AC_DEFUN(GEOM_FIND_L_OPTION,[
geom_lib=$1
geom_func=$2
geom_dirs='$3'
geom_otherlibs=$4
geom_saved_LIBS=$LIBS
AC_MSG_CHECKING([how to link with $geom_lib])
for geom_z in $geom_dirs ; do
  geom_z=`eval echo $geom_z`
  if test "$geom_z" != "" ; then
    geom_l_option="-L$geom_z"
  else
    geom_l_option=""
  fi
  LIBS="$geom_l_option $geom_lib $geom_otherlibs"
  AC_TRY_LINK_FUNC($geom_func,
	           [ GEOM_L_OPTION="$geom_l_option"
                     break ],
	           [ GEOM_L_OPTION="0" ]
	          )
done
LIBS=$geom_saved_LIBS
if test "$GEOM_L_OPTION" != "0" ; then
  AC_MSG_RESULT([$geom_l_option $geom_lib])
else
  AC_MSG_RESULT([not found])
fi
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
AC_DEFUN(GEOM_FIND_I_OPTION,[
geom_header=$1
geom_dirs='$2'
geom_saved_CPPFLAGS=$CPPFLAGS
GEOM_I_OPTION="0"
AC_MSG_CHECKING([for $geom_header])
for geom_z in $geom_dirs ; do
  geom_z=`eval echo $geom_z`
  if test "$geom_z" != "" ; then
    CPPFLAGS="-I$geom_z"
  else
    CPPFLAGS=""
  fi
  AC_TRY_CPP([ #include <$geom_header> ],
	     [ GEOM_I_OPTION=$CPPFLAGS
               break ]
	     )
done
CPPFLAGS=$geom_saved_CPPFLAGS
if test "$GEOM_I_OPTION" = "0" ; then
  AC_MSG_RESULT([not found])
else
  if test "$GEOM_I_OPTION" != "" ; then
    AC_MSG_RESULT($GEOM_I_OPTION)
  else
    AC_MSG_RESULT([(found with no -I required)])
  fi
fi
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
AC_DEFUN(GEOM_FIND_LIBC_VERSION,[
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
AC_DEFUN(GEOM_AC_PROG_CXX_WORKS,
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

AC_DEFUN(GEOM_AC_PROG_CXX,
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

# Define a conditional.

AC_DEFUN(AM_CONDITIONAL,
[AC_SUBST($1_TRUE)
AC_SUBST($1_FALSE)
if $2; then
  $1_TRUE=
  $1_FALSE='#'
else
  $1_TRUE='#'
  $1_FALSE=
fi])

dnl Copyright (C) 1988 Eleftherios Gkioulekas <lf@amath.washington.edu>
dnl  
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl 
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl 
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software 
dnl Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
dnl 
dnl As a special exception to the GNU General Public License, if you 
dnl distribute this file as part of a program that contains a configuration 
dnl script generated by Autoconf, you may include it under the same 
dnl distribution terms that you use for the rest of that program.
 
# ------------------------------------------------------------------------
# The following macro is useful for deep packages. It allows you to
# link all the header files *.h under a certain set of directories
# to be linked under an include directory from the toplevel.
# To use this feature in your configure.in call:
#   LF_LINK_HEADERS(dir1 dir2 dir3 .... , [directory] )
# where directory -> put links under include/directory
#       dir1 ...  -> the directories with header files we want to link
# WARNING: This macro will do  --> rm -rf include
# ------------------------------------------------------------------------

AC_DEFUN(LF_LINK_HEADERS,[
  # Find out how to link files if we haven't already
  if test -z "$LN_S"
  then
    AC_PROG_LN_S
  fi

  # Remove the include directory if we haven't done that yet
  if test -z "$lf_link_headers"
  then
    lf_link_headers="we are all Kosh"
    rm -rf "$srcdir/include"
  fi

  # Get the directory from the second argument which is optional
  ifelse([$2], ,  
         [lf_directory="$srcdir/include"] , 
         [lf_directory="$srcdir/include/$2"])
  ${srcdir}/mkinstalldirs "$lf_directory"

  # Link them
  lf_subdirs="`echo $1`"
  for lf_dir in $lf_subdirs
  do
    # Otherwise go ahead and link
    echo "linking headers from $srcdir/$lf_dir"
    # Check if the Headers file exists
    if test -f "$srcdir/$lf_dir/Headers"
    then
      for lf_file in `(cd $srcdir/$lf_dir; cat Headers)`
      do
        rm -f $lf_directory/$lf_file
        $LN_S "`pwd`/$srcdir/$lf_dir/$lf_file" "$lf_directory/$lf_file"
      done
    else
      echo "Warning: No Headers file for $srcdir/$lf_dir"
    fi
  done
])

# --------------------------------------------------------------------------
# The following macro is useful for deep packages. 
# To use it in your configure.in call
#   LF_SET_INCLUDES(dir1 dir2 dir3 ...)
# and in your Makefile.am set
#   @default-includes@
# Then this symbol will be substituted with
#   INCLUDES = -I$(prefix) -I$(top_builddir)/include/dir1 ...
# The prefix entry will be skipped if the prefix is equal to /usr/local
# or /usr
# --------------------------------------------------------------------------

AC_DEFUN(LF_SET_INCLUDES,[
  # See whether to put an entry for the prefix
  if test "$prefix" = "/usr" || test "$prefix" = "/usr/local"
  then
    default_includes=""
  else
    default_includes="-I\$(prefix) "
  fi

  # Get the directory list
  lf_dirs="`echo $1`"
  
  # Check whether we have a lib directory. If so put it in
  AC_MSG_CHECKING([whether sources have a lib directory])
  if test -d "`pwd`/$srcdir/lib"
  then
    AC_MSG_RESULT([yes])
    default_includes="$default_includes -I\$(top_srcdir)/lib"
  else
    AC_MSG_RESULT([no])
  fi

  # Now add in the directories. If the list is empty then just add include/
  # If the list is not empty then add the subdirectories.
  if test -z "$lf_dirs"
  then
    default_includes="$default_includes -I\$(top_srcdir)/include"
  else
    for lf_dir in $lf_dirs
    do
      default_includes="$default_includes -I\$(top_srcdir)/include/$lf_dir"
    done
  fi

  # And that's it
  AC_SUBST(default_includes)
])

