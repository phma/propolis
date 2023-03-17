/******************************************************/
/*                                                    */
/* hamming.cpp - Hamming codes                        */
/*                                                    */
/******************************************************/
/* Copyright 2017-2021 Pierre Abbat.
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
#include <cmath>
#include "hamming.h"
using namespace std;

/* Hamming codes, as used in Propolis, are of length at least 3. Each of
 * the five layers consists of Hamming code blocks; all five layers have
 * the same configuration of blocks. To pack as few data bits as possible,
 * use at most two 5-bit blocks, followed by 3-bit blocks. 2**n-bit blocks
 * are not used. Examples:
 * 13: 13 (9); 7,6 (7); 7,3,3 (6)
 * 31: 31 (26); 21,10 (22); 13,12,6 (20); 7,7,7,7,3 (17); 7,6,6,6,3,3 (15);
 *   7,6,6,3,3,3,3 (14); 7,6,3,3,3,3,3,3 (13); 7,3,3,3,3,3,3,3,3 (12)
 * 29694: 29694 (29679); ... 7425,(4)3712,(3)3711 (29597); (21)127,(429)63 (26973);
 *   ... (4242)7 (16968); (9898)3 (9898)
 */

int databits(int codebits)
{
  int ret=codebits;
  while (codebits)
  {
    codebits/=2;
    ret--;
  }
  return ret;
}

vector<int> databits(vector<int> codebits)
{
  int i;
  vector<int> ret;
  for (i=0;i<codebits.size();i++)
    ret.push_back(databits(codebits[i]));
  return ret;
}

int totaldatabits(vector<int> codebits)
{
  int i;
  int ret=0;
  for (i=0;i<codebits.size();i++)
    ret+=databits(codebits[i]);
  return ret;
}

vector<signed char> Hamming::getCode()
{
  return code;
}

void Hamming::setCode(vector<signed char> Code)
{
  code=Code;
}

void Hamming::push_back(char byte)
// Appends a data byte. Check bytes are added as needed.
{
  int i;
  while ((code.size()&(code.size()+1))==0)
    code.push_back(0);
  code.push_back(byte);
  for (i=1;i<=code.size();i<<=1)
    if (i&code.size())
      code[i-1]^=byte;
}

string Hamming::dumpLetters()
// Use this when the Hamming contains five-bit letters.
{
  int i;
  string ret;
  char ch;
  for (i=0;i<code.size();i++)
  {
    if (i&(i+1))
      ch=code[i]|'@';
    else
      ch=code[i]|'`';
    if (ch==127)
      ch='?';
    ret.push_back(ch);
  }
  return ret;
}

vector<int> Hamming::belief()
{
  int i;
  vector<int> ret;
  for (i=0;i<code.size();i++)
    ret.push_back(code[i]);
  return ret;
}

void Hamming::propagate()
/* Corrects the error in code fuzzily. Each number in code represents one bit,
 * with 127 meaning 0 and -127 meaning 1. If a number is 0, it remains 0,
 * even if setting it to ±127 would result in a correct code. Hopefully the 0
 * will be changed by the litteron in the next step.
 */
{
  vector<double> syndrome,adjusted;
  double max;
  int i,j,bit,sz=code.size();
  while (sz&(sz+1))
    sz|=sz>>1;
  for (i=1;i<=sz;i*=2)
  {
    syndrome.push_back(1);
    for (j=0;j<=sz;j++)
      if (j&i)
	syndrome.back()*=(j<=code.size())?code[j-1]:127;
  }
  max=0;
  for (i=0;i<syndrome.size();i++)
    if (fabs(syndrome[i])>max)
      max=fabs(syndrome[i]);
  if (max==0)
    max=1;
  for (i=0;i<syndrome.size();i++)
    syndrome[i]/=max;
  for (i=1;i<=code.size();i++)
  {
    adjusted.push_back(1);
    for (j=0;j<syndrome.size();j++)
    {
      bit=1-2*((i>>j)&1);
      adjusted.back()*=(1+(bit*syndrome[j]))/2;
    }
    adjusted.back()=(1-2*adjusted.back())*code[i-1];
  }
  max=0;
  for (i=0;i<adjusted.size();i++)
    if (fabs(adjusted[i])>max)
      max=fabs(adjusted[i]);
  if (max==0)
    max=1;
  for (i=0;i<adjusted.size();i++)
    code[i]=lrint(cbrt(adjusted[i]/max)*127);
}
