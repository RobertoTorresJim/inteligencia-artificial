

#include <stdio.h>
#include <unistd.h>

#define u_int	unsigned int
#define u_short	unsigned short
#define u_char 	unsigned char

#define INPUTFILE		1

#define MAXFILEBUF	        1024  // se recomienda longitudes de buffer
                                      // multiplos de 256. Hay que verificar
                                      // la longitud optima para Linux que
                                      // permita alcanzar el maximo desempe~o.

#define MAXMAINBUF              MAXFILEBUF/4 // 2 mas por que el indice 
                                                 // s->i se sale del arreglo
                                                 // bf[] en una posicion.
 
typedef struct vstream {
   u_char marcador;
   u_int bf[MAXMAINBUF];	/* buffer */
   u_char b;			/* next word */
   u_int i; 			/* index */
   u_int W; 			/* buffer word */
   u_int W1; 			/* buffer word */
   u_int Bsize; 		/* buffer size */
   FILE *fp;		
} VSTREAM;


#define nextbits(x)		(pic->s.W >> (32 - (x)))
#define getbits(x)		bits((x),&(pic->s))
#define bytealigned()		bits((s->b % 8),s)


unsigned int cambia (unsigned int data);
void InitFileBuff (char *filename, VSTREAM *s);
void recarga_de_buffer (VSTREAM *s);
unsigned int bits (int n, VSTREAM *s);
void dump (unsigned char *b, int s);
int elimina_ff00 (unsigned char *buffer, int longitud, VSTREAM *s);

