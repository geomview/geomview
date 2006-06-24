#! /bin/sh

# shell script for converting .ps files to .pdf
#   (This script converts all .ps files in the current directory to .pdf)
# contributed by Mikhail Tchernychev [mikhail_tchernychev@yahoo.com}
# 2000-11-14

#convert all .eps into .pdf

for i in *.ps
do

base=`basename $i .ps`

echo $i

rm -f t.eps t1.eps
ps2epsi $i t.eps	

if test x`which epstopdf` = x
then
    gawk -v file=$i '$1=="%%BoundingBox:"{ \
	print $1, 0, 0, $4-$2, $5-$3; \
	print -1*$2, -1*$3, "translate";\
	}\
	$1!="%%BoundingBox:" {print $0;}' t.eps  > t1.eps

    gscall=`gawk -v file=$i '$1=="%%BoundingBox:" { 
	outfile=file;
	gsub(".ps",".pdf", outfile); \
	printf "gs -g%dx%d -sDEVICE=pdfwrite -sOutputFile=%s -dBATCH %s\n", \
	10*$(NF-1), 10*($NF), outfile, FILENAME; exit; }' t1.eps`
    echo $gscall

    $gscall

else

    epstopdf t.eps
    mv t.pdf $base.pdf

fi


done 

rm -f t.eps t1.eps
