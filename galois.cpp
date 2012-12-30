/*****************************
 * 
 *
 * Multiplication and Arithmetic on Galois Field GF(256)
 *
 * From Mee, Daniel, "Magnetic Recording, Volume III", Ch. 5 by Patel.
 * 
 * (c) 1991 Henry Minsky
 * Modified 2011 by Pierre Abbat for 5 bits
 *
 ******************************/
 
 
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "galois.h"

/* This is an irreducible polynomial
 * of degree 5 and cycle length 31.
 * The high order 1 bit is implicit */
/* x^5 + x^2 + 1 */
#define PPOLY 0x05 


int gexp[64];
int glog[32];
int gmultab[32][32];
int ginvtab[32];


static void init_exp_table (void);

void init_mul_table()
{
  int i,j;
  memset(gmultab,0,sizeof(gmultab));
  for (i=1;i<32;i++)
    for (j=1;j<32;j++)
      gmultab[i][j]=gexp[glog[i]+glog[j]];
}

void init_inv_table()
{
  int i;
  ginvtab[0]=0; // 1/0 is undefined, but this is used in the whitening code
  for (i=1;i<32;i++)
    ginvtab[i]=gexp[31-glog[i]];
}

void
init_galois_tables (void)
{	
  /* initialize the table of powers of alpha */
  init_exp_table();
  init_mul_table();
  init_inv_table();
}


static void
init_exp_table (void)
{
  int i, z;
  int pinit,p1,p2,p3,p4,p5,p6,p7,p8;

  pinit = p2 = p3 = p4 = p5 = p6 = p7 = p8 = 0;
  p1 = 1;
	
  gexp[0] = 1;
  gexp[31] = gexp[0];
  glog[0] = 0;			/* shouldn't log[0] be an error? */
	
  for (i = 1; i < 32; i++) {
    pinit = p5;
    p5 = p4;
    p4 = p3;
    p3 = p2 ^ pinit;
    p2 = p1;
    p1 = pinit;
    gexp[i] = p1 + p2*2 + p3*4 + p4*8 + p5*16;
    gexp[i+31] = gexp[i];
  }
	
  for (i = 1; i < 32; i++) {
    for (z = 0; z < 32; z++) {
      if (gexp[z] == i) {
	glog[i] = z;
	break;
      }
    }
  }
}

int gmult(int a, int b)
{
  return (gmultab[a][b]);
}
		

int ginv (int elt) 
{ 
  return (ginvtab[elt]);
}

