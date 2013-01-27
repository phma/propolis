/* 
 * Reed Solomon Encoder/Decoder 
 *
 * (c) Henry Minsky (hqm@ua.com), Universal Access 1991-1995
 */

#include <stdio.h>
#include <ctype.h>
#include "galois.h"
#include "rs.h"

/* Encoder parity bytes */
int pBytes[31];

/* Decoder syndrome bytes */
int synBytes[31];

/* generator polynomial */
int genPoly[31][32];

bool DEBUG = false;

static void
compute_genpoly (int genpoly[][32]);

/* Initialize lookup tables, polynomials, etc. */
void
initialize_ecc ()
{
  /* Initialize the galois field arithmetic tables */
    init_galois_tables();

    /* Compute the encoder generator polynomial */
    compute_genpoly(genPoly);
}

void
zero_fill_from (unsigned char buf[], int from, int to)
{
  int i;
  for (i = from; i < to; i++) buf[i] = 0;
}
/********** polynomial arithmetic *******************/

void add_polys (int dst[], int src[]) 
{
  int i;
  for (i = 0; i < 31; i++) dst[i] ^= src[i];
}

void copy_poly (int dst[], int src[]) 
{
  int i;
  for (i = 0; i < 31; i++) dst[i] = src[i];
}

void scale_poly (int k, int poly[]) 
{	
  int i;
  for (i = 0; i < 31; i++) poly[i] = gmult(k, poly[i]);
}


void zero_poly (int poly[]) 
{
  int i;
  for (i = 0; i < 32; i++) poly[i] = 0;
}


/* multiply by z, i.e., shift right by 1 */
static void mul_z_poly (int src[])
{
  int i;
  for (i = 32-1; i > 0; i--) src[i] = src[i-1];
  src[0] = 0;
}

/* polynomial multiplication */
void
mult_polys (int dst[], int p1[], int p2[])
{
  int i, j;
  int tmp1[64];
	
  for (i=0; i < (64); i++) dst[i] = 0;
	
  for (i = 0; i < 32; i++) {
    for(j=32; j<(64); j++) tmp1[j]=0;
		
    /* scale tmp1 by p1[i] */
    for(j=0; j<32; j++) tmp1[j]=gmult(p2[j], p1[i]);
    /* and mult (shift) tmp1 right by i */
    for (j = (64)-1; j >= i; j--) tmp1[j] = tmp1[j-i];
    for (j = 0; j < i; j++) tmp1[j] = 0;
		
    /* add into partial product */
    for(j=0; j < (64); j++) dst[j] ^= tmp1[j];
  }
}



/* debugging routines */
void
print_parity (void)
{ 
  int i;
  printf("Parity Bytes: ");
  for (i = 0; i < 4/*FIXME*/; i++) 
    printf("[%d]:%x, ",i,pBytes[i]);
  printf("\n");
}


void
print_syndrome (void)
{ 
  int i;
  printf("Syndrome Bytes: ");
  for (i = 0; i < 4/*FIXME*/; i++) 
    printf("[%d]:%x, ",i,synBytes[i]);
  printf("\n");
}

/* Append the parity bytes onto the end of the message */
void
build_codeword (char msg[], int nbytes, char dst[])
{
  int i;
  int npar=31-nbytes;
	
  for (i = 0; i < nbytes; i++) dst[i] = msg[i];
	
  for (i = 0; i < npar; i++) {
    dst[i+nbytes] = pBytes[npar-1-i]|'`';
  }
}
	
/**********************************************************
 * Reed Solomon Decoder 
 *
 * Computes the syndrome of a codeword. Puts the results
 * into the synBytes[] array.
 */
 
/*void
decode_data(unsigned char data[], int nbytes)
{
  int i, j, sum;
  for (j = 0; j < NPAR;  j++) {
    sum	= 0;
    for (i = 0; i < nbytes; i++) {
      sum = data[i] ^ gmult(gexp[j+1], sum);
    }
    synBytes[j]  = sum;
  }
}*/


/* Check if the syndrome is zero */
/*int
check_syndrome (void)
{
  int i, nz = 0;
  for (i =0 ; i < NPAR; i++) {
    if (synBytes[i] != 0) nz = 1;
  }
  return nz;
}*/


void
debug_check_syndrome (void)
{	
  int i;
	
  for (i = 0; i < 3; i++) {
    printf(" inv log S[%d]/S[%d] = %d\n", i, i+1, 
	   glog[gmult(synBytes[i], ginv(synBytes[i+1]))]);
  }
}


/* Create a generator polynomial for an n byte RS code. 
 * The coefficients are returned in the genPoly arg.
 * Make sure that the genPoly array which is passed in is 
 * at least n+1 bytes long.
 */

static void
compute_genpoly (int genpoly[][32])
{
  int i, tp[32], tp1[256];
	
  /* multiply (x + a^n) for n = 1 to nbytes */

  zero_poly(genpoly[0]);
  genpoly[0][0] = 1;
  zero_poly(tp);
  tp[1] = 1;

  for (i = 1; i <= 31; i++) {
    tp[0]=gexp[i];		/* set up x+a^n */
    mult_polys(genpoly[i], tp, genpoly[i-1]);
  }
}

/* Simulate a LFSR with generator polynomial for n byte RS code. 
 * Pass in a pointer to the data array, and amount of data. 
 *
 * The parity bytes are deposited into pBytes[], and the whole message
 * and parity are copied to dest to make a codeword.
 * 
 */

void
encode_data (char msg[], int nbytes, char dst[])
{
  int i, LFSR[31],dbyte, j,NPAR;
  NPAR=31-nbytes;
	
  for(i=0; i < 31; i++) LFSR[i]=0;

  for (i = 0; i < nbytes; i++) {
    dbyte = (msg[i]&31) ^ LFSR[NPAR-1];
    for (j = NPAR-1; j > 0; j--) {
      LFSR[j] = LFSR[j-1] ^ gmult(genPoly[NPAR-1][j], dbyte);
    }
    LFSR[0] = gmult(genPoly[NPAR-1][0], dbyte);
  }

  for (i = 0; i < NPAR; i++) 
    pBytes[i] = LFSR[i];
	
  build_codeword(msg, nbytes, dst);
}

