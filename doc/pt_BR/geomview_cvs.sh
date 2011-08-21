#commands for manual-pt_BR creation (manual compilation only)
#texi2dvi needs two excutions for index and links creation
#created on a slackware 12 under GPL by
#Jorge Barros de Abreu
cd ../figs
eps2pdf1.sh
cd ../pt_BR
texi2dvi --pdf --batch ../geomview-pt_BR.texi
texi2dvi --pdf --batch ../geomview-pt_BR.texi
makeinfo --force --enable-encoding ../geomview-pt_BR.texi -o geomview.info
texi2html -lang pt --split=chapter --output=tmp-html ../geomview-pt_BR.texi
rm *.vr *.tp *.toc *.pg *.ky *BR.log *.fn* *.cp *.aux
cd ../figs
rm *.pdf
cd ../pt_BR
