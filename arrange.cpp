#include <cstdio>
#include <cmath>
#include <cstring>
#include "arrange.h"
#include "galois.h"

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
char bitctrot[]="@BDLHTXYPEIZQKSWAFJ\\RMU[CNV]G^O_",
bitctunrot[]=   "@PAXBIQ\\DJRMCUY^HLTNEVZOFGKWS[]_";

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

void codematrix::setndata(int nd)
{
  int i;
  ndata=nd;
  rows[0].data[29]=rows[0].data[30]='`';
  rows[0].data[27]=ndata%31+'A';
  rows[0].data[26]=(ndata/31)%31+'A';
  rows[0].data[25]=(ndata/961)%32+'@';
  for (i=1;i<=nrows;i++)
    rows[i].setndata((ndata+leftover)/nrows+(i<=(ndata+leftover)%nrows)-leftover/nrows-(i<=leftover%nrows));
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
