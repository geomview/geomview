## FIXME: cannot use wildcards

EXTRA_DIST = *.html \
	pix/ap.gif pix/cam.gif pix/color.gif pix/command.gif \
	pix/fig2.gif pix/initial.gif pix/light.gif pix/load.gif \
	pix/main.gif pix/mat.gif pix/myfilelist.gif pix/save.gif \
	pix/secondlinelist.gif pix/tools.gif pix/trefdodecinter.gif \
	pix/world.gif


all: geomview_toc.html

*.html geomview_toc.html: $(srcdir)/../geomview.texi
	texi2html -split_node -menu $(srcdir)/../geomview.texi
	-rm -f index.html
	-ln geomview_toc.html index.html

# Commented this out to avoid warning about rule for
# "geomview_toc.html" being redefined.
#*.html: $(srcdir)/../geomview.texi
#	texi2html -split_node -menu $(srcdir)/../geomview.texi
#	-rm -f index.html
#	-ln geomview_toc.html index.html

MAINTAINERCLEANFILES = *.html
