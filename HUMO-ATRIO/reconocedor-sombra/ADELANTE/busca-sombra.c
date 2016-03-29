#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "jpeg.h"
#include "ltqnorm.h"
#include "ppm.h"

#define INTERVAL   0.999

#define LASTMARK   1

typedef struct gmm {
int numcomp; // numero de componentes normales
double pi[2]; // peso 
double mu[2]; // media
double sigma[2]; // desviacion estandard

double LimInf [2]; // Limite inferior de confiabilidad
double LimSup [2]; // Limite superios de confiabilidad
} GMM;


int sign (int data);
double brillo (short blk1 [], short blk2 []);
double amplifi (short blk1 [], short blk2 []);
double atenua (short blk1 [], short blk2 []);
double igual (short blk1 [], short blk2 []);
double signx (short blk1 [], short blk2 []);
void def_interval (GMM *gmm, double interval);
void print_interval (GMM gmm, char *t);
int conform (GMM gmm, double ind);
void load_model (GMM *signx, GMM *brillo, GMM *igual, GMM *amplif, GMM *atenua);

void put_mark (char *file, int blk, int action);

main (int argc, char *argv [])
{
  GMM gmm_atenua;
  GMM gmm_amplif;
  GMM gmm_brillo;
  GMM gmm_igual;
  GMM gmm_signx;
  PPM picbin; //creacion de imagen binaria
  int m,n;

 // resultados de la verificacion de conformidad de un dato puntual con el modelo
  int r_brillo, r_atenu, r_ampli, r_igual, r_signx; 

  int i, j, n_mcu; 
// MCU: Minimum Coding Unit. Un MCU contiene bloques de luminancia y de color
  DCT_AXIS2100 *MCUvect1, *MCUvect2;
  short *blkp1, *blkp2; // apuntador aun bloque de coeficientes DCT

// variable indices de las diferentes caracteristicas extraidas de un par de
// bloques de la DCT
  double indi_atenua, indi_igual, indi_amplifi, indi_signx, indi_brillo;

  if (argc != 3) {
    printf ("se requiere: %s <pic 1>.jpg <pic 2>.jpg\n", argv [0]);
    exit (1);
  }


// carga los datos de los modelos
  load_model(&gmm_signx, &gmm_brillo, &gmm_igual, &gmm_amplif, &gmm_atenua);

// Descomprime una imagen JPEG y entrega un vector de MCU, los cuales contienen
// los bloques de coeficientes DCT.
  MCUvect1 = get_MCU_from_JPEG (argv [1], &n_mcu, &m, &n);
  MCUvect2 = get_MCU_from_JPEG (argv [2], &n_mcu, &m, &n);
  
  picbin.horizontal = m/8;//cambia el tamaño de la imagen binaria
  picbin.vertical = n/8;//cambia el tamalo de la imagen bianria

  picbin.key = BYN;
  picbin.filename = "picbin.ppm";
  create_picture (&picbin);

  memset(picbin.picture,0xff,picbin.horizontal*picbin.vertical);

  
  for (i = 0; i < n_mcu;i++) { 

// extre del vector de MCUs un bloque de luminancia Y.
    blkp1 = get_Y_block (MCUvect1, i);
    blkp2 = get_Y_block (MCUvect2, i);

// calcula los indices de cambios de brillo, atenuacion, igual ... para el par
// de bloques blkp1/blkp2
    indi_brillo  = brillo (blkp1, blkp2);
    indi_atenua  = atenua (blkp1, blkp2);
    indi_igual   = igual (blkp1, blkp2);
    indi_amplifi = amplifi (blkp1, blkp2);
    indi_signx   = signx (blkp1, blkp2);

// Verifica que los indices calculados conformen su correspondiente modelo
// el resultado de esta verificacion es, o FALSO (0), o VERDADERO (1).
    r_brillo = conform (gmm_brillo, indi_brillo);
    r_atenu = conform (gmm_atenua, indi_atenua);
    r_ampli = conform (gmm_amplif, indi_amplifi);
    r_igual = conform (gmm_igual, indi_igual);
    r_signx = conform (gmm_signx, indi_signx);

// Si los 5 indices conforman sus correspondientes modelos, podemos decir
// que hemos encontrado un bloque ligado a la presencia de una sombra
// en movimiento.

    if (r_brillo && r_atenu && r_ampli && r_igual && r_signx)
      put_mark (argv [2], i, 0); // marca el i-esimo bloque de la imagen 
                                 // contenida en el archivo 'argv [2]'

      picbin.picture[i] =0x00;
  }
  put_mark (argv [2], i, LASTMARK); // indica que el la imagen marcada debe
                                    // ser escrita en un archivo llamado
                                    // 'pic.
  save_picture(picbin);
}



