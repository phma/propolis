/******************************************************/
/*                                                    */
/* outformat.h - output formats                       */
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
/* outformat.h
 * Definitions common to PostScript and raster output formats.
 * Eventually this will contain a class of which the PostScript and SVG output routines
 * will be derived classes.
 */
/* DIM_XDIM: distance between bits
 * DIM_LTR: distance between letters, sqrt(12)*PS_XDIM
 * DIM_DIAMETER: diameter of the hexagon that encloses the symbol
 * DIM_DIAPOTHEM: twice the apothem, sqrt(3/4)*PS_DIAMETER
 */
#ifndef OUTFORMAT_H
#define OUTFORMAT_H

#define DIM_XDIM 0
#define DIM_LTR 1
#define DIM_DIAPOTHEM 2
#define DIM_DIAMETER 3

#define FMT_PS 0
#define FMT_PNM 1
#define FMT_PNG 2
#define FMT_JPEG 3
#define FMT_HEXMAP 4
#define FMT_INFO 255

#define PATTERN_8191 1
#define PATTERN_8191B 2
#define PATTERN_MUL256 3

#endif
