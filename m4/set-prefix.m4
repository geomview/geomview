dnl set prefix to its proper value, so that one can define substitutions
dnl depending on the value of prefix
AC_DEFUN([GEOMVIEW_SET_PREFIX],
[AC_ARG_VAR([DEFAULT_PREFIX],[Default installation prefix])
test "x${DEFAULT_PREFIX}" = "x" && DEFAULT_PREFIX="${ac_default_prefix}"
test "x$prefix" = xNONE && prefix="${DEFAULT_PREFIX}"
PREFIX="${prefix}"
# Let make expand exec_prefix.
test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'
expanded_prefix=`eval eval eval echo ${prefix}`
AC_SUBST(expanded_prefix)
expanded_exec_prefix=`eval eval eval echo ${exec_prefix}`
AC_SUBST(expanded_exec_prefix)
])
