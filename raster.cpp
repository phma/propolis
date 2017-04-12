#include <cstdio>
#include <fstream>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <vector>
#include "raster.h"
#include "letters.h"

using namespace std;
fstream rfile;
int drawmode=2;
/* 0: each bit is drawn as a full hexagon
 * 1: each bit is drawn as 3/4 of a hexagon, and interstices are filled in with straight lines
 * 2: each bit is drawn as a circle (68% of the hexagon) and they're connected by lines
 */
vector<complex<double> > subsample;
/* Corners for 85 are 42 (ur), 43 (ll), 36 (ul), and 49 (lr).
 * More generally, corners are:
 * ur: (q²+(q+1)²-1)/2
 * ll: (q²+(q+1)²+1)/2
 * ul: q²
 * lr: (q+1)²
 */
int nsubsamples,ur,ll,ul,lr;
unsigned int regbits[13][4]=
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
  {0xfe80fe80,0xfe80fe80,0xfe80fe80,0xfe80fe80}
};

void initsubsample(int q)
{
  int i,x,y;
  ul=q*q;
  lr=(q+1)*(q+1);
  nsubsamples=ul+lr;
  ur=nsubsamples/2;
  ll=ur+1;
  subsample.resize(nsubsamples);
  for (i=x=y=0;i<nsubsamples;i++)
  {
    subsample[i]=complex<double>((double)x/nsubsamples,(double)y/nsubsamples);
    x+=2*q+1;
    y++;
    if (2*x>nsubsamples)
      x-=nsubsamples;
    if (2*y>nsubsamples)
      y-=nsubsamples;
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
  if (fname=="")
    fname="/dev/stdout";
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

void checkregbits()
{
  int i,j,shift,index,bit1,bit2;
  for (i=0;i<128;i++)
    for (j=0;j<13;j++)
    {
      shift=i&31;
      index=i>>5;
      bit1=(regbits[j][index]>>shift)&1;
      shift=(127-i)&31;
      index=(127-i)>>5;
      bit2=(regbits[j][index]>>shift)&1;
      if (bit1==bit2)
	printf("%2d %3d\n",j,i);
    }
}

void rasterdraw(int size,double width,double height,
	    double scale,int dim,int imagetype,string filename)
/* scale is in pixels. imagetype is currently ignored.
 */
{
  int i,j,k;
  int pixel;
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
  cor0.location=cor0.region=0;
  cor1=cor0;
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
      if (ul) // if ul=0 then lr=1 which is out of range, and there's only one subsample, so no need to check the corners
      {
	cor0=locregion((middle+subsample[ur])/scale+offset);
	cor1=locregion((middle+subsample[ll])/scale+offset);
	if (cor1==cor0 && cor0.region<7) // regions 7-12 are non-convex sets
	{
	  cor0=locregion((middle+subsample[ul])/scale+offset);
	  cor1=locregion((middle+subsample[lr])/scale+offset);
	}
      }
      if (cor1==cor0 && cor0.region<7)
	pixel=nsubsamples*filletbit(middle/scale+offset);
      else
        for (k=pixel=0;k<nsubsamples;k++)
        {
	  z=(middle+subsample[k])/scale+offset;
          pixel+=filletbit(z);
        }
      rfile<<(char)(255-(255*pixel+ur)/nsubsamples);
    }
  rclose();
}
