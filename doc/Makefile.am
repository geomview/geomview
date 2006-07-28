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

DOCDIR = $(datadir)/doc/$(PACKAGE_NAME)-$(PACKAGE_VERSION)

geomview.pdf:  $(PDFIMAGES)
geomview.ps:   $(EPSIMAGES)
geomview.dvi:  $(EPSIMAGES)
geomview.html: $(PNGIMAGES)

TEXINPUTS = $(srcdir):

all-local: html pdf

if MAINTAINER_MODE
figs/%.eps: figs/%.ps
	$(PS2EPSI) $< $@

figs/%.pdf: figs/%.eps
	$(EPSTOPDF) $<

figs/%.png: figs/%.pdf
	$(PDFIMAGESPROG) $< $(<:.pdf=)
	$(PNMTOPNG) $(<:.pdf=-000.ppm) > $@
	-rm -f $(<:.pdf=-000.ppm)

html-local: $(PNGIMAGES)
	-if test -d $(builddir)/figs; then \
	  $(mkdir_p) geomview.html/figs/; \
	  for file in $(PNGIMAGES); do \
	    if ! test -f "geomview.html/$$file"; then \
	      $(INSTALL_DATA) "$$file" "geomview.html/$$file"; \
	    fi; \
	  done; \
	fi

endif

dist-hook: html pdf

install-info-hook: $(PNGIMAGES)
	-$(mkdir_p) "$(DESTDIR)$(infodir)/figs"
	for file in $(PNGIMAGES); do \
		$(INSTALL_DATA) "$(srcdir)/$$file" "$(DESTDIR)$(infodir)/$$file"; \
	done

install-data-local: html pdf
	-$(mkdir_p) "$(DESTDIR)$(DOCDIR)/html/figs"
	for file in $(PNGIMAGES); do \
		$(INSTALL_DATA) "$(srcdir)/geomview.html/$$file" "$(DESTDIR)$(DOCDIR)/html/$$file"; \
	done
	-for file in $(srcdir)/geomview.html/*.html; do \
		$(INSTALL_DATA) "$$file" "$(DESTDIR)$(DOCDIR)/html/"; \
	done
	$(INSTALL_DATA) $(srcdir)/geomview.pdf "$(DESTDIR)$(DOCDIR)/"

uninstall-local:
	-rm -rf "$(DESTDIR)$(DOCDIR)/html/"
	-rm -f "$(DESTDIR)$(DOCDIR)/geomview.pdf"

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
