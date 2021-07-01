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
 * Bytes 8 and 9: PAGESIZE, little-endian
 * Bytes 10 and 11: symbol size
 * Bytes 12 and 13: additional information about an element format, currently 0000
 * Bytes 14 and 15: CRC of the hexagonal array
 * This is followed by strips:
 * Bytes 0 and 1: x-coordinate of start of strip divided by PAGEMOD
 * Bytes 2 and 3: y-coordinate of start of strip divided by PAGEMOD
 * Bytes 4 and 5: number of pages in strip
 * Example (little-endian):
 * 47 29 0c 05 00 00 01 06 7f 00 02 00 00 00 xx xx fa ff fa ff 07 00 <7×16 bytes of data>
 * Numbers of bits are expected to be 1, 2 (for art masks), 8, and 16. If 16, the data
 * will be stored in two-byte words, little-endian.
 * At first the program will read only files with its native PAGERAD;
 * later it will be able to convert them.
 * The meaning of the magic numbers is:
 * 71/41: approximation to √3, indicating hexagonal
 * 12: number of bits in a letter's bit pattern
 * 5: number of bits in a letter.
 */

void writeHeader(ostream &file,int bits,int size,int check)
{
  writebeint(file,0x47290c05);
  writeleshort(file,0);
  file.put(bits);
  file.put(PAGERAD);
  writeleshort(file,PAGESIZE);
  writeleshort(file,size);
  writeleshort(file,0);
  writeleshort(file,check);
}

void writeHvec(ostream &file,hvec h)
{
  writeleshort(file,h.getx());
  writeleshort(file,h.gety());
}

hvec readHvec(istream &file)
{
  int x,y;
  x=readleshort(file);
  y=readleshort(file);
  return hvec(x,y);
}

void writeHexArray(string fileName,harray<char> &hexArray,int bits,int size)
{
  ofstream file(fileName,ios::binary);
  vector<hvec> pages;
  vector<char> page,packedPage;
  int i,j,stripStart,stripEnd,mask=(1<<bits)-1;
  assert(bits>0 && bits<=8);
  hexArray.prune();
  writeHeader(file,bits,size,hexArray.crc());
  pages=hexArray.listPages();
  for (stripStart=0;stripStart<pages.size();stripStart=stripEnd)
  {
    for (stripEnd=stripStart;stripEnd<pages.size() && pages[stripEnd]-stripEnd==
         pages[stripStart]-stripStart;stripEnd++);
    writeHvec(file,pages[stripStart]);
    writeleshort(file,stripEnd-stripStart);
    for (i=stripStart;i<stripEnd;i++)
    {
      page=hexArray.getPage(pages[i]);
      packedPage.resize((PAGESIZE+7)*bits/8);
      for (j=0;j<packedPage.size();j++)
	packedPage[j]=0;
      for (j=0;j<page.size();j++)
	packedPage[(j*bits)/8]+=(page[j]&mask)<<((j*bits)%8);
      file.write(&packedPage[0],packedPage.size());
    }
  }
}

Header readHeader(istream &file)
/* bits is returned as -1 if the file was written with a different page size
 * than this program is compiled with. It is returned as -2 if the header is
 * invalid or the format is in the future.
 */
{
  int magic,version,pagesize,pagerad,extra;
  Header ret;
  magic=readbeint(file);
  version=readleshort(file);
  ret.bits=file.get()&255;
  pagerad=file.get()&255;
  pagesize=readleshort(file);
  ret.size=readleshort(file);
  extra=readleshort(file);
  ret.check=readleshort(file);
  if (pagerad!=PAGERAD || pagesize!=PAGESIZE)
    ret.bits=-1;
  if (pagesize!=pagerad*(pagerad+1)*3+1 || version>0 || magic!=0x47290c05)
    ret.bits=-2;
  return ret;
}

Header readHexArray(string fileName,harray<char> &hexArray)
{
  Header ret;
  ifstream file(fileName,ios::binary);
  int i,j,stripLength,mask;
  hvec stripStart;
  vector<char> page,packedPage;
  ret=readHeader(file);
  if (ret.bits>8)
    ret.bits=-3;
  if (ret.bits>0)
  {
    hexArray.clear();
    mask=(1<<ret.bits)-1;
    while (file.good() && ret.bits>0)
    {
      stripStart=readHvec(file);
      stripLength=readleshort(file);
      packedPage.resize((PAGESIZE+7)*ret.bits/8);
      page.resize(PAGESIZE);
      if (file.eof())
	stripLength=0;
      for (i=0;i<stripLength;i++)
      {
	file.read(&packedPage[0],packedPage.size());
	if (file.eof())
	  ret.bits=-4;
	for (j=0;j<PAGESIZE;j++)
	  page[j]=0;
	for (j=0;j<PAGESIZE;j++)
	  page[j]+=(packedPage[(j*ret.bits)/8]>>((j*ret.bits)%8))&mask;
	  packedPage[(j*ret.bits)/8]+=(page[j]&mask)<<((j*ret.bits)%8);
	hexArray.putPage(stripStart+i,page);
      }
    }
  }
  if (ret.bits>0 && hexArray.crc()!=ret.check)
    ret.bits=-5;
  return ret;
}
