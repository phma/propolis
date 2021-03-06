/******************************************************/
/*                                                    */
/* hvec.h - Eisenstein integers                       */
/*                                                    */
/******************************************************/
/* Copyright 2012-2021 Pierre Abbat.
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
/* Hexagonal vector (Eisenstein or Euler integers) and array of bytes subscripted by hexagonal vector
 * The largest hvec used in this program has a norm of about
 * than 95²*12, or 108300.
 */
#ifndef HVEC_H
#define HVEC_H

class hvec;

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <vector>
#include <map>
#include <complex>
#include "pn8191.h"

#define M_SQRT_3_4 0.86602540378443864676372317
// The continued fraction expansion is 0;1,6,2,6,2,6,2,...
#define M_SQRT_3 1.73205080756887729352744634
#define M_SQRT_1_3 0.5773502691896257645091487805

//PAGERAD should be 1 or 6 mod 8, which makes PAGESIZE 7 mod 8.
// The maximum is 147 because of the file format.
#define PAGERAD 6
#define PAGESIZE (PAGERAD*(PAGERAD+1)*3+1)
#define sqr(a) ((a)*(a))
extern const std::complex<double> omega,ZLETTERMOD;

class hvec
{
private:
  int x,y; // x is the real part, y is at 120°
  void divmod(hvec b);
public:
  hvec()
  {x=y=0;}
  hvec(int xa)
  {x=xa;
   y=0;
   }
  hvec(int xa,int ya)
  {x=xa;
   y=ya;
   }
  hvec(std::complex<double> z);
  hvec operator+(hvec b);
  hvec operator-();
  hvec operator-(hvec b);
  hvec operator*(hvec b);
  hvec& operator*=(hvec b);
  hvec& operator+=(hvec b);
  hvec operator/(hvec b);
  hvec operator%(hvec b);
  bool operator==(hvec b);
  bool operator!=(hvec b);
  friend bool operator<(const hvec a,const hvec b); // only for the map
  unsigned long norm();
  int pageinx(int size,int nelts);
  int pageinx();
  int letterinx();
  int getx()
  {return x;
   }
  int gety()
  {return y;
   }
  operator std::complex<double>() const
  {
    return std::complex<double>(x-y/2.,y*M_SQRT_3_4);
  }
  void inc(int n);
  bool cont(int n);
};

hvec start(int n);
hvec nthhvec(int n,int size,int nelts);
extern const hvec LETTERMOD,PAGEMOD;
extern int debughvec;

class sixvec
{
public:
  double v[6];
  sixvec();
  sixvec(std::complex<double> z);
  sixvec operator+(const sixvec b);
  sixvec operator-(const sixvec b);
  sixvec operator*(const double b);
  sixvec operator/(const double b);
  sixvec operator+=(const sixvec b);
  sixvec operator/=(const double b);
  bool operator!=(const sixvec b);
  double norm();
};

template <typename T> class harray
{
  std::map<hvec,T *> index;
public:
  harray()=default;
  harray(const harray &h);
  harray &operator=(const harray &h);
  ~harray();
  T& operator[](hvec i);
  std::vector<hvec> listPages();
  std::vector<T> getPage(hvec q);
  void putPage(hvec q,std::vector<T> pagevec);
  int pageCrc(hvec q);
  int crc();
  void clear();
  void prune();
};

template <typename T> harray<T>::harray(const harray<T> &h)
{
  typename std::map<hvec,T *>::const_iterator i;
  for (i=h.index.begin();i!=h.index.end();i++)
    if (i->second)
    {
      index[i->first]=(T*)calloc(PAGESIZE,sizeof(T));
      memcpy(index[i->first],i->second,PAGESIZE*sizeof(T));
    }
}

template <typename T> harray<T> &harray<T>::operator=(const harray<T> &h)
{
  typename std::map<hvec,T *>::const_iterator i;
  if (this==&h)
    return *this;
  clear();
  for (i=h.index.begin();i!=h.index.end();i++)
    if (i->second)
    {
      index[i->first]=(T*)calloc(PAGESIZE,sizeof(T));
      memcpy(index[i->first],i->second,PAGESIZE*sizeof(T));
    }
  return *this;
}

template <typename T> harray<T>::~harray<T>()
{
  clear();
}

template <typename T> T& harray<T>::operator[](hvec i)
{
  hvec q,r;
  q=i/PAGEMOD;
  r=i%PAGEMOD;
  if (!index[q])
    index[q]=(T*)calloc(PAGESIZE,sizeof(T));
  return index[q][r.pageinx()];
}

template <typename T> std::vector<hvec> harray<T>::listPages()
{
  typename std::map<hvec,T *>::iterator i;
  std::vector<hvec> ret;
  for (i=index.begin();i!=index.end();++i)
    if (i->second)
      ret.push_back(i->first);
  return ret;
}

template <typename T> std::vector<T> harray<T>::getPage(hvec q)
{
  T *page;
  std::vector<T> ret;
  int i;
  if (!index[q]) // This shouldn't happen, as this is for getting a page that already exists.
    index[q]=(T*)calloc(PAGESIZE,sizeof(T));
  page=index[q];
  for (i=0;i<PAGESIZE;i++)
    ret.push_back(page[i]);
  return ret;
}

template <typename T> void harray<T>::putPage(hvec q,std::vector<T> pagevec)
{
  T *page;
  int i;
  assert(pagevec.size()>=PAGESIZE);
  if (!index[q])
    index[q]=(T*)calloc(PAGESIZE,sizeof(T));
  page=index[q];
  for (i=0;i<PAGESIZE;i++)
    page[i]=pagevec[i];
}

template <typename T> int harray<T>::pageCrc(hvec q)
{
  hvec center=q*PAGEMOD;
  int i,ret=0;
  T *page=index[q];
  for (i=0;page && i<PAGESIZE;i++)
    ret^=::crc(page[i],center+nthhvec(i,PAGERAD,PAGESIZE));
  return ret;
}

template <typename T> int harray<T>::crc()
{
  std::vector<hvec> pglist=listPages();
  int i,ret=0;
  for (i=0;i<pglist.size();i++)
    ret^=pageCrc(pglist[i]);
  return ret;
}

template <typename T> void harray<T>::clear()
{
  typename std::map<hvec,T *>::iterator i;
  for (i=index.begin();i!=index.end();++i)
  {
    free(i->second);
    i->second=NULL;
  }
  index.clear();
}

template <typename T> void harray<T>::prune()
{
  typename std::map<hvec,T *>::iterator i;
  char *page;
  for (i=index.begin();i!=index.end();++i)
  {
    page=(char *)i->second;
    if (*page==0 && memcmp(page,page+1,PAGESIZE*sizeof(T)-1)==0)
    {
      free(i->second);
      i->second=NULL;
    }
  }
}

int region(std::complex<double> z);
void testcomplex();
void testsixvec();
void testpageinx();

#endif
