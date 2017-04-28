#include <cstdio>
#include "hvec.h"
#include "contour.h"

using namespace std;

bool isedge(hvec z)
{hvec q,r;
 q=z/2;
 r=z%2;
 return (hbits[q]^hbits[q+r])&1;
 }

bool ismarked(hvec z)
{hvec q,r;
 int bit=1;
 q=z/2;
 r=z%2;
 if (r==hvec(1,1)) bit=2;
 if (r==hvec(1,0)) bit=3;
 if (r==hvec(0,-1)) bit=4;
 return (hbits[q]>>bit)&1;
 }

void mark(hvec z)
{hvec q,r;
 int bit=1;
 q=z/2;
 r=z%2;
 if (r==hvec(1,1)) bit=2;
 if (r==hvec(1,0)) bit=3;
 if (r==hvec(0,-1)) bit=4;
 hbits[q]|=1<<bit;
 }

vector<hvec> trace(hvec stpoint)
/* Traces a contour. The start point, because of the way the iteration works,
 * is the leftmost point of the bottom line, so it starts going right.
 */
{hvec dir(1), left(1,1), right(0,-1);
 vector<hvec> contour;
 while (isedge(stpoint) && !ismarked(stpoint))
   {contour.push_back(stpoint);
    mark(stpoint);
    if (isedge(stpoint+dir))
       ;
    else if (isedge(stpoint+dir*left))
       dir*=left;
    else
       dir*=right;
    stpoint+=dir;
    }
 return contour;
 }

vector<vector<hvec> > traceall(int size)
{vector<hvec> contour;
 vector<vector<hvec> > contours;
 hvec i;
 for (i=start(size*8+9);i.cont(size*8+9);i.inc(size*8+9))
     {contour=trace(i);
      if (contour.size())
         contours.push_back(contour);
      }
 return contours;
 }
