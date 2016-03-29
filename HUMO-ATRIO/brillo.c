

#include <stdio.h>


#define OPEN   1
#define DATA   2
#define CLOSE  3

#define POSITIVO   1
#define NEGATIVO   2
#define CERO       3

typedef struct redline {
  FILE *fp;
  char buffer [1024];
  char *ptr;
}RDLINE;


int sign (int data);

void texit (int condition, char *message, char *arg, int ex);
char *rdline(RDLINE *f, char *file, char action);
double brillo (char *filename);

main (int argc, char *argv [])
{
  RDLINE file;
  char *ptr;


   system ("find . | grep blk > tmp.dat");
   rdline (&file, "tmp.dat", OPEN); 

   while (1) {
     
     if ((ptr = rdline (&file, NULL, DATA)) == NULL) break;

#ifdef DEBUG
     printf ("file: %s\n", ptr);
#endif
     printf ("%lf\n", brillo (ptr));

   }
   rdline (&file, NULL, CLOSE); 
}


double brillo (char *filename)
{
  RDLINE file;
  char *ptr;
  int i, cp1, cp2; // coeficiente pic 1, 2.

  rdline (&file, filename, OPEN); 

  ptr = rdline (&file, NULL, DATA);
  sscanf (ptr, "%d %d %d", &i, &cp1, &cp2);

  rdline (&file, NULL, CLOSE); 

  return cp2 - cp1;

}

#define POSITIVO   1
#define NEGATIVO   2
#define CERO       3

int sign (int data)
{
  if (data < 0)
    return NEGATIVO;
  else if (data > 0)
    return POSITIVO;
  else
    return CERO;
}




void texit (int condition, char *message, char *arg, int ex)
{
  if (condition) {
    if (arg == NULL)
      printf (message);
    else
      printf (message, arg);

    if (ex)
      exit (1);
  }
}


char *rdline(RDLINE *f, char *file, char action)
{
  switch (action) {

    case OPEN:
      f->fp = fopen (file, "r");
      texit (f->fp == NULL, "problemas para abrir %d", file, 1);
      break;

    case DATA:
      fgets (f->buffer, 1024, f->fp);
      f->ptr = f->buffer;
      f->ptr[strlen (f->ptr) - 1] = 0; // eliminamos '\n'
      if (feof (f->fp))
        return NULL;
      else
        return f->ptr;

      break;

    case CLOSE:
      fclose (f->fp);
      break;

    default:
      ;
  }

}
