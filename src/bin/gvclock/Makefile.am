module_SCRIPTS = gvclock

module_DATA = .geomview-gvclock

.geomview-gvclock:
	echo '(emodule-define "Clock" "gvclock")' > $@

# handled by configure
#gvclock: gvclock.in Makefile
#	sed -e 's,[@]AWK[@],$(AWK),g' < $< > $@
#	chmod +x gvclock

all-local: $(top_builddir)/modules/.geomview-gvclock

$(top_builddir)/modules/.geomview-gvclock: Makefile gvclock
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Clock" "$(subdir)/gvclock")' > $@
	chmod +x gvclock

CLEANFILES = \
 $(top_builddir)/modules/.geomview-gvclock \
 $(module_DATA) \
 $(module_SCRIPTS)
