/******************************************************/
/*                                                    */
/* ps.h - PostScript output                           */
/*                                                    */
/******************************************************/
/* Copyright 2012-2017 Pierre Abbat.
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
void psdraw(std::vector<std::vector<hvec> > contours,int size,double width,double height,
	    double scale,int dim,double inkspread,std::string filename);
// All dimensions are in millimeters. If height and width
// are 0, they will be set to the symbol size.
