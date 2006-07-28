INCLUDES = $(default_includes) $(MOTIFINCLUDE)
SUBDIRS = interface

EXTRA_DIST = animate.1

man_MANS = animate.1

module_PROGRAMS = animate
animate_SOURCES = \
 animate.c xanimate.c glob.c animate.h glob.h xanimate.h \
 interface/animate.mib interface/command.mib interface/info.mib
animate_LDADD = $(MOTIFUILIB) $(MOTIFLIBS) $(XMULIBS) $(XLIBS)
animate_LDFLAGS = -rpath $(libdir)

module_DATA = .geomview-animate

.geomview-animate:
	echo '(emodule-define "Animator" "animate")' > .geomview-animate

# Populate local "modules" directory, allowing animate to run in the build dir.
all-local:
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Animator" "$(subdir)/animate")' > $(top_builddir)/modules/.geomview-animate

# noinst_SCRIPTS = install-geomview-dotfile
# bin_SCRIPTS = .geomview-animate
# 
# install-geomview-dotfile: animate
# 	$(top_builddir)/mkmodulesdir $(top_builddir)
# 	echo '(emodule-define "Animator" "$(subdir)/animate")' > $(top_builddir)/modules/.geomview-animate

DISTCLEANFILES = .geomview-animate
