#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED
@TOP@

/* MACHTYPE from old Geomview makefile system */
#undef MACHTYPE

/* Does -lXmu exist on this system? */
#define HAVE_XMU 0

#define AIX 0

#undef sgi

#undef unix

#undef alloca

/* ACCEPT_ARG3_TYPE should be the datatype of the 3rd arg to the
   'accept' system call.  It's 'int' on some systems, 'unsigned long'
   on some, and could be something else on others.  Default to int */
#define ACCEPT_ARG3_TYPE int

/* another from old Geomview makefile system */
#undef __linux__

/* On linux, is the function __setfpucw in the standard libary? */
/* (This is used in gvmain.c) */
#undef HAVE_SETFPUCW

/* Use experimental motion averaging code? */
#define EXPERIMENTAL_MOTION_AVERAGING 0

/* D1 debugging? */
#define D1_DEBUG 0

/* ALISTADDR is a hack to get around the fact that on dpvc's LinuxPPC    */
/* system, some of the occurences of '&a_list' had to be replaced with   */
/* 'a_list'.  I'm not sure why this was the case, but to work around it, */
/* I replaced those occurences of '&a_list' in the source with           */
/* 'ALISTADDR a_list', so we can define ALISTADDR to be empty on         */
/* LinuxPPC and '&' otherwise.                                           */
#define ALISTADDR &

/* BINARY_POPEN_REQUIRES_B says whether the system's popen() call needs  */
/* a 'b' in the 2nd argument when the data coming through its pipe is	 */
/* binary.  This comes up in mgtexture.c when popen is called to pipe	 */
/* a compressed image through gzip.  Initially Stuart wrote the call	 */
/* with the 2nd arg being "rb", but this didn't work on my Linux	 */
/* system so I added this switch to allow it to be compiled with just	 */
/* "r", which does work.  For now, I'm just setting this to 0 to force	 */
/* "r" on all systems, but it might be necessary to use "rb" on some	 */
/* systems which treat binary files differently (Windows?), in which	 */
/* case the configure script should be modified to set			 */
/* BINARY_POPEN_REQUIRES_B to 1 for those systems.			 */
/* mbp Tue Sep 12 12:33:38 2000						 */
#define BINARY_POPEN_REQUIRES_B 0

/* HAVE_FMEMOPEN is 1 if and only if the function fmemopen() is		 */
/* available on the system.  This is used in src/lib/oogl/util/futil.c   */
/* to decide how to implement fstropen().  See also USE_FSTROPEN_1	 */
/* below.
#define HAVE_FMEMOPEN 0

/* USE_FSTROPEN_1 determines whether we forceably use one of our	 */
/* manual implementations of fstropen (in src/lib/oogl/util/futil.c)	 */
/* rather than calling fmemopen or CC_fmemopen__FPci.  This is		 */
/* relevant only on Linux systems.  Normally you shouldn't have to set	 */
/* this at all; futil.c will use HAVE_FMEMOPEN, defined above, to	 */
/* decide what to do.  If this doesn't work for you (the symptom is	 */
/* that Geomview crashes upon startup while trying to read the initial	 */
/* appearance string), then try configuring with			 */
/* "--enable-fstropen-1", which sets USE_FSTROPEN_1.			 */
/* See the comments in in src/lib/oogl/util/futil.c for details.         */
#define USE_FSTROPEN_1 0

@BOTTOM@

#if D1_DEBUG
#define D1PRINT(args) printf args
#else
#define D1PRINT(args)
#endif

#endif /* CONFIG_H_INCLUDED */
