INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(OOGLLIBS)  \
	$(FORMSLIBS) \
	$(XLIBS)     \
        -lm

EXTRA_DIST = \
    HingeCube HingeTetra HingeDiamond HingeTriangle HingeDodec HingeLongcube \
    hingedata hinge.1 hingehelp panel.fd

bin_PROGRAMS = hinge

bin_SCRIPTS = .geomview-hinge

hinge_SOURCES = hui.c panel.c hingedata.h hui.h hinge.c lang.c panel.h \
    hinge.h hingehelp.h lang.h version.h

noinst_SCRIPTS = install-geomview-dotfile

install-geomview-dotfile: hinge
	$(top_builddir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Hinge" "$(subdir)/hinge")' > $(top_builddir)/modules/.geomview-hinge


.geomview-hinge:
	echo '(emodule-define "Hinge" "hinge")' > .geomview-hinge
