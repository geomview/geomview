#! /bin/sh

# geomview shell script driver

GEOMROOT=%GEOMROOT%
GEOMVIEW_GVX=%GEOMVIEW_GVX%

########################################################################
#
#		  DO NOT CHANGE ANYTHING BELOW HERE		       #
#
########################################################################

%LD_LIBRARY_PATH_SETTINGS%
export LD_LIBRARY_PATH

### Geomview and/or its modules use the following environment
### variables.  For each of these that does not already have a value,
### this shell script assigns a value based on the setting of
### geomview_dir above.  You can override these values if you want by
### setting them in your shell environment before invoking this
### script.
### 
### GEOMVIEW_DIR: pathname of geomview distribution directory
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

: ${GEOMDATA=%GEOMDATA%}

: ${GEOMVIEW_LOAD_PATH=.:${GEOMDATA}/geom:${GEOMDATA}}
: ${GEOMVIEW_EMODULE_PATH=%MODULESDIR%}
: ${GEOMVIEW_SYSTEM_INITFILE=${GEOMDATA}/.geomview}

export GEOMROOT GEOMVIEW_GVX GEOMVIEW_LOAD_PATH GEOMDATA \
        GEOMVIEW_EMODULE_PATH GEOMVIEW_SYSTEM_INITFILE


gvx="${GEOMVIEW_GVX}"
case "$gvx" in
  *gvx.[a-zA-Z3])
    gvx="`expr match \"$GEOMVIEW_GVX\" '(.*)\.[^./]*' \| \"$GEOMVIEW_GVX\"`" ;;
esac

can_C3D() {
  case "$DISPLAY" in
    :*|unix:*) test -w /dev/fbs/ffb0 || return 1 ;;
  esac
  case `ldd "${gvx}.C3D" 2>&1` in
    *found*|*"No such"*) return 1 ;;
  esac
}

gvx_option() {
 case "$1" in
  -ogl|-OGL|-opengl|-OpenGL|-OPENGL)
	suf=.OGL; suf2=.MESA
	if [ "$MACHTYPE" = "solaris" ]; then
	    suf=.MESA
	    if can_C3D; then suf=.C3D; fi
	fi
	shift ;;
  -mesa|-Mesa|-MESA)
	suf=.MESA; suf2=.OGL ;;
  -x11|-X11)
	suf=.X11; shift ;;
  -c3d|-C3D|-Creator3D|-creator3d)
	suf=.C3D; suf2=.MESA ;;
  -h|-help|-HELP|-\?*)
    echo "For advice on selecting which geomview graphics option to use,
see also \"geomview -graphics\"."
    ;;
  -grap*|-Grap*|-GRAP*)
    if [ -f "$gvx.X11" -o -f "$gvx.MESA" -o -f "$gvx.C3D" -o -f "$gvx.OGL" ]; then
	g=`basename $0`
	echo "The $0 script can invoke any of several executables:"
	test -f "$gvx.X11" && \
	  echo "  with geomview's own X software rendering: use $g -x11"
	test -f "$gvx.MESA" && \
	  echo "  with Mesa OpenGL-like software rendering: use $g -mesa"
	test -f "$gvx.X11" && \
	  echo "  with Sun OpenGL for UltraSparcs with Creator3D: use $g -creator3d"
	test -f "$gvx.OGL" && \
	  echo "  with Open GL: use $g -opengl"
	echo "If given, this option MUST BE THE FIRST on geomview's command line."
	echo "Using some variant of Open GL allows transparency and texture-mapping;"
	echo "software OpenGL implementations are slower than $g -x11, but hardware ones"
	echo "run faster."
	echo "To change the default, setenv GEOMVIEW_GVXOPT to the appropriate option."
    fi >&2
    exit 1
    ;;
  *) test ;;
 esac
}

gvx_option ${GEOMVIEW_GVXOPT}

if gvx_option "$1"; then shift; fi


if [ "$MACHTYPE" = "solaris" -a -z "$suf" -a -w /dev/fbs/ffb0 ] && can_C3D; then
  echo "$0: Invoking Creator3D version (see \"geomview -graphics\" for info)" >&2
  suf=.C3D
fi

if [ -f "$gvx$suf" ]; then
  GEOMVIEW_GVX="$gvx$suf"
elif [ -f "$gvx$suf2" ]; then
  GEOMVIEW_GVX="$gvx$suf2"
elif [ -f "$gvx" ]; then
  GEOMVIEW_GVX="$gvx"
fi
export GEOMVIEW_GVX

case "$1" in
  -dbx) GEOMVIEW_DEBUG=1 exec dbx $GEOMVIEW_GVX ;;
  -edge) GEOMVIEW_DEBUG=1 exec edge $GEOMVIEW_GVX ;;
  -gdb) GEOMVIEW_DEBUG=1 exec gdb $GEOMVIEW_GVX ;;
  -xdb) GEOMVIEW_DEBUG=1 exec xdb $GEOMVIEW_GVX ;;
  -ddd) GEOMVIEW_DEBUG=1 exec ddd $GEOMVIEW_GVX ;;
  *) exec $GEOMVIEW_GVX ${1+"$@"} ;;
esac
