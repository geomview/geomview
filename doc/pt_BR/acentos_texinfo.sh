#!/bin/bash
#powered by Jorge Barros de Abreu under GPL
#ficmatin01 at solar dot com dot br
# this script needs "sed" and "mktemp" command
#adjust iso8859-1 characters to texinfo 
ARQ_TMP='mkstemp'
if [ $# = 0 ] #testa se nao existe parametro
 then
                echo -e "\n\tScript for diacritical marks in texinfo"
  echo -e "\n\tThis script needs "sed" and "mktemp" command"
                echo -e "\tusage: $0 file.texi"
                echo -e "\tthe output is renamed to file.texi itself"
                echo
                exit 1
fi
cat $1 | sed "/é/s/é/@'{e}/g" > $ARQ_TMP
cat $ARQ_TMP | sed "/á/s/á/@'{a}/g" > $1
# cat $1 | sed "/ç/s/ç/@value{cedilha}/g" > $ARQ_TMP
# cat $1 | sed "/Ç/s/Ç/@value{Cedilha}/g" > $ARQ_TMP
cat $1 | sed "/ç/s/ç/@,{c}/g" > $ARQ_TMP
cat $ARQ_TMP | sed "/ã/s/ã/@~{a}/g" > $1
cat $1 | sed "/Á/s/Á/@'{A}/g" > $ARQ_TMP
cat $ARQ_TMP | sed "/à/s/à/@\`{a}/g" > $1
cat $1 | sed "/À/s/À/@\`{A}/g" > $ARQ_TMP
cat $ARQ_TMP | sed "/É/s/É/@'{E}/g" > $1
cat $1 | sed "/Í/s/Í/@'{I}/g" > $ARQ_TMP
cat $ARQ_TMP | sed "/ú/s/ú/@'{u}/g" > $1
cat $1 | sed "/Ú/s/Ú/@'{U}/g" > $ARQ_TMP
cat $ARQ_TMP | sed "/Ç/s/Ç/@,{C}/g" > $1
cat $1 | sed "/õ/s/õ/@~{o}/g" > $ARQ_TMP
cat $ARQ_TMP | sed "/Õ/s/Õ/@~{O}/g" > $1
cat $1 | sed "/ü/s/ü/@\"{u}/g" > $ARQ_TMP
cat $ARQ_TMP | sed "/Ü/s/Ü/@\"{U}/g" > $1
cat $1 | sed "/Ã/s/Ã/@~{A}/g" > $ARQ_TMP
cat $ARQ_TMP | sed "/ó/s/ó/@'{o}/g" > $1
cat $1 | sed "/Ê/s/Ê/@^{E}/g" > $ARQ_TMP
cat $ARQ_TMP | sed "/ê/s/ê/@^{e}/g" > $1
cat $1 | sed "/í/s/í/@'{i}/g" > $ARQ_TMP
cat $ARQ_TMP | sed "/ê/s/ê/@^{e}/g" > $1
cat $1 | sed "/ô/s/ô/@^{o}/g" > $ARQ_TMP
cat $ARQ_TMP | sed "/Ô/s/Ô/@^{O}/g" > $1
cat $1 | sed "/â/s/â/@^{a}/g" > $ARQ_TMP
cat $ARQ_TMP | sed "/Â/s/Â/@^{A}/g" > $1
rm $ARQ_TMP