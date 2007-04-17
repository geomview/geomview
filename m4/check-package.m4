dnl GEOMVIEW_CHECK_PACKAGE()
dnl
dnl Check for a library + header files
dnl
dnl Arguments:
dnl
dnl $1: symbolic name (fancy)
dnl $2: library name (base name, lib$2[.so|.a]
dnl $3: library path (-L$3)
dnl $4: additional libraries needed (e.g. -lm -lGL)
dnl $5: header name
dnl $6: include path for $5 (-I$6)
dnl $7: additional include paths (e.g. when $5 includes some other headers)
dnl $8: a space separated list of the keywords
dnl
dnl     optional, required, disabled, enabled
dnl
dnl     optional: failing to detect the package is not fatal
dnl     required: configure is terminated when the package is not detected
dnl               (default)
dnl     disabled: package is initially disabled and needs the appropriate
dnl               --with-blah switch, otherwise the package is not checked for
dnl     enabled:  package is initially enabled (the default)
dnl
dnl Default is to check for libraries below $prefix/lib/ and for header-files
dnl below $prefix/include/
dnl
dnl $5 may be empty (e.g. to check for a Fortran library). In this case
dnl $6 and $7 are ignored
dnl
dnl This Macro defines the following variables (UPNAME means a
dnl canonicalized version of $1: i.e. uppercase and - converted to _)
dnl
dnl Makefile-substitution
dnl
dnl UPNAME[]_INCLUDE_PATH
dnl UPNAME[]_LIB_PATH                  
dnl UPNAME[]_INCLUDE
dnl UPNAME[]_LIB       linker flags excluding $4
dnl UPNAME[]_ALL_LIB   linker flags including $4
dnl
dnl config.h preprocessor macros
dnl
dnl HAVE_LIB$2
dnl HAVE_$5_H
dnl
dnl Automake conditional
dnl
dnl AM_CONDITIONAL([HAVE_]UPNAME, [test -n "$[]UPNAME[_LIB]"])

dnl
dnl Helper-macros
dnl

dnl
dnl GEOMVIEW_CHECK_PKG_OPT(PKGNAME)
dnl
AC_DEFUN([GEOMVIEW_CHECK_PKG_OPT],
[m4_define([UPNAME], [m4_bpatsubst(m4_toupper([$1]),-,_)])
m4_if($2,enabled,
      [m4_define([NEGDEFAULT],[without])
       m4_define([NEGVERB],[disable])
       m4_define([DEFAULT],[autodetect])],
      [m4_define([NEGDEFAULT],[with])
       m4_define([NEGVERB],[enable])
       m4_define([DEFAULT],[disabled])])
AC_ARG_WITH($1,
AC_HELP_STRING(--[]NEGDEFAULT[]-$1,
[NEGVERB use of package $1 (default: DEFAULT)]),
	[if test -z "$[]UPNAME[_DISABLE]"; then
	    case "$withval" in
		yes)
			UPNAME[_DISABLE]=no
			;;
		no)
			UPNAME[_DISABLE]=yes
			AC_MSG_RESULT([Disabling "$1"])
			;;
		*)
	      AC_MSG_ERROR(["$withval" should have been either "yes" or "no"])
			  ;;
	    esac
	fi],
	[if test -z "$[]UPNAME[_DISABLE]"; then
		m4_if($2,enabled,
                      [UPNAME[_DISABLE]=no],
                      [UPNAME[_DISABLE]=yes])
	fi])
])
dnl
dnl GEOMVIEW_CHECK_PKG_HDR_OPT(PKGNAME, INCLUDEPATH)
dnl
AC_DEFUN([GEOMVIEW_CHECK_PKG_HDR_OPT],
[m4_define([UPNAME], [m4_bpatsubst(m4_toupper([$1]),-,_)])
AC_ARG_WITH($1-headers,
m4_if($2,[],
[AC_HELP_STRING([--with-$1-headers=DIR],
	       [use $1 include files below directory DIR (default: PREFIX/include/)])],
[AC_HELP_STRING([--with-$1-headers=DIR],
	       [use $1 include files below directory DIR (default: $2)])]),
[case "$withval" in
    yes) AC_MSG_ERROR("option \"--with-$1-headers\" requires an argument")
	;;
    no) AC_MSG_ERROR("option \"--with-$1-headers\" requires an argument")
	;;
    *) UPNAME[_INCLUDE_PATH]="$withval"
	;;
esac],
test -z "`eval eval eval echo ${UPNAME[_INCLUDE_PATH]}`" && UPNAME[_INCLUDE_PATH]="${DEFAULT_INCDIR}")
])
dnl
dnl headers and libraries below the same directory :(
dnl
AC_DEFUN([GEOMVIEW_CHECK_PKG_DIR_OPT],
[m4_define([UPNAME], [m4_bpatsubst(m4_toupper([$1]),-,_)])
AC_ARG_WITH($1-dir,
AC_HELP_STRING([--with-$1-dir=DIR],
              [use $1 library (and headers) below directory DIR (no default)]),
[case "$withval" in
    yes) AC_MSG_ERROR("option \"--with-$1-dir\" requires an argument")
	;;
    no) AC_MSG_ERROR("option \"--with-$1-dir\" requires an argument")
	;;
    *) UPNAME[_LIB_PATH]="$withval"
       UPNAME[_INCLUDE_PATH]="$withval"
	;;
esac])
])
dnl
dnl the macro itself
dnl
AC_DEFUN([GEOMVIEW_CHECK_PACKAGE],
[AC_REQUIRE([GEOMVIEW_SET_PREFIX])
dnl m4_if($#,8,,
dnl  [m4_if($#,9,,
dnl     [errprint([$0] needs eight (8) or nine (9) arguments, but got $#
dnl	)
dnl      m4exit(1)])])
dnl
dnl upcase $1
dnl
m4_define([UPNAME], [m4_bpatsubst(m4_toupper([$1]),-,_)])
dnl
dnl need to use m4_if, the $i arguments are not shell variables
dnl
m4_define([ENABLED],[enabled])
m4_define([OPTIONAL],[required])
m4_if($#,8,[
  m4_foreach_w([ac_gv_lvar],[$8],
    [m4_if(ac_gv_lvar,[disabled],
           [m4_define([ENABLED],ac_gv_lvar)],
           ac_gv_lvar,[enabled],
           [m4_define([ENABLED],ac_gv_lvar)],
           ac_gv_lvar,[optional],
           [m4_define([OPTIONAL],ac_gv_lvar)],
           ac_gv_lvar,[required],
           [m4_define([OPTIONAL],ac_gv_lvar)])])])
m4_if(OPTIONAL,[optional],[GEOMVIEW_CHECK_PKG_OPT([$1],[ENABLED])])
dnl
dnl bail out if package is completely disabled
dnl
if test "${UPNAME[_DISABLE]}" = yes; then
	:
else

m4_if($3,[],[DEFAULT_LIBDIR="${libdir}"],
	    [DEFAULT_LIBDIR="$3"])

if test "${DEFAULT_LIBDIR}" = ''
then
	DEFAULT_LIBDIR="'${libdir}'"
fi

dnl
dnl Optionally use an alternate name (e.g. MesaGL instead of GL etc.)
dnl
AC_ARG_WITH($1-name,
AC_HELP_STRING([--with-$1-name=NAME], [use NAME as the name of the $1 library (without leading "lib" prefix and trailing suffix). Default: "$2"]),
[case "$withval" in
    yes) AC_MSG_ERROR("option \"--with-$1-name\" requires an argument")
	;;
    no) AC_MSG_ERROR("option \"--with-$1-name\" requires an argument")
	;;
    *) UPNAME[_NAME]="$withval"
	;;
