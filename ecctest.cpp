/******************************************************/
/*                                                    */
/* ecctest.cpp - test error-correcting code           */
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
#include <iostream>
#include "ecctest.h"
#include "random.h"
using namespace std;
/* This test consists of creating a size-33 symbol (3360 data/check letters)
 * containing the number φ-1 in base 32, with the specified redundancy,
 * flipping, setting, or clearing bits at random positions, in a circle at
 * random position, or along a random line, and attempting to decode the
 * erroneous symbol. The fraction of bits, radius of circle, or width of line
 * that makes the symbol undecodable half the time is how much error it
 * can stand.
 */

double StepFinder::init()
{
  lastBit=2;
  x=0.5;
  upStep=downStep=0.125;
  runStraight=runAlternate=0;
  return x;
}

double StepFinder::step(int bit)
{
  if (bit)
    x+=upStep;
  else
    x-=downStep;
  runStraight++;
  runAlternate++;
  if (bit==lastBit)
    runAlternate=0;
  if (bit+lastBit==1)
    runStraight=0;
  lastBit=bit;
  if (bit)
    upStep*=1+(31*runStraight-32*runAlternate)/16384.;
  else
    downStep*=1+(31*runStraight-32*runAlternate)/16384.;
  return x;
}

bool StepFinder::finished()
{
  return upStep<DBL_EPSILON && downStep<DBL_EPSILON;
}

void testStep()
{
  vector<EccPoint> graph;
  EccPoint eccPoint;
  int i;
  graph.resize(2);
  graph[0].x=0;
  graph[0].y=1;
  graph[1].x=1;
  graph[1].y=0;
  
}

void ecctest()
{
  mpz_class num=1,denom=1;
  mpz_class phibig;
  int i;
  string phistr;
  while (denom>>8401==0)
  {
    num+=denom;
    swap(num,denom);
  }
  num<<=16800;
  phibig=num/denom;
  for (i=3359;i>=0;i--)
    phistr+=(char)((mpz_class)((phibig>>(5*i))&31)).get_ui()+'@';
  for (i=0;i<0;i+=70)
    cout<<phistr.substr(i,70)<<endl;
  testStep();
}
