PSIMAGES = figs/ap.ps figs/command.ps figs/light.ps figs/mat.ps\
 figs/secondlinelist.ps figs/cam.ps figs/fig2.ps figs/load.ps\
 figs/myfilelist.ps figs/tools.ps figs/color.ps figs/initial.ps\
 figs/main.ps figs/save.ps figs/trefdodecinter.ps
EPSIMAGES = $(PSIMAGES:.ps=.eps)
PDFIMAGES = $(PSIMAGES:.ps=.pdf)
PNGIMAGES = $(PSIMAGES:.ps=.png)

EXTRA_DIST = geomview.pdf geomview.html README gvplot.txt oogltour \
  example1.c example2.c example3.c example4.tcl \
  geomview.1 geomview.5 geomview.5.man lisp.3 oogl.5 oogl.5.man OOGL.m.txt \
  motion.tex \
  $(PSIMAGES) $(PNGIMAGES) $(PDFIMAGES) $(EPSIMAGES)

AM_MAKEINFOFLAGS = --no-validate
info_TEXINFOS = geomview.texi

geomview.pdf:  $(PDFIMAGES)
geomview.ps:   $(EPSIMAGES)
geomview.dvi:  $(EPSIMAGES)
geomview.html: $(PNGIMAGES)

TEXINPUTS = $(srcdir):

figs/%.eps: figs/%.ps
	$(PS2EPSI) $< $@

figs/%.pdf: figs/%.eps
	$(EPSTOPDF) $<

figs/%.png: figs/%.pdf
	$(PDFIMAGESPROG) $< $(<:.pdf=)
	$(CONVERT) $(<:.pdf=-000.ppm) $@
	-rm -f $(<:.pdf=-000.ppm)

html-local: $(PNGIMAGES)
	-mkdir -p geomview.html/figs/
	-cp $(PNGIMAGES) geomview.html/figs/

dist-local: html pdf ps

CLEANFILES = geomview.dvi

MAINTAINERCLEANFILES = \
 $(PNGIMAGES) $(PDFIMAGES) $(EPSIMAGES) geomview.html geomview.pdf geomview.ps
