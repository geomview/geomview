INCLUDES = $(default_includes) $(FORMSINCLUDE)

LDADD = $(OOGLLIBS)  \
	$(FORMSLIBS) \
	$(XLIBS)     \
	-lm

EXTRA_DIST = .geomview-maniview \
   spaceshipSpace.geom wholecube.geom        \
   borrom4.wa controlpanel.fd dodec.vect gvinit hook.off \
   maniview.1 maniviewhelp

datagroupsdir = $(prefix)/data/groups

datagroups_DATA = 120.dgp 3torus.dgp borrom.dgp borrom2.dgp borrom4.dgp \
                  good_klein_quick.dgp klein_bricks.dgp klein_whole.dgp \
                  notknot.dgp

modules_PROGRAMS = maniview

modules_DATA = .geomview-maniview

maniview_SOURCES = \
    callbacks.c controlpanel.c maniview.c controlpanel.h gvinit.h \
    maniview.h maniviewhelp.h
