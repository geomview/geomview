EXTRA_DIST = .geomview-gvclock

module_SCRIPTS = gvclock
CLEANFILES = $(module_SCRIPTS)

module_DATA = .geomview-gvclock

.geomview-gvclock:
	echo '(emodule-define "Clock" "gvclock")' > $@

# handled by configure
#gvclock: gvclock.in Makefile
#	sed -e 's,[@]AWK[@],$(AWK),g' < $< > $@
#	chmod +x gvclock

all-local:
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Clock" "$(subdir)/gvclock")' > $(top_builddir)/modules/.geomview-gvclock
	chmod +x gvclock
