dnl set prefix to its proper value, so that one can define substitutions
dnl depending on the value of prefix
AC_DEFUN([GEOMVIEW_SET_PREFIX],
[test "x$prefix" = xNONE && prefix=$ac_default_prefix
# Let make expand exec_prefix.
test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'
expanded_prefix=`eval echo ${prefix}`
AC_SUBST(expanded_prefix)
expanded_exec_prefix=`eval echo ${exec_prefix}`
AC_SUBST(expanded_exec_prefix)
])
