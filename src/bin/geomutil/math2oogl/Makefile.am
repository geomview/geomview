INCLUDES = @default_includes@

bin_PROGRAMS = math2oogl
man_MANS = math2oogl.1gv

math2oogl_SOURCES = math2oogl.c

LDADD = $(SOCKETLIBS)

EXTRA_DIST = BezierPlot.m OOGL.m Geomview.m OOGL.m.txt $(man_MANS)

mathematicadir = $(pkgdatadir)/Mathematica
mathematica_DATA = BezierPlot.m OOGL.m Geomview.m OOGL.m.txt

doc_DATA = OOGL.m.txt
