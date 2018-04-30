#include <array>
#include <cstdio>
#include <iostream>
#include <cmath>
#include <cstring>
#include "arrange.h"
#include "galois.h"
#include "rs.h"
#include "letters.h"
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

/* Metadata letters are put at the six corners and, for size>17, in the center.
 *    + - - +       + - - +
 *   / [   \ \     / [   \ \
 *  /         \   /         \
 * X Z       ] X X Y   Z   ] X
 *  \         /   \         /
 *   \ _   ^ /     \ _   ^ /
 *    + - - +       + - - +
 * Size<=30:
 * @-Y: unwritten zeros
 * Z:	zero (@) used as index marker
 * [:	nhammingblocks/31+1
 * \:	nhammingblocks%31+1
 * ]:	encoding (ASCII, numeric, Unicode, etc.)
 * ^-_: Lagrange check letters
 * Size>30:
 * @-W: unwritten zeros
 * Y:	zero (@) used as index marker
 * Z:	(nhammingblocks)/961
 * [:	((nhammingblocks)%961)/31+1
 * \:	(nhammingblocks)%31+1
 * ]:	encoding (ASCII, numeric, Unicode, etc.)
 * ^-_: Lagrange check letters
 * Examples: (using encoding 1)
 * nhamm YZ[\]
 *     1  @AAA
 *    12  @ALA
 *    31  @A_A
 *    32  @BAA
 *   927  @^]A this is the most that can fit in size 30, as 4-byte Hamming blocks are not allowed
 *   928 @@^^A
 *   961 @@__A
 *   962 @AAAA
 * 29316 @^PUA
 * 29903 @_DSA
 * 30495 @_WVA
 * 30752 @___A
 */

codematrix thematrix;
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

int whiten(int letter,int row,int column)
{
  int highbits; // save the high 3 bits of the letter byte, which indicate whether it's data or check
  highbits=letter&-32;
  letter=(letter+column+1)&31;
  letter='_'-bitctrot[gmult(letter,column+1)];
  letter=bitctrot[oddmul(letter,row)&31];
  return (letter&31)|highbits;
}

