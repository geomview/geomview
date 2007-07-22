texi2dvi --pdf --batch --lang=pt geomview_pt_BR.texi
makeinfo --force --enable-encoding geomview_pt_BR.texi -o geomview.info
texi2html --lang=pt --split=chapter --output=html geomview_pt_BR.texi