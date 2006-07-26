XFORMS_SUBDIRS = cplxview example flythrough ginsu graffiti hinge \
 stereo sweep tackdown transformer

# Geomview's top-level configure script sets $(src_bin_module_subdirs)
# to the list of subdirectories of this directory (src/bin) that have
# a 'configure.in' file in them.  That way we can insert a new module
# that has been released as a package with its own configure.in into
# an existing Geomview source tree by just unpacking it into this
# directory.  We don't want to distribute these module directories
# with the main Geomview distribution, though, so we define
# DIST_SUBDIRS without $(src_bin_module_subdirs).

NORMAL_SUBDIRS = geomview togeomview geomutil animate

DIST_SUBDIRS = $(NORMAL_SUBDIRS) $(XFORMS_SUBDIRS)

SUBDIRS = $(NORMAL_SUBDIRS) $(src_bin_module_subdirs)

if HAVE_XFORMS
SUBDIRS += $(XFORMS_SUBDIRS)
endif