esac],
UPNAME[_NAME]="$2")
dnl
dnl headers and libraries below the same directory :(
dnl If we have no header to check for, then this additional option does not
dnl make sense, hence the m4_if()
dnl
m4_if($5,[],[],[GEOMVIEW_CHECK_PKG_DIR_OPT([$1])])
dnl
dnl location of library
dnl
AC_ARG_WITH($1-lib,
m4_if($3,[],
[AC_HELP_STRING([--with-$1-lib=DIR],
               [use $1 library below directory DIR (default: EPREFIX/lib/)])],
[AC_HELP_STRING([--with-$1-lib=DIR],
               [use $1 library below directory DIR (default: $3)])]),
[case "$withval" in
    yes) AC_MSG_ERROR("option \"--with-$1-lib\" requires an argument")
	;;
    no) AC_MSG_ERROR("option \"--with-$1-lib\" requires an argument")
	;;
    *) UPNAME[_LIB_PATH]="$withval"
	;;
esac],
test -z "`eval eval eval echo ${UPNAME[_LIB_PATH]}`" && UPNAME[_LIB_PATH]="${DEFAULT_LIBDIR}")
dnl
dnl now for the header file
dnl
m4_if($5,[],[],
	[m4_if($6,[],[DEFAULT_INCDIR="${includedir}"
                     GEOMVIEW_CHECK_PKG_HDR_OPT([$1], [PREFIX/include/])],
		     [DEFAULT_INCDIR="$6"
                     GEOMVIEW_CHECK_PKG_HDR_OPT([$1], [$6/])])])
