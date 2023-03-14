/******************************************************/
/*                                                    */
/* arrange.cpp - arrange letters in symbol            */
/*                                                    */
/******************************************************/
/* Copyright 2013-2023 Pierre Abbat.
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
#include <array>
#include <cstdio>
#include <iostream>
#include <cmath>
#include <cstring>
#include <cassert>
#include "arrange.h"
#include "letters.h"
#include "lagrange.h"
using namespace std;

/* Change in code (April 2017):
 * After over four years of wondering how this layer of Reed-Solomon and
 * letter code can be decoded, taking advantage of the one-bit error letter
 * being decodable as one of two or three letters, I decided on a better way:
 * There are five layers of criss-crossed Hamming codes, one for each bit in
 * a letter. Decoding is by belief propagation, similar to LDPC codes or turbo
 * codes. All the Hamming codes are updated, then all the letters are updated,
 * and so alternately, until they are all sure enough or they give up.
 *
 * The metadata will have check letters in F31 Lagrange polynomials.
 */

/* Metadata letters are put at the six corners and, for size>30, in the center.
 *    + - - +       + - - +
 *   / a   ` \     / a   ` \
 *  /         \   /         \
 * X b       \ X X c   b   \ X
 *  \         /   \         /
 *   \ \   ^ /     \ |   ^ /
 *    + - - +       + - - +
 * Size<=30:
 * b:	zero (@) used as index marker
 * a:	(nhammingblocks-1)/31+1
 * `:	(nhammingblocks-1)%31+1
 * \:	encoding (ASCII, numeric, Unicode, etc.)
 * ^-|: Lagrange check letters
 * Size>30:
 * c:	zero (@) used as index marker
 * b:	(nhammingblocks-1)/961+1
 * a:	((nhammingblocks-1)%961)/31+1
 * `:	(nhammingblocks-1)%31+1
 * \:	encoding (ASCII, numeric, Unicode, etc.)
 * ^-|: Lagrange check letters
 *
 * If the letter at \ is @, which means "test pattern", the number of Hamming
 * blocks is meaningless, and the check letters are undefined. The letters at
 * | and ^ should not be @, so that the orientation is unique.
 *
 * Examples: (using encoding 1)
 * nhamm cba`\
 *     1  @AAA
 *    12  @ALA
 *    31  @A_A
 *    32  @BAA
 *   927  @^\A this is the most that can fit in size 30, as 4-byte Hamming blocks are not allowed
 *   928 @A^]A
 *   961 @A__A
 *   962 @BAAA
 * 29316 @^PUA
 * 27971 @___A
 */

CodeMatrix theMatrix;
//               @ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_
char bitctrot[]="@BDLHTXYPEIZQKSWAFJ\\RMU[CNV]G^O_",
bitctunrot[]=   "@PAXBIQ\\DJRMCUY^HLTNEVZOFGKWS[]_",
invoddmul[]=    "@UF[\\QBWXM^STIZOPEVKLARGH]NCDYJ_";
short bitpermtab0[]=
{ // These are the 26 involution permutations of 5 bits, which combine
  043210, // with the 31 nonzero letters to yield 806-long whitening.
  043201,043120,042310,034210,003214,
  043012,041230,023410,040213,013240,
  042301,034120,002314,034201,003124,
  041032,021430,020413,010243,013042,
  040123,012340,034012,001234,023401
};
char prime[]={2,3,5,7,11};
vector<array<char,32> > bitpermtab;
int debugwhiten;

int oddmul(int a,int b)
{
  return ((2*a+1)*(2*b+1)/2)&31;
}

int odddiv(int a,int b)
{
  return oddmul(a,invoddmul[b&31]);
}

void fillbitpermtab()
{
  int i,j,b;
  array<char,32> perm;
  for (i=0;i<26;i++)
  {
    for (j=0;j<32;j++)
    {
      perm[j]=0;
      for (b=0;b<5;b++)
        if ((j>>b)&1)
          perm[j]+=1<<((bitpermtab0[i]>>(3*b))&7);
    }
    bitpermtab.push_back(perm);
  }
}

