#include <streambuf.h>
#include <strstream.h>
#include <stdiostream.h>
#include <iostream.h>

// This C++ glue allows futil.c to use the GNU libc (Linux)
// facilities for turning strings into streams, and for seeking back to
// a marked point in a stream.

struct stdio_mark {
    stdiobuf *sb;
    streammarker *m;
};

strstreambuf *
CC_fmemopen(char *mem, int len)
{
    return new strstreambuf(mem, len);
}

stdio_mark *
CC_stdio_setmark(stdio_mark *sm, stdiobuf *f)
{
    if(sm) delete sm->m;
    else   sm = new stdio_mark;
    sm->sb = f;
    sm->m = new streammarker(f);
    return sm;
}

int
CC_stdio_seekmark(struct stdio_mark *sm)
{ return (*sm->sb).seekmark(*sm->m); }

void
CC_stdio_freemark(stdio_mark *sm)
{  // Don't delete sm->sb -- we were just borrowing it!
   delete sm->m;
   delete sm;
}