dnl
dnl now check if the library and header files exist
dnl
m4_if(OPTIONAL,[optional],
  [AC_CHECK_LIB(${UPNAME[_NAME]}, main,
    [UPNAME[_LIB]="-L${UPNAME[_LIB_PATH]} -l${UPNAME[_NAME]}"
     UPNAME[_ALL_LIB]="-L${UPNAME[_LIB_PATH]} -l${UPNAME[_NAME]} $4"],
    [UPNAME[_LIB]=""
     UPNAME[_ALL_LIB]=""
     UPNAME[_LIB_PATH]=""
     UPNAME[_INCLUDE_PATH]=""],
    -L`eval eval eval echo ${UPNAME[_LIB_PATH]}` $4)],
  [AC_CHECK_LIB(${UPNAME[_NAME]}, main,
     [UPNAME[_LIB]="-L${UPNAME[_LIB_PATH]} -l${UPNAME[_NAME]}"
      UPNAME[_ALL_LIB]="-L${UPNAME[_LIB_PATH]} -l${UPNAME[_NAME]} $4"],
     [case "$host" in
	*darwin*)
		AC_MSG_RESULT([Running the test for "$1" again with -framework switch])
		;;
	*)
		AC_MSG_ERROR([Library "lib${UPNAME[_NAME]}" was not found])
		;;
	esac],
     -L`eval eval eval echo $UPNAME[_LIB_PATH]` `eval eval eval echo $4`)])

dnl
dnl On MacOS X we have that funky -framework switch ...
dnl So just run the test again with the framework switch in case the
dnl package was not found.
dnl
if test "x${UPNAME[_LIB]}" = "x" ; then
  case "$host" in
	*darwin*)
	eval "unset ac_cv_lib_${UPNAME[_NAME]}___main"
m4_if(OPTIONAL,[optional],
  [AC_CHECK_FRAMEWORK(${UPNAME[_NAME]}, main,
    [UPNAME[_LIB]="-F${UPNAME[_LIB_PATH]} -framework ${UPNAME[_NAME]}"
     UPNAME[_ALL_LIB]="-F${UPNAME[_LIB_PATH]} -framework ${UPNAME[_NAME]} $4"],
    [UPNAME[_LIB]=""
     UPNAME[_ALL_LIB]=""
     UPNAME[_LIB_PATH]=""
     UPNAME[_INCLUDE_PATH]=""],
    -F`eval eval eval echo ${UPNAME[_LIB_PATH]}` `eval eval eval echo $4`)],
  [AC_CHECK_FRAMEWORK(${UPNAME[_NAME]}, main,
     [UPNAME[_LIB]="-F${UPNAME[_LIB_PATH]} -framework ${UPNAME[_NAME]}"
      UPNAME[_ALL_LIB]="-F${UPNAME[_LIB_PATH]} -framework ${UPNAME[_NAME]} $4"],
     [AC_MSG_ERROR([Framework "${UPNAME[_NAME]}" was not found])],
     -F`eval eval eval echo $UPNAME[_LIB_PATH]` `eval eval eval echo $4`)])
	;;
  esac
fi

if test "x${UPNAME[_LIB]}" = "x" ; then
	:
else
  m4_if($5,[],,[
    dnl
    dnl  check for the header file
    dnl
    [gv_ac_]UPNAME[_save_CPPFLAGS]="$CPPFLAGS"
    m4_define([gv_ac_inctemp],[m4_bpatsubst([gv_ac_$5],[[].-[]],_)])
    unset gv_ac_inctemp
    for incdir in "${UPNAME[_INCLUDE_PATH]}"; do
      if test -n "`eval eval eval echo ${incdir}`"; then
	CPPFLAGS="-I`eval eval eval echo ${incdir}` $7 ${[gv_ac_]UPNAME[_save_CPPFLAGS]}"
      fi
      AC_CHECK_HEADERS($5,[gv_ac_inctemp="$5"])
      if ! test "${gv_ac_inctemp}" = "$5"; then
	eval "[unset ac_cv_header_]m4_bpatsubst([$5],[[].-[]],_)"
      else
     	break
      fi
    done
    if ! test "${gv_ac_inctemp}" = "$5"; then
      m4_if(OPTIONAL,[optional],
        [UPNAME[_LIB]=""
	UPNAME[_ALL_LIB]=""
	UPNAME[_LIB_PATH]=""
	UPNAME[_INCLUDE_PATH]=""],
        [AC_MSG_ERROR([Header file "$5" was not found])])
    else
      UPNAME[_ALL_INCLUDES]="-I${incdir} $7"
      UPNAME[_INCLUDES]="-I${incdir}"
      UPNAME[_INCLUDE_PATH]="${incdir}"
    fi
    CPPFLAGS="${[gv_ac_]UPNAME[_save_CPPFLAGS]}"
  ])
  dnl
  dnl define makefile substitutions and config.h macros
  dnl
  if test "x${UPNAME[_LIB]}" = "x" ; then
	:
  else
    AC_DEFINE(m4_bpatsubst(m4_toupper([HAVE_LIB$2]),-,_),
              1, Define to 1 if you have lib$2)
  fi
fi

fi dnl disable fi

AM_CONDITIONAL([HAVE_]UPNAME, [test -n "$[]UPNAME[_LIB]"])
AC_SUBST(UPNAME[_INCLUDE_PATH])
AC_SUBST(UPNAME[_INCLUDES])
AC_SUBST(UPNAME[_ALL_INCLUDES])
AC_SUBST(UPNAME[_LIB_PATH])
AC_SUBST(UPNAME[_LIB])
AC_SUBST(UPNAME[_ALL_LIB])
AC_SUBST(UPNAME[_NAME])
])
