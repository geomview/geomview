INCLUDES = $(default_includes) $(MOTIFINCLUDE)

LDADD =  $(OOGLLIB) $(MOTIFUILIB) $(MOTIFLIBS) $(XMULIBS) $(XLIBS)

man_MANS =

EXTRA_DIST = .geomview-clipboard

module_PROGRAMS = clipboard
clipboard_SOURCES = clipboard.c clipboard.mib clipx.c clip.h
clipboard_DEPENDENCIES = $(OOGLLIB)
clipboard_LDFLAGS = -rpath $(libdir)

module_DATA = .geomview-clipboard

.geomview-clipboard:
	echo '(emodule-define "Clipboard" "clipboard")' > .geomview-clipboard

all-local:
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Clipboard" "$(subdir)/clipboard")' > $(top_builddir)/modules/.geomview-clipboard

