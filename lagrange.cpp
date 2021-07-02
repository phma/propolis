/******************************************************/
/*                                                    */
/* lagrange.cpp - Lagrange polynomial interpolation   */
/*                                                    */
/******************************************************/
/* Copyright 2018 Pierre Abbat.
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
#include <cassert>
#include "lagrange.h"
using namespace std;

const char inv31[]=
{
   0,
   1,16,21, 8,25,26,
   9, 4, 7,28,17,13,
  12,20,29, 2,11,19,
  18,14, 3,24,27,22,
   5, 6,23,10,15,30
};

int31::int31()
{
  n=0;
}

int31::int31(int m)
{
  n=m%31;
  if (n<0)
    n+=31;
}

int int31::val()
{
  return n;
}

char int31::locase()
// |^\`abcdefghijklmnopqrstuvwxyz{
{
  char ret=n;
  while (ret<'^')
    ret+=31;
  return ret;
}

int31 int31::operator+(int31 b)
{
  return int31(n+b.n);
}

int31 int31::operator-(int31 b)
{
  return int31(n-b.n);
}

int31 int31::operator*(int31 b)
{
  return int31(n*b.n);
}

int31 int31::operator/(int31 b)
{
  assert(b.n);
  return int31(n*inv31[b.n]);
}

int31& int31::operator+=(int31 b)
{
  n=(n+b.n)%31;
  return *this;
}

int31& int31::operator-=(int31 b)
{
  n=(n-b.n+31)%31;
  return *this;
}

int31& int31::operator*=(int31 b)
{
  n=(n*b.n)%31;
  return *this;
}

bool int31::operator==(int31 b)
{
  return n==b.n;
}

poly31::poly31()
{
  int i;
  for (i=0;i<30;i++)
    coeff[i]=0;
}

int31 poly31::operator()(int31 x)
{
  int i;
  int31 acc,pow;
  for (acc=i=0,pow=1;i<30;i++)
  {
    acc+=pow*coeff[i];
    pow*=x;
  }
  return acc;
}

poly31& poly31::operator=(int31 b)
{
  int i;
  coeff[0]=b;
  for (i=1;i<30;i++)
    coeff[i]=0;
}

poly31 poly31::operator<<(int n)
{
  int i;
  poly31 ret;
  n%=30;
  if (n<0)
    n+=30;
  for (i=0;i<30;i++)
    ret.coeff[(i+n)%30]=coeff[i];
  return ret;
}

poly31 poly31::operator>>(int n)
{
  int i;
  poly31 ret;
  n%=30;
  if (n<0)
    n+=30;
  for (i=0;i<30;i++)
    ret.coeff[i]=coeff[(i+n)%30];
  return ret;
}

poly31 poly31::operator+(int31 b)
{
  poly31 ret=*this;
  ret.coeff[0]+=b;
  return ret;
}

poly31& poly31::operator+=(int31 b)
{
  coeff[0]+=b;
  return *this;
}

poly31 poly31::operator+(poly31 &b)
{
  poly31 ret;
  int i;
  for (i=0;i<30;i++)
    ret.coeff[i]=coeff[i]+b.coeff[i];
  return ret;
}

poly31 poly31::operator+=(const poly31 &b)
{
  int i;
  for (i=0;i<30;i++)
    coeff[i]+=b.coeff[i];
  return *this;
}

poly31 poly31::operator-(int31 b)
{
  poly31 ret=*this;
  ret.coeff[0]-=b;
  return ret;
}

poly31 poly31::operator-(poly31 &b)
{
  poly31 ret;
  int i;
  for (i=0;i<30;i++)
    ret.coeff[i]=coeff[i]-b.coeff[i];
  return ret;
}

poly31 poly31::operator*(int31 b)
{
  poly31 ret;
  int i;
  for (i=0;i<30;i++)
    ret.coeff[i]=coeff[i]*b;
  return ret;
}

poly31& poly31::operator*=(int31 b)
{
  int i;
  for (i=0;i<30;i++)
    coeff[i]*=b;
  return *this;
}

poly31 poly31::operator*(poly31 &b)
{
  poly31 ret;
  int i,j;
  for (i=0;i<30;i++)
    for (j=0;j<30;j++)
      ret.coeff[(i+j)%30]+=coeff[i]*b.coeff[j];
  return ret;
}

poly31& poly31::operator*=(const poly31 &b)
{
  poly31 tmp;
  int i,j;
  for (i=0;i<30;i++)
    for (j=0;j<30;j++)
      tmp.coeff[(i+j)%30]+=coeff[i]*b.coeff[j];
  for (i=0;i<30;i++)
    coeff[i]=tmp.coeff[i];
  return *this;
}

poly31 poly31::operator/(int31 b)
{
  poly31 ret;
  int i;
  assert(b.n);
  b.n=inv31[b.n];
  for (i=0;i<30;i++)
    ret.coeff[i]=coeff[i]*b;
  return ret;
}

poly31& poly31::operator/=(int31 b)
{
  int i;
  assert(b.n);
  b.n=inv31[b.n];
  for (i=0;i<30;i++)
    coeff[i]*=b;
  return *this;
}

poly31 interceptor(int31 x)
{
  poly31 ret;
  ret+=int31(1);
  ret=(ret<<1)-x;
  return ret;
}

poly31 impulse(int bitmask,int31 x)
/* Returns a polynomial which is 0 at all arguments where bitmask is 1,
 * except at x, where it is 1.
 */
{
  int i;
  poly31 ret;
  ret+=int31(1);
  for (i=0;i<31;i++)
    if ((bitmask&(1<<i)) && i!=x.val())
      ret*=interceptor(int31(i));
  ret/=ret(x);
  return ret;
}

poly31 decodingPoly7[105],decodingPoly6[60];
// The first 5 of 105 and first 4 of 60 are used for encoding.
short index7[105],index6[60];

void initPoly()
{
  int i,j,k,n,mask;
  for (i=n=0;i<7;i++)
    for (j=0;j<i;j++)
    {
      mask=127-(1<<i)-(1<<j);
      for (k=0;k<7;k++)
	if (k!=i && k!=j)
	{
	  decodingPoly7[n]=impulse(mask,int31(k));
	  index7[n++]=mask+(k<<8);
	}
    }
  for (i=n=0;i<6;i++)
    for (j=0;j<i;j++)
    {
      mask=63-(1<<i)-(1<<j);
      for (k=0;k<6;k++)
	if (k!=i && k!=j)
	{
	  decodingPoly6[n]=impulse(mask,int31(k));
	  index6[n++]=mask+(k<<8);
	}
    }
}

void putMetadataCheck(vector<int31> &metadata)
/* metadata should be a vector of 6 or 7 int31s with the ones in 0 and 1 unset.
 * It sets them according to the other 4 or 5 int31s.
 */
{
  poly31 checkpoly;
  int i;
  if (decodingPoly6[0](0)==int31(0))
    initPoly();
  if (metadata.size()==6)
    for (i=0;i<4;i++)
      checkpoly+=decodingPoly6[i]*metadata[i+2];
  if (metadata.size()==7)
    for (i=0;i<5;i++)
      checkpoly+=decodingPoly7[i]*metadata[i+2];
  metadata[0]=checkpoly(0);
  metadata[1]=checkpoly(1);
}
