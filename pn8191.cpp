/******************************************************/
/*                                                    */
/* pn8191.cpp - 8191-bit pseudonoise code             */
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
#include "pn8191.h"

const int poly13=0x2b85; //13 11 9 8 7 2 0
short pncode[8191][256];

void fillpn()
{
  int i,j,a,b;
  pncode[0][1]=1;
  pncode[0][0]=0;
  for (i=1;i<8191;i++)
  {
    pncode[i][1]=pncode[i-1][1]<<1;
    pncode[i][0]=0;
    if (pncode[i][1]&0x2000)
      pncode[i][1]^=poly13;
    //printf("%4x ",pncode[i]);
  }
  for (j=2;j<256;j*=2)
    for (i=0;i<8191;i++)
      pncode[i][j]=pncode[(i+8191-256)%8191][j/2];
  for (i=0;i<8191;i++)
    for (j=3;j<256;j++)
    {
      a=j&-j;
      b=j-a;
      if (b)
	pncode[i][j]=pncode[i][a]^pncode[i][b];
    }
}

int crc(unsigned n,hvec pos)
{
  int i,ipos,bytes[4],ret=0;
  for (i=0;i<4;i++)
    bytes[i]=(n>>(8*i))&255;
  ipos=(pos.getx()+90*pos.gety())%8191;
  if (ipos<0)
    ipos+=8191;
  for (i=0;i<4;i++)
    ret^=pncode[(ipos+(8191-2048)*i)%8191][bytes[i]];
  return ret;
}
