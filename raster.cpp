#include <cstdio>
#include <fstream>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include "hvec.h"
#include "raster.h"
#include "letters.h"

using namespace std;
fstream rfile;
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

void ropen(string fname)
{
  rfile.open(fname.c_str(),ios_base::out|ios_base::binary);
}

void rclose()
{
  rfile.close();
}

void pgmheader(int width,int height)
{
  rfile<<"P5\n"<<width<<" "<<height<<endl<<255<<endl;
}

void rasterdraw(int size,double width,double height,
	    double scale,int dim,int imagetype,string filename)
/* scale is in pixels. imagetype is currently ignored.
 */
{
  int i,j,k;
  char pixel;
  int pwidth,pheight;
  complex<double> z,offset(0,-1/M_SQRT_3);
  hvec bend,dir,center,lastcenter,jump;
  char letter;
  double symwidth,symheight;
  ropen(filename);
  switch (dim)
  {
    case DIM_LTR:
      scale/=sqrt(12);
      break;
    case DIM_DIAMETER:
      scale=scale/(8*(size+2));
      break;
    case DIM_DIAPOTHEM:
      scale=scale/(sqrt(48)*(size+2));
      break;
    case DIM_XDIM:
      break;
    default:
      throw(invalid_argument("rasterdraw: unknown dimension"));
    }
  if (scale<=0)
    throw(range_error("rasterdraw: scale must be positive"));
  symwidth=scale*(sqrt(48)*(size+2));
  symheight=scale*(8*(size+2));
  if (width<0 || height<0)
    throw(range_error("psdraw: paper size must be nonnegative"));
  if (!height)
    height=symheight;
  if (!width)
    width=symwidth;
  pwidth=ceil(width);
  pheight=ceil(height);
  pgmheader(pwidth,pheight);
  for (i=0;i<pheight;i++)
    for (j=0;j<pwidth;j++)
    {
      z=complex<double>(j-pwidth/2.,i-pheight/2.)/scale+offset;
      pixel=255-255*(hbits[z]&1);
      rfile<<pixel;
    }
  rclose();
}
