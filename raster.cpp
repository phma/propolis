#include <cstdio>
#include <fstream>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include "raster.h"
#include "letters.h"

using namespace std;
fstream rfile;
int drawmode=2;
/* 0: each bit is drawn as a full hexagon
 * 1: each bit is drawn as 3/4 of a hexagon, and interstices are filled in with straight lines
 * 2: each bit is drawn as a circle (68% of the hexagon) and they're connected by lines
 */
complex<double> subsample85[85]; // Corners are 42 (ur), 43 (ll), 36 (ul), and 49 (lr).
int regbits[13][4]=
/*  1 0
 * 4 3 2
 *  6 5
 * 0 aaaaaaaa   aaaaaaaa   aaaaaaaa   aaaaaaaa
 * 1 cccccccc   cccccccc   cccccccc   cccccccc
 * 2 f0f0f0f0   f0f0f0f0   f0f0f0f0   f0f0f0f0
 * 3 ff00ff00   ff00ff00   ff00ff00   ff00ff00
 * 4 ffff0000   ffff0000   ffff0000   ffff0000
 * 5 00000000   ffffffff   00000000   ffffffff
 * 6 00000000   00000000   ffffffff   ffffffff
 */
{
  {0xff00ff00,0xff00ff00,0xff00ff00,0xff00ff00}, // bit 3
  {0xee88ee88,0xee88ee88,0xee88ee88,0xee88ee88}, // majority of bits 3, 1, 0
  {0xfaa0faa0,0xfaa0faa0,0xfaa0faa0,0xfaa0faa0},
  {0xf000f000,0xfff0fff0,0xf000f000,0xfff0fff0},
  {0x00000000,0xff00ff00,0xff00ff00,0xffffffff},
  {0xff000000,0xff000000,0xffffff00,0xffffff00},
  {0xffcccc00,0xffcccc00,0xffcccc00,0xffcccc00},
  {0xfa00fa00,0xffa0ffa0,0xfa00fa00,0xffa0ffa0}, // bit 3, unless bits 0, 2, and 5 all agree against it
  {0xf000f000,0xff00ff00,0xff00ff00,0xfff0fff0},
  {0xff000000,0xff00ff00,0xff00ff00,0xffffff00},
  {0xff00cc00,0xff00cc00,0xffccff00,0xffccff00},
  {0xff88ee00,0xff88ee00,0xff88ee00,0xff88ee00},
  {0xfe80fe80,0xfe80fe80,0xfe80ff00,0xfe80fe80}
};

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

locreg locregion(complex<double> z)
{
  locreg lr;
  lr.location=z;
  lr.region=region(z-(complex<double>)lr.location);
  if ((drawmode==1 && lr.region>6) || drawmode==0)
    lr.region=0;
  return lr;
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

int bit7(hvec place)
{
  hvec k;
  int bits;
  for (k=start(1),bits=0;k.cont(1);k.inc(1))
    bits=(bits<<1)|(hbits[place+k]&1);
  return bits;
}

int filletbit(complex<double> z)
// Returns whether the point is white or black in the symbol as drawn with fillets and lines.
{
  int index,shift;
  locreg place;
  place=locregion(z);
  index=bit7(place.location);
  shift=index&31;
  index>>=5;
  return (regbits[place.region][index]>>shift)&1;
}

void rasterdraw(int size,double width,double height,
	    double scale,int dim,int imagetype,string filename)
/* scale is in pixels. imagetype is currently ignored.
 */
{
  int i,j,k;
  char pixel;
  int pwidth,pheight;
  complex<double> z,middle,offset(0,-1/M_SQRT_3);
  hvec bend,dir,center,lastcenter,jump;
  locreg cor0,cor1;
  char letter;
  double symwidth,symheight;
  ropen(filename);
  switch (dim)
  {
    case DIM_LTR:
      scale/=sqrt(12);
      break;
    case DIM_DIAMETER:
      scale=scale/(sqrt(48)*(size+2));
      break;
    case DIM_DIAPOTHEM:
      scale=scale/(6*(size+2));
      break;
    case DIM_XDIM:
      break;
    default:
      throw(invalid_argument("rasterdraw: unknown dimension"));
    }
  if (scale<=0)
    throw(range_error("rasterdraw: scale must be positive"));
  symwidth=scale*(6*(size+2));
  symheight=scale*(sqrt(48)*(size+2));
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
      middle=complex<double>(j-pwidth/2.,pheight/2.-i);
      cor0=locregion((middle+subsample85[42])/scale+offset);
      cor1=locregion((middle+subsample85[43])/scale+offset);
      if (cor1==cor0 && cor0.region<7) // regions 7-12 are non-convex sets
      {
        cor0=locregion((middle+subsample85[36])/scale+offset);
        cor1=locregion((middle+subsample85[49])/scale+offset);
      }
      if (cor1==cor0 && cor0.region<7)
	pixel=85*filletbit(middle/scale+offset);
      else
        for (k=pixel=0;k<85;k++)
        {
	  z=(middle+subsample85[k])/scale+offset;
          pixel+=filletbit(z);
        }
      rfile<<(char)(255-3*pixel);
    }
  rclose();
}
