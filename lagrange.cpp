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