int unwhiten(int letter,int row,int column)
{
  int highbits; // save the high 3 bits of the letter byte, which indicate whether it's data or check
  highbits=letter&-32;
  if (debugwhiten)
    printf("r%d c%d %c->",row,column,(letter&31)+'@');
  letter=bitctunrot[letter&31]-'@';
  if (debugwhiten)
    printf("%c->",(letter&31)+'@');
  letter='_'-bitctrot[oddmul(letter,invoddmul[row&31])];
  letter&=31;
  if (debugwhiten)
    printf("%c->",(letter&31)+'@');
  letter=gmult(letter,ginv(column+1))-column-1;
  if (debugwhiten)
    printf("%c\n",(letter&31)+'@');
  return (letter&31)|highbits;
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

void testwhiten()
{
  int letter,row,column,white[31],unwhite[31],histo[256],i,x,y,z,inx;
  char boxes[4960],triples[12][3];
  char teststr[]="PACK@MY@BOX@WITH@FIVE@DOZEN@LIQUOR@JUGS@";
  memset(boxes,0,sizeof(boxes));
  memset(histo,0,sizeof(histo));
  for (row=i=0;row<32;row++)
  {
    for (column=0;column<31;column++)
    {
      letter=teststr[i];
      if (!letter)
        letter=teststr[i=0];
      i++;
      white[column]=whiten(letter,row,column);
      unwhite[column]=unwhiten(white[column],row,column);
    }
    for (column=0;column<31;column++)
      putchar(white[column]);
    putchar(' ');
    for (column=0;column<31;column++)
      putchar(unwhite[column]);
    putchar('\n');
  }      
  for (i=row=0;i<4096;i++)
    if (invletters[i]&0x8000)
    {
      putchar(((invletters[i])    &31)+'@');
      putchar(((invletters[i]>>5) &31)+'@');
      putchar(((invletters[i]>>10)&31)+'@');
      triples[row][0]=((invletters[i])    &31);
      triples[row][1]=((invletters[i]>>5) &31);
      triples[row][2]=((invletters[i]>>10)&31);
      row++;
      putchar('\n');
    }
  for (row=i=0;row<32;row++)
    for (column=0;column<31;column++)
      for (i=0;i<12;i++)
      {
	x=unwhiten(triples[i][0],row,column);
	y=unwhiten(triples[i][1],row,column);
	z=unwhiten(triples[i][2],row,column);
	inx=tripleindex(x,y,z);
	boxes[inx]++;
	if (inx<10)
	  printf("inx=%d i=%d row=%d column=%d\n",inx,i,row,column);
      }
  for (x=2;x<31;x++)
    for (y=1;y<x;y++)
      for (z=0;z<y;z++)
	if (boxes[tripleindex(x,y,z)]+1)
	  printf("%c%c%c%3d  ",x+'@',y+'@',z+'@',boxes[tripleindex(x,y,z)]);
  putchar('\n');
  debugwhiten=1;
  unwhiten('F',5,16);
  unwhiten('Y',26,16);
  debugwhiten=0;
  for (i=0;i<4960;i++)
    histo[boxes[i]]++;
  for (i=0;i<10;i++)
    printf("%2d %2d\n",i,histo[i]);
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
  for (i=nletters=nblocks=2;nblocks<29791;i++)
  {
    nletters=ndataletters(i);
    if (nletters%3==1)
      nblocks=(nletters-4)/3; // 1 block of 7 and the rest of 3
    else // nletters%3 is never 2
      nblocks=nletters/3;
    criss=crissCrossFactor(nletters);
    printf("%3d %5d %5d %5d %5d\n",i,nletters,nblocks,criss[0],criss[1]);
  }
}

vector<int> arrangeHamming(int nletters,int nblocks)
{
  int i,nb1,xs,xs0,xs1,blocksize0,blocksize1;
  vector<int> ret;
  for (blocksize1=3;blocksize1*nblocks<=nletters;blocksize1=2*blocksize1+1);
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

int findsize(int n,double redundancy)
/* Finds the size of a symbol holding n letters with the specified redundancy.
 * If redundancy is out of the range (0.03,0.97), it is forced into that range.
 * More precisely, each row must have at least one data letter and at least one check letter.
 * Returns 0 if invalid.
 */
{
  int i,nrows,nletters,best;
  double target,diff,closest;
  bool cont;
  if (redundancy<0.03)
    redundancy=0.03;
  if (redundancy>0.97)
    redundancy=0.97;
  target=n/(1-redundancy);
  if (target<=0)
    return 0;
  for (i=2,best=nrows=0,closest=1e30+target;nrows<=n+2;i++)
  {
    nletters=ndataletters(i);
    nrows=(nletters+30)/31;
    //printf("n=%d %d>=%d %d>=%d %d>=%d\n",i,nletters,target,n,nrows,nletters-n,nrows);
    if (n>=nrows && nletters-n>=nrows && fabs(nletters-target)<closest)
    {
      closest=fabs(nletters-target);
      best=i;
    }
  }
  return best;
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

int CodeMatrix::findSize(int n,double redundancy)
/* redundancy should be between 0 and 2/3. The symbol size will be set so that
 * there is at least that much redundancy, unless this is impossible, in which
 * case it will be set to the maximum redundancy, which is all Hamming blocks
 * of size 3, except for one of size 7 in smaller symbols. n is the number of
 * data letters, not including check-count or check-padding letters.
 */
{
  int minSize,nBlocks,step;
  nData=n++; // increment n for the check-count letter
  minSize=trunc(sqrt(n/(1-redundancy)/3))-1;
  if (minSize<2)
    minSize=2;
  //cout<<nData<<" data letters, redundancy "<<redundancy<<endl;
  for (size=minSize;;size++)
  {
    nLetters=ndataletters(size);
    step=rint(sqrt(nLetters/3));
    //cout<<"Size "<<size<<", "<<nLetters<<" letters, step "<<step<<"\n";
    for (nBlocks=floor(nLetters/3);nBlocks>0;nBlocks-=step)
    {
      hammingSizes=arrangeHamming(nLetters,nBlocks);
      nDataCheck=totaldatabits(hammingSizes);
      //cout<<"Size "<<size<<", "<<nBlocks<<" blocks, "<<nDataCheck<<" data and check letters, redundancy "<<(nLetters-nDataCheck)/(double)nLetters<<endl;
      if (nDataCheck>nData)
	break;
    }
    if (nBlocks<1)
      nBlocks=1;
    for (;nBlocks*3<=nLetters;nBlocks++)
    {
      hammingSizes=arrangeHamming(nLetters,nBlocks);
      nDataCheck=totaldatabits(hammingSizes);
      //cout<<"Size "<<size<<", "<<nBlocks<<" blocks, "<<nDataCheck<<" data and check letters, redundancy "<<(nLetters-nDataCheck)/(double)nLetters<<endl;
      if (nDataCheck>nData && (nLetters-nDataCheck)/(double)nLetters>=redundancy)
	break;
      if (nDataCheck<=nData && nDataCheck>0)
	nBlocks+=step-1;
    }
    if (nDataCheck==0) // went past the maximum number of blocks
    {
      nBlocks-=2; // -1 to get to previous number, -1 to undo last for-loop increment
      hammingSizes=arrangeHamming(nLetters,nBlocks);
      nDataCheck=totaldatabits(hammingSizes);
      //cout<<"Size "<<size<<", "<<nBlocks<<" blocks, "<<nDataCheck<<" data and check letters, redundancy "<<(nLetters-nDataCheck)/(double)nLetters<<endl;
    }
    if (nDataCheck>nData)
      break;
  }
  if (nBlocks>31*31*31)
    size=0;
  return size;
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
}

row::row()
{
  strcpy(data,"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
}

void row::setunwritten(int n)
{
  int i;
  for (i=0;i<n;i++)
    data[i]=' ';
  for (;i<31;i++)
    if (data[i]<'@')
      data[i]+=32;
}

int row::getunwritten()
{
  int i,n;
  for (i=n=0;i<31;i++)
    n+=data[i]==' ';
  return n;
}

void row::setndata(int n)
{
  int i;
  for (i=0;i<32 && n;i++)
    if (data[i]>'?')
    {
      data[i]&=0x5f;
      n--;
    }
  for (;i<31;i++)
    if (data[i]<'`')
      data[i]+=32;
}

int row::getndata()
{
  int i,n;
  for (i=n=0;i<31;i++)
    n+=(data[i]&0xe0)=='@';
  return n;
}

void row::printdelete()
{
  int i;
  for (i=0;i<31;i++)
    if (data[i]==127)
      data[i]=63;
}

void row::restoredelete()
{
  int i;
  for (i=0;i<31;i++)
    if (data[i]==63)
      data[i]=127;
}

void row::whiten(int rownum)
{
  int i;
  for (i=0;i<31;i++)
    if (data[i]>63)
      data[i]=::whiten(data[i],rownum,i);
}

void row::unwhiten(int rownum)
{
  int i;
  for (i=0;i<31;i++)
    if (data[i]>63)
      data[i]=::unwhiten(data[i],rownum,i);
}

void row::shuffle(int rownum)
{
  int i,j;
  char copy[32];
  rownum=rownum%31;
  memcpy(copy,data,32);
  for (i=0;i<31;i++)
    if (copy[i]>63)
    {
      j=i;
      do
	j=gmult(ginv(j+1),rownum+1)-1;
      while (copy[j]<64);
      data[j]=copy[i];
    }
}

void row::unshuffle(int rownum)
{
  int i,j;
  char copy[32];
  rownum=rownum%31;
  memcpy(copy,data,32);
  for (i=0;i<31;i++)
    if (copy[i]>63)
    {
      j=i;
      do
	j=gmult(ginv(j+1),rownum+1)-1;
      while (copy[j]<64);
      data[i]=copy[j];
    }
}

void row::scramble(int rownum)
{
  whiten(rownum);
  shuffle(rownum);
}

void row::unscramble(int rownum)
{
  unshuffle(rownum);
  unwhiten(rownum);
}

void row::encode()
{
  encode_data(data,getunwritten()+getndata(),data);
}

void codematrix::setsize(int sz)
{
  int i;
  size=sz;
  nletters=ndataletters(sz);
  nrows=(nletters+30)/31;
  leftover=31*nrows-nletters;
  rows.resize(nrows+1);
  rows[0].setunwritten(25-(sz>17)); // if sz>17, the letter in the middle is a metadatum
  for (i=1;i<=nrows;i++)
    rows[i].setunwritten(leftover/nrows+(i<=leftover%nrows));
}

int codematrix::getsize()
{
  return size;
}

bool codematrix::setndata(int nd,bool marrel)
{
  int i;
  bool canfit=true;
  ndata=nd;
  if (ndata>((size>17)?30752:961))
    canfit=false;
  if (!marrel && (ndata<nrows || ndata>nrows*30-leftover))
    canfit=false;
  rows[0].data[29]=rows[0].data[30]='`';
  rows[0].data[27]=ndata%31+'A';
  rows[0].data[26]=(ndata/31)%31+'A';
  rows[0].data[25]=(ndata/961)%32+'@';
  for (i=1;i<=nrows;i++)
    rows[i].setndata((ndata+leftover)/nrows+(i<=(ndata+leftover)%nrows)-leftover/nrows-(i<=leftover%nrows));
  return canfit;
}

void codematrix::setdata(string str,int encoding)
{
  int i,j;
  for (i=0;i<str.length() && i<ndata;i++)
  {
    j=i+leftover;
    rows[j%nrows+1].data[j/nrows]=str[i];
  }
  rows[0].data[28]=encoding|'@';
}

void codematrix::scramble()
{
  int i;
  for (i=1;i<rows.size();i++)
    rows[i].scramble(i);
}

void codematrix::encode()
{
  int i;
  for (i=0;i<rows.size();i++)
    rows[i].encode();
}

void codematrix::unscramble()
{
  int i;
  for (i=1;i<rows.size();i++)
    rows[i].unscramble(i);
}

void codematrix::arrange(harray<char> &hletters)
{
  int i;
  hvec k;
  if (size>17)
  {
    hletters[hvec(-size,0)]    =rows[0].data[24];
    hletters[hvec(0,0)]        =rows[0].data[25];
  }
  else
  {
    hletters[hvec(-size,0)]    =rows[0].data[25];
  }
  hletters[hvec(0,size)]     =rows[0].data[26];
  hletters[hvec(size,size)]  =rows[0].data[27];
  hletters[hvec(size,0)]     =rows[0].data[28];
  hletters[hvec(0,-size)]    =rows[0].data[29];
  hletters[hvec(-size,-size)]=rows[0].data[30];
  for (i=leftover,k=start(size);i<=nrows*31;i++,k.inc(size))
  {
    if (k.norm()==sqr(size) || (size>17 && k==0))
      k.inc(size); // skip the 6 or 7 metadata letters
    hletters[k]=rows[i%nrows+1].data[i/nrows];
  }
}

void codematrix::dump()
{
  int i,j;
  for (i=0;i<rows.size();i++)
  {
    rows[i].printdelete();
    printf("%s\n",rows[i].data);
    rows[i].restoredelete();
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

void testshuffle()
{
  int i,j,n;
  for (i=0;i<31;i++)
  {
    for (j=0;j<31;j++)
    {
      n=gmult(ginv(i+1),(bitctrot[j]&31)+1)-1;
      putchar((n)+'@');
    }
    putchar('\n');
  }
}