int whiten(int letter,int index)
/* Apply an index-dependent substitution cipher so that a symbol encoding
 * all the same letter looks random. If the written letters are too many
 * all white or too many all black, reading the symbol can result in a
 * framing error.
 */
{
  int highbits;
  highbits=letter&-32;
  letter&=31;
  if (bitpermtab.size()==0)
    fillbitpermtab();
  letter=bitpermtab[index%26][letter^(index%31+1)];
  letter|=highbits;
  return letter;
}

int unwhiten(int letter,int index)
{
  int highbits;
  highbits=letter&-32;
  letter&=31;
  if (bitpermtab.size()==0)
    fillbitpermtab();
  letter=bitpermtab[index%26][letter]^(index%31+1);
  letter|=highbits;
  return letter;
}

int tripleindex(int x,int y,int z)
// x!=y, y!=z, z!=x
{
  if (x>y)
  {
    x^=y;
    y^=x;
    x^=y;
  }
  if (y>z)
  {
    z^=y;
    y^=z;
    z^=y;
  }
  if (x>y)
  {
    x^=y;
    y^=x;
    x^=y;
  }
  z=(z-2)*(z-1)*z/6;
  y=(y-1)*y/2;
  return x+y+z;
}

int ndataletters(int n)
/* Returns the number of data letters (including check letters) in a symbol of size n.
 * The number of metadata letters (including 2 check letters) is either 6 or 7.
 */
{
  int nletters;
  nletters=n*(n+1)*3+1;
  nletters-=6+(nletters>961*3);
  return nletters;
}

unsigned gcd(unsigned a,unsigned b)
{
  while (a&&b)
  {
    if (a>b)
    {
      b^=a;
      a^=b;
      b^=a;
    }
    b%=a;
  }
  return a+b;
}

array<int,2> crissCrossFactor(int n)
/* Used for criss-crossing the five layers of Hamming codes.
 * Returns the number such that its least power which is 1 or -1
 * is as small as possible at least 5. The five layers are rotated by
 * primes to make them not line up on any letter.
 * Example: n=55 (size 4 symbol), returns (16,8). Four blocks: 14,14,14,13.
 *        aaAaAAAaAAAAAAbbBbBBBbBBBBBBccCcCCCcCCCCCCddDdDDDdDDDDD
 * 08 +02 CDaabbccddaAbBcCdDAABBCCDDaAbBcCdDAABBCCDDAABBCCDDAABBC
 * 18 +03 AAaaDdDdCCCccBBBBAAaAaDDDDCCcCcBBBbbAAAADDDddCCCCBBbBbA
 * 13 +05 BdAbCabCDACDABdABCaBcDbcDABDABCABCDbCdAcdaBCaBCDBCDAcDA
 * 43 +07 DcACbDBaCACBDBACAdBDcACbdBacADbDCaCBDBACAdBDcACbDBaCAdB
 * 28 +11 CcCCCddDdDDaAAAAAAbBBBBBBcCCCCCCdDDDDDDaaAaAAAbbBbBBBcc
 *       Bit 0             Bit 1             Bit 2
 *     * B B C *         * B b A *         * c D A *
 *    C C D D A A       C C C B B b       C D B C D A
 *   C D D A A B B     A D D D d d C     c d a B C a B
 *  C d D A A B B C   B B B b b A A A   A B C D b C d A
 * * D D a A b B c * * D D C C c C c * * D A B D A B C *
 *  d D A A B B C C   B A A a A a D D   B C a B c D b c
 *   d a A b B c C     C C c c B B B     A C D A B d A
 *    a b b c c d       a D d D d C       b C a b C D
 *     * C D a *         * A A a *         * B d A *
 *       Bit 3             Bit 4
 *     * A d B *         * B c c *
 *    C b D B a C       b b B b B B
 *   C A d B D c A     a a A a A A A
 *  D C a C B D B A   C d D D D D D D
 * * d B a c A D b * * B c C C C C C *
 *  A d B D c A C b   A A b B B B B B
 *   A C B D B A C     D D a A A A A
 *    C b D B a C       C C d d D d
 *     * D c A *         * C c C *
 */
{
  int i,j,k,minsofar=n,maxsofar=0;
  array<int,2> ret;
  for (i=1;i*i*i*i*i>n;i++);
  for (;i<n;i++)
  {
    for (j=0,k=1;j<minsofar && (j==0 || (k>1 && k<n-1));j++)
      k=(k*i)%n;
    if (j>=5 && j<minsofar && (k==1 || k==n-1))
    {
      minsofar=j;
      ret[0]=i;
    }
  }
  for (i=1;i<n;i++)
    if (gcd(i,n)==1)
    {
      minsofar=n;
      for (j=0,k=i;j<5;j++,k=(k*ret[0])%n)
      {
        if (k<minsofar)
          minsofar=k;
        if (n-k<minsofar)
          minsofar=n-k;
      }
      if (minsofar>maxsofar)
        maxsofar=minsofar;
    }
  ret[1]=maxsofar;
  return ret;
}

