INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(OOGLLIBS)  \
	$(FORMSLIBS) \
	$(XLIBS)     \
	-lm

EXTRA_DIST = .geomview-maniview \
   spaceshipSpace.geom wholecube.geom        \
   borrom4.wa controlpanel.fd dodec.vect gvinit hook.off \
   maniview.1 maniviewhelp

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
	$(top_srcdir)/mkmodulesdir $(top_builddir)
	echo '(emodule-define "Maniview" "$(subdir)/maniview")' > $(top_builddir)/modules/.geomview-maniview
