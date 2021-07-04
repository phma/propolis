/******************************************************/
/*                                                    */
/* ps.cpp - PostScript output                         */
/*                                                    */
/******************************************************/
/* Copyright 2012-2021 Pierre Abbat.
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

/* The PostScript file is 35.7 times the information contained in it;
 * when gzipped, it is 5.85 times as big as the information.
 * Converted to PDF, it gets bigger, 147 times as big as the information.
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include "hvec.h"
#include "ps.h"
#include "letters.h"

using namespace std;

struct arc
{hvec center;
 char letter;
 };

FILE *psfile;
int pages;
double scale=1;
int orientation;

void widen(double factor)
{fprintf(psfile,"currentlinewidth %f mul setlinewidth\n",factor);
 }

void setcolor(double r,double g,double b)
{fprintf(psfile,"%f %f %f setrgbcolor\n",r,g,b);
 }

void psopen(const char * psfname)
{
  if (strlen(psfname))
    psfile=fopen(psfname,"w");
  else
    psfile=stdout;
}

void psclose()
{
  if (psfile!=stdout)
  {
    fclose(psfile);
    printf("scale=%f\n",scale);
    sleep(3);
  }
}

/* PostScript prolog.
 * hmove takes an hvec on the stack (as two numbers) and moves relatively.
 * The arcs are represented by single letters a-r and A-R.
 * A 330 to 30  G 30 to 150  M 90 to 270
 * B 30 to 90   H 90 to 210  N 150 to 330
 * C 90 to 150  I 150 to 270 O 210 to 30
 * D 150 to 210 J 210 to 330 P 270 to 90
 * E 210 to 270 K 270 to 30  Q 330 to 150
 * F 270 to 330 L 330 to 90  R 30 to 210
 * S, T, and U are used when starting a new subpath.
 * F, K, and P use S; J and O use T; N uses U.
 * The scanning proceeds from bottom to top in lines that go
 * from left to right, so these are the only letters a subpath can start with.
 * A whole circle (isolated dot) is FABCDE before compression, then PCDE, then W.
 */
void psprolog()
{fprintf(psfile,"%%!PS-Adobe-3.0\n\
%%%%BeginProlog\n\
%%%%Pages: (atend)\n\
%%%%BoundingBox: 0 0 596 843\n\
%% A4 paper.\n\
\n\
/. %% ( x y )\n\
{ newpath 0.3 0 360 arc fill } bind def\n\
\n\
/- %% ( x1 y1 x2 y2 )\n\
{ newpath moveto lineto stroke } bind def\n\
\n\
/mmscale { 720 254 div dup scale } bind def\n\
\n\
/hmove %% ( x1 y1 x2 y2 - x1' y1' )\n\
{ -2 div exch 1 index 3 sqrt mul neg 3 1 roll add exch\n\
  3 -1 roll add 3 1 roll add exch } bind def\n\
\n\
/rad 3 sqrt 4 div def\n\
/inkspread 0 def\n\
/brad { rad inkspread sub } def\n\
/wrad { rad inkspread add } def\n\
\n\
%% black left, white left - left is inside contour\n\
/bl { /lrad { brad } def /rrad { wrad } def } def\n\
/wl { /lrad { wrad } def /rrad { brad } def } def\n\
/A %% ( x y )\n\
{ hmove 1 index 1 index lrad -30 30 arc } bind def\n\
/B %% ( x y )\n\
{ hmove 1 index 1 index lrad 30 90 arc } bind def\n\
/C %% ( x y )\n\
{ hmove 1 index 1 index lrad 90 150 arc } bind def\n\
/D %% ( x y )\n\
{ hmove 1 index 1 index lrad 150 210 arc } bind def\n\
/E %% ( x y )\n\
{ hmove 1 index 1 index lrad 210 270 arc } bind def\n\
/F %% ( x y )\n\
{ hmove 1 index 1 index lrad 270 330 arc } bind def\n\
/a %% ( x y )\n\
{ hmove 1 index 1 index rrad 30 -30 arcn } bind def\n\
/b %% ( x y )\n\
{ hmove 1 index 1 index rrad 90 30 arcn } bind def\n\
/c %% ( x y )\n\
{ hmove 1 index 1 index rrad 150 90 arcn } bind def\n\
/d %% ( x y )\n\
{ hmove 1 index 1 index rrad 210 150 arcn } bind def\n\
/e %% ( x y )\n\
{ hmove 1 index 1 index rrad 270 210 arcn } bind def\n\
/f %% ( x y )\n\
{ hmove 1 index 1 index rrad 330 270 arcn } bind def\n\
/G %% ( x y )\n\
{ hmove 1 index 1 index lrad 30 150 arc } bind def\n\
/H %% ( x y )\n\
{ hmove 1 index 1 index lrad 90 210 arc } bind def\n\
/I %% ( x y )\n\
{ hmove 1 index 1 index lrad 150 270 arc } bind def\n\
/J %% ( x y )\n\
{ hmove 1 index 1 index lrad 210 330 arc } bind def\n\
/K %% ( x y )\n\
{ hmove 1 index 1 index lrad -90 30 arc } bind def\n\
/L %% ( x y )\n\
{ hmove 1 index 1 index lrad -30 90 arc } bind def\n\
/g %% ( x y )\n\
{ hmove 1 index 1 index rrad 150 30 arcn } bind def\n\
/h %% ( x y )\n\
{ hmove 1 index 1 index rrad 210 90 arcn } bind def\n\
/i %% ( x y )\n\
{ hmove 1 index 1 index rrad 270 150 arcn } bind def\n\
/j %% ( x y )\n\
{ hmove 1 index 1 index rrad 330 210 arcn } bind def\n\
/k %% ( x y )\n\
{ hmove 1 index 1 index rrad 30 -90 arcn } bind def\n\
/l %% ( x y )\n\
{ hmove 1 index 1 index rrad 90 -30 arcn } bind def\n\
/M %% ( x y )\n\
{ hmove 1 index 1 index lrad 90 270 arc } bind def\n\
/N %% ( x y )\n\
{ hmove 1 index 1 index lrad 150 330 arc } bind def\n\
/O %% ( x y )\n\
{ hmove 1 index 1 index lrad -150 30 arc } bind def\n\
/P %% ( x y )\n\
{ hmove 1 index 1 index lrad -90 90 arc } bind def\n\
/Q %% ( x y )\n\
{ hmove 1 index 1 index lrad -30 150 arc } bind def\n\
/R %% ( x y )\n\
{ hmove 1 index 1 index lrad 30 210 arc } bind def\n\
/m %% ( x y )\n\
{ hmove 1 index 1 index rrad 270 90 arcn } bind def\n\
/n %% ( x y )\n\
{ hmove 1 index 1 index rrad 330 150 arcn } bind def\n\
/o %% ( x y )\n\
{ hmove 1 index 1 index rrad 30 -150 arcn } bind def\n\
/p %% ( x y )\n\
{ hmove 1 index 1 index rrad 90 -90 arcn } bind def\n\
/q %% ( x y )\n\
{ hmove 1 index 1 index rrad 150 -30 arcn } bind def\n\
/r %% ( x y )\n\
{ hmove 1 index 1 index rrad 210 30 arcn } bind def\n\
/S\n\
{ hmove 1 index 1 index lrad sub moveto 0 0 } bind def\n\
/T\n\
{ hmove 1 index lrad 0.75 sqrt mul sub 1 index lrad 2 div sub moveto 0 0 } bind def\n\
/U\n\
{ hmove 1 index lrad 0.75 sqrt mul sub 1 index lrad 2 div add moveto 0 0 } bind def\n\
/V\n\
{ closepath } def\n\
/W %% ( x y )\n\
{ hmove 1 index 1 index lrad -90 270 arc } bind def\n\
%%%%EndProlog\n\
");
 pages=0;
 //scale=10;
 fflush(psfile);
 }

