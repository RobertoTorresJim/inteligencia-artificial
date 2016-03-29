
// Une premiere version du decodificateur a ete obtenue le 
// 12 de diciembre de 2004 vers 12:30.

// Une version bcp plus stable a ete obtenu le 30 decembre
// vers 1h15 du matin. Il semble que celle-ci est bien stable.
// Itzamatitlan, Morelos.


#define ROJASJPEG

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "bits.h"


#define ARCHIVO         1

#define SOF            0xC0  // Start Of Frame, Baseline DCT
#define SOI            0xD8  // Start Of Image
#define SOS            0xDA  // Start Of Scan
#define EOI            0xD9  // End Of Image
#define DHT            0xC4  // Define Huffman Tables
#define DQT            0xDB  // Define Quantization Tables
#define COM            0xFE  // Comment


#define MAX_COMP_SCAN     5  // 255 por norma, el 0 no se utiliza, 
			     // 3 para camaras AXIS 2100. 
                             //

#define MAX_NUM_BLOQUES   4  // Esto depende del factor de muestreo H y V
                             // asi como del numero de componentes en el 'scan'
                             // Para AXIS 2100 este es igual a 4

#define MAX_NUM_TAB_HUFF  4  // CCITT Rec. T.81; 1992 E;Pag 38, Parrafo 'Tdj:'
                             // Je crois que lorsque la norme dit que "le 
                             // maximum nombre des tables est egale a 4", elle
                             // etablie 0 <= Th <= 3. Cela veut dire qu'en
                             // realite une image JPEG contient 2*Th tableaux
                             // d'huffman. A verifier!?
#define TABLA_DC          0
#define TABLA_AC          1

//-----------------

#define DCTSIZE2          64

//------------------- Frame header ---------------------------


unsigned HVsum;

typedef struct frame {
  unsigned Lf; // Frame header length
  unsigned P;  // Sample precision
  unsigned Y;  // Number of lines
  unsigned X;  // Number of samples per line
  unsigned Nf; // Number of image components in frame
  unsigned C[MAX_COMP_SCAN];  // Component identifier
  unsigned H[MAX_COMP_SCAN];  // Horizontal sampling factor 
  unsigned V[MAX_COMP_SCAN];  // Vertical sampling factor
  unsigned Tq[MAX_COMP_SCAN]; // Quantization table destination selector

// ---------- Variables creadas para la implantacion ------------------
// ---------- del decodificador.                     ------------------
// --------------------------------------------------------------------
  unsigned char Hmax;
  unsigned char Vmax;
  int bloques_en_MCU;
  int DC_anterior[5];
  int bloq_pic;

} FRAME;


//------------------------ Scan header ---------------------------

typedef struct scan {
  unsigned Ls; // Start of scan marker
  unsigned Ns; // Number of image components in scan
  unsigned Cs[MAX_COMP_SCAN]; // Scan component selector 
  unsigned Td[MAX_COMP_SCAN]; // DC entropy coding table destination selector
  unsigned Ta[MAX_COMP_SCAN]; // AC entropy coding table destination selector
  unsigned Ss; // Start of spectral selection
  unsigned Se; // End of spectral selection
  unsigned Ah; // Successive aproximation bit position high 
  unsigned Al; // Successive aproximation bit position low or point transform 
} SCAN;



// Dos tablas de cuantificacion son necesarias. Una para los coeficientes 
// AC y otra para los DC. Esto explica porque los vectores de esta estructura 
// integran el numero dos

//---------------- Quantization Table -------------------------

typedef struct dqt { // define quantization tables
  unsigned Lq [2]; // Quantization table definition length
  unsigned Pq [2]; // Quantization table element precision
  unsigned Tq [2]; // Quantization table destination identifier
  unsigned short Q [2][64];// Quantization table element
  unsigned char N; // Numero of Quntization tables
} QT;

typedef struct comm {
  unsigned Lc;
  unsigned char *Cm; // Comment bytes. Aqui tendremos Lc - 2 Cm.
			       // Este espacio se reservara via 'malloc'.
}COMM;


//---------------------- huffman Table -------------------------

// Para la camara AXIS 2100 hay cuatro tablas de huffman en las figuras,
// dos para los coeficientes AC y dos para los coeficientes DC. En otras 
// imagenes JPEG puede variar el numero de tablas de huffman. 

#define HUFF_LOOKAHEAD 8

