/* Alphabet of 32 letters for hexagonal code
 *     * *
 *    * * *
 *   * * * *
 *    * * *
 * Each letter is a 12-bit code for a 5-bit plaintext. They are read
 * with bit 11 at the top and bit 0 at the bottom right. Rotating a
 * codeword 120° and flipping all bits do consistent things to plaintext,
 * but neither rotating nor changing two bits (the Hamming distance is 2)
 * does anything related to a Reed-Solomon code.
 * 
 * Codes for single 1 bits are:
 *  00001   00010   00100   01000   10000
 *   0 0     1 1     1 1     1 0     0 1
 *  0 0 0   1 1 1   0 0 0   1 0 0   0 0 1
 * 0 0 0 0 0 0 0 0 0 0 0 0 1 1 0 0 0 0 1 1
 *  1 1 1   0 0 0   0 0 0   1 0 0   0 0 1
 * Rotating a code does this:
 *  00100   01010   10001
 *   1 1     0 0     0 0
 *  0 0 0   0 0 0   0 0 0
 * 0 0 0 0 0 0 0 1 1 0 0 0
 *  0 0 0   0 0 1   1 0 0
 */
#include <cstdio>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <cassert>
#include "letters.h"
#include "raster.h"

BIT16 letters[38]={
0x000, // 00000  00 000 0000 000
0x007, // 00001  00 000 0000 111
0xf80, // 00010  11 111 0000 000
0x01f, // 00011  00 000 0011 111
0xc00, // 00100  11 000 0000 000
0x09b, // 00101  00 001 0011 011
0xf40, // 00110  11 110 1000 000
0xd99, // 00111  11 011 0011 001
0xa64, // 01000  10 100 1100 100
0x067, // 01001  00 000 1100 111
0x009, // 01010  00 000 0001 001
0x277, // 01011  00 100 1110 111
0x488, // 01100  01 001 0001 000
0x5bf, // 01101  01 011 0111 111
0xfbb, // 01110  11 111 0111 011
0xb66, // 01111  10 110 1100 110
0x499, // 10000  01 001 0011 001
0x044, // 10001  00 000 1000 100
0xa40, // 10010  10 100 1000 000
0xb77, // 10011  10 110 1110 111
0xd88, // 10100  11 011 0001 000
0xff6, // 10101  11 111 1110 110
0xf98, // 10110  11 111 0011 000
0x59b, // 10111  01 011 0011 011
0x266, // 11000  00 100 1100 110
0x0bf, // 11001  00 001 0111 111
0xf64, // 11010  11 110 1100 100
0x3ff, // 11011  00 111 1111 111
0xfe0, // 11100  11 111 1100 000
0x07f, // 11101  00 000 1111 111
0xff8, // 11110  11 111 1111 000
0xfff, // 11111  11 111 1111 111
/*
// Side borders. The corner borders are 04, 12, 11, 01, 0a, 0c.
0xe40, //        11 100 1000 000
0xa44, //        10 100 1000 100
0x047, //        00 000 1000 111
0x00f, //        00 000 0001 111
0x489, //        01 001 0001 001
0xc88};//        11 001 0001 000
*/
// Side borders. The corner borders are 02, 1a, 18, 1d, 05, 07.
0xfe4, //        11 111 1100 100
0xf66, //        11 110 1100 110
0x27f, //        00 100 1111 111
0x0ff, //        00 001 1111 111
0xd9b, //        11 011 0011 011
0xf99};//        11 111 0011 001
BIT16 invletters[4096];
/* Inverse letter table format:
 * 1xxxxxyyyyyzzzzz a bit pattern that could be any of three letters
 * 010000xxxxxyyyyy a bit pattern that could be any of two letters
 * 00100000000xxxxx a bit pattern that differs by 1 bit from one valid letter
 * 00010000000xxxxx a bit pattern of a valid letter
 * 0011000000000000 an unwritten zero (not in the table)
 * 01100000dddddddd an undecodable bit pattern indicating a framing error
 *                  dddddddd is in the size-8 hexagon, 0 through 216
 * 0000000000000000 an undecodable bit pattern that counts as erasure in RS
 */
