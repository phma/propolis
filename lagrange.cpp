#include <cassert>
#include "lagrange.h"

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
