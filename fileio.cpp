/******************************************************/
/*                                                    */
/* fileio.cpp - file input/output                     */
/*                                                    */
/******************************************************/
/* Copyright 2021 Pierre Abbat.
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
#include "binio.h"
#include "fileio.h"
using namespace std;

/* Files for storing hexagonal arrays:
 * Bytes 0-3: Magic numbers 71, 41, 12, 5
 * Bytes 4 and 5: Version number, currently 0
 * Byte 6: number of bits per element
 * Byte 7: PAGERAD
 * Bytes 8 and 9: PAGESIZE, native endianness
 * Bytes 10 and 11: additional information about an element format, currently 0000
 * This is followed by strips:
 * Bytes 0 and 1: x-coordinate of start of strip divided by PAGEMOD
 * Bytes 2 and 3: y-coordinate of start of strip divided by PAGEMOD
 * Bytes 4 and 5: number of pages in strip
 * Example (little-endian):
 * 47 29 0c 05 00 00 01 06 7f 00 00 00 fa ff fa ff 07 00 <7×16 bytes of data>
 * Numbers of bits are expected to be 1, 2 (for art masks), 8, and 16. If 16, the data
 * will be stored in two-byte words, with the same endianness as PAGESIZE.
 * At first the program will read only files with its native endianness and PAGERAD;
 * later it will be able to convert them.
 * The meaning of the magic numbers is:
 * 71/41: approximation to √3, indicating hexagonal
 * 12: number of bits in a letter's bit pattern
 * 5: number of bits in a letter.
 */

