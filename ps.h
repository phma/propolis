/******************************************************/
/*                                                    */
/* ps.h - PostScript output                           */
/*                                                    */
/******************************************************/

#include <string>
#include "hvec.h"
/* PS_XDIM: distance between bits
 * PS_LTR: distance between letters, sqrt(12)*PS_XDIM
 * PS_DIAMETER: diameter of the hexagon that encloses the symbol
 * PS_DIAPOTHEM: twice the apothem, sqrt(3/4)*PS_DIAMETER
 */
#define PS_XDIM 0
#define PS_LTR 1
#define PS_DIAPOTHEM 2
#define PS_DIAMETER 3
extern FILE *psfile;
extern int orientation;
void psprolog();
void startpage();
void endpage();
void pstrailer();
void psopen(char * psfname);
void psclose();
void widen(double factor);
void setcolor(double r,double g,double b);
void setscale(double minx,double miny,double maxx,double maxy);
void testpage();
void psdraw(vector<vector<hvec> > contours,int size,double width,double height,
	    double scale,int dim,double inkspread,string filename);
// All dimensions are in millimeters. If height and width
// are 0, they will be set to the symbol size.
