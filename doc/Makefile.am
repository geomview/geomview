PSIMAGES = figs/ap.ps figs/command.ps figs/light.ps figs/mat.ps\
 figs/secondlinelist.ps figs/cam.ps figs/fig2.ps figs/load.ps\
 figs/myfilelist.ps figs/tools.ps figs/color.ps figs/initial.ps\
 figs/main.ps figs/save.ps figs/trefdodecinter.ps
EPSIMAGES = $(PSIMAGES:.ps=.eps)
PDFIMAGES = $(PSIMAGES:.ps=.pdf)
PNGIMAGES = $(PSIMAGES:.ps=.png)

man_MANS = geomview.5gv lisp.3gv oogl.5gv

EXTRA_DIST = geomview.pdf geomview.html README README.gvplot oogltour\
  example1.c example2.c example3.c example4.tcl\
  OOGL.m.txt motion.tex $(man_MANS)\
  $(PSIMAGES) $(PNGIMAGES) $(PDFIMAGES) $(EPSIMAGES)

AM_MAKEINFOFLAGS = --no-validate
info_TEXINFOS = geomview.texi
infoimgdir = $(infodir)
nobase_infoimg_DATA = $(PNGIMAGES)

dochtmldir = $(docdir)/html
nobase_dochtml_DATA = $(PNGIMAGES)
dochtml_DATA = $(wildcard geomview.html/* $(srcdir)/geomview.html/*)
doc_DATA = geomview.pdf

geomview.pdf:  $(PDFIMAGES)
geomview.ps:   $(EPSIMAGES)
geomview.dvi:  $(EPSIMAGES)
geomview.html: $(PNGIMAGES)

TEXINPUTS = $(srcdir):

all-local: html pdf

figs/%.eps: figs/%.ps
	@test -d figs || mkdir figs
	$(PS2EPSI) $< $@

figs/%.pdf: figs/%.eps
	@test -d figs || mkdir figs
	$(EPSTOPDF) $<

figs/%.png: figs/%.pdf
	@test -d figs || mkdir figs
	$(PDFIMAGESPROG) $< $(<:.pdf=)
	$(PNMTOPNG) $(<:.pdf=-000.ppm) > $@
	-rm -f $(<:.pdf=-000.ppm)

dist-hook: html pdf

MAINTAINERCLEANFILES = \
 $(PNGIMAGES) $(PDFIMAGES) $(EPSIMAGES) geomview.pdf

maintainerclean-local:
	-rm -rf geomview.html

#
# override to disable deletion of .pdf, .html by the user.
#
mostlyclean-aminfo:
	-rm -rf geomview.aux geomview.cp geomview.cps geomview.fn geomview.fns \
	  geomview.ky geomview.kys geomview.log geomview.pg \
	  geomview.pgs geomview.tmp geomview.toc geomview.tp \
	  geomview.tps geomview.vr geomview.vrs geomview.dvi \
	  geomview.ps
