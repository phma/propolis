/******************************************************/
/*                                                    */
/* raster.h - raster image output                     */
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
#include <string>
#include "hvec.h"
#include "outformat.h"

struct locreg
{
  hvec location;
  int region;
  bool operator==(locreg b)
  {
    return this->location==b.location && this->region==b.region;
  }
};

void initsubsample(int q);
/* q=1: one dot per square
 * q=2: five dots per square
 * q=6: 85 dots per square, the usual
 * q=10: 221 dots per square, the most that makes sense
 */
void rasterdraw(int size,double width,double height,
	    double scale,int dim,int imagetype,std::string filename);
int filletbit(std::complex<double> z,harray<char> &canvas=hbits);
void checkregbits();
