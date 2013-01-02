#include <vector>
#include <cstdio>
#include <cmath>
#include "arrange.h"

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
  int i,nrows,nletters,target;
  bool cont;
  if (redundancy<0.03)
    redundancy=0.03;
  if (redundancy>0.97)
    redundancy=0.97;
  target=lrint(n/(1-redundancy));
  if (target<=0)
    return 0;
  for (i=2;i<=n+2;i++)
  {
    nletters=ndataletters(i);
    nrows=(nletters+30)/31;
    //printf("n=%d %d>=%d %d>=%d %d>=%d\n",i,nletters,target,n,nrows,nletters-n,nrows);
    if (nletters>=target && n>=nrows && nletters-n>=nrows)
      break;
  }
  if (i>n+2)
    return 0;
  else
    return i;
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
  int i,size;
  double redundancy;
  redundancy=0.25;
  printf("redundancy 0.25\n");
  for (i=1;i<40000;i+=decinc(i))
  {
    size=findsize(i,redundancy);
    printf("%5d %3d\n",i,size);
  }
}