BIT16 rotlow[]=
{
  0x000,0x400,0x080,0x480,0x008,0x408,0x088,0x488,
  0x800,0xc00,0x880,0xc80,0x808,0xc08,0x888,0xc88,
  0x100,0x500,0x180,0x580,0x108,0x508,0x188,0x588,
  0x900,0xd00,0x980,0xd80,0x908,0xd08,0x988,0xd88,
  0x010,0x410,0x090,0x490,0x018,0x418,0x098,0x498,
  0x810,0xc10,0x890,0xc90,0x818,0xc18,0x898,0xc98,
  0x110,0x510,0x190,0x590,0x118,0x518,0x198,0x598,
  0x910,0xd10,0x990,0xd90,0x918,0xd18,0x998,0xd98
}, rothigh[]=
{
  0x000,0x001,0x200,0x201,0x020,0x021,0x220,0x221,
  0x002,0x003,0x202,0x203,0x022,0x023,0x222,0x223,
  0x040,0x041,0x240,0x241,0x060,0x061,0x260,0x261,
  0x042,0x043,0x242,0x243,0x062,0x063,0x262,0x263,
  0x004,0x005,0x204,0x205,0x024,0x025,0x224,0x225,
  0x006,0x007,0x206,0x207,0x026,0x027,0x226,0x227,
  0x044,0x045,0x244,0x245,0x064,0x065,0x264,0x265,
  0x046,0x047,0x246,0x247,0x066,0x067,0x266,0x267
};
// @ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_
// @LXTQBIZCPDWR^UYFJAMH[O\EV]NKGS_
char rotateletter[]=
{
  0x00,0x0c,0x18,0x14,0x11,0x02,0x09,0x1a,0x03,0x10,0x04,0x17,0x12,0x1e,0x15,0x19,
  0x06,0x0a,0x01,0x0d,0x08,0x1b,0x0f,0x1c,0x05,0x16,0x1d,0x0e,0x0b,0x07,0x13,0x1f
};
  
int debugletters;
const hvec FRAMEMOD(FRAMERAD+1,2*FRAMERAD+1);
const hvec twelve[]=
{hvec( 0,-2),
 hvec(-1,-2),
 hvec(-2,-2),
 hvec( 1,-1),
 hvec( 0,-1),
 hvec(-1,-1),
 hvec(-2,-1),
 hvec( 1, 0),
 hvec( 0, 0),
 hvec(-1, 0),
 hvec( 1, 1),
 hvec( 0, 1)};
const complex<double> ninedisp[]=
{
  complex<double>(0,M_SQRT_3_4),
  SLIVER_CENTROID,
  -SLIVER_CENTROID/omega,
  SLIVER_CENTROID*omega,
  0,
  -SLIVER_CENTROID,
  SLIVER_CENTROID/omega,
  -SLIVER_CENTROID*omega,
  complex<double>(0,-M_SQRT_3_4)
};
const short int weights[]={333,31,31,31,1812,31,31,31,333};

int bitcount(int n)
{n=((n&0xaaaaaaaa)>>1)+(n&0x55555555);
 n=((n&0xcccccccc)>>2)+(n&0x33333333);
 n=((n&0xf0f0f0f0)>>4)+(n&0x0f0f0f0f);
 n=((n&0xff00ff00)>>8)+(n&0x00ff00ff);
 n=((n&0xffff0000)>>16)+(n&0x0000ffff);
 return n;
 }

void degauss()
{int i,j,tmp,cont;
 BIT16 basis[32];
 for (i=0;i<32;i++)
     basis[i]=letters[i];
 do
   {cont=0;
    for (i=0;i<32;i++)
        {for (j=0;j<32;j++)
             if (i!=j)
                {tmp=basis[i]^basis[j];
	         //printf("tmp=%d basis[%d]=%d basis[%d]=%d\n",tmp,i,basis[i],j,basis[j]);
                 if (bitcount(tmp)<bitcount(basis[i]) || bitcount(tmp)<bitcount(basis[j]))
                    {if (bitcount(basis[i])<bitcount(basis[j]))
                        basis[j]=tmp;
		     else
   		        basis[i]=tmp;
		     cont=1;
		     }
	         if ((i<j)^(basis[i]<basis[j]))
   	            {tmp=basis[i];
	             basis[i]=basis[j];
	             basis[j]=tmp;
	             }
	         }
         }
    } while (cont);
 for (j=0;j<32 && debugletters;j++)
     printf("%2d %03x\n",j,basis[j]);
 }

int rotate(int bitpattern)
// Rotates 120° left.
{
  return rothigh[bitpattern>>6]|rotlow[bitpattern&63];
}

