INCLUDES = $(default_includes) $(MOTIFINCLUDE)
SUBDIRS = interface
EXTRA_DIST = animate.1

bin_PROGRAMS = animate
animate_SOURCES = animate.c xanimate.c glob.c animate.h glob.h xanimate.h
LDADD = ../../lib/mib/libmib.a $(MOTIFLIBS) $(XMULIBS) $(XLIBS) $(MATHLIB)

noinst_SCRIPTS = install-geomview-dotfile
bin_SCRIPTS = .geomview-animate

install-geomview-dotfile: animate
	$(top_builddir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Animator" "$(subdir)/animate")' > $(top_builddir)/modules/.geomview-animate


.geomview-animate:
	echo '(emodule-define "Animator" "animate")' > .geomview-animate