void listsizes()
{
  int i,nletters,nblocks;
  array<int,2> criss;
  cout<<"Size Letters Blocks Criss-cross\n";
  for (i=nletters=nblocks=2;nblocks<29791;i++)
  {
    nletters=ndataletters(i);
    if (nletters%3==1)
      nblocks=(nletters-4)/3; // 1 block of 7 and the rest of 3
    else // nletters%3 is never 2
      nblocks=nletters/3;
    criss=crissCrossFactor(nletters);
    printf("%3d   %5d  %5d  %5d %5d\n",i,nletters,nblocks,criss[0],criss[1]);
  }
}

vector<int> arrangeHamming(int nletters,int nblocks)
{
  int i,nb1,xs,xs0,xs1,blocksize0,blocksize1;
  vector<int> ret;
  for (blocksize1=3;nblocks && blocksize1*nblocks<=nletters;blocksize1=2*blocksize1+1);
  blocksize0=blocksize1/2;
  for (nb1=0;nb1<nblocks && nb1*blocksize1+(nblocks-nb1)*blocksize0<nletters;nb1++);
  xs=nb1*blocksize1+(nblocks-nb1)*blocksize0-nletters;
  xs1=xs%(nb1+nblocks);
  xs0=0;
  if (xs1>nb1)
    xs0=xs1-nb1;
  xs/=(nb1+nblocks);
  for (i=0;i<nblocks;i++)
    ret.push_back(((i<nb1)?blocksize1-2*xs:blocksize0-xs)-(nblocks-1-i<xs0)-(i<nb1 && nb1-1-i<xs1));
  for (i=0;i<ret.size();i++)
    if ((ret[i]&(ret[i]-1))==0)
      ret.clear(); // Powers of 2 are not allowed.
  return ret;
}

int CodeMatrix::getSize()
{
  return size;
}

int CodeMatrix::getNLetters()
{
  return nLetters;
}

int CodeMatrix::getNData()
{
  return nData;
}

int CodeMatrix::getNDataCheck()
{
  return nDataCheck;
}

double CodeMatrix::getRedundancy()
{
  return (nLetters-nDataCheck)/(double)nLetters;
}

