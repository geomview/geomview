SUBDIRS = m4 doc src xforms
EXTRA_DIST = reconf boot geomview.sh data build INSTALL.Geomview mkinstalldirs mkmodulesdir

noinst_SCRIPTS = geomview

geomview: geomview.sh Makefile
	echo top_builddir is $(top_builddir)
	sed -e 's|%GEOMROOT%|$(top_builddir)|g'                              \
	    -e 's|%GEOMVIEW_GVX%|$(top_builddir)/src/bin/geomview/x11/gvx|g' \
	    -e 's|%MACHTYPE%|$(MACHTYPE)|g'                                  \
	    -e 's|%MODULESDIR%|modules|g'                                    \
	    -e "s|%LD_LIBRARY_PATH_SETTINGS%|$(LD_LIBRARY_PATH_SETTINGS)|"   \
	    < geomview.sh > geomview
	chmod +x geomview

DISTCLEANFILES = geomview

distclean-local:
	rm -rf modules

datadir = ${prefix}/data

dataDATA = data/* data/.geomview

install-data-local: install-dataDATA
uninstall-local: uninstall-dataDATA

install-dataDATA: $(dataDATA)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(datadir)
	@list='$(dataDATA)'; for p in $$list; do \
	  if test -e $(srcdir)/$$p; then \
	    echo " cp -r $(srcdir)/$$p $(DESTDIR)$(datadir)"; \
	    cp -r $(srcdir)/$$p $(DESTDIR)$(datadir); \
	  else if test -e $$p; then \
	    echo " cp -r $$p $(DESTDIR)$(datadir)"; \
	    cp -r $$p $(DESTDIR)$(datadir); \
	  fi; fi; \
	done

uninstall-dataDATA:
	@$(NORMAL_UNINSTALL)
	list='$(dataDATA)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(datadir)/$$p; \
	done