const char combinetable[]="ABLbalBCGcbgCDHdchDEIediEFJfejFAKafkLCQgaqGDRhbrHEMicmIFNjdnJAOkeoKBPlfpPCTTDUUEW";

void pstrailer()
{fprintf(psfile,"%%%%BeginTrailer\n\
%%%%Pages: %d\n\
%%%%EndTrailer\n\
",pages);
 }

void startpage()
{++pages;
 fprintf(psfile,"%%%%Page: %d %d\ngsave\n",pages,pages);
 }

void endpage()
{fputs("grestore showpage\n",psfile);
 fflush(psfile);
 }

void plotpoint(double x,double y)
{
  fprintf(psfile,"%.1f %.1f .\n",(x+105)*360/127,(y+148.5)*360/127);
}

void testpage()
{psopen("propolis.ps");
 psprolog();
 startpage();
 fprintf(psfile,"100 100 newpath 0 1 B -1 0 C -1 -1 D 0 -1 E 1 0 F 1 1 A closepath stroke pop pop\n");
 endpage();
 pstrailer();
 psclose();
 }

void psdraw(vector<vector<hvec> > contours,int size,double width,double height,
	    double scale,int dim,double inkspread,string filename)
/* inkspread is the amount by which ink spreads when printing.
 * It is subtracted from the black areas. It is in millimeters.
 * The max is 0.433 X units.
 */
{int i,j,k,narc;
 hvec bend,dir,center,lastcenter,jump;
 char letter;
 vector<arc> arclist;
 arc marc;
 double symwidth,symheight;
 psopen(filename.c_str());
 psprolog();
 startpage();
 switch (dim)
    {case DIM_LTR:
     scale/=sqrt(12);
     break;
     case DIM_DIAMETER:
     scale=scale/(sqrt(48)*(size+2));
     break;
     case DIM_DIAPOTHEM:
     scale=scale/(6*(size+2));
     break;
     case DIM_XDIM:
     break;
     default:
     throw(invalid_argument("psdraw: unknown dimension"));
     }
 if (scale<=0)
    throw(range_error("psdraw: scale must be positive"));
 if (fabs(inkspread)>0.433*scale)
    printf("Inkspread %f clipped to %f\n",inkspread,0.433*scale);
 if (inkspread>0.433*scale)
    inkspread=0.433*scale;
 if (inkspread<-0.433*scale)
    inkspread=-0.433*scale;
 symwidth=scale*(6*(size+2));
 symheight=scale*(sqrt(48)*(size+2));
 if (width<0 || height<0)
    throw(range_error("psdraw: paper size must be nonnegative"));
 if (!height)
    height=symheight;
 if (!width)
    width=symwidth;
 fprintf(psfile,"%f dup scale /inkspread %f def\n",scale*360/127,inkspread/scale);
 // 360/127 is a millimeter in points
 fprintf(psfile,"%f %f\n",width/2/scale,(height/2+scale/sqrt(3))/scale);
 // scale/sqrt(3) corrects for that the origin of the coordinate system is in one dot,
 // but the center of the symbol is in the center of a triangle, one of whose vertices
 // is the origin.
 /*fprintf(psfile,"newpath %f %f moveto %f %f lineto %f %f lineto %f %f lineto %f %f lineto %f %f lineto closepath stroke\n",
	 symwidth/2/scale,0.0,symwidth/scale,symheight/4/scale,symwidth/scale,3*symheight/4/scale,
	 symwidth/2/scale,symheight/scale,0.0,3*symheight/4/scale,0.0,symheight/4/scale);*/
 lastcenter=0;
 for (i=0;i<contours.size();i++)
     {// Repeat the last two points, as I'm going to scan it with a three-point window.
      contours[i].push_back(contours[i][0]);
      contours[i].push_back(contours[i][1]);
      // Find all the bends and assign them letters.
      for (j=0;j<contours[i].size()-2;j++)
          {dir=contours[i][j+2]-contours[i][j];
           bend=contours[i][j+2]+contours[i][j]-contours[i][j+1]*2;
           center=bend+contours[i][j+1];
           if (bend.norm())
	      {//printf("Bend at %d %d bend=%d %d\n",center.getx(),center.gety(),bend.getx(),bend.gety());
	       if (bend==hvec(-1,0))
	          letter='A';
	       if (bend==hvec(-1,-1))
	          letter='B';
	       if (bend==hvec(0,-1))
	          letter='C';
	       if (bend==hvec(1,0))
	          letter='D';
	       if (bend==hvec(1,1))
	          letter='E';
	       if (bend==hvec(0,1))
	          letter='F';
	       if (dir/bend==hvec(1,2))
	          letter+='a'-'A';
	       marc.center=center;
	       marc.letter=letter;
	       arclist.push_back(marc);
	       }
           }
      // If the last bend has the same center as the first, move it to the front,
      // so that they can be combined. But if arclist[0] has the same center as
      // arclist[3], then the contour is just a tiny circle, so don't bother.
      while (arclist[0].center==arclist[arclist.size()-1].center && arclist[0].center!=arclist[3].center)
        {arclist.insert(arclist.begin(),arclist.back());
         arclist.pop_back();
         }
      // Combine all consecutive bends with the same center.
      for (j=0;j<arclist.size()-1;j++)
	  for (k=0;arclist[j].center==arclist[j+1].center && k<81;k+=3)
	      if (arclist[j].letter==combinetable[k] && arclist[j+1].letter==combinetable[k+1])
	         {arclist[j].letter=0;
	          arclist[j+1].letter=combinetable[k+2];
	          }
      // Make sure that the 0th element is null and the 1st is the first actual bend.
      if (arclist[0].letter)
         {arclist.insert(arclist.begin(),arclist[0]);
          arclist[0].letter=0;
          }
      if (!arclist[1].letter)
         {arclist[1].letter=arclist[2].letter;
          arclist[2].letter=0;
          }
      // Insert a code for moveto before the first bend.
      switch (arclist[1].letter)
         {case 'F':
          case 'K':
          case 'P':
	  case 0: // isolated dot, letter is W but it's arclist[5]
          arclist[0].letter='S';
          break;
          case 'J':
          case 'O':
          arclist[0].letter='T';
          break;
          case 'N':
          arclist[0].letter='U';
          break;
	  default:
	  throw(logic_error("psdraw: first arc of subpath is "+arclist[0].letter));
          }
      // Write whether black or white is on the left (inside) of this contour.
      // This affects the inkspread compensation.
      if (hbits[arclist[0].center/2]&1)
	 fprintf(psfile,"bl ");
      else
         fprintf(psfile,"wl ");
      for (j=0;j<arclist.size();j++)
          if (arclist[j].letter)
	     {narc=(narc+1)&63;
	      jump=arclist[j].center-lastcenter;
	      if (jump.norm())
	         fprintf(psfile,"%d %d %c%c",jump.getx()/2,
		         jump.gety()/2,arclist[j].letter,narc?' ':'\n');
	      else
	         fprintf(psfile,"%c%c",arclist[j].letter,narc?' ':'\n');
	      lastcenter=arclist[j].center;
	      }
      fprintf(psfile,"V ");
      arclist.clear();
      }
 fprintf(psfile,"eofill pop pop\n");
 endpage();
 pstrailer();
 psclose();
 }
