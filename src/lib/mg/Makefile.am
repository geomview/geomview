# if X11RENDER
# X11RENDER_DIRS = x11
# else
# X11RENDER_DIRS =
# endif

SUBDIRS = buf common ps rib
EXTRA_DIST = mg.attr mg.doc mgP.h

if MGOPENGL
SUBDIRS += opengl
endif

if MGX11
SUBDIRS += x11
endif