void load_model (GMM *signx, GMM *brillo, GMM *igual, GMM *amplif, GMM *atenua)
{

///// modelo para cambios de signo ////////////////////

  signx->numcomp = 2;

  signx->pi[0]= 0.5108;
  signx->mu[0]= 0.9;
  signx->sigma[0]= 0.3219;

  signx->pi[1]= 0.4892; 
  signx->mu[1]= 1.612;
  signx->sigma[1]= 0.5309;

  def_interval (signx, INTERVAL);

///// modelo para cambios de brillo ////////////////////

  brillo->numcomp = 2;

  brillo->pi[0]= 0.365;
  brillo->mu[0]= -242.0;
  brillo->sigma[0]= 4.36;

  brillo->pi[1]= 0.635; 
  brillo->mu[1]= -237.2;
  brillo->sigma[1]= 12.53;

  def_interval (brillo, INTERVAL);

///// modelo indice de coeficientes iguales /////////////

  igual->numcomp = 2;

  igual->pi[0]= 0.601;
  igual->mu[0]= 0.25;
  igual->sigma[0]= 0.05701;

  igual->pi[1]= 0.399;
  igual->mu[1]= 0.3206;
  igual->sigma[1]= 0.04996;

  def_interval (igual, INTERVAL);

///// modelo indice de amplificacion /////////////

  amplif->numcomp = 2;

  amplif->pi[0]= 0.6789;
  amplif->mu[0]= 0.8;
  amplif->sigma[0]= 0.2863;

  amplif->pi[1]= 0.3211;
  amplif->mu[1]= 1.42;
  amplif->sigma[1]= 0.3558;

  def_interval (amplif, INTERVAL);

///// modelo indice de atenuacion /////////////

  atenua->numcomp = 2;

  atenua->pi[0]= 0.4489;
  atenua->mu[0]= -1.8;
  atenua->sigma[0]= 0.3181;

  atenua->pi[1]= 0.5511;
  atenua->mu[1]= -1.261;
  atenua->sigma[1]= 0.2599;

  def_interval (atenua, INTERVAL);

}

int conform (GMM gmm, double ind)
{
  int i, res;
 
  res = 0; 
  for (i = 0;i < gmm.numcomp;i++)
    if (gmm.LimInf[i] <= ind && ind <= gmm.LimSup [i])
      res++;
  return (!!res);
}

void def_interval (GMM *gmm, double interval)
{
  int i;

  for (i = 0;i < gmm->numcomp;i++) {
    gmm->LimInf[i] = gmm->mu[i] + gmm->sigma [i]*ltqnorm (0.5 - interval/2.0);
    gmm->LimSup[i] = gmm->mu[i] + gmm->sigma [i]*ltqnorm (0.5 + interval/2.0);
  }

}

double brillo (short blk1 [], short blk2 [])
{ // Solo para ilustrar
  return blk2 [0] - blk1 [0];
}


double signx (short blk1 [], short blk2 [])
{
  int i;
  int util;
  int sum;

   for (util = 0, i = 1;i < 64;i++)
      if (blk1[i] != 0 && blk2[i] != 0)
        util++;

   sum = 0;
   for (i = 1;i < 64;i++)
     if (sign (blk1[i]) != sign (blk2[i]) && blk1[i] != 0 && blk2[i] != 0)
       sum += abs (blk1 [i]) + abs (blk2 [i]);

  return (double)sum/(double)util;

}

double amplifi (short blk1 [], short blk2 [])
{
  int i;
  int util;
  int sum;

  for (util = 0, i = 1;i < 64;i++)
    if (blk1[i] != 0 && blk2[i] != 0)
      util++;

  sum = 0;
  for (i = 1;i < 64;i++)
    if (sign (blk1[i]) == sign (blk2[i]) && abs (blk1[i]) < abs (blk2[i]) && blk1[i] != 0 && blk2[i] != 0)  {
      sum += abs (blk2[i]) - abs (blk1[i]);
    }

  return (double)sum/(double)util;
}

double igual  (short blk1 [], short blk2 [])
{
  int i;
  int util;
  int sum;

  util = 0;
  for (i = 1;i < 64;i++)
    if (blk1[i] != 0 && blk2[i] != 0)
      util++;

  sum = 0;
  for (i = 1;i < 64;i++)
    if (blk1[i] == blk2[i] && blk1 [i] != 0 && blk2[i] != 0)
      sum += 1;

  return (double)sum/(double)util;

}


double atenua (short blk1 [], short blk2 [])
{
  int i;
  int util;
  int sum;

  for (util = 0, i = 1;i < 64;i++)
    if (blk1[i] != 0 && blk2[i] != 0)
      util++;

  sum = 0;
  for (i = 1;i < 64;i++)
    if (sign (blk1[i]) == sign (blk2[i]) && abs (blk1[i]) > abs (blk2[i]) && blk1[i] != 0 && blk2[i] != 0)  {
      sum += abs (blk2[i]) - abs (blk1[i]);
    }

  return (double)sum/(double)util;
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


