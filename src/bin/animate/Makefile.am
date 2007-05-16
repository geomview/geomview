INCLUDES = $(default_includes) $(MOTIFINCLUDE)
SUBDIRS = interface

EXTRA_DIST = animate.1gv

man_MANS = animate.1gv

module_PROGRAMS = animate
animate_SOURCES = \
 animate.c xanimate.c glob.c animate.h glob.h xanimate.h \
 interface/animate.mib interface/command.mib interface/info.mib
animate_LDADD = $(MOTIFUILIB) $(MOTIFLIBS) $(XMULIBS) $(XLIBS)
#animate_LDFLAGS = -rpath $(libdir)

module_DATA = .geomview-animate

.geomview-animate:
	echo '(emodule-define "Animator" "animate")' > .geomview-animate

# Populate local "modules" directory, allowing animate to run in the build dir.
all-local: $(top_builddir)/modules/.geomview-animate

$(top_builddir)/modules/.geomview-animate: Makefile
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Animator" "$(subdir)/animate")' > $@


CLEANFILES = $(top_builddir)/modules/.geomview-animate $(module_DATA)

