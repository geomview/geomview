INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(OOGLLIBS)  \
	$(FORMSLIBS) \
	$(XLIBS)     \
        -lm

EXTRA_DIST = \
    HingeCube HingeTetra HingeDiamond HingeTriangle HingeDodec HingeLongcube \
    hingedata hinge.1 hingehelp panel.fd

modules_PROGRAMS = hinge

modules_DATA = .geomview-hinge

hinge_SOURCES = hui.c panel.c hingedata.h hui.h hinge.c lang.c panel.h \
    hinge.h hingehelp.h lang.h version.h
