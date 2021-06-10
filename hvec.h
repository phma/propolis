/* Hexagonal vector (Eisenstein or Euler integers) and array of bytes subscripted by hexagonal vector
 * The largest hvec used in this program has a norm of about
 * than 95²*12, or 108300.
 */
#ifndef HVEC_H
#define HVEC_H

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <vector>
#include <map>
#include <complex>
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

template <typename T> void harray<T>::clear()
{
  typename std::map<hvec,T *>::iterator i;
  for (i=index.begin();i!=index.end();++i)
  {
    free(i->second);
    i->second=NULL;
  }
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

/* Files for storing hexagonal arrays:
 * Byte 0: number of bits per element
 * Byte 1: PAGERAD
 * Bytes 2 and 3: PAGESIZE, native endianness
 * Bytes 4 and 5 additional information about an element format, currently 0000
 * This is followed by strips:
 * Bytes 0 and 1: x-coordinate of start of strip divided by PAGEMOD
 * Bytes 2 and 3: y-coordinate of start of strip divided by PAGEMOD
 * Bytes 4 and 5: number of pages in strip
 * Example (little-endian):
 * 01 06 7f 00 00 00 fa ff fa ff 07 00 <7×16 bytes of data>
 * Numbers of bits are expected to be 1, 2 (for art masks), 8, and 16. If 16, the data
 * will be stored in two-byte words, with the same endianness as PAGESIZE.
 * At first the program will read only files with its native endianness and PAGERAD;
 * later it will be able to convert them.
 */

extern harray<char> hletters,hbits;
#endif
