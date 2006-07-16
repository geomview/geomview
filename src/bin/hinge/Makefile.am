INCLUDES = $(default_includes) $(XFORMSINCLUDE)

LDADD =	$(XFORMSLIBS) \
	$(XLIBS)     \
	$(OOGLLIB)

EXTRA_DIST = \
    HingeCube HingeTetra HingeDiamond HingeTriangle HingeDodec HingeLongcube \
    hingedata hinge.1 hingehelp panel.fd .geomview-hinge

module_PROGRAMS = hinge
hinge_SOURCES = hui.c panel.c hingedata.h hui.h hinge.c lang.c panel.h \
    hinge.h hingehelp.h lang.h version.h
hinge_DEPENDENCIES = $(OOGLLIB)
hinge_LDFLAGS = -rpath $(libdir)

module_DATA = .geomview-hinge

.geomview-hinge:
	echo '(emodule-define "Hinge" "hinge")' > .geomview-hinge

all-local:
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Hinge" "$(subdir)/hinge")' > $(top_builddir)/modules/.geomview-hinge
