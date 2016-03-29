// Ultima versión, Evry, Francia, lunes 5 de diciembre 2005, 2:16 AM
// Esta versión ya soporta imagenes en blanco y negro, clave P5
//


#include <stdlib.h>

#define P6  1
#define P5  2

#define COLOR P6
#define BYN   P5

typedef struct ppm_picture {
  char *filename;
  char key;
  unsigned char *picture;
  unsigned int horizontal;
  unsigned int vertical;
  unsigned int maxvalue;
} PPM;

typedef struct pixel {
  unsigned int x;
  unsigned int y;
  unsigned char rojo;
  unsigned char verde;
  unsigned char azul;
  unsigned char gris;
} PIXEL;

/*
void put_pixel (PPM pic, PIXEL p);
void save_picture (PPM p);
void load_picture(PPM *p);
void get_pixel (PPM pic, PIXEL *p);
void create_picture(PPM *p, char *name, int horizontal, int vertical);
*/


void load_picture(PPM *p)
{
  FILE *fp;
  char buffer[80];
  char clave [80];
  char character;

  fp = fopen (p->filename, "r");
  if (fp == NULL) {
    printf ("Problemas al abrir el archiv %s\n", p->filename);
    exit (1);
  }

  fgets(clave, 80, fp); //printf ("clave -> %s\n", clave);
  character = getc (fp);
  if (character == '#') {
    fgets(buffer, 80, fp); //printf ("comentario %s\n", buffer);
  } else {
    ungetc (character, fp);
  }

  fscanf (fp, "%d %d", &p->horizontal, &p->vertical);  getc (fp);
  fscanf (fp, "%d", &p->maxvalue);  getc (fp);

//  printf ("-->%s<--", clave);

  if (strcmp ("P6\n", clave) == 0) {
    p->key = P6;
    p->picture = (unsigned char *) malloc (p->horizontal*p->vertical*3);
    fread (p->picture, 1, p->horizontal*p->vertical*3, fp);
  } else if (strcmp ("P5\n", clave) == 0) {
    p->key = P5;
    p->picture = (unsigned char *) malloc (p->horizontal*p->vertical);
    fread (p->picture, 1, p->horizontal*p->vertical, fp);
  } else {
    printf ("Unknown code: load picture\n");
    exit (1);
  }
  
  fclose (fp);
}

//void create_picture(PPM *p, char *name, int horizontal, int vertical)
//
//p->key = P5 or P6, that depends on the wanted type of picture
//

void create_picture(PPM *p)
{
  p->maxvalue = 255;
  if (p->key == P6) {
    p->picture = (unsigned char *) malloc (p->horizontal*p->vertical*3);
  } else if (p->key == P5) {
    p->picture = (unsigned char *) malloc (p->horizontal*p->vertical);
  } else {
    printf ("Unknown code: create picture\n");
    exit (1);
  }
}




void save_picture (PPM p)
{
  FILE *fp;

  fp = fopen (p.filename, "w");

  if (fp == NULL){
    printf ("problemas al abrir el archivo %s\n", p.filename);
    exit (1);
  }
  if (p.key == P6) {
    fprintf (fp, "P6\n"); 
    fprintf (fp,"%d %d\n", p.horizontal, p.vertical);
    fprintf (fp,"%d\n", p.maxvalue);
    fwrite (p.picture, 1, p.horizontal*p.vertical*3, fp);
  } else if (p.key == P5) {
    fprintf (fp, "P5\n"); 
    fprintf (fp,"%d %d\n", p.horizontal, p.vertical);
    fprintf (fp,"%d\n", p.maxvalue);
    fwrite (p.picture, 1, p.horizontal*p.vertical, fp);
  } else {
    printf ("Unknown code: save picture\n"); 
    exit (1);
  }
  fclose (fp);
}

