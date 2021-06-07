/******************************************************/
/*                                                    */
/* genetic.cpp - genetic algorithm                    */
/*                                                    */
/******************************************************/
/* Copyright 2021 Pierre Abbat.
 * This file is part of Propolis.
 * 
 * The Propolis program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Propolis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License and Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Propolis. If not, see <http://www.gnu.org/licenses/>.
 */
/* A genome consists of a single gene, which is a permutation of 32 12-bit
 * patterns with 0x000 held fixed to @. The fitness is the sum of square
 * roots of bit difference counts. A mutation consists of swapping bit patterns.
 * Children are produced by taking some bit patterns from the mother and some
 * from the father in such a way that the child has 32 different bit patterns.
 */
#include "genetic.h"
#include "random.h"
using namespace std;

LetterMap::LetterMap()
{
  int i;
  fit=NAN;
  for (i=0;i<32;i++)
    bitPatterns[i]=letters[i];
}

LetterMap::LetterMap(array<BIT16,5> init)
/* Initializes a LetterMap with 80 bits, which should be chosen at random.
 * 31!=8.2e33; 2**80=1.2e24. This doesn't completely shuffle the letters,
 * but it should be good enough to start breeding.
 */
{
  int i,j,k,l;
  array<BIT16,32> temp;
  fit=NAN;
  for (i=0;i<32;i++)
    bitPatterns[i]=letters[i];
  for (i=0;i<5;i++)
  {
    temp[0]=bitPatterns[0];
    temp[16]=bitPatterns[16];
    for (j=1;j<16;j++)
      if ((init[i]>>j)&1)
      {
	temp[j]=bitPatterns[30-j];
	temp[30-j]=bitPatterns[j];
      }
      else
      {
	temp[j]=bitPatterns[j];
	temp[30-j]=bitPatterns[30-j];
      }
    for (j=0;j<32;j++)
    {
      k=(j+11*(init[i]&1))%32;
      l=j*2-31*(j>15);
      bitPatterns[l]=temp[k];
    }
  }
}

LetterMap::LetterMap(LetterMap &mother,LetterMap &father)
{
  int i,j;
  unsigned rand;
  array<char,32> cperm,cycles;
  array<char,4096> bp;
  fit=NAN;
  // Compute the mother-inverse-father permutation
  for (i=0;i<32;i++)
    bp[mother.bitPatterns[i]]=i;
  for (i=0;i<32;i++)
    cperm[bp[i]]=father.bitPatterns[i];
  // Find cycles
  for (i=0;i<32;i++)
    cycles[i]=64;
  for (i=0;i<32;i++)
  {
    j=i;
    while (cycles[j]==64)
    {
      cycles[j]=i;
      j=cperm[j];
    }
  }
  // Copy each cycle from the mother or the father, at random
  rand=rng.uirandom();
  for (i=0;i<32;i++)
    if ((rand>>cycles[i])&1)
      bitPatterns[i]=mother.bitPatterns[i];
    else
      bitPatterns[i]=father.bitPatterns[i];
}

void LetterMap::mutate()
{
  int r=rng.usrandom();
  int a=r&31,b=(r>>5)&31,c=(r>>10)&31;
  int toIgnore=-1;
  if (a==b || a==0)
    toIgnore=0;
  if (b==c || b==0)
    toIgnore=1;
  if (c==a || c==0)
    toIgnore=2;
  switch (toIgnore)
  {
    case 0:
      swap(bitPatterns[b],bitPatterns[c]);
      break;
    case 1:
      swap(bitPatterns[c],bitPatterns[a]);
      break;
    case 2:
      swap(bitPatterns[a],bitPatterns[b]);
      break;
    default:
      swap(bitPatterns[a],bitPatterns[b]);
      swap(bitPatterns[b],bitPatterns[c]);
  }
}
