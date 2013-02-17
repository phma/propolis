/******************************************************/
/*                                                    */
/* ps.h - PostScript output                           */
/*                                                    */
/******************************************************/

#include <string>
#include <vector>
#include "hvec.h"
#include "outformat.h"
extern FILE *psfile;
extern int orientation;
void psprolog();
void startpage();
void endpage();
void pstrailer();
void psopen(const char * psfname);
void psclose();
void widen(double factor);
void setcolor(double r,double g,double b);
void setscale(double minx,double miny,double maxx,double maxy);
void testpage();
void plotpoint(double x,double y);
void psdraw(vector<vector<hvec> > contours,int size,double width,double height,
	    double scale,int dim,double inkspread,string filename);
// All dimensions are in millimeters. If height and width
// are 0, they will be set to the symbol size.
