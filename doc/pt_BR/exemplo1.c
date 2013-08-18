/*
 * example1.c: oscillating mesh
 *
 * Esse módulo exemplo é distribuído com o manual do geomview.
 * Se você não estiver lendo esse código no manual veja o capítulo
 * "Módulos Externos" do manual para maiores detalhes.
 *
 * Esse módulo cria uma malha oscilante.
 */
/* versão português do Brasil
 * baseado em
 * e24062d6f5a21d3f6877eee7452a52eb  example1.c
 * compilar com "gcc exemplo1.c -lm -o exemplo1"
 * ficmatinfmag
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

  xmin = ymin = -5;  /* Ajusta o intervalo de        */
  xmax = ymax = 5;   /*  x e de y no gráfico         */
  xdim = ydim = 24;  /* Ajusta a resolução de x e y  */
  dt = 0.01;         /* O incremento de tempo é 0.01 */

  /* Ajuste do Geomview.  Iniciamos enviando o comando
   *            (geometry exemplo { : qualqueroisa})
   * para o geomview.  Esse comando diz ao geomview para
   * criar um geom chamado "exemplo" que é uma instância
   * do controlador "qualquercoisa".
   */
  printf("(geometry exemplo { : qualquercoisa })\n");
  fflush(stdout);

  /* Continuar até ser desativado.
   */
  for (t=0; ; t+=dt) {
    UpdateMesh(xmin, xmax, ymin, ymax, xdim, ydim, t);
  }
}

/* UpdateMesh envia uma iteração de malha ao geomview. 
 * A iteração de malha consiste de um comando da forma
 *    (read geometry { define qualquercoisa
 *       MESH
 *       ...
 *    })
 * onde ... são os dados atual da malha.  Esse comando diz ao
 * geomview para fazer o valor do controlador "qualquercoisa"
 * ser a malha especificada.
 */
UpdateMesh(xmin, xmax, ymin, ymax, xdim, ydim, t)
     float xmin, xmax, ymin, ymax, t;
     int xdim, ydim;
{
  int i,j;
  float x,y, dx,dy;

  dx = (xmax-xmin)/(xdim-1);
  dy = (ymax-ymin)/(ydim-1);

  printf("(read geometry { define qualquercoisa \n");
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
