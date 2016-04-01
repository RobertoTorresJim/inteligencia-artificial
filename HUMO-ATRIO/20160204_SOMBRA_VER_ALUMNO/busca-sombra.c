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
//double igual (short blk1 [], short blk2 []);
double signx (short blk1 [], short blk2 []);
double perdida (short blk1 [], short blk2 []);
double ganancia (short blk1 [], short blk2 []);
void def_interval (GMM *gmm, double interval);
void print_interval (GMM gmm, char *t);
int conform (GMM gmm, double ind);
void load_model (GMM *signx, GMM *brillo, GMM *amplif, GMM *atenua, GMM *perdida, GMM *ganancia);
int erosion(PPM picbin, int i);


void put_mark (char *file, int blk, int action);
main (int argc, char *argv [])
{
  GMM gmm_atenua;
  GMM gmm_amplif;
  GMM gmm_brillo;
 // GMM gmm_igual;
  GMM gmm_signx;
  GMM gmm_perdida;
  GMM gmm_ganancia;
  PPM picbin;
  int m,n;

 // resultados de la verificacion de conformidad de un dato puntual con el modelo
  int r_brillo, r_atenu, r_ampli, r_igual, r_signx, r_perdida, r_ganancia;

  int i, j, n_mcu;
// MCU: Minimum Coding Unit. Un MCU contiene bloques de luminancia y de color
  DCT_AXIS2100 *MCUvect1, *MCUvect2;
  short *blkp1, *blkp2; // apuntador aun bloque de coeficientes DCT

// variable indices de las diferentes caracteristicas extraidas de un par de
// bloques de la DCT
  double indi_atenua, indi_igual, indi_amplifi, indi_signx, indi_brillo, indi_perdida, indi_ganancia;

  if (argc != 3) {
    printf ("se requiere: %s <pic 1>.jpg <pic 2>.jpg\n", argv [0]);
    exit (1);
  }
// carga los datos de los modelos
  load_model(&gmm_signx, &gmm_brillo, &gmm_amplif, &gmm_atenua, &gmm_perdida, &gmm_ganancia);
// Descomprime una imagen JPEG y entrega un vector de MCU, los cuales contienen
// los bloques de coeficientes DCT.
  MCUvect1 = get_MCU_from_JPEG (argv [1], &n_mcu );
  MCUvect2 = get_MCU_from_JPEG (argv [2], &n_mcu );

   picbin.horizontal = 1920/8;
   picbin.vertical = 1080/8;//cambia el tamalo de la imagen bianria

   picbin.key = BYN;
   picbin.filename = "picbin.ppm";
   create_picture (&picbin);

   memset(picbin.picture,0xff,picbin.horizontal*picbin.vertical);

  for (i = 0; i < n_mcu; i++) {

// extre del vector de MCUs el i-esimo bloque de luminancia Y.
    blkp1 = get_Y_block (MCUvect1, i);
    blkp2 = get_Y_block (MCUvect2, i);

// calcula los indices de cambios de brillo, atenuacion, igual ... para el par
// de bloques blkp1/blkp2
    indi_brillo   = brillo (blkp1, blkp2);
    indi_atenua   = atenua (blkp1, blkp2);
//    indi_igual   = igual (blkp1, blkp2);
    indi_amplifi  = amplifi (blkp1, blkp2);
    indi_signx    = signx (blkp1, blkp2);
    indi_perdida  = perdida(blkp1, blkp2);
    indi_ganancia = ganancia(blkp1, blkp2);

// Verifica que los indices calculados conformen su correspondiente modelo
// el resultado de esta verificacion es, o FALSO (0), o VERDADERO (1).
    r_brillo = conform (gmm_brillo, indi_brillo);
    r_atenu = conform (gmm_atenua, indi_atenua);
    r_ampli = conform (gmm_amplif, indi_amplifi);
//    r_igual = conform (gmm_igual, indi_igual);
    r_signx = conform (gmm_signx, indi_signx);
    r_perdida = conform (gmm_perdida, indi_perdida);
    r_ganancia = conform (gmm_ganancia, indi_ganancia);

// Si los 5 indices conforman sus correspondientes modelos, podemos decir
// que hemos encontrado un bloque ligado a la presencia de una sombra
// en movimiento.

    if((indi_perdida>indi_ganancia)){
       if(indi_perdida<indi_brillo){
            if (r_brillo && r_atenu && r_ampli && r_signx/* && r_perdida && r_ganancia*/){
            picbin.picture[i] =0x00; //creacion de la pic bianria
            if(erosion(picbin,i)){
              printf("%d",erosion(picbin,i));
              put_mark (argv [2], i, 0); // marca el i-esimo bloque de la imagen
          }
        }                                  // contenida en el archivo 'argv [2]'
      }
    }
  }//termina for
  put_mark (argv [2], i, LASTMARK); // indica que el la imagen marcada debe
                                    // ser escrita en un archivo llamado
                                    // 'pic.pnm'
	save_picture(picbin);
}// termina main

