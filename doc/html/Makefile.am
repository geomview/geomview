EXTRA_DIST = *.html pix

all: geomview_toc.html

geomview_toc.html: ../geomview.texi
	texi2html -split_node -menu ../geomview.texi
	-rm -f index.html
	-ln geomview_toc.html index.html

*.html: ../geomview.texi
	texi2html -split_node -menu ../geomview.texi
	-rm -f index.html
	-ln geomview_toc.html index.html

MAINTAINERCLEANFILES = *.html