int invar12(int bitpattern)
// Invariant under 120° rotation and flipping all bits. Not invariant under mirroring.
{
  int r1,r2,inv;
  r1=rotate(bitpattern);
  r2=rotate(r1);
  inv=bitpattern^r1^r2;
  if (inv&0x800)
    inv^=0xfff;
  return inv;
}

void drawletter(int letter,hvec place)
// letter is from 0x00 to 0x25; place is any hvec
{letter=letters[letter];
 place*=LETTERMOD;
 hbits[place+hvec( 0,-2)]=(letter>> 0)&1;
 hbits[place+hvec(-1,-2)]=(letter>> 1)&1;
 hbits[place+hvec(-2,-2)]=(letter>> 2)&1;
 hbits[place+hvec( 1,-1)]=(letter>> 3)&1;
 hbits[place+hvec( 0,-1)]=(letter>> 4)&1;
 hbits[place+hvec(-1,-1)]=(letter>> 5)&1;
 hbits[place+hvec(-2,-1)]=(letter>> 6)&1;
 hbits[place+hvec( 1, 0)]=(letter>> 7)&1;
 hbits[place+hvec( 0, 0)]=(letter>> 8)&1;
 hbits[place+hvec(-1, 0)]=(letter>> 9)&1;
 hbits[place+hvec( 1, 1)]=(letter>>10)&1;
 hbits[place+hvec( 0, 1)]=(letter>>11)&1;
 }

hvec roundframe(sixvec s)
{
  hvec h,closest;
  double norm,distance,mindist;
  sixvec sv;
  norm=s.norm();
  assert(norm>0);
  s/=(norm/M_SQRT_3);
  for (h=start(FRAMERAD),mindist=10;h.cont(FRAMERAD);h.inc(FRAMERAD))
  {
    sv=sixvec((complex<double>)h/(complex<double>)FRAMEMOD);
    distance=(sv-s).norm();
    if (distance<mindist)
    {
      mindist=distance;
      closest=h;
    }
  }
  return closest;
}

