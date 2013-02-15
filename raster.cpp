#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include "hvec.h"
#include "raster.h"
#include "letters.h"

complex<double> subsample85[85];

void initsubsample()
{
  int i,x,y;
  for (i=x=y=0;i<85;i++)
  {
    subsample85[i]=complex<double>(x/85.,y/85.);
    x+=13;
    y++;
    if (x>42)
      x-=85;
    if (y>42)
      y-=85;
  }
}
