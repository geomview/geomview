INCLUDES = $(default_includes)

LDADD =  $(OOGLLIB)

man_MANS = nose.1gv

EXTRA_DIST = $(man_MANS)

module_PROGRAMS = nose
nose_SOURCES = nose.c
nose_DEPENDENCIES = $(OOGLLIB)

module_DATA = .geomview-nose

.geomview-nose: Makefile
	echo '(emodule-define "Nose" "nose")' > .geomview-nose

all-local: $(top_builddir)/modules/.geomview-nose

$(top_builddir)/modules/.geomview-nose: Makefile
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Nose" "$(subdir)/nose")' > $@

CLEANFILES = $(top_builddir)/modules/.geomview-nose $(module_DATA)