void fillinvletters()
{
  int i,j,k,l,m,n,t,inv[4096],il,in,stats[6],watch=0x2af;
  int suminvar[32][32][32][32];
  sixvec torussum[4096],watchlast;
  hvec disp;
  complex<double> frame;
  memset(inv,0,sizeof(inv));
  memset(suminvar,0,sizeof(suminvar));
  /* Fill the inverse letter table with all patterns that are
   * either letters or one bit different from letters.
   */
  for (i=0;i<32;i++)
  {
    inv[letters[i]]=i+32;
    for (j=0;j<12;j++)
    {
      il=letters[i]^(1<<j);
      in=i+64;
      while (in&inv[il])
        in<<=8;
      inv[il]|=in;
    }
  }
  for (i=0;i<4096;i++)
  {
    if (inv[i]&0x400000)
      invletters[i]=((inv[i]&0x1f0000)>>6)|((inv[i]&0x1f00)>>3)|(inv[i]&0x1f)|0x8000;
    else if (inv[i]&0x4000)
      invletters[i]=((inv[i]&0x1f00)>>3)|(inv[i]&0x1f)|0x4000;
    else if (inv[i]&0x40)
      invletters[i]=(inv[i]&0x1f)|0x2000;
    else if (inv[i]&0x20)
      invletters[i]=(inv[i]&0x1f)|0x1000;
    else
      invletters[i]=0;
  }
  /* Find all possible reads caused by framing errors. Fill a size-1 array
   * with four letters as follows:
   *         j j
   *        j j j
   *   l l j j j j k k
   *  l l l j j j k k k
   * l l l l i i k k k k
   *  l l l i + i k k k
   *   k k i i i i l l
   *  k k k i i i l l l
   * k k k k j j l l l l
   *  k k k j j j l l l
   *       j j j j
   *        j j j
   * where + is the origin. Then copy bits of the central letter to the retuse corners:
   *         j j
   *      k j j j l
   *   l l j j j j k k
   *  l l l j j j k k k
   * l l l l i i k k k k
   *  l l l i + i k k k
   * j k k i i i i l l j
   *  k k k i i i l l l
   * k k k k j j l l l l
   *  k k k j j j l l l
   *     l j j j j k
   *        j j j
   * Then read it with an offset in each of 108 regions. The area covered by the reads is this:
   *         x x
   *      * x x x *
   *   s s o o o o # #
   *  s s o o o o o # #
   * s s o o o o o o # #
   *  s o o o + o o o #
   * * # o o o o o o s *
   *  # # o o o o o s s
   * # # # o o o o s s s
   *  # # # x x x s s s
   *     * x x x x *
   *        x x x
   * Some results:
   *   * *
   *  * * o
   * * o * * (0xf5c) is not found anywhere
   *  * o o
   *   * *
   *  o * o
   * * o o o (0xd44) is found somewhere
   *  * o o
   * 410 (10%) of bit patterns are not found, 3354 (82%) are framing errors, and the rest are decodable as 1, 2, or 3 letters.
   * 1080 bit patterns are framing errors only because of filleting.
   *   o o
   *  * o *
   * o * o * 0x2af comes up interstitially from PAM*, PIM*, M*PA, and M*PI.
   *  * * *
   *   * *
   *  * o *
   * o o * * 0xe9a does not occur. One of these is a bug. Should come up from F*L^, F*P^, or some permutation thereof.
   *  o * o
   */
  for (i=0;i<32;i++)
  {
    hletters[0]=i;
    for (j=0;j<32;j++)
    {
      hletters[hvec(1,2)]=hletters[hvec(-1,-2)]=hletters[1]=hletters[-1]=j;
      printf("%c%c",i+'@',j+'@');
      fflush(stdout);
      for (k=0;k<32;k++)
      {
        hletters[hvec(2,1)]=hletters[hvec(-2,-1)]=hletters[hvec(0,1)]=hletters[hvec(0,-1)]=k;
        for (l=0;l<32;l++)
	{
	  hletters[hvec(1,-1)]=hletters[hvec(-1,1)]=hletters[hvec(1,1)]=hletters[hvec(-1,-1)]=l;
          for (disp=start(2);disp.cont(2);disp.inc(2))
            drawletter(hletters[disp],disp);
	  for (n=0;n<9;n+=8)
	    for (t=0;t<12;t++)
	    {
	      il=0;
	      frame=ninedisp[n]-(complex<double>)twelve[t];
	      for (m=0;m<12;m++)
	        il|=filletbit((complex<double>)twelve[m]+frame)<<m;
	      torussum[il]+=sixvec(frame/ZLETTERMOD)*weights[n];
	      suminvar[i][j][k][l]+=invar12(il)*weights[n];
	    }
	  if (watchlast!=torussum[watch])
	  {
	    printf("%c%c ",k+'@',l+'@');
	    for (n=0;n<6;n++)
	      printf("%.0f ",torussum[watch].v[n]);
	    printf("\n%c%c",i+'@',j+'@');
	    watchlast=torussum[watch];
	  }
	}
      }
      printf("\b\b");
    }
  }
  memset(stats,0,sizeof(stats));
  for (i=0;i<4096;i++)
  {
    if (!invletters[i] && torussum[i].norm())
      invletters[i]=0x6000+roundframe(torussum[i]).pageinx(FRAMERAD,FRAMESIZE);
    for (j=11;j>=0 && debugletters;j--)
    {
      putchar(((i>>j)&1)+'0');
      if (j==10 || j==7 || j==3 || j==0)
	putchar(' ');
    }
    switch(invletters[i]&0xf000)
    {
      case 0x1000:
	if (debugletters)
	  printf("%03x: V  %c\n",i,((invletters[i]>>0)&31)+64);
	stats[0]++;
	break;
      case 0x2000:
	if (debugletters)
	  printf("%03x: 1  %c\n",i,((invletters[i]>>0)&31)+64);
	stats[1]++;
	break;
      case 0x4000:
	if (debugletters)
	  printf("%03x: 2 %c%c\n",i,((invletters[i]>>5)&31)+64,((invletters[i]>>0)&31)+64);
	stats[2]++;
	break;
      case 0x6000:
	if (debugletters)
	  printf("%03x: F    %4d %10.0f\n",i,(invletters[i]>>0)&2047,((invletters[i]>>0)&31)+64,torussum[i].norm());
	stats[4]++;
	break;
      case 0x0000:
	if (debugletters)
	  printf("%03x: N\n",i);
	stats[5]++;
	break;
      default:
        if (debugletters)
	  printf("%03x: 3%c%c%c\n",i,((invletters[i]>>10)&31)+64,((invletters[i]>>5)&31)+64,((invletters[i]>>0)&31)+64);
	stats[3]++;
    }
  }
  for (i=0;i<0;i++)
    for (j=0;j<32;j++)
      for (k=0;k<32;k++)
	for (l=0;l<32;l++)
	{
	  if (suminvar[i][j][k][l]!=suminvar[31-i][31-j][31-k][31-l])
	    printf("%c%c%c%c 2  ",i+'@',j+'@',k+'@',l+'@');
	  if (suminvar[i][j][k][l]!=suminvar[rotateletter[i]][rotateletter[l]][rotateletter[j]][rotateletter[k]])
	    printf("%c%c%c%c 3  ",i+'@',j+'@',k+'@',l+'@');
	}
  printf("%4d are exactly a letter\n",stats[0]);
  printf("%4d differ by one bit from 1 letter\n",stats[1]);
  printf("%4d differ by one bit from 2 letters\n",stats[2]);
  printf("%4d differ by one bit from 3 letters\n",stats[3]);
  printf("%4d are framing errors\n",stats[4]);
  printf("%4d don't match anything\n",stats[5]);
}

