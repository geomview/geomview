#include <QvDB.h>
#include <QvInput.h>
#include <QvNode.h>
#include <QvGroup.h>
#include <QvState.h>
#include <QvToWebOOGL.h>

int
main(int argc, char **argv)
{
    QvDB::init();

    QvInput	in;
    QvGroup	root;
    char        *handle = NULL;

    if (argc == 2) {
      handle = argv[1];
    } else if (argc > 2) {
      fprintf(stderr, "Usage: vrml2oogl [handle] < vrmlfile > ooglfile\n\
Convert VRML file (possibly compressed/gzipped) into an OOGL file.\n\
handle: Return GCL not OOGL, where the OOGL geometry is defined as the handle.\n\
The handle generally corresponds to a URL.\n");
      return(1);
    }

    if(! root.readChildren(&in)) {
	fprintf(stderr, "vrml2oogl: Couldn't parse VRML.\n");
    }


    if (handle) {
      fprintf(stdout, "(progn (read geometry {define \"%s\" {\n", handle);
    } else {
      fprintf(stdout, "{\n");
    }

    QvState state;
    OOGLhandle(handle);
    OOGLfile(stdout);
    root.ToWebOOGL(&state);

    if (handle) {
      int i;
      fprintf(stdout, "}})\n");
      for (i = 0; i < OOGLnumuniq(); i++) {
	fprintf(stdout, 
		" (emodule-transmit \"weboogl.perl\" \"(NeedURL %s)\n\")\n",
		OOGLgeturls(i));
      }
      fprintf(stdout, " )\n ");
    } else {
      fprintf(stdout, "}\n");
    }


    return 0;
}
