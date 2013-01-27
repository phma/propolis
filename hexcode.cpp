/* Hexagonal code
 */

#include <cstdio>
#include <stdexcept>
#include "hvec.h"
#include "letters.h"
#include "galois.h"
#include "contour.h"
#include "ps.h"
#include "rs.h"
#include "arrange.h"

hvec a,b,q,r;
harray<char> hletters,hbits;
hvec twelve[]=
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

void border(int n)
// Draws a border for a symbol of size n. The border is one bigger.
{int i;
 n++;
 if (n<1)
    throw(range_error("border: size<0"));
 for (i=1;i<n;i++)
     {drawletter(0x20,hvec(n,i));
      drawletter(0x21,hvec(i,n));
      drawletter(0x22,hvec(-i,n-i));
      drawletter(0x23,hvec(-n,-i));
      drawletter(0x24,hvec(-i,-n));
      drawletter(0x25,hvec(i,i-n));
      }
 drawletter(0x02,hvec(n,0));
 drawletter(0x1a,hvec(n,n));
 drawletter(0x18,hvec(0,n));
 drawletter(0x1d,hvec(-n,0));
 drawletter(0x05,hvec(-n,-n));
 drawletter(0x07,hvec(0,-n));
 }

const int poly13=0x2b85; //13 11 9 8 7 2 0
short pncode[8191];

void fillpn()
{int i;
 pncode[0]=1;
 for (i=1;i<8191;i++)
     {pncode[i]=pncode[i-1]<<1;
      if (pncode[i]&0x2000)
         pncode[i]^=poly13;
      //printf("%4x ",pncode[i]);
      }
 }

void testgalois()
{
  int i,j;
  for (i=0;i<32;i++)
     {for (j=0;j<32;j++)
          printf("%c ",gmult(i,j)+'@');
      printf("\n");
      }
  printf("\n");
  for (i=0;i<32;i++)
    printf("%c ",ginv(i)+'@');
  printf("\n");
  for (i=0;i<31;i++)
     {for (j=0;j<32;j++)
          printf("%c ",genPoly[i][j]+'@');
      printf("\n");
      }
  printf("\n");
}

void pattern19()
{hletters[hvec(0,2)]=1;
 hletters[hvec(1,2)]=2;
 hletters[hvec(2,2)]=3;
 hletters[hvec(-1,1)]=4;
 hletters[hvec(0,1)]=5;
 hletters[hvec(1,1)]=6;
 hletters[hvec(2,1)]=7;
 hletters[-2]=8;
 hletters[-1]=9;
 hletters[0]=10;
 hletters[1]=11;
 hletters[2]=12;
 hletters[hvec(-2,-1)]=13;
 hletters[hvec(-1,-1)]=14;
 hletters[hvec(0,-1)]=15;
 hletters[hvec(1,-1)]=16;
 hletters[hvec(-2,-2)]=17;
 hletters[hvec(-1,-2)]=18;
 hletters[hvec(0,-2)]=19;
 }

void pnpattern(int size)
//size is at least 52 to see whole pattern
{hvec i;
 int j;
 for (i=start(size);i.cont(size);i.inc(size))
     {j=i.getx()*5+i.gety()*450;
      j%=8191;
      while (j<0) j+=8191;
      hletters[i]=31&pncode[j];
      }
 }

void pnbitpattern(int size)
//size is at least 15 to see whole pattern
{hvec i,i1;
 int j,k;
 for (i=start(size);i.cont(size);i.inc(size))
     for (k=0;k<12;k++)
         {i1=i*LETTERMOD+twelve[k];
          j=i1.getx()*5+i1.gety()*450;
          j%=8191;
          while (j<0) j+=8191;
          hbits[i1]=1&pncode[j];
          }
 }

void testletter()
{int i,j,c;
 bool letterbdy;
 hvec k;
 pattern19();
 letterbdy=true;
 b=LETTERMOD;
 debughvec=0;
 for (k=start(2);k.cont(2);k.inc(2))
     drawletter(hletters[k],k);
 border(2);
 for (i=-20;i<=20;i++)
     {for (j=-34;j<=34;j++)
          if ((i+j)&1)
             {a=hvec((j-i-1)/2,-i);
	      r=a%b;
	      c=i?j?' ':'|':'-';
	      if (letterbdy)
	         switch (r.letterinx())
	           {case 0:
	            case 3:
	            c='/';
	            break;
	            case 7:
	            case 10:
	            c='\\';
	            break;
	            case 2:
	            case 1:
	            c='_';
	            break;
	            }
	      putchar(c);
	      }
	  else
	     {a=hvec((j-i)/2,-i);
	      //q=a/b;
              //r=a%b;
	      //c=((q.getx()+22*q.gety())&31)+'@'+32*(r==hvec(0));
	      //c=137*sin(q.getx())+1840*cos(exp(1)*q.gety()+1);
	      //c=hletters[q];
	      //c=((letters[c&31]>>r.letterinx())&1)*10+32;
              c=(hbits[a]&1)*10+32;
              putchar(c);
              }
      putchar('\n');
      }
 }

