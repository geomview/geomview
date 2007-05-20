/*
 * example3.c: external module with bi-directional communication
 *
 * This example module is distributed with the geomview manual.
 * If you are not reading this in the manual, see the "External
 * Modules" chapter of the manual for an explanation.
 *
 * This module is the same as the "Nose" program that is distributed
 * with geomview.  It illustrates how a module can find out about
 * and respond to user pick events in geomview.  It draws a little box
 * at the point where a pick occurrs.  The box is yellow if it is not
 * at a vertex, and magenta if it is on a vertex.  If it is on an edge,
 * the program also marks the edge.
 *
 * To compile:
 *
 *   cc -I/u/gcg/ngrap/include -g -o example3 example3.c \
 *      -L/u/gcg/ngrap/lib/sgi -loogl -lm
 *
 * You should replace "/u/gcg/ngrap" above with the pathname of the
 * geomview distribution directory on your system.
 */

#include <stdio.h>
#include "lisp.h"               /* We use the OOGL lisp library */
#include "pickfunc.h"           /* for PICKFUNC below */
#include "3d.h"                 /* for 3d geometry library */

/* boxstring gives the OOGL data to define the little box that
 * we draw at the pick point.  NOTE:  It is very important to
 * have a newline at the end of the OFF object in this string.
 */
char boxstring[] = "\
INST\n\
transform\n\
.04 0 0 0\n\
0 .04 0 0\n\
0 0 .04 0\n\
0 0 0 1\n\
geom\n\
OFF\n\
8 6 12\n\
\n\
-.5 -.5 -.5     # 0   \n\
.5 -.5 -.5      # 1   \n\
.5  .5 -.5      # 2   \n\
-.5  .5 -.5     # 3   \n\
-.5 -.5  .5     # 4   \n\
.5 -.5  .5      # 5   \n\
.5  .5  .5      # 6   \n\
-.5  .5  .5     # 7   \n\
\n\
4 0 1 2 3\n\
4 4 5 6 7\n\
4 2 3 7 6\n\
4 0 1 5 4\n\
4 0 4 7 3\n\
4 1 2 6 5\n";

progn()
{
  printf("(progn\n");
}

endprogn()
{
  printf(")\n");
  fflush(stdout);
}

Initialize()
{
  extern LObject *Lpick();  /* This is defined by PICKFUNC below but must */
  			    /* be used in the following LDefun() call */
  LInit();
  LDefun("pick", Lpick, NULL);

  progn(); {
    /* Define handle "littlebox" for use later
     */
    printf("(read geometry { define littlebox { %s }})\n", boxstring);

    /* Express interest in pick events; see geomview manual for explanation.
     */
    printf("(interest (pick world * * * * nil nil nil nil nil))\n");

    /* Define "pick" object, initially the empty list (= null object).
     * We replace this later upon receiving a pick event.
     */
    printf("(geometry \"pick\" { LIST } )\n");

    /* Make the "pick" object be non-pickable.
     */
    printf("(pickable \"pick\" no)\n");

    /* Turn off normalization, so that our pick object will appear in the
     * right place.
     */
    printf("(normalization \"pick\" none)\n");

    /* Don't draw the pick object's bounding box.
     */
    printf("(bbox-draw \"pick\" off)\n");

  } endprogn();
}

/* The following is a macro call that defines a procedure called
 * Lpick().  The reason for doing this in a macro is that that macro
 * encapsulates a lot of necessary stuff that would be the same for
 * this procedure in any program.  If you write a geomview module that
 * wants to know about user pick events you can just copy this macro
 * call and change the body to suit your needs; the body is the last
 * argument to the macro and is delimited by curly braces.
 *
 * The first argument to the macro is the name of the procedure to
 * be defined, "Lpick".
 *
 * The next two arguments are numbers which specify the sizes that
 * certain arrays inside the body of the procedure should have.
 * These arrays are used for storing the face and path information
 * of the picked object.  In this module we don't care about this
 * information so we declare them to have length 1, the minimum
 * allowed.
 *
 * The last argument is a block of code to be executed when the module
 * receives a pick event.  In this body you can refer to certain local
 * variables that hold information about the pick.  For details see
 * Example 3 in the Extenal Modules chapter of the geomview manual.
 */
PICKFUNC(Lpick,
{           
  handle_pick(pn>0, &point, vn>0, &vertex, en>0, edge);
},
/* ND-stuff */)

handle_pick(picked, p, vert, v, edge, e)
     int picked;                /* was something actually picked?     */
     int vert;                  /* was the pick near a vertex?        */
     int edge;                  /* was the pick near an edge?         */
     HPoint3 *p;                /* coords of pick point               */
     HPoint3 *v;                /* coords of picked vertex            */
     HPoint3 e[2];              /* coords of endpoints of picked edge */
{
  Normalize(&e[0]);             /* Normalize makes 4th coord 1.0 */
  Normalize(&e[1]);
  Normalize(p);
  progn(); {
    if (!picked) {
      printf("(geometry \"pick\" { LIST } )\n");
    } else {
      /*
       * Put the box in place, and color it magenta if it's on a vertex,
       * yellow if not.
       */
      printf("(xform-set pick { 1 0 0 0  0 1 0 0  0 0 1 0  %g %g %g 1 })\n",
             p->x, p->y, p->z);
      printf("(geometry \"pick\"\n");
      if (vert) printf("{ appearance { material { diffuse 1 0 1 } }\n");
      else printf("{ appearance { material { diffuse 1 1 0 } }\n");
      printf("  { LIST { :littlebox }\n");
      
      /*
       * If it's on an edge and not a vertex, mark the edge
       * with cyan boxes at the endpoins and a yellow line
       * along the edge.
       */
      if (edge && !vert) {
        e[0].x -= p->x; e[0].y -= p->y; e[0].z -= p->z;
        e[1].x -= p->x; e[1].y -= p->y; e[1].z -= p->z;
        printf("{ appearance { material { diffuse 0 1 1 } }\n\
  LIST\n\
   { INST transform 1 0 0 0 0 1 0 0 0 0 1 0 %f %f %f 1 geom :littlebox }\n\
   { INST transform 1 0 0 0 0 1 0 0 0 0 1 0 %f %f %f 1 geom :littlebox }\n\
   { VECT\n\
          1 2 1\n\
          2\n\
          1\n\
          %f %f %f\n\
          %f %f %f\n\
          1 1 0 1\n\
   }\n\
  }\n",
               e[0].x, e[0].y, e[0].z,
               e[1].x, e[1].y, e[1].z,
               e[0].x, e[0].y, e[0].z,
               e[1].x, e[1].y, e[1].z);
      }
      printf("    }\n  }\n)\n");
    }

  } endprogn();

}

Normalize(HPoint3 *p)
{
  if (p->w != 0) {
    p->x /= p->w;
    p->y /= p->w;
    p->z /= p->w;
    p->w = 1;
  }
}

main()
{
  Lake *lake;
  LObject *lit, *val;
  extern char *getenv();

  Initialize();

  lake = LakeDefine(stdin, stdout, NULL);
  while (!feof(stdin)) {

    /* Parse next lisp expression from stdin.
     */
    lit = LSexpr(lake);

    /* Evaluate that expression; this is where Lpick() gets called.
     */
    val = LEval(lit);

    /* Free the two expressions from above.
     */
    LFree(lit);
    LFree(val);
  }
}
