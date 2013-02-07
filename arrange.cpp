#include <cstdio>
#include <cmath>
#include <cstring>
#include "arrange.h"
#include "galois.h"
#include "rs.h"
#include "letters.h"

/* Metadata letters are put at the six corners and, for size>17, in the center.
 *    + - - +       + - - +
 *   / [   \ \     / [   \ \
 *  /         \   /         \
 * X Z       ] X X Y   Z   ] X
 *  \         /   \         /
 *   \ _   ^ /     \ _   ^ /
 *    + - - +       + - - +
 * Size<=17:
 * @-Y: unwritten zeros
 * Z:	zero (@) used as index marker
 * [:	ndataletters/31+1
 * \:	ndataletters%31+1
 * ]:	encoding (ASCII, numeric, Unicode, etc.)
 * ^-_: Reed-Solomon check letters
 * Size>17:
 * @-W: unwritten zeros
 * Y:	zero (@) used as index marker
 * Z:	(ndataletters-1)/961
 * [:	((ndataletters-1)%961)/31+1
 * \:	(ndataletters-1)%31+1
 * ]:	encoding (ASCII, numeric, Unicode, etc.)
 * ^-_: Reed-Solomon check letters
 * Examples: (using encoding 1)
 * ndata YZ[\]
 *     1  @AAA
 *    12  @ALA
 *    31  @A_A
 *    32  @BAA
 *   883  @]OA this is the most that can fit in size 17, as each of the 30 rows must have at least 1 check letter
 *   884 @@]PA
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
int debugwhiten;

int oddmul(int a,int b)
{
  return ((2*a+1)*(2*b+1)/2)&31;
}

int odddiv(int a,int b)
{
  return oddmul(a,invoddmul[b&31]);
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
  nletters-=6+(nletters>961);
  return nletters;
}

void listsizes()
{
  int i,nletters,nrows,leftover;
  for (i=nletters=nrows=2;nletters-nrows<30752;i++)
  {
    nletters=ndataletters(i);
    nrows=(nletters+30)/31;
    leftover=31*nrows-nletters;
    printf("%3d %5d %4d %2d\n",i,nletters,nrows,leftover);
  }
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

int decinc(int i)
{
  int inc;
  for (inc=1;i>9;i/=10)
    inc*=10;
  return inc;
}

void testfindsize()
{
  int i,size03,size25,size50,size75,size97;
  for (i=1;i<40000;i+=decinc(i))
  {
    size03=findsize(i,0.03);
    size25=findsize(i,0.25);
    size50=findsize(i,0.50);
    size75=findsize(i,0.75);
    size97=findsize(i,0.97);
    printf("%5d %3d %3d %3d %3d %3d\n",i,size03,size25,size50,size75,size97);
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
