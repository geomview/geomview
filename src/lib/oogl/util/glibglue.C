#include <streambuf.h>
#include <strstream.h>
#include <stdiostream.h>
#include <iostream.h>

// This C++ glue allows futil.c to use the GNU libc (Linux)
// facilities for turning strings into streams, and for seeking back to
// a marked point in a stream.

struct stdio_mark {
    istdiostream* instream;
    streammarker *m;
};

strstreambuf *
CC_fmemopen(char *mem, int len)
{
    return new strstreambuf(mem, len);
}

stdio_mark *
CC_stdio_setmark(stdio_mark *sm, FILE* f)
{
    if(sm) delete sm->m;
    else   sm = new stdio_mark;
    sm->instream = new istdiostream(f);
    sm->m = new streammarker(sm->instream->rdbuf());
    return sm;
}

int
CC_stdio_seekmark(struct stdio_mark *sm)
{
    return sm->instream->rdbuf()->seekmark(*sm->m);
}

void
CC_stdio_freemark(stdio_mark *sm)
{
   delete sm->instream;
   delete sm->m;
   delete sm;
}

