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

@BOTTOM@

#if D1_DEBUG
#define D1PRINT(args) printf args
#else
#define D1PRINT(args)
#endif

#endif /* CONFIG_H_INCLUDED */
