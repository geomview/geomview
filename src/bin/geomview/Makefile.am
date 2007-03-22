SUBDIRS = . common x11

man_MANS = geomview.1gv

EXTRA_DIST = clang.c clang.h $(man_MANS)

all-local: clang

clang.c clang.h clang:
	$(top_srcdir)/src/lib/oogl/lisp/lisp2c -cprefix "gv_" -o clang $(srcdir)/common/*.c $(srcdir)/x11/*.c

DISTCLEANFILES = clang.c clang.h
