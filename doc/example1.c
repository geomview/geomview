/*
 * example1.c: oscillating mesh
 *
 * This example module is distributed with the geomview manual.
 * If you are not reading this in the manual, see the "External
 * Modules" chapter of the manual for more details.
 *
 * This module creates an oscillating mesh.
 */

#include <math.h>
#include <stdio.h>

/* F is the function that we plot
 */
float F(x,y,t)
     float x,y,t;
{
  float r = sqrt(x*x+y*y);
  return(sin(r + t)*sqrt(r));
}

main(argc, argv)        
     char **argv;
{
  int xdim, ydim;
  float xmin, xmax, ymin, ymax, dx, dy, t, dt;

  xmin = ymin = -5;             /* Set x and y            */
  xmax = ymax = 5;              /*    plot ranges         */
  xdim = ydim = 24;             /* Set x and y resolution */
  dt = 0.1;                     /* Time increment is 0.1  */

  /* Geomview setup.  We begin by sending the command
   *            (geometry example { : foo})
   * to geomview.  This tells geomview to create a geom called
   * "example" which is an instance of the handle "foo".
   */
  printf("(geometry example { : foo })\n");
  fflush(stdout);

  /* Loop until killed.
   */
  for (t=0; ; t+=dt) {
    UpdateMesh(xmin, xmax, ymin, ymax, xdim, ydim, t);
  }
}

/* UpdateMesh sends one mesh iteration to geomview.  This consists of
 * a command of the form
 *    (read geometry { define foo
 *       MESH
 *       ...
 *    })
 * where ... is the actual data of the mesh.  This command tells
 * geomview to make the value of the handle "foo" be the specified
 * mesh.
 */
UpdateMesh(xmin, xmax, ymin, ymax, xdim, ydim, t)
     float xmin, xmax, ymin, ymax, t;
     int xdim, ydim;
{
  int i,j;
  float x,y, dx,dy;

  dx = (xmax-xmin)/(xdim-1);
  dy = (ymax-ymin)/(ydim-1);

  printf("(read geometry { define foo \n");
  printf("MESH\n");
  printf("%1d %1d\n", xdim, ydim);
  for (j=0, y = ymin; j<ydim; ++j, y += dy) {
    for (i=0, x = xmin; i<xdim; ++i, x += dx) {
      printf("%f %f %f\t", x, y, F(x,y,t));
    }
    printf("\n");
  }
  printf("})\n");
  fflush(stdout);
}