typedef struct huffman {
  unsigned Lh [MAX_NUM_TAB_HUFF];    // Huffman table definition length
  unsigned Tc [MAX_NUM_TAB_HUFF];    // Table class
  unsigned Th [MAX_NUM_TAB_HUFF];    // Huffman table destination identifer
  unsigned L [MAX_NUM_TAB_HUFF][17]; // Number of Huffman codes of length i; L1, L2, .. Ln
  unsigned char *v [MAX_NUM_TAB_HUFF]; // Value associated with each Huffman code
  unsigned N; // Numero de tablas de huffman encontradas.

// terminan las variables descritas en la norma van aqui estas variables ?? 
// por el momento si, 14 dic 2004. Se trata de las tablas derivadas de las 
// tablas de Huffman necesarias par efectuar el proceso de decodificacion. 

  int mincode [MAX_NUM_TAB_HUFF][17];            /* smallest code of length k */
  int maxcode[MAX_NUM_TAB_HUFF][18];            /* largest code of length k (-1 if none) */
/* (maxcode[17] is a sentinel to ensure huff_DECODE terminates) */
  int valptr[MAX_NUM_TAB_HUFF][17];               /* huffval[] index of 1st symbol of length k */

  int look_nbits  [MAX_NUM_TAB_HUFF][1 << HUFF_LOOKAHEAD];
  unsigned char look_sym [MAX_NUM_TAB_HUFF][1 << HUFF_LOOKAHEAD];


} HUFFMAN;



//----------------- estructuras no inspiradas en la norma ------------------

typedef struct bloque {
  int Cs;      //  Component identifier 'Ci' para el componente, pag 36 norma.
               //  El nombre 'Cs' quiza no sea el mas adecuado ya que 
               //  se confunde con 'Scan Component Selector'.

  int indx_ac; // Indice de la tabla de Huffman para los coeficientes AC
  int indx_dc; // Indice de la tabla de Huffman para los coeficientes DC
} BLOQUE;


typedef struct jpeg {
  COMM com;
  FRAME frame;
  SCAN scan;
  QT qt; // Quantization table
  HUFFMAN hufft;

//  ------- estructuras no inspiradas en los campos de la norma ------
  VSTREAM s; // Esta variable contiene todas las variables que interesan
	     // a la extraccion de bits del flujo binario. 

  BLOQUE bloque [MAX_NUM_BLOQUES]; // 

} JPEG;


typedef struct coef_dct_AXIS2100 {
  char Ya [DCTSIZE2]; // Eliminar 
  char Yb [DCTSIZE2]; // Eliminar 
  char Cr [DCTSIZE2]; // Eliminar
  char Cb [DCTSIZE2]; // Eliminar
  short coef [4][DCTSIZE2]; // Es suficiente un tipo 'short'?
} DCT_AXIS2100;


static const int ZAG[DCTSIZE2+16] = {
  0,  1,  8, 16,  9,  2,  3, 10,
 17, 24, 32, 25, 18, 11,  4,  5,
 12, 19, 26, 33, 40, 48, 41, 34,
 27, 20, 13,  6,  7, 14, 21, 28,
 35, 42, 49, 56, 57, 50, 43, 36,
 29, 22, 15, 23, 30, 37, 44, 51,
 58, 59, 52, 45, 38, 31, 39, 46,
 53, 60, 61, 54, 47, 55, 62, 63,
  0,  0,  0,  0,  0,  0,  0,  0, /* extra entries in case k>63 below */
  0,  0,  0,  0,  0,  0,  0,  0
};


#ifdef MAIN
unsigned char dezigzag [] = { 0,  1,  5,  6, 14, 15, 27, 28,
                              2,  4,  7, 13, 16, 26, 29, 42,
                              3,  8, 12, 17, 25, 30, 41, 43,
                              9, 11, 18, 24, 31, 40, 44, 53,
                             10, 19, 23, 32, 39, 45, 52, 54,
                             20, 22, 33, 38, 46, 51, 55, 60,
                             21, 34, 37, 47, 50, 56, 59, 61,
                             35, 36, 48, 49, 57, 58, 62, 63};
#endif


#define huff_EXTEND(x,s)  ((x) < (1<<((s)-1)) ? (x) + (((-1)<<(s)) + 1) : (x))


void toma_encabezado (JPEG *pic);
void base_line_DCT (JPEG *pic);
void Huffman_Tables (JPEG *pic);
void Start_Of_Scan (JPEG *pic);
void Define_Quantization_Tables (JPEG *pic);
void comment (JPEG *pic);
void Init_Decode (JPEG *pic, char *archivo);
void imprime_bloque (short *coef, int tam, char *title);


int slow_DECODE (int indx, int min_bits, JPEG *pic);
void MCUinit (JPEG *pic);
void End_Of_Image ();
void Start_Of_Image ();

DCT_AXIS2100 *get_MCU_from_JPEG (char *file, int *n_mcu, int *h, int *v);
short *get_Y_block (DCT_AXIS2100  *mcu_lst, int i);