int CodeMatrix::findSize(int n,double redundancy)
/* redundancy should be between 0 and 2/3. The symbol size will be set so that
 * there is at least that much redundancy, unless this is impossible, in which
 * case it will be set to the maximum redundancy, which is all Hamming blocks
 * of size 3, except for one of size 7 in smaller symbols. n is the number of
 * data letters, not including check-count or check-padding letters.
 *
 * It is possible, even if the specified redundancy is somewhat less than 2/3,
 * for the actual redundancy to be less than the specified redundancy. For
 * small symbols (fewer than 250 data letters or smaller than size 16), the
 * maximum guaranteed redundancy is 11/17 (0.647), which occurs from 1 to 9
 * letters. For large symbols, the maximum guaranteed redundancy is 1484/2263
 * (0.65488), which occurs at 779 letters.
 */
{
  int minSize,nBlocks,maxBlocks,minBl,maxBl;
  int lastGoodSize=0,lastGoodBlocks;
  bool goodRedundancy=false,tooFar=false;
  nData=n++; // increment n for the check-count letter
  minSize=trunc(sqrt(n/(1-redundancy)/3))-1;
  if (minSize<2)
    minSize=2;
  //cout<<nData<<" data letters, redundancy "<<redundancy<<endl;
  for (size=minSize;;size++)
  {
    nLetters=ndataletters(size);
    if (nLetters%3)
      maxBlocks=(nLetters-4)/3;
    else
      maxBlocks=nLetters/3;
    //cout<<"Size "<<size<<", "<<nLetters<<" letters\n";
    minBl=0;
    maxBl=maxBlocks+1;
    while (maxBl-minBl>1)
    {
      nBlocks=(minBl+maxBl)/2;
      hammingSizes=arrangeHamming(nLetters,nBlocks);
      nDataCheck=totaldatabits(hammingSizes);
      //cout<<"Size "<<size<<", "<<nBlocks<<" blocks, "<<nDataCheck<<" data and check letters, redundancy "<<(nLetters-nDataCheck)/(double)nLetters<<endl;
      if (nDataCheck>nData)
	minBl=nBlocks;
      else
	maxBl=nBlocks;
    }
    nBlocks=minBl;
    if (nBlocks)
    {
      hammingSizes=arrangeHamming(nLetters,nBlocks);
      nDataCheck=totaldatabits(hammingSizes);
    }
    else
      nDataCheck=nLetters;
    goodRedundancy=(nLetters-nDataCheck)/(double)nLetters>redundancy;
    tooFar=nDataCheck-nData>32;
    if (nBlocks && nDataCheck>nData && !tooFar)
    {
      lastGoodBlocks=nBlocks;
      lastGoodSize=size;
    }
    if (goodRedundancy || tooFar)
      break;
  }
  if (lastGoodSize<size)
  {
    size=lastGoodSize;
    nBlocks=lastGoodBlocks;
    nLetters=ndataletters(size);
    hammingSizes=arrangeHamming(nLetters,nBlocks);
    nDataCheck=totaldatabits(hammingSizes);
  }
  hammingBlocks.clear();
  if (nBlocks>31*31*31)
    size=0;
  return size;
}

void CodeMatrix::setDataCheck(string str,int encoding)
/* str should have the check letters already appended, and findSize should
 * have been called already.
 */
{
  int64_t i,j,k;
  array<int,2> ccf;
  array<int,5> ccf5;
  vector<int31> lagrange;
  vector<signed char> unCrissCrossed,ham1;
  for (i=k=0;i<hammingSizes.size();i++)
  {
    if (i>=hammingBlocks.size())
      hammingBlocks.push_back(Hamming());
    for (j=0;j<databits(hammingSizes[i]);j++,k++)
      if (k<str.length())
	hammingBlocks[i].push_back(str[k]);
  }
  for (i=0;i<hammingSizes.size();i++)
  {
    ham1=hammingBlocks[i].getCode();
    for (j=0;j<ham1.size();j++)
      unCrissCrossed.push_back(ham1[j]);
  }
  ccf=crissCrossFactor(nLetters);
  ccf5[0]=ccf[1];
  for (i=1;i<5;i++)
    ccf5[i]=(ccf5[i-1]*ccf[0])%nLetters;
  data.clear();
  data.resize(nLetters,0x40);
  for (i=0;i<nLetters;i++)
    for (j=0;j<5;j++)
      data[(i*ccf5[j]+prime[j])%nLetters]|=unCrissCrossed[i]&(1<<j);
  for (i=0;i<nLetters;i++)
    data[i]=whiten(data[i],i);
  metadata.clear();
  metadata.push_back('@');
  k=hammingBlocks.size()-1;
  if (size>30)
    metadata.push_back(k/961+'A');
  metadata.push_back((k%961)/31+'A');
  metadata.push_back(k%31+'A');
  metadata.push_back(encoding+'@');
  lagrange.push_back(int31(0));
  lagrange.push_back(int31(0));
  for (i=metadata.size()-1;i>=0;i--)
    lagrange.push_back(int31(metadata[i]-'A'));
  putMetadataCheck(lagrange);
  metadata.push_back(lagrange[1].val()+'A');
  metadata.push_back(lagrange[0].val()+'A');
}

