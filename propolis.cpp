/* Hexagonal code
 */

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <getopt.h>
#include "hvec.h"
#include "letters.h"
#include "galois.h"
#include "contour.h"
#include "ps.h"
#include "rs.h"
#include "arrange.h"
#include "encoding.h"
#include "raster.h"

hvec a,b,q,r;
harray<char> hletters,hbits;

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
 psdraw(traceall(size),size,210,297,200,DIM_DIAPOTHEM,0,"outline.ps");
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
 printf("norm(b)=%ld\n",b.norm());
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
 printf("norm(b)=%ld\n",b.norm());
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
 initsubsample(1);
 readinvletters();
 }

void testsetdata()
{
  hvec k;
  checkinvletters();
  thematrix.setsize(7);
  thematrix.setndata(100);
  thematrix.setdata("LATE@ONE@MORNING@IN@THE@MIDDLE@OF@THE@NIGHT@TWO@DEAD@BOYS@GOT@UP@TO@FIGHT@BACK@TO@BACK@THEY@FACED@EACH@OTHER",5);
  thematrix.dump();
  thematrix.encode();
  thematrix.dump();
  thematrix.scramble();
  thematrix.dump();
  thematrix.arrange(hletters);
  thematrix.unscramble();
  thematrix.dump();
  for (k=start(thematrix.getsize());k.cont(thematrix.getsize());k.inc(thematrix.getsize()))
    drawletter(hletters[k]&31,k);
  border(thematrix.getsize());
  psdraw(traceall(thematrix.getsize()),thematrix.getsize(),210,297,200,DIM_DIAPOTHEM,0,"lateonemorning.ps");
}

void makesymbol(string text,int asize,double redundancy,int format,string outfilename)
{
  hvec k;
  int i,size;
  bool canfit;
  vector<encoded> encodings;
  checkinvletters();
  encodings=encodedlist(text);
  for (i=0;i<encodings.size();i++)
  {
    if (asize<2)
      size=findsize(encodings[i].codestring.size(),redundancy);
    else
      size=asize;
    thematrix.setsize(size);
    canfit=thematrix.setndata(encodings[i].codestring.size());
    if (canfit)
      break;
  }
  if (!canfit)
    throw(runtime_error("makesymbol: No encoding fits the specified size"));
  thematrix.setdata(encodings[i].codestring,encodings[i].encoding);
  thematrix.encode();
  thematrix.scramble();
  thematrix.arrange(hletters);
  thematrix.unscramble();
  for (k=start(thematrix.getsize());k.cont(thematrix.getsize());k.inc(thematrix.getsize()))
    drawletter(hletters[k]&31,k);
  border(thematrix.getsize());
  switch (format)
  {
    case FMT_PS:
      psdraw(traceall(thematrix.getsize()),thematrix.getsize(),210,297,200,DIM_DIAPOTHEM,0,outfilename);
      break;
    case FMT_PNM:
      rasterdraw(thematrix.getsize(),0,0,600,DIM_DIAPOTHEM,format,outfilename);
      break;
    case FMT_INFO:
      cout<<"Redundancy: "<<redundancy<<endl;
      cout<<"Size: "<<size<<endl;
      cout<<"Encoding: "<<encodings[i].encoding<<endl;
      cout<<"Codetext: "<<encodings[i].codestring<<endl;
      break;
    default:
      cerr<<"Format should be pgm, ps, or info"<<endl;
  }
}