/*
void put_pixel (PPM pic, PIXEL p)
{
  if (p.x >= pic.horizontal || p.y >= pic.vertical)
    return;
  if (pic.key == P6) {
    (unsigned char)*(pic.picture+p.y*(pic.horizontal*3)+p.x*3+0) = p.rojo; 
    (unsigned char)*(pic.picture+p.y*(pic.horizontal*3)+p.x*3+1) = p.verde;
    (unsigned char)*(pic.picture+p.y*(pic.horizontal*3)+p.x*3+2) = p.azul;
  } else if (pic.key == P5) {
    (unsigned char)*(pic.picture+p.y*(pic.horizontal*1)+p.x*1+0) = p.gris; 
  } else {
    printf ("Unknown code: put_pixel\n"); 
    exit (1);
  }
}
*/

void put_pixel (PPM pic, PIXEL p)
{
	int pos;
  if (p.x >= pic.horizontal || p.y >= pic.vertical)
    return;
  if (pic.key == P6) {
    //(unsigned char)*(pic.picture+p.y*(pic.horizontal*3)+p.x*3+0) = p.rojo; 
    //(unsigned char)*(pic.picture+p.y*(pic.horizontal*3)+p.x*3+1) = p.verde;
    //(unsigned char)*(pic.picture+p.y*(pic.horizontal*3)+p.x*3+2) = p.azul;
	pos = p.y*(pic.horizontal * 3) + p.x*3;
	pic.picture [pos] = p.rojo;
	pic.picture [pos+1] = p.verde;
	pic.picture [pos+2] = p.azul;
  } else if (pic.key == P5) {
//    (unsigned char)*(pic.picture+p.y*(pic.horizontal*1)+p.x*1+0) = p.gris; 
  } else {
    printf ("Unknown code: put_pixel\n"); 
    exit (1);
  }
}


void get_pixel (PPM pic, PIXEL *p)
{
  if (p->x >= pic.horizontal || p->y >= pic.vertical)
    return;
  if (pic.key == P6) {
    p->rojo  = *(pic.picture+p->y*(pic.horizontal*3)+p->x*3+0); 
    p->verde = *(pic.picture+p->y*(pic.horizontal*3)+p->x*3+1);
    p->azul  = *(pic.picture+p->y*(pic.horizontal*3)+p->x*3+2);
  } else if (pic.key == P5) {
    p->gris = *(pic.picture+p->y*(pic.horizontal*1)+p->x*1+0); 
  } else {
    printf ("Unknown code: get_pixel\n"); 
    exit (1);
  }
}



void put_line (PPM pic, PIXEL p1, PIXEL p2)
{
  double a, b, c;
  double y, yinc;
  double x, xinc;
  int i, n;

  a = (double)((int)p2.x - (int)p1.x);
  b = (double)((int)p2.y - (int)p1.y);

  c = sqrt (a*a + b*b); // hipotenusa

  n = abs((int) (c + 0.5)); // se redondea la hipotenusa y el quien define
                            // el numero de ciclos para dibujar la linea

  yinc = (double)((int)p2.y - (int)p1.y)/(double)n;
  xinc = (double)((int)p2.x - (int)p1.x)/(double)n;

  y = p1.y;
  x = p1.x;
 
  for (i = 0; i < n;i++) {
    p1.x = (int)(x+0.5); // El color del punto puede ser definido aqui
    p1.y = (int)(y+0.5); // o antes de llamar put_line en p1.
    put_pixel (pic, p1);
    y += yinc; 
    x += xinc; 
  }

}

/*
void curba (char rojo, char verde, char azul, gint *buffer, gint count, int xoff, int yoff)
{
  PIXEL p1, p2;
  int i;

  p1.rojo = rojo;
  p1.verde = verde;
  p1.azul = azul;

  for (i = 1;i < count;i++) {
    p1.x = (i-1)*zfctor + xoff;
    p1.y = buffer [i-1] + yoff;
    p2.x = i*zfctor + xoff;
    p2.y = buffer [i] + yoff;
    put_line (pic, p1, p2);
  }

}

*/

