/* Hexagonal vector (Eisenstein or Euler integers) and array of bytes subscripted by hexagonal vector
 * The largest hvec used in this program has a norm of about
 * than 95²*12, or 108300.
 */
#ifndef HVEC_H
#define HVEC_H

#include <cmath>
#include <cstdlib>
#include <map>
#include <complex>
#define M_SQRT_3_4 0.86602540378443864676372317
// The continued fraction expansion is 0;1,6,2,6,2,6,2,...
#define M_SQRT_3 1.73205080756887729352744634
using namespace std;

#define PAGERAD 6
#define PAGESIZE (PAGERAD*(PAGERAD+1)*3+1)
#define sqr(a) ((a)*(a))
extern const complex<double> omega;

class hvec
{
private:
  int x,y; // x is the real part, y is at 120°
  static int numx,numy,denx,deny,quox,quoy,remx,remy;
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
  hvec(complex<double> z);
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
  int pageinx();
  int letterinx();
  int getx()
  {return x;
   }
  int gety()
  {return y;
   }
  operator complex<double>() const
  {
    return complex<double>(x-y/2.,y*M_SQRT_3_4);
  }
  void inc(int n);
  bool cont(int n);
};

hvec start(int n);
extern const hvec LETTERMOD,PAGEMOD;
extern int debughvec;

class sixvec
{
private:
  double v[6];
public:
  sixvec();
  sixvec(complex<double> z);
  sixvec operator+(const sixvec b);
  sixvec operator-(const sixvec b);
  sixvec operator*(const double b);
  sixvec operator/(const double b);
  sixvec operator+=(const sixvec b);
  double norm();
};

template <typename T> class harray
{map<hvec,T *> index;
 public:
 T& operator[](hvec i);
 void clear();
 };

template <typename T> T& harray<T>::operator[](hvec i)
{hvec q,r;
 q=i/PAGEMOD;
 r=i%PAGEMOD;
 if (!index[q])
    index[q]=(T*)calloc(PAGESIZE,sizeof(T));
 return index[q][r.pageinx()];
 }

template <typename T> void harray<T>::clear()
{typename map<hvec,T *>::iterator i;
 for (i=index.start();i!=index.end();i++)
     {free(i->second);
      i->second=NULL;
      }
 }

int region(complex<double> z);
void testcomplex();
void testsixvec();

extern harray<char> hletters,hbits;
#endif