void readinvletters()
//TODO: search in /usr/share, or $PREFIX/share
{
  fstream infile;
  infile.open("invletters.dat",ios_base::in|ios_base::binary);
  if (infile.is_open())
  {
    infile.read((char *)invletters,sizeof(invletters));
    infile.close();
  }
}

void debugframingerror()
{
  int i,j,k,l,m,n,t,r,c,ch,il,in,stats[6],readings0[9][12],readings1[9][12];
  bool flip=false; // either flip all the bits, or rotate 120°
  bool err;
  hvec disp,a,rem;
  complex<double> frame;
  i=13;
  j=21;
  k=16;
  l=1;
  printf("%c%c%c%c\n",i+'@',j+'@',k+'@',l+'@');
  hletters[0]=i;
  hletters[hvec(1,2)]=hletters[hvec(-1,-2)]=hletters[1]=hletters[-1]=j;
  hletters[hvec(2,1)]=hletters[hvec(-2,-1)]=hletters[hvec(0,1)]=hletters[hvec(0,-1)]=k;
  hletters[hvec(1,-1)]=hletters[hvec(-1,1)]=hletters[hvec(1,1)]=hletters[hvec(-1,-1)]=l;
  for (disp=start(2);disp.cont(2);disp.inc(2))
    drawletter(hletters[disp],disp);
  for (n=0;n<9;n++)
    for (t=0;t<12;t++)
    {
      il=0;
      frame=ninedisp[n]-(complex<double>)twelve[t];
      for (m=0;m<12;m++)
        il|=filletbit((complex<double>)twelve[m]+frame)<<m;
      readings0[n][t]=il;
    }
  for (r=-5;r<=6;r++)
  {
    for (c=-10;c<=10;c++)
      if ((c+r)&1)
      {
	a=hvec((c-r-1)/2,-r);
	rem=a%LETTERMOD;
	ch=i?j?' ':'|':'-';
	switch (rem.letterinx())
	{
	  case 0:
	  case 3:
	    ch='/';
	    break;
	  case 7:
	  case 10:
	    ch='\\';
	    break;
	  case 2:
	  case 1:
	    ch='_';
	    break;
	}
	putchar(ch);
      }
      else
      {
	a=hvec((c-r)/2,-r);
	ch=(hbits[a]&1)*10+32;
	putchar(ch);
      }
    for (c=0;c<9;c++)
      printf(" %03x",readings0[c][r+5]);
    putchar('\n');
  }
  rasterdraw(1,0,0,600,DIM_DIAPOTHEM,FMT_PNM,"debug0.pgm");
  if (flip)
  {
    hletters[0]=31-i;
    hletters[hvec(1,2)]=hletters[hvec(-1,-2)]=hletters[1]=hletters[-1]=31-j;
    hletters[hvec(2,1)]=hletters[hvec(-2,-1)]=hletters[hvec(0,1)]=hletters[hvec(0,-1)]=31-k;
    hletters[hvec(1,-1)]=hletters[hvec(-1,1)]=hletters[hvec(1,1)]=hletters[hvec(-1,-1)]=31-l;
  }
  else
  {
    hletters[0]=rotateletter[i];
    hletters[hvec(1,2)]=hletters[hvec(-1,-2)]=hletters[1]=hletters[-1]=rotateletter[l];
    hletters[hvec(2,1)]=hletters[hvec(-2,-1)]=hletters[hvec(0,1)]=hletters[hvec(0,-1)]=rotateletter[j];
    hletters[hvec(1,-1)]=hletters[hvec(-1,1)]=hletters[hvec(1,1)]=hletters[hvec(-1,-1)]=rotateletter[k];
  }
  for (disp=start(2);disp.cont(2);disp.inc(2))
    drawletter(hletters[disp],disp);
  for (n=0;n<9;n++)
    for (t=0;t<12;t++)
    {
      il=0;
      frame=ninedisp[n]-(complex<double>)twelve[t];
      for (m=0;m<12;m++)
        il|=filletbit((complex<double>)twelve[m]+frame)<<m;
      readings1[n][t]=il;
    }
  for (r=-5;r<=6;r++)
  {
    for (c=-10;c<=10;c++)
      if ((c+r)&1)
      {
	a=hvec((c-r-1)/2,-r);
	rem=a%LETTERMOD;
	ch=i?j?' ':'|':'-';
	switch (rem.letterinx())
	{
	  case 0:
	  case 3:
	    ch='/';
	    break;
	  case 7:
	  case 10:
	    ch='\\';
	    break;
	  case 2:
	  case 1:
	    ch='_';
	    break;
	}
	putchar(ch);
      }
      else
      {
	a=hvec((c-r)/2,-r);
	ch=(hbits[a]&1)*10+32;
	putchar(ch);
      }
    for (c=0;c<9;c++)
    {
      if (flip)
	err=readings0[c][r+5]+readings1[c][r+5]!=4095;
      printf("%c%03x",err+' ',readings1[c][r+5]);
    }
    putchar('\n');
  }
  rasterdraw(1,0,0,600,DIM_DIAPOTHEM,FMT_PNM,"debug1.pgm");
}

