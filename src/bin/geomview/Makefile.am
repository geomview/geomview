SUBDIRS = . common x11

EXTRA_DIST = geomview.1 clang.c clang.h

man_MANS = geomview.1

all-local: clang

clang.c clang.h clang:
	$(top_srcdir)/src/lib/oogl/lisp/lisp2c -cprefix "gv_" -o clang $(srcdir)/common/*.c $(srcdir)/x11/*.c

DISTCLEANFILES = clang.c clang.h
