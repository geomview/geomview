#! /bin/sh

# geomview shell script driver

if test -x %GEOMVIEW_LTGVX%; then
    GEOMVIEW_GVX=%GEOMVIEW_LTGVX%
else
    GEOMVIEW_GVX=%GEOMVIEW_GVX%
fi

########################################################################
#
#		  DO NOT CHANGE ANYTHING BELOW HERE		       #
#
########################################################################

%LD_LIBRARY_PATH_SETTINGS%
export LD_LIBRARY_PATH

# Really necessary?
export LC_NUMERIC="POSIX"

### Geomview and/or its modules use the following environment
### variables.  For each of these that does not already have a value,
### this shell script assigns a value based on the setting of
### geomview_dir above.  You can override these values if you want by
### setting them in your shell environment before invoking this
### script.
### 
### GEOMVIEW_GVX: pathname of executable file "gvx"
### GEOMVIEW_LOAD_PATH: colon-separated list of directories to search
### 	for files
### GEOMVIEW_EMODULE_PATH: colon-separated list of directories to search
### 	for modules
### GEOMVIEW_SYSTEM_INITFILE: pathname of an initialization file to
### 	read upon startup
### GEOMDATA: [ used only by some modules; for backward compatibility ]
### 	pathname of the data directory

MACHTYPE=%MACHTYPE%

suf=

: ${GEOMROOT=%GEOMROOT%}

: ${GEOMDATA=%GEOMDATA%}

: ${GEOMVIEW_DATA_DIR=%GEOMDATA%}
: ${GEOMVIEW_LOAD_PATH=.:${GEOMDATA}/geom:${GEOMDATA}}
: ${GEOMVIEW_EMODULE_DIR=%MODULESDIR%}
: ${GEOMVIEW_EMODULE_PATH=${HOME}:${GEOMVIEW_EMODULE_DIR}}
: ${GEOMVIEW_SYSTEM_INITFILE=${GEOMDATA}/.geomview}
: ${GEOMVIEW_DOC_DIR=%GEOMDOCDIR%}

export GEOMVIEW_GVX GEOMVIEW_LOAD_PATH GEOMVIEW_DATA_DIR GEOMDATA
export GEOMVIEW_EMODULE_PATH GEOMVIEW_EMODULE_DIR
export GEOMVIEW_SYSTEM_INITFILE
export GEOMVIEW_DOC_DIR

if [ -d /cygdrive ]; then
    TCL_LIBRARY=/usr/share/tcl8.4
    TK_LIBRARY=/usr/share/tk8.4
    export TCL_LIBRARY TK_LIBRARY
fi

gvx="${GEOMVIEW_GVX}"
case "$gvx" in
  *gvx.[a-zA-Z3])
    gvx="`expr match \"${GEOMVIEW_GVX}\" '(.*)\.[^./]*' \| \"${GEOMVIEW_GVX}\"`" ;;
esac

if [ -f "$gvx$suf" ]; then
  GEOMVIEW_GVX="$gvx$suf"
elif [ -f "$gvx$suf2" ]; then
  GEOMVIEW_GVX="$gvx$suf2"
elif [ -f "$gvx" ]; then
  GEOMVIEW_GVX="$gvx"
fi
export GEOMVIEW_GVX

case "$1" in
    -dbx) GEOMVIEW_DEBUG=1 exec dbx ${GEOMVIEW_GVX} ;;
    -edge) GEOMVIEW_DEBUG=1 exec edge ${GEOMVIEW_GVX} ;;
    -gdb) GEOMVIEW_DEBUG=1 exec gdb ${GEOMVIEW_GVX} ;;
    -xdb) GEOMVIEW_DEBUG=1 exec xdb ${GEOMVIEW_GVX} ;;
    -ddd) GEOMVIEW_DEBUG=1 exec ddd ${GEOMVIEW_GVX} ;;
    -h|--help) cat <<EOF
Script options:

-dbx|edge|gdb|xdb|ddd Setup the environment and start the specfified debugger

-h|--help Print this help

--print-geomview-data-dir     Print the location of the main data directory
--print-geomview-emodule-dir  Print the system emodule path
--print-geomview-load-path    Print the data load-path
--print-geomview-emodule-path Print the emodule search-path
--print-geomview-system-initfile    Print the path to the system-wide init file
--print-geomview-gvx          Print the path to the gvx executable
--print-geomview-lib          Print the path to the geomview library
--print-geomview-include      Print the path to the geomview include directory

Options understood by the Geomview binary:

EOF
    exec ${GEOMVIEW_GVX} -h
    ;;

    --version) echo %PACKAGE_VERSION% ;;
    --print-geomview-data-dir) echo ${GEOMVIEW_DATA_DIR} ;;
    --print-geomview-emodule-dir) echo ${GEOMVIEW_EMODULE_DIR} ;;
    --print-geomview-load-path) echo ${GEOMVIEW_LOAD_PATH} ;;
    --print-geomview-emodule-path) echo ${GEOMVIEW_EMODULE_PATH} ;;
    --print-geomview-system-initfile) echo ${GEOMVIEW_SYSTEM_INITFILE} ;;
    --print-geomview-gvx) echo ${GEOMVIEW_GVX} ;;
    --print-geomview-lib) echo %GEOMVIEW_LIB% ;;
    --print-geomview-include) echo %GEOMVIEW_INCLUDE% ;;

  *) exec ${GEOMVIEW_GVX} ${1+"$@"} ;;
esac