void testoutline()
{int i,j,c;
 int size=2;
 bool letterbdy;
 hvec k;
 vector<hvec> contour;
 pnpattern(size);
 letterbdy=false;
 b=LETTERMOD;
 debughvec=0;
 for (k=start(size);k.cont(size);k.inc(size))
     drawletter(hletters[k],k);
 border(size);
 for (i=-25;i<=25;i++)
     {for (j=-37;j<=37;j++)
          if ((i+j)&1)
             {a=hvec((j-i-1)/2,-i);
	      r=a%b;
	      c=i?j?' ':'|':'-';
	      if (letterbdy)
	         switch (r.letterinx())
	           {case 0:
	            case 3:
	            c='/';
	            break;
	            case 7:
	            case 10:
	            c='\\';
	            break;
	            case 2:
	            case 1:
	            c='_';
	            break;
	            }
	      putchar(c);
	      }
	  else
	     {a=hvec((j-i)/2,-i);
	      //q=a/b;
              //r=a%b;
	      //c=((q.getx()+22*q.gety())&31)+'@'+32*(r==hvec(0));
	      //c=137*sin(q.getx())+1840*cos(exp(1)*q.gety()+1);
	      //c=hletters[q];
	      //c=((letters[c&31]>>r.letterinx())&1)*10+32;
              c=isedge(a)*10+32;
              putchar(c);
              }
      putchar('\n');
      }
 //contour=trace(5);
 for (i=0;i<contour.size();i++)
     printf("%d,%d ",contour[i].getx(),contour[i].gety());
 putchar('\n');
 psdraw(traceall(size),size,210,297,200,PS_DIAPOTHEM,0);
 // 0.07 is about my printer's inkspread
 }

void testdiv()
{int i,j,c;
 b=2;
 debughvec=1;
 for (i=0;i<7;i++)
     {a=hvec(0,i);
      q=a/b;
      r=a%b;
      printf("%d %d %d %d\n",q.getx(),q.gety(),r.getx(),r.gety());
      }
 debughvec=0;
 for (i=-10;i<=10;i++)
     {for (j=-34;j<=34;j++)
          if ((i+j)&1)
             putchar(i?j?' ':'|':'-');
	  else
	     {a=hvec((j-i)/2,-i);
	      q=a/b;
              r=a%b;
	      if (a!=q*b+r)
		 putchar('!');
              c=((q.getx()+20*q.gety())&31)+'@'+32*(r==hvec(0));
	      if (c==127) c=63;
              putchar(c);
              }
      putchar('\n');
      }
 printf("norm(b)=%d\n",b.norm());
 }

void testrem()
{int i,j,c;
 b=LETTERMOD;
 debughvec=0;
 for (i=-10;i<=10;i++)
     {for (j=-34;j<=34;j++)
          if ((i+j)&1)
             if (j==-34)
	        putchar(i?j?' ':'|':'-');
	     else;
	  else
	     {a=hvec((j-i)/2,-i);
	      q=a/b;
              r=a%b;
	      printf("%2d",r.letterinx());
              }
      putchar('\n');
      }
 printf("norm(b)=%d\n",b.norm());
 }

void fac8191()
{int x,y;
 hvec d,q,r;
 y=91;
 for (x=0;x<60;x++)
     {d=hvec(x,y);
      q=hvec(8191)/d;
      r=hvec(8191)%d;
      printf("%d %d norm=%ld q=%d %d r=%d %d\n",x,y,d.norm(),q.getx(),q.gety(),r.getx(),r.gety());
      }
 }

void initialize()
{initialize_ecc();
 fillpn();
 fillinvletters();
 }

void testsetdata()
{
  thematrix.setsize(7);
  thematrix.setndata(100);
  thematrix.setdata("LATE@ONE@MORNING@IN@THE@MIDDLE@OF@THE@NIGHT@TWO@DEAD@BOYS@GOT@UP@TO@FIGHT@BACK@TO@BACK@THEY@FACED@EACH@OTHER",5);
  thematrix.dump();
  thematrix.encode();
  thematrix.dump();
  thematrix.scramble();
  thematrix.dump();
  thematrix.unscramble();
  thematrix.dump();
}

void testencode()
{
  thematrix.setsize(3);
  thematrix.setndata(29);
  thematrix.setdata("@@@@@@@@@@@@@@@@@@@@@@@@@@@@A",5);
  thematrix.dump();
  thematrix.encode();
  thematrix.dump();
  thematrix.scramble();
  thematrix.dump();
  thematrix.unscramble();
  thematrix.dump();
}

int main(int argc,char **argv)
{
  initialize();
  testgalois();
  //testoutline();
  //listsizes();
  //testfindsize();
  testencode();
  //testshuffle();
  //testwhiten();
  return 0;
}
