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
#include <vector>
#include <iostream>
#include "genetic.h"
#include "random.h"
#include "threads.h"
using namespace std;
namespace cr=std::chrono;

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
	temp[j]=bitPatterns[32-j];
	temp[32-j]=bitPatterns[j];
      }
      else
      {
	temp[j]=bitPatterns[j];
	temp[32-j]=bitPatterns[32-j];
      }
    for (j=0;j<32;j++)
    {
      k=j?(j+10*(init[i]&1))%31+1:0;
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

void LetterMap::computeFitness()
{
  int i,j;
  fit=0;
  for (i=0;i<32;i++)
    for (j=0;j<i;j++)
      switch (bitcount(bitPatterns[i]^bitPatterns[j]))
      {
	case 2:
	  fit+=sqrt(bitcount(i^j));
	  break;
	case 3:
	  fit+=sqrt(bitcount(i^j))*2/3;
	  break;
      }
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

bool operator==(const LetterMap &l,const LetterMap &r)
{
  int i;
  bool ret=true;
  for (i=0;ret && i<32;i++)
    if (l.bitPatterns[i]!=r.bitPatterns[i])
      ret=false;
  return ret;
}

bool operator<(const LetterMap &l,const LetterMap &r)
/* Returns true if l.fit is *greater* than r.fit. This function is for sorting,
 * and the fittest LetterMap goes to the zeroth element of the vector.
 */
{
  return l.fit>r.fit;
}

void shuffle(vector<LetterMap> &pop)
{
  int i;
  for (i=pop.size();i>1;i-=2)
    swap(pop[i-1],pop[rng.rangerandom(i)]);
}

void dumpfit(vector<LetterMap> &pop)
{
  int i;
  for (i=0;i<pop.size();i++)
  {
    cout<<i<<' '<<pop[i].fitness()<<"  ";
    if (i==pop.size()-1 || i%5==4)
      cout<<endl;
  }
}

double prog(int nsteady,int niter)
// Decreases to 0 as progress is made.
{
  int endpt=niter/3+20;
  return ((double)endpt-nsteady)/(endpt+0.5);
}

void findLetterMapGenetic()
{
  vector<LetterMap> population;
  vector<int> delenda;
  int mutationRate=256; // out of 65536
  array<BIT16,5> initBits;
  double lastFitness=0;
  int i,j,sz,dim,nParents,popLimit,niter=0,nsteady=0;
  cr::nanoseconds elapsed;
  cr::time_point<cr::steady_clock> timeStart;
  popLimit=1024;
  for (i=0;i<popLimit;i++)
  {
    for (j=0;j<5;j++)
      initBits[j]=rng.usrandom();
    population.push_back(LetterMap(initBits));
  }
  for (i=0;i<population.size();i++)
    population[i].computeFitness();
  delenda.clear();
}
