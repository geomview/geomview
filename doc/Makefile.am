SUBDIRS = html
EXTRA_DIST = geomview.pdf README gvplot.txt oogltour \
  example1.c example2.c example3.c example4.tcl \
  geomview.1 geomview.5 geomview.5.man lisp.3 oogl.5 oogl.5.man OOGL.m.txt \
  figs/ap.ps figs/cam.ps figs/color.ps figs/command.ps \
  figs/fig2.ps figs/initial.ps figs/light.ps figs/load.ps figs/main.ps \
  figs/mat.ps figs/myfilelist.ps figs/save.ps figs/secondlinelist.ps \
  figs/tools.ps figs/trefdodecinter.ps

MAKEINFO = makeinfo --no-validate
info_TEXINFOS = geomview.texi

all: geomview.pdf

TEXINPUTS = $(srcdir):

SUFFIXES = .pdf

if HAVE_TEXI2PDF

.texi.pdf:
	TEXINPUTS=.:$$TEXINPUTS \
	  MAKEINFO='$(MAKEINFO) -I $(srcdir)' $(TEXI2PDF) $<

else

# We make geomview.pdf depend on geomview.texi rather than on geomview.dvi,
# because 'make clean' removes geomview.dvi and we don't want to force
# 'geomview.pdf' to have to be regenerated in this case --- it really only
# needs to be regenerated if geomview.texi changes.
geomview.pdf: $(srcdir)/geomview.texi
	tex geomview.texi
	tex geomview.texi
	$(DVIPDF) geomview.dvi geomview.pdf

endif

# Provide rules for building geomview.dvi directly in case we want to do
# that.  These rules aren't invoked by anything else in this file; just
# here for use in development.
geomview.dvi: $(srcdir)/geomview.texi
	tex geomview.texi
	tex geomview.texi

MAINTAINERCLEANFILES = geomview.pdf
