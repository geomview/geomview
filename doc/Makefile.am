SUBDIRS = html
EXTRA_DIST = README gvplot.doc oogltour \
  example1.c example2.c example3.c example4.tcl \
  geomview.1 geomview.5 geomview.5.man lisp.3 oogl.5 oogl.5.man OOGL.m.doc \
  figs geomview.pdf

MAKEINFO = makeinfo --no-validate
info_TEXINFOS = geomview.texi

all: geomview.pdf

# We make geomview.pdf depend on geomview.texi rather than on geomview.dvi,
# because 'make clean' removes geomview.dvi and we don't want to force
# 'geomview.pdf' to have to be regenerated in this case --- it really only
# needs to be regenerated if geomview.texi changes.
geomview.pdf: geomview.texi
	tex geomview.texi
	tex geomview.texi
	dvipdf geomview.dvi geomview.pdf

# Provide rules for building geomview.dvi directly in case we want to do
# that.  These rules aren't invoked by anything else in this file; just
# here for use in development.
geomview.dvi: geomview.texi
	tex geomview.texi
	tex geomview.texi

MAINTAINERCLEANFILES = geomview.pdf