int erosion(PPM picbin, int i){
  int bandera = 0;
  int validaIzq = 0;
  int validaDer = 0;
  int j, k;
    if(picbin.picture[i] == 0x00){
      for (j = 0; j < 32160;){

        if(i == j){
          validaIzq = 1;
        }// valida los bordes
        j = j+240;
      }
        for(k = 0; k < 32399;){

          if(i == k){
            validaDer = 1;
          }
          k = k+240;
        }
      if(i-1 > 0 && validaIzq == 0){
        if(picbin.picture[i-1] == 0x00){
          bandera = bandera + 1;
        }
      }
      if(i-241 > 0 && validaIzq == 0){
        if(picbin.picture[i-241] == 0x00){
          bandera = bandera + 1;
        }
      }
      if(i-240 > 0){
        if(picbin.picture[i-240] == 0){
          bandera = bandera + 1;
        }
      }
      if(i-239 > 0 && validaDer == 0){
        if(picbin.picture[i-239] == 0x00){
          bandera = bandera + 1;
        }
      }
      if(i+1 < 32400 && validaDer == 0){
        if(picbin.picture[i+1] == 0x00){
          bandera = bandera + 1;
        }
      }
      if(i+239 < 32400 && validaIzq == 0){
        if(picbin.picture[i+239] == 0x00){
          bandera = bandera + 1;
        }
      }
      if(i+240 < 32400){
        if(picbin.picture[i+240] == 0x00){
          bandera = bandera + 1;
        }
      }
      if(i+241 < 32400 && validaDer == 0){
        if(picbin.picture[i+241] == 0x00){
          bandera = bandera + 1;
        }
      }
    validaDer = 0;
    validaIzq = 0;
  }
  if (bandera > 3)
    return 1;
  else return 0;
}

void load_model (GMM *signx, GMM *brillo, GMM *amplif, GMM *atenua, GMM *perdida, GMM *ganancia)
{
///// modelo para cambios de signo ////////////////////
  signx->numcomp = 2;

  signx->pi[0]= 0.3524;
  signx->mu[0]= 1.0;
  signx->sigma[0]= 0.2781;

  signx->pi[1]= 0.6476;
  signx->mu[1]= 1.809;
  signx->sigma[1]= 0.7004;

  def_interval (signx, INTERVAL);
///// modelo para cambios de brillo ////////////////////
  brillo->numcomp = 2;

  brillo->pi[0]= 0.7251;
  brillo->mu[0]= -140.0;
  brillo->sigma[0]= 85.99;

  brillo->pi[1]= 0.2749;
  brillo->mu[1]= 120.0;
  brillo->sigma[1]= 76.39;

  def_interval (brillo, INTERVAL);
///// modelo indice de coeficientes iguales /////////////
/*
  igual->numcomp = 2;

  igual->pi[0]= 0.601;
  igual->mu[0]= 0.25;
  igual->sigma[0]= 0.05701;

  igual->pi[1]= 0.399;
  igual->mu[1]= 0.3206;
  igual->sigma[1]= 0.04996;

  def_interval (igual, INTERVAL);
*/
///// modelo indice de amplificacion /////////////

  amplif->numcomp = 2;

  amplif->pi[0]= 0.6262;
  amplif->mu[0]= 0.800;
  amplif->sigma[0]= 0.4524;

  amplif->pi[1]= 0.3738;
  amplif->mu[1]= 1.907;
  amplif->sigma[1]= 0.8355;

  def_interval (amplif, INTERVAL);

///// modelo indice de atenuacion /////////////

  atenua->numcomp = 2;

  atenua->pi[0]= 0.2722;
  atenua->mu[0]= -1.5000;
  atenua->sigma[0]= 1.0579;

  atenua->pi[1]= 0.7278;
  atenua->mu[1]= -0.6142;
  atenua->sigma[1]= 0.5075;

  def_interval (atenua, INTERVAL);

//// modelo indice de perdida ////////////////

  perdida->numcomp = 2;

  perdida->pi[0]= 0.6249;
  perdida->mu[0]= 0.2500;
  perdida->sigma[0]= 0.1141;

  perdida->pi[1]= 0.3751;
  perdida->mu[1]= 0.5363;
  perdida->sigma[1]= 0.2567;

  def_interval (perdida, INTERVAL);

//// modelo indice de ganancia ////////////

  ganancia->numcomp =2;

  ganancia->pi[0]= 0.91121;
  ganancia->mu[0]= 0.4000;
  ganancia->sigma[0]= 0.1883;

  ganancia->pi[1]= 0.08879;
  ganancia->mu[1]= 0.9704;
  ganancia->sigma[1]= 0.2255;

  def_interval (ganancia, INTERVAL);

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
{//solo para ilustrar
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

/*double igual  (short blk1 [], short blk2 [])
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

}*/


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




double perdida (short blk1 [], short blk2 [])
{
  int i;
  int util;
  int sum;

  for (util = 0, i = 1; i < 64; i++)
    if (blk1[i] != 0 && blk2[i] != 0)
      util++;
  sum = 0;
  for (i = 1; i < 64; i++)
    if(blk1[i] != 0 && blk2[i] == 0){
      sum += abs (blk1[i]);
      }
  return (double)sum/(double)util;
}

double ganancia (short blk1 [], short blk2 [])
{
  int i;
  int util;
  int sum;

  for (util = 0, i = 1; i < 64; i++)
    if (blk1[i] != 0 && blk2[i] != 0)
          util++;
    sum = 0;
    for (i = 1; i < 64; i++)
       if(blk1[i] == 0 && blk2[i] != 0){
          sum += abs (blk2[i]);
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