void makepattern(int pattern,int asize,int format,string outfilename)
{
  hvec k;
  int i,size;
  int letterpattern; // 0: bit pattern; 1: unarranged letter pattern; 2: arranged letter pattern
  //checkinvletters();
  size=asize;
  switch (pattern)
  {
    case PATTERN_8191:
      pnpattern(size);
      letterpattern=1;
      break;
    case PATTERN_8191B:
      pnbitpattern(size);
      letterpattern=0;
      break;
    default:
      cerr<<"Pattern should be 8191 or 8191b"<<endl;
  }
  if (letterpattern>1)
    thematrix.arrange(hletters);
  if (letterpattern)
    for (k=start(size);k.cont(size);k.inc(size))
      drawletter(hletters[k]&31,k);
  border(size);
  switch (format)
  {
    case FMT_PS:
      psdraw(traceall(size),size,210,297,200,DIM_DIAPOTHEM,0,outfilename);
      break;
    case FMT_PNM:
      rasterdraw(size,0,0,1800,DIM_DIAPOTHEM,format,outfilename);
      break;
    case FMT_INFO:
      cout<<"Size: "<<size<<endl;
      break;
    default:
      cerr<<"Format should be pgm, ps, or info"<<endl;
  }
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

void testencodings()
{
  dumpenc(encodedlist("2147483648"));
  dumpenc(encodedlist("AOEUIDHTNS"));
}

void testraster()
{
  int size=16;
  pnbitpattern(size);
  border(size);
  rasterdraw(size,0,0,500,DIM_DIAPOTHEM,FMT_PNM,"8191.pgm");
}

void testmain()
{
  //testgalois();
  //testoutline();
  listsizes();
  //testfindsize();
  //testsetdata();
  //testshuffle();
  //testwhiten();
  //testenc();
  //testcomplex();
  //testraster();
  //testsixvec();
  testpageinx();
  testroundframe();
  testrotate();
  debugframingerror();
  checkregbits();
}

double stringtod(string str)
{
  char *endptr;
  double d;
  d=strtod(str.c_str(),&endptr);
  if (*endptr)
    return nan("");
  else
    return d;
}

double parse_redundancy(string red)
/* Parses a string representing a real number in one of three forms:
 * 0.45: leading zero is optional
 * 45%: decimal point is optional
 * 9/20: decimal points are optional
 * Returns NaN on error. Any non-numeric character other than '/' or '%' is an error,
 * as is having more than one '/' or '%'.
 */
{
  size_t firstslash,lastslash,firstpercent,lastpercent,separator;
  string left,right;
  double dleft,dright,result;
  firstslash=red.find_first_of('/');
  lastslash=red.find_last_of('/');
  firstpercent=red.find_first_of('%');
  lastpercent=red.find_last_of('%');
  if (firstslash!=lastslash || firstpercent!=lastpercent || (firstpercent!=string::npos && firstslash!=string::npos))
    result=nan("");
  else
  {
    separator=firstpercent+firstslash-string::npos;
    left=red.substr(0,separator);
    right=red.substr(separator+1,string::npos);
    dleft=stringtod(left);
    dright=stringtod(right);
    if (firstpercent!=string::npos)
      if (right.size())
        result=nan("");
      else
	result=dleft/100;
    else if (firstslash!=string::npos)
      result=dleft/dright;
    else
      result=dleft;
  }
  return result;
}

void copyleft()
{
  cout<<"Propolis version "<<VERSION<<" © Pierre Abbat 2011-2017\nReed-Solomon code by Henry Minsky\nGPL v3 licensed\n";
}

int formatnum(const char *optarg)
{
  string optstr(optarg);
  if (optstr=="pnm" || optstr=="pgm")
    return FMT_PNM;
  if (optstr=="ps")
    return FMT_PS;
  if (optstr=="info")
    return FMT_INFO;
  cerr<<"Unrecognized output format"<<endl;
  return -1;
}

int patternnum(const char *optarg)
{
  string optstr(optarg);
  if (optstr=="8191")
    return PATTERN_8191;
  if (optstr=="8191b")
    return PATTERN_8191B;
  cerr<<"Unrecognized pattern"<<endl;
  return -1;
}

int main(int argc,char **argv)
{
  int testflag=0,option_index=0,makedata=0;
  int c,quality;
  double redundancy=0;
  int size=0;
  string text,infilename,outfilename;
  stringbuf filebuf;
  fstream infile;
  int format=FMT_PS,pattern=0;
  static option long_options[]=
  {
    {"test",       no_argument,      0,0},
    {"size",       required_argument,0,0},
    {"redundancy", required_argument,0,0},
    {"text",       required_argument,0,0},
    {"writetables",no_argument,      0,0},
    {"format",     required_argument,0,0},
    {"input",      required_argument,0,0},
    {"output",     required_argument,0,0},
    {"pattern",    required_argument,0,0},
    {"quality",    required_argument,0,0},
    {0,            0,                0,0}
  };
  initialize();
  while (1)
  {
    option_index=-1;
    c=getopt_long(argc,argv,"s:r:t:f:i:o:",long_options,&option_index);
    if (c<0)
      break;
    switch (c)
    {
      case 0:
	//printf("option %d\n",option_index);
	break;
      case 's':
	option_index=1;
	break;
      case 'r':
	option_index=2;
	break;
      case 't':
	option_index=3;
	break;
      case 'f':
	option_index=5;
	break;
      case 'i':
	option_index=6;
	break;
      case 'o':
	option_index=7;
	break;
      default:
	printf("c=%d\n",c);
    }
    switch (option_index)
    {
      case 0:
	testflag=1;
	break;
      case 1:
	size=atoi(optarg);
	redundancy=nan("");
	break;
      case 2:
	redundancy=parse_redundancy(optarg);
	if (redundancy>0)
	  size=0;
	else
	  cerr<<"Could not parse redundancy: "<<optarg<<endl;
	break;
      case 3:
	text=optarg;
	break;
      case 4:
	makedata=1;
	break;
      case 5:
	format=formatnum(optarg);
	break;
      case 6:
	infilename=optarg;
	break;
      case 7:
	outfilename=optarg;
	break;
      case 8:
	pattern=patternnum(optarg);
	break;
      case 9:
	quality=atoi(optarg);
	if (quality<0)
	  quality=0;
	if (quality>10)
	  quality=10;
	initsubsample(quality);
	break;
    }
  }
  if (makedata)
  {
    fillinvletters();
    writeinvletters();
  }
  if (testflag)
    testmain();
  else if (pattern)
    makepattern(pattern,size,format,outfilename);
  else if (text.size())
  {
    makesymbol(text,size,redundancy,format,outfilename);
  }
  else if (infilename.size())
  {
    infile.open(infilename.c_str(),ios_base::in|ios_base::binary);
    infile>>&filebuf;
    makesymbol(filebuf.str(),size,redundancy,format,outfilename);
    infile.close();
  }
  else
  {
    copyleft();
    //cout<<"size "<<size<<" redundancy "<<redundancy<<" text "<<text<<endl;
  }
  return 0;
}
