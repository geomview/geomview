top_geom_srcdir = $(top_srcdir)/../../..

INCLUDES = -I$(top_srcdir)/../../../include $(default_includes) $(XFORMSINCLUDE)

LDADD = $(OOGLLIBS)  $(OOGLLIBS)   \
	$(XFORMSLIBS) \
	$(XLIBS)     \
	-lm

EXTRA_DIST = .geomview-maniview \
   spaceshipSpace.geom wholecube.geom        \
   borrom4.wa controlpanel.fd dodec.vect gvinit hook.off \
   maniview.1 maniviewhelp $(datagroups_DATA)

datagroupsdir = $(geomdatadir)/groups

datagroups_DATA = 120.dgp 3torus.dgp borrom.dgp borrom2.dgp borrom4.dgp \
                  good_klein_quick.dgp klein_bricks.dgp klein_whole.dgp \
                  notknot.dgp

module_PROGRAMS = maniview
maniview_SOURCES = \
    callbacks.c controlpanel.c maniview.c controlpanel.h gvinit.h \
    maniview.h maniviewhelp.h

module_DATA = .geomview-maniview


.geomview-maniview:
	echo '(emodule-define "Maniview" "maniview")' > .geomview-maniview

all-local:
	$(top_geom_srcdir)/mkmodulesdir $(top_geom_builddir)
	echo '(emodule-define "Maniview" "src/bin/maniview/maniview")' > $(top_geom_builddir)/modules/.geomview-maniview

MAINTAINERCLEANFILES = \
  aclocal.m4 config.h.in configure stamp-h.in Makefile.in