void writeinvletters()
{
  fstream outfile;
  outfile.open("invletters.dat",ios_base::out|ios_base::binary);
  if (outfile.is_open())
  {
    outfile.write((char *)invletters,sizeof(invletters));
    outfile.close();
  }
}

void checkinvletters()
{
  int i,r;
  hvec g,h;
  bool valid=true;
  for (i=0;i<32;i++)
    if (invletters[letters[i]]!=(i|0x1000))
      valid=false;
  for (i=0;i<4096;i++)
  {
    if ((invletters[i]&0xf000)==0x6000)
    {
      h=nthhvec(invletters[i]-0x6000,FRAMERAD,FRAMESIZE);
      r=rotate(i);
      if ((invletters[r]&0xf000)==0x6000)
      {
        g=nthhvec(invletters[r]-0x6000,FRAMERAD,FRAMESIZE);
        if (g!=h*omega && false)
          printf("i=%03x, f.e. %4d (%3d,%3d)  r=%03x, f.e. %4d (%3d,%3d)\n",
	         i,invletters[i]-0x6000,h.getx(),h.gety(),
	         r,invletters[r]-0x6000,g.getx(),g.gety());
      }
      else
        printf("i=%03x, invletters[i]=%04x   r=%03x, invletters[r]=%04x\n",
	       i,invletters[i],
	       r,invletters[r]);
	
      r=4095-i;
      if ((invletters[r]&0xf000)==0x6000)
      {
        g=nthhvec(invletters[r]-0x6000,FRAMERAD,FRAMESIZE);
        if (g!=h)
          printf("i=%03x, f.e. %4d (%3d,%3d)  r=%03x, f.e. %4d (%3d,%3d)\n",
	         i,invletters[i]-0x6000,h.getx(),h.gety(),
	         r,invletters[r]-0x6000,g.getx(),g.gety());
      }
      else
        printf("i=%03x, invletters[i]=%04x   r=%03x, invletters[r]=%04x\n",
	       i,invletters[i],
	       r,invletters[r]);
	
    }
  }
  if (!valid)
    throw(runtime_error("invletters.dat is missing or corrupt. Run \"propolis --writetables\" to create it."));
}

void testroundframe()
{
  hvec h;
  sixvec s,t;
  s=sixvec(complex<double>(0.25,0.2));
  t=sixvec(complex<double>(0.25,-0.2));
  h=roundframe(s);
  printf("0.25, 0.2: %d,%d\n",h.getx(),h.gety());
  h=roundframe(t);
  printf("0.25,-0.2: %d,%d\n",h.getx(),h.gety());
  s+=t;
  h=roundframe(s);
  printf("Average  : %d,%d\n",h.getx(),h.gety());
}

void testrotate()
{
  int i;
  for (i=0;i<4096;i++)
    if (rotate(rotate(rotate(i)))!=i)
      printf("rotate(%03x)=%03x\n",i,rotate(i));
}
