INCLUDES = $(default_includes) $(XFORMSINCLUDE)

LDADD = $(XFORMSLIBS)	\
	$(XLIBS)	\
	-lm

EXTRA_DIST = .geomview-flythrough flyhelp flythrough.1 panel.fd

module_PROGRAMS = flythrough
flythrough_SOURCES = main.c flyhelp.h panel.c panel.h

module_DATA = .geomview-flythrough

.geomview-flythrough:
	echo '(emodule-define "Flythrough" "flythrough")' > .geomview-flythrough

all-local:
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Flythrough" "$(subdir)/flythrough")' > $(top_builddir)/modules/.geomview-flythrough

# bin_PROGRAMS = flythrough
# 
# bin_SCRIPTS = .geomview-flythrough
# 
# 
# noinst_SCRIPTS = install-geomview-dotfile
# 
# install-geomview-dotfile: flythrough
# 	$(top_builddir)/mkmodulesdir $(top_builddir)
# 	echo '(emodule-define "Flythrough" "$(subdir)/flythrough")' > $(top_builddir)/modules/.geomview-flythrough
