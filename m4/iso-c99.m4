#
# We need (at least) two features from the C99 standard: variadic
# macros and variable length arrays. We check here whether or those
# are available.
#
# Note that we do not further modify the compiler flags, we just check
# whether variadic macros and variable length arrays are available.
#
# The macro takes 2 arguments:
#
# $1: optional: a set of compiler flags in addition to CFLAGS
#
AC_DEFUN([GEOMVIEW_ISO_C99_CHECK],
[AC_REQUIRE([AC_PROG_CC])
_alberta_save_cflags="${CFLAGS}"
m4_if($#,1,[CFLAGS="$1"])
AC_LANG_PUSH([C])
AC_MSG_CHECKING([for ISO C99 features with "${CC} ${CFLAGS}"])
AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM(
[[extern void exit(int status);
#define FOO(a, ...)  (a, __VA_ARGS__)
extern int foo(int a, int b, int c);]],
[[int bar[foo FOO(3, 4, 5)];
exit(bar[0]);]])],
[HAVE_ISO_C99=1
AC_MSG_RESULT([variadic macros and variable length arrays are available])],
[HAVE_ISO_C99=0
AC_MSG_RESULT([variadic macros and/or vairable length arrays are NOT available])])
AC_LANG_POP([C])
CFLAGS="${_alberta_save_cflags}"
AC_DEFINE_UNQUOTED([HAVE_ISO_C99],[${HAVE_ISO_C99}],
[Define to 1 if ISO C99 features are available (e.g. variadic macros and variable length arrays)])
])
