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
#include <cmath>
#include <set>
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

bool operator<(const EccPoint &l,const EccPoint &r)
{
  return l.x<r.x;
}

void updateGraph(vector<EccPoint> &graph)
// Update the y values by computing a moving average of the result values.
{
  int i,begin=0,end=0,nint,n1,sz=graph.size();
  double radius;
  bool intervalGood;
  nint=lrint(sqrt(sz));
  n1=graph[0].result;
  for (i=0;i<sz;i++)
  {
    intervalGood=false;
    while (!intervalGood)
    {
      int prevEnd=end,prevBegin=begin;
      if (begin)
	radius=2*graph[i].x-graph[begin].x-graph[begin-1].x;
      else
	radius=2*graph[i].x-2*graph[begin].x;
      if (end<sz-1)
	radius+=graph[end].x+graph[end+1].x-2*graph[i].x;
      else
	radius+=2*graph[end].x-2*graph[i].x;
      radius/=4;
      while (graph[begin].x<graph[i].x-radius)
	n1-=graph[begin++].result;
      while (graph[end].x<graph[i].x+radius && end<sz-1)
	n1+=graph[++end].result;
      while (end-begin+1>nint+1)
      {
	n1-=graph[begin++].result;
	n1-=graph[end--].result;
      }
      while (end-begin+1<nint-1)
      {
	if (begin)
	  n1+=graph[--begin].result;
	if (end<sz-1)
	  n1+=graph[++end].result;
      }
      if (end-begin+1>nint)
      {
	if (graph[end].x+graph[begin].x>2*graph[i].x)
	  n1-=graph[end--].result;
	else if (graph[end].x+graph[begin].x>2*graph[i].x)
	  n1-=graph[begin++].result;
      }
      if (end-begin+1<nint)
      {
	if (graph[end].x+graph[begin].x>2*graph[i].x && begin)
	  n1+=graph[--begin].result;
	else if (graph[end].x+graph[begin].x>2*graph[i].x && end<sz-1)
	  n1+=graph[++end].result;
      }
      intervalGood=end==prevEnd && begin==prevBegin;
    }
    graph[i].y=(double)n1/(end-begin+1);
  }
}

set<double> newx(vector<EccPoint> &graph)
{
  set<double> ret;
  int i,sz=graph.size();
  double cross;
  for (i=0;i<sz-1;i++)
  {
    if ((i==0 && graph[i].y!=1) || (i==sz-2 && graph[i+1].y!=0) ||
	graph[i].y>graph[i+1].y || (graph[i].y>=0.5 && graph[i+1].y<=0.5))
      ret.insert((graph[i].x+graph[i+1].x)/2);
    if (graph[i].y>0.5 && graph[i+1].y<0.5)
      cross=i+0.5;
    if (graph[i].y==0.5)
      cross=i;
  }
  for (i=0;i<sz-1;i++)
    if (fabs(i+0.5-cross)<sqrt(sz))
      ret.insert((graph[i].x+graph[i+1].x)/2);
  return ret;
}

array<double,2> crossHalf(vector<EccPoint> &graph)
{
  array<double,2> ret;
  int i=0,j=graph.size()-1,mid;
  while (j-i>1)
  {
    mid=(i+j)/2;
    if (graph[mid].y>0.5)
      i=mid;
    else
      j=mid;
  }
  ret[0]=graph[i].x;
  ret[1]=graph[j].x;
  return ret;
}

void testStep()
{
  vector<EccPoint> graph;
  EccPoint eccPoint;
  set<double> newxs;
  array<double,2> xhalf;
  int i;
  set<double>::iterator j;
  graph.resize(2);
  graph[0].x=0;
  graph[0].result=1;
  graph[1].x=1;
  graph[1].result=0;
  updateGraph(graph);
  do
  {
    newxs=newx(graph);
    for (j=newxs.begin();j!=newxs.end();++j)
    {
      eccPoint.x=*j;
      eccPoint.result=rng.frandom(1-eccPoint.x);
      graph.push_back(eccPoint);
    }
    sort(graph.begin(),graph.end());
    updateGraph(graph);
    xhalf=crossHalf(graph);
  } while (xhalf[1]-xhalf[0]>1e-3);
  cout<<"crosses half at "<<(xhalf[0]+xhalf[1])/2<<endl;
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
