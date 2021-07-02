/******************************************************/
/*                                                    */
/* propolis.cpp - main program                        */
/*                                                    */
/******************************************************/
/* Copyright 2013-2021 Pierre Abbat.
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
/* Hexagonal code
 */

#include <boost/program_options.hpp>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "hvec.h"
#include "pn8191.h"
#include "letters.h"
#include "contour.h"
#include "random.h"
#include "ps.h"
#include "fileio.h"
#include "arrange.h"
#include "ecctest.h"
#include "lagrange.h"
#include "encoding.h"
#include "raster.h"
#include "threads.h"
#include "genetic.h"

#define tassert(x) testfail|=(!(x))

using namespace std;
namespace po=boost::program_options;

hvec a,b,q,r;
harray<char> hletters,hbits;
bool testfail=false;

void border(int n)
// Draws a border for a symbol of size n. The border is one bigger.
{
  int i;
  n++;
  if (n<1)
    throw(range_error("border: size<0"));
  for (i=1;i<n;i++)
  {
    drawletter(0x20,hvec(n,i));
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

void pattern19()
{
  hletters[hvec(0,2)]=1;
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
{
  hvec i;
  int j;
  for (i=start(size);i.cont(size);i.inc(size))
  {
    j=i.getx()*5+i.gety()*450;
    j%=8191;
    while (j<0) j+=8191;
    hletters[i]=31&pncode[j][1];
  }
}

void pnbitpattern(int size)
//size is at least 15 to see whole pattern
{
  hvec i,i1;
  int j,k;
  for (i=start(size);i.cont(size);i.inc(size))
    for (k=0;k<12;k++)
    {
      i1=i*LETTERMOD+twelve[k];
      j=i1.getx()*5+i1.gety()*450;
      j%=8191;
      while (j<0) j+=8191;
      hbits[i1]=1&pncode[j][1];
    }
}

void bit256pattern(int size)
//size is at least 15 to see whole pattern
{
  hvec i,i1;
  int j,k;
  for (i=start(size);i.cont(size);i.inc(size))
    for (k=0;k<12;k++)
    {
      i1=i*LETTERMOD+twelve[k];
      j=i1.getx()*5+i1.gety()*450;
      j%=8191;
      while (j<0) j+=8191;
      hbits[i1]=(j&255)==0;
    }
}

void testletter()
{
  int i,j,c;
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
  {
    for (j=-34;j<=34;j++)
      if ((i+j)&1)
      {
	a=hvec((j-i-1)/2,-i);
	r=a%b;
	c=i?j?' ':'|':'-';
	if (letterbdy)
	  switch (r.letterinx())
	  {
	    case 0:
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
      {
	a=hvec((j-i)/2,-i);
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
{
  int i,j,c;
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
  {
    for (j=-37;j<=37;j++)
      if ((i+j)&1)
      {
	a=hvec((j-i-1)/2,-i);
	r=a%b;
	c=i?j?' ':'|':'-';
	if (letterbdy)
	  switch (r.letterinx())
	  {
	    case 0:
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
	{
	  a=hvec((j-i)/2,-i);
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
{
  int i,j,c;
  b=2;
  debughvec=1;
  for (i=0;i<7;i++)
  {
    a=hvec(0,i);
    q=a/b;
    r=a%b;
    printf("%d %d %d %d\n",q.getx(),q.gety(),r.getx(),r.gety());
  }
  debughvec=0;
  for (i=-10;i<=10;i++)
  {
    for (j=-34;j<=34;j++)
      if ((i+j)&1)
	putchar(i?j?' ':'|':'-');
      else
      {
	a=hvec((j-i)/2,-i);
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
{
  int i,j,c;
  b=LETTERMOD;
  debughvec=0;
  for (i=-10;i<=10;i++)
  {
    for (j=-34;j<=34;j++)
      if ((i+j)&1)
	if (j==-34)
	  putchar(i?j?' ':'|':'-');
	else;
      else
      {
	a=hvec((j-i)/2,-i);
	q=a/b;
	r=a%b;
	printf("%2d",r.letterinx());
      }
    putchar('\n');
  }
  printf("norm(b)=%ld\n",b.norm());
}

void fac8191()
{
  int x,y;
  hvec d,q,r;
  y=91;
  for (x=0;x<60;x++)
  {
    d=hvec(x,y);
    q=hvec(8191)/d;
    r=hvec(8191)%d;
    printf("%d %d norm=%ld q=%d %d r=%d %d\n",x,y,d.norm(),q.getx(),q.gety(),r.getx(),r.gety());
  }
}

void initialize()
{
  fillpn();
  fillLetters(0,0,0,0);
  readinvletters();
  startThreads(thread::hardware_concurrency());
}

void testlagrange()
{
  int i;
  poly31 poly=impulse(0x7f,3);
  for (i=0;i<31;i++)
    cout<<poly(int31(i)).locase();
  cout<<endl;
}

void testsetdata()
{
  hvec k;
  checkinvletters();
  theMatrix.findSize(108,0.2);
  theMatrix.setData("LATE@ONE@MORNING@IN@THE@MIDDLE@OF@THE@NIGHT@TWO@DEAD@BOYS@GOT@UP@TO@FIGHT@BACK@TO@BACK@THEY@FACED@EACH@OTHER",5);
  theMatrix.dump();
  theMatrix.arrange(hletters);
  for (k=start(theMatrix.getSize());k.cont(theMatrix.getSize());k.inc(theMatrix.getSize()))
    drawletter(hletters[k]&31,k);
  border(theMatrix.getSize());
  psdraw(traceall(theMatrix.getSize()),theMatrix.getSize(),210,297,200,DIM_DIAPOTHEM,0,"lateonemorning.ps");
}

void makesymbol(string text,int asize,double redundancy,int format,string outfilename)
{
  hvec k;
  int i,size;
  bool canfit;
  double red,hired,lored;
  vector<encoded> encodings;
  checkinvletters();
  encodings=encodedlist(text);
  for (i=0;i<encodings.size();i++)
  {
    if (asize<2 || redundancy>0)
      theMatrix.findSize(encodings[i].codestring.size(),redundancy);
    else
    {
      lored=0;
      hired=43/64.; // a little more than 2/3, which is the max
      size=0;
      do
      {
	red=(lored+hired)/2;
	size=theMatrix.findSize(encodings[i].codestring.size(),red);
	if (size>asize)
	  hired=red;
	if (size<asize)
	  lored=red;
      }
      while (size!=asize && red<2/3. && red>0.5/encodings[i].codestring.size());
    }
    canfit=(asize<2 && theMatrix.getSize()>1) || (asize>1 && theMatrix.getSize()==asize);
    if (canfit)
      break;
  }
  if (!canfit)
    throw(runtime_error("makesymbol: No encoding fits the specified size"));
  theMatrix.setData(encodings[i].codestring,encodings[i].encoding);
  theMatrix.arrange(hletters);
  size=theMatrix.getSize();
  redundancy=theMatrix.getRedundancy();
  for (k=start(size);k.cont(size);k.inc(size))
    drawletter(hletters[k]&31,k);
  border(size);
  switch (format)
  {
    case FMT_PS:
      psdraw(traceall(size),size,210,297,200,DIM_DIAPOTHEM,0,outfilename);
      break;
    case FMT_PNM:
      rasterdraw(size,0,0,600,DIM_DIAPOTHEM,format,outfilename);
      break;
    case FMT_HEXMAP:
      writeHexArray(outfilename,hbits,1,size);
      break;
    case FMT_INFO:
      cout<<"Redundancy: "<<redundancy<<endl;
      cout<<"Size: "<<size<<endl;
      cout<<"Encoding: "<<encodings[i].encoding<<endl;
      cout<<"Codetext: "<<encodings[i].codestring<<endl;
      break;
    default:
      cerr<<"Format should be pgm, ps, hmap, or info"<<endl;
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
    case PATTERN_MUL256:
      bit256pattern(size);
      letterpattern=0;
      break;
    default:
      cerr<<"Pattern should be 8191 or 8191b"<<endl;
  }
  if (letterpattern>1)
    theMatrix.arrange(hletters);
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

void testcrc()
{
  int i,j;
  int check;
  hvec center;
  for (i=0;i<32;i++)
  {
    check=crc(1<<i,256*i);
    tassert(check==1);
    //for (j=12;j>=0;j--)
      //cout<<((check>>j)&1);
    //cout<<endl;
  }
  hbits.clear();
  for (i=0;i>524288;i++)
  {
    for (j=0;j<19;j++)
      hbits[nthhvec(j,2,19)]=(i>>j)&1;
    if (hbits.crc()==0)
      printf("%05x %2d\n",i,bitcount(i));
  }
  /* Bit pattern 0x14005 has CRC 0 and only four bits set.
   *   * o o
   *  o o * o
   * o o o o o
   *  o o o o
   *   * o *
   * Place this pattern at a random position and make sure the CRC is still 0.
   */
  center=nthhvec(rng.rangerandom(1801),24,1801);
  hbits.clear();
  hbits[center+hvec(0,2)]=1;
  hbits[center+hvec(0,-2)]=1;
  hbits[center+hvec(-2,-2)]=1;
  hbits[center+hvec(1,1)]=1;
  printf("Center (%d,%d) CRC %x\n",center.getx(),center.gety(),hbits.crc());
  tassert(hbits.crc()==0);
}

void testmain()
{
  //testoutline();
  degauss();
  listsizes();
  testfindsize();
  testCheckLetters();
  testlagrange();
  //testsetdata();
  testenc();
  testencodings();
  //testcomplex(); // these three are slow
  //testraster();
  //testsixvec();
  testpageinx();
  testroundframe();
  testrotate();
  debugframingerror();
  checkregbits();
  writeAmbig();
  //findLetterAssignment();
  testcrc();
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
  cout<<"Propolis version "<<VERSION<<" © Pierre Abbat 2011-2021\nGPL v3 licensed\n";
}

int formatnum(string optstr)
{
  if (optstr=="pnm" || optstr=="pgm")
    return FMT_PNM;
  if (optstr=="ps")
    return FMT_PS;
  if (optstr=="hmap")
    return FMT_HEXMAP;
  if (optstr=="info")
    return FMT_INFO;
  cerr<<"Unrecognized output format"<<endl;
  return -1;
}

int patternnum(string optstr)
{
  if (optstr=="8191")
    return PATTERN_8191;
  if (optstr=="8191b")
    return PATTERN_8191B;
  if (optstr=="mul256")
    return PATTERN_MUL256;
  cerr<<"Unrecognized pattern"<<endl;
  return -1;
}

int lastSizeRed=0;

pair<string,string> checkSizeRed(const string &s)
{
  if (s.find("-s")==0 || s.find("--size")==0)
    lastSizeRed='s';
  if (s.find("-r")==0 || s.find("--red")==0)
    lastSizeRed='r';
  return make_pair(string(),string());
}

int main(int argc,char **argv)
{
  int testflag=0,option_index=0,makedata=0;
  bool geneletters=false,doEcctest=false;
  int c,quality;
  double redundancy=0;
  int size=0;
  string text,infilename,outfilename;
  stringbuf filebuf;
  string redundancyStr,formatStr,patternStr;
  fstream infile;
  int format=FMT_PS,pattern=0;
  bool validCmd=true,helpFlag=false;
  po::options_description generic("Options");
  po::options_description hidden("Hidden options");
  po::options_description cmdline_options;
  po::positional_options_description p;
  po::variables_map vm;
  generic.add_options()
    ("size,s",po::value<int>(&size),"Symbol size")
    ("redundancy,r",po::value<string>(&redundancyStr)->default_value("3/7"),"Redundancy (0,2/3]")
    ("text,t",po::value<string>(&text),"Text to encode")
    ("input,i",po::value<string>(&infilename),"File containing text to encode")
    ("output,o",po::value<string>(&outfilename),"Output file")
    ("format,f",po::value<string>(&formatStr)->default_value("ps"),"Output format")
    ("quality",po::value<int>(&quality)->default_value(1),"Quality of raster image (0-10)")
    ("pattern",po::value<string>(&patternStr),"Write a test pattern")
    ("writetables","Write decoding tables")
    ("geneletters","Optimize letters with genetic algorithm")
    ("ecctest","Test error correction")
    ("test","Run tests")
    ("help","Show options");
  initialize();
  cmdline_options.add(generic).add(hidden);
  debugletters=0;
  try
  {
    po::store(po::command_line_parser(argc,argv).options(cmdline_options)
	      .extra_parser(checkSizeRed).positional(p).run(),vm);
    po::notify(vm);
    if (vm.count("test"))
      testflag=1;
    if (vm.count("writetables"))
      makedata=1;
    if (vm.count("geneletters"))
      geneletters=true;
    if (vm.count("ecctest"))
      doEcctest=true;
    if (vm.count("help"))
      cout<<"Usage: propolis [options]\n"<<generic;
    cout<<"count(size)="<<vm.count("size")<<" count(redundancy)="<<vm.count("redundancy")<<" lastSizeRed="<<lastSizeRed<<endl;
  }
  catch (exception &ex)
  {
    cerr<<ex.what()<<endl;
    validCmd=false;
  }
  if (redundancyStr.length())
  {
    redundancy=parse_redundancy(redundancyStr);
    if (isnan(redundancy))
    {
      cerr<<"Could not parse redundancy: "<<redundancyStr<<endl;
      validCmd=false;
    }
  }
  if (size>1 && redundancy>0)
  {
    if (lastSizeRed=='s')
      redundancy=0;
    if (lastSizeRed=='r')
      size=0;
  }
  if (formatStr.length())
  {
    format=formatnum(formatStr);
    if (format<0)
      validCmd=false;
  }
  if (patternStr.length())
  {
    pattern=patternnum(patternStr);
    if (format<0)
      validCmd=false;
  }
  waitForThreads(TH_RUN);
  if (validCmd)
  {
    if (quality<0)
      quality=0;
    if (quality>10)
      quality=10;
    initsubsample(quality);
    if (makedata)
    {
      fillinvletters();
      writeinvletters();
    }
    if (geneletters)
      findLetterMapGenetic();
    if (doEcctest)
      ecctest();
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
  }
  waitForThreads(TH_STOP);
  joinThreads();
  if (testfail)
    cout<<"Test failed\n";
  return testfail;
}
