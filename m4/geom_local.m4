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
# Find out what -I argument we need in order to get the Motif header
# files (<Xm/Xm.h>, etc).  If --with-motif=DIR was specified, first
# look in DIR/include.  If that fails, or if --with-motif wasn't
# specified, try with no -I option.  If that fails, try
# /usr/local/include.  Set MOTIFINCLUDE to whichever option works
# (including the -I).
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
