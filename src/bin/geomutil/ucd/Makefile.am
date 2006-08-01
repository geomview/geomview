INCLUDES = @default_includes@

LDADD = $(STUBLIB) ${OOGLLIB}
DEPENDENCIES = $(STUBLIB) ${OOGLLIB}
#AM_LDFLAGS = -rpath $(libdir)

man_MANS = anytoucd.1 ucdtooff.1

bin_PROGRAMS = anytoucd ucdtooff

anytoucd_SOURCES = anytoucd.c make_frame.c stack.c vec4.h

ucdtooff_SOURCES = ucdtooff.c vec4.h

EXTRA_DIST = ucd_defs.h anytoucd.1 ucdtooff.1