string appendCheckLetters(string str,int len)
{
  int i,j,acc;
  len-=str.length();
  assert(len>0 && len<=32);
  for (i=0;i<len;i++)
  {
    for (j=0,acc='@'+len-i-1;j<str.length();j++)
      acc=bitctrot[oddmul(acc,str[j])];
    acc^=i;
    str+=acc;
  }
  return str;
}

string verifyCheckLetters(string str)
/* Returns the string passed to appendCheckLetters if the check letters
 * are correct. If they are wrong, returns "!". There is a small chance
 * of falsely saying that the check letters are correct and returning garbage.
 */
{
  int i,j,acc;
  string checks;
  for (i=0;(checks.length()==0 || i<=(checks[0]&31)) && str.length();i++)
  {
    for (acc=i+'@',j=0;j<str.length()-1;j++)
      acc=bitctrot[oddmul(acc,str[j])];
    checks+=(char)(acc^str[j]^'@');
    str.pop_back();
  }
  for (i=0;i<checks.length();i++)
    if (i+checks[i]!=checks.length()+'@'-1)
      str="!";
  if (checks.length()==0 || checks.length()>32)
    str="!";
  return str;
}

void CodeMatrix::setData(string str,int encoding)
{
  setDataCheck(appendCheckLetters(str,nDataCheck),encoding);
}

void CodeMatrix::dump()
{
  int i,column=0;
  string ham;
  cout<<"Size: "<<size<<" Letters: "<<nLetters<<" Data: "<<nData<<" Data+Check: "<<nDataCheck<<endl;
  cout<<"Hamming blocks:\n";
  for (i=0;i<hammingBlocks.size();i++)
  {
    ham=hammingBlocks[i].dumpLetters();
    if (column)
      if (column+ham.length()>78)
      {
	cout<<endl;
	column=0;
      }
      else
      {
	cout<<' ';
	column++;
      }
    cout<<ham;
    column+=ham.length();
  }
  cout<<endl;
  column=0;
  cout<<"Metadata: ";
  for (i=0;i<metadata.size();i++)
    cout<<metadata[i];
  cout<<"\nData:\n";
  for (i=0;i<data.size();i++)
  {
    cout<<data[i];
    if (i%64==63)
      cout<<endl;
  }
  if (i%64)
    cout<<endl;
}

void CodeMatrix::arrange(harray<char> &hletters)
{
  int i;
  hvec k;
  assert(metadata.size()>=6);
  hletters[hvec(-size,0)]=metadata[0];
  if (metadata.size()==7)
  {
    hletters[hvec(0,0)]        =metadata[1];
    hletters[hvec(0,size)]     =metadata[2];
    hletters[hvec(size,size)]  =metadata[3];
    hletters[hvec(size,0)]     =metadata[4];
    hletters[hvec(0,-size)]    =metadata[5];
    hletters[hvec(-size,-size)]=metadata[6];
  }
  else
  {
    hletters[hvec(0,size)]     =metadata[1];
    hletters[hvec(size,size)]  =metadata[2];
    hletters[hvec(size,0)]     =metadata[3];
    hletters[hvec(0,-size)]    =metadata[4];
    hletters[hvec(-size,-size)]=metadata[5];
  }
  for (i=0,k=start(size);i<data.size();i++,k.inc(size))
  {
    if (k.norm()==sqr(size) || (metadata.size()==7 && k==0))
      k.inc(size); // skip the 6 or 7 metadata letters
    hletters[k]=data[i];
  }
}

