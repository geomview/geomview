DEFS = -DHAVE_CONFIG_H

INCLUDES = -I$(geomviewincludes) \
 -I$(geomviewincludes)/private \
 $(default_includes) \
 -I$(TCL_INCLUDE_PATH) \
 -I$(TK_INCLUDE_PATH)

LDADD = $(TK_LIB) $(TCL_LIB) $(XLIBS) $(OOGLLIB)

man_MANS =

module_tcl_DATA = Labeler
module_PROGRAMS = labeler
labeler_SOURCES = labeler.c testwish.c

EMODULES = .geomview-labeler

if LOCAL_EMODULE
  homedir = $(HOME)
  home_DATA = $(EMODULES)
  tclpath = $(module_tcldir)
  modulepath = $(moduledir)/
else
  module_DATA = $(EMODULES)
  tclpath = $$GEOMVIEW_EMODULE_DIR/tcl
if !GEOMVIEW_QUERY
    modulepath = $(moduledir)/
else
    modulepath =
endif
endif

.geomview-labeler: Makefile
	echo '(emodule-define "Labeler" "$(modulepath)labeler $(tclpath)/Labeler")' > $@

.geomview: Makefile
	echo '(emodule-define "Labeler" "$(abs_builddir)/labeler $(abs_srcdir)/Labeler")' > $@

all-local: .geomview

CLEANFILES = $(EMODULES) .geomview

EXTRA_DIST = $(module_tcl_DATA) $(man_MANS)