void CodeMatrix::unarrange(harray<uint16_t> &hglyphs)
{
  int i;
  hvec k;
  metaglyphs.clear();
  metaglyphs.push_back(hglyphs[hvec(-size,0)]);
  if (size>30)
    metaglyphs.push_back(hglyphs[hvec(0,0)]);
  metaglyphs.push_back(hglyphs[hvec(0,size)]);
  metaglyphs.push_back(hglyphs[hvec(size,size)]);
  metaglyphs.push_back(hglyphs[hvec(size,0)]);
  metaglyphs.push_back(hglyphs[hvec(0,-size)]);
  metaglyphs.push_back(hglyphs[hvec(-size,-size)]);
  glyphs.clear();
  for (i=0,k=start(size);k.cont(size);i++,k.inc(size))
  {
    if (k.norm()==sqr(size) || (metaglyphs.size()==7 && k==0))
      k.inc(size); // skip the 6 or 7 metadata letters
    glyphs[i]=hglyphs[k];
  }
}

int decinc(int i)
{
  int inc;
  for (inc=1;i>9;i/=10)
    inc*=10;
  return inc;
}

void testfindsize()
{
  int i,j,size00,size20,size40,size60,size67;
  vector<int> blksizes;
  int lastsize[2];
  double red,lastred[2];
  CodeMatrix cm;
  for (i=1;i<=10;i++)
  {
    blksizes=arrangeHamming(31,i);
    cout<<i<<':';
    for (j=0;j<blksizes.size();j++)
      cout<<' '<<blksizes[j];
    cout<<": "<<totaldatabits(blksizes)<<endl;
  }
  for (i=1;i<40000;i+=decinc(i))
  {
    size00=cm.findSize(i,0);
    size20=cm.findSize(i,0.2);
    size40=cm.findSize(i,0.4);
    size60=cm.findSize(i,0.6);
    size67=cm.findSize(i,0.67);
    printf("%5d %3d %3d %3d %3d %3d\n",i,size00,size20,size40,size60,size67);
  }
  for (i=1;i>2048;i++)
  {
    size67=cm.findSize(i,0.67);
    red=cm.getRedundancy();
    //if (i>1 && fabs(lastred[1]+red-2*lastred[0])>1.5e-4)
      //printf("%5d %3d %17.15f\n",i-1,lastsize[0],lastred[0]);
    lastsize[1]=lastsize[0];
    lastred[1]=lastred[0];
    lastsize[0]=size67;
    lastred[0]=red;
  }
}

void testbitctrot()
{
  int i;
  for (i=0;i<32;i++)
    putchar(bitctunrot[bitctrot[i]-'@']);
  putchar('\n');
  for (i=0;i<32;i++)
    putchar(bitctunrot[31-i]+bitctrot[i]-'@');
  putchar('\n');
}

void testCheckLetters()
{
  string teststr="PACK@MY@BOX@WITH@FIVE@DOZEN@LIQUOR@JUGS@";
  string checkstr,verifystr,checkstr1,verifystr1;
  int i;
  for (i=41;i<73;i++)
  {
    checkstr1=checkstr=appendCheckLetters(teststr,i);
    checkstr1[i-41]='[';
    verifystr=verifyCheckLetters(checkstr);
    verifystr1=verifyCheckLetters(checkstr1);
    cout<<checkstr;
    if (verifystr==teststr && verifystr1=="!")
      cout<<" ✓";
    cout<<'\n';
  }
}
