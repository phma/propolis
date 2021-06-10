/******************************************************/
/*                                                    */
/* binio.cpp - binary input/output                    */
/*                                                    */
/******************************************************/
/* Copyright 2019 Pierre Abbat.
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
#include <cstring>
#include "binio.h"
#include "config.h"

using namespace std;

void endianflip(void *addr,int n)
{
  int i;
  char *addr2;
  addr2=(char *)addr;
  for (i=0;i<n/2;i++)
  {
    addr2[i]^=addr2[n-1-i];
    addr2[n-1-i]^=addr2[i];
    addr2[i]^=addr2[n-1-i];
  }
}

void writebeshort(std::ostream &file,short i)
{
  char buf[2];
  *(short *)buf=i;
#ifndef BIGENDIAN
  endianflip(buf,2);
#endif
  file.write(buf,2);
}

void writeleshort(std::ostream &file,short i)
{
  char buf[2];
  *(short *)buf=i;
#ifdef BIGENDIAN
  endianflip(buf,2);
#endif
  file.write(buf,2);
}

short readbeshort(std::istream &file)
{
  char buf[2];
  file.read(buf,2);
#ifndef BIGENDIAN
  endianflip(buf,2);
#endif
  return *(short *)buf;
}

short readleshort(std::istream &file)
{
  char buf[2];
  file.read(buf,2);
#ifdef BIGENDIAN
  endianflip(buf,2);
#endif
  return *(short *)buf;
}

void writebeint(std::ostream &file,int i)
{
  char buf[4];
  *(int *)buf=i;
#ifndef BIGENDIAN
  endianflip(buf,4);
#endif
  file.write(buf,4);
}

void writeleint(std::ostream &file,int i)
{
  char buf[4];
  *(int *)buf=i;
#ifdef BIGENDIAN
  endianflip(buf,4);
#endif
  file.write(buf,4);
}

int readbeint(std::istream &file)
{
  char buf[4];
  file.read(buf,4);
#ifndef BIGENDIAN
  endianflip(buf,4);
#endif
  return *(int *)buf;
}

int readleint(std::istream &file)
{
  char buf[4];
  file.read(buf,4);
#ifdef BIGENDIAN
  endianflip(buf,4);
#endif
  return *(int *)buf;
}

void writebelong(std::ostream &file,long long i)
{
  char buf[8];
  *(long long *)buf=i;
#ifndef BIGENDIAN
  endianflip(buf,8);
#endif
  file.write(buf,8);
}

void writelelong(std::ostream &file,long long i)
{
  char buf[8];
  *(long long *)buf=i;
#ifdef BIGENDIAN
  endianflip(buf,8);
#endif
  file.write(buf,8);
}

long long readbelong(std::istream &file)
{
  char buf[8];
  file.read(buf,8);
#ifndef BIGENDIAN
  endianflip(buf,8);
#endif
  return *(long long *)buf;
}

long long readlelong(std::istream &file)
{
  char buf[8];
  file.read(buf,8);
#ifdef BIGENDIAN
  endianflip(buf,8);
#endif
  return *(long long *)buf;
}

float readbefloat(std::istream &file)
{
  char buf[4];
  file.read(buf,4);
#ifndef BIGENDIAN
  endianflip(buf,4);
#endif
  return *(float *)buf;
}

float readlefloat(std::istream &file)
{
  char buf[4];
  file.read(buf,4);
#ifdef BIGENDIAN
  endianflip(buf,4);
#endif
  return *(float *)buf;
}

void writebefloat(std::ostream &file,float f)
{
  char buf[4];
  *(float *)buf=f;
#ifndef BIGENDIAN
  endianflip(buf,4);
#endif
  file.write(buf,4);
}

void writelefloat(std::ostream &file,float f)
{
  char buf[4];
  *(float *)buf=f;
#ifdef BIGENDIAN
  endianflip(buf,4);
#endif
  file.write(buf,4);
}

void writebedouble(std::ostream &file,double f)
{
  char buf[8];
  *(double *)buf=f;
#ifndef BIGENDIAN
  endianflip(buf,8);
#endif
  file.write(buf,8);
}

void writeledouble(std::ostream &file,double f)
{
  char buf[8];
  *(double *)buf=f;
#ifdef BIGENDIAN
  endianflip(buf,8);
#endif
  file.write(buf,8);
}

double readbedouble(std::istream &file)
{
  char buf[8];
  file.read(buf,8);
#ifndef BIGENDIAN
  endianflip(buf,8);
#endif
  return *(double *)buf;
}

double readledouble(std::istream &file)
{
  char buf[8];
  file.read(buf,8);
#ifdef BIGENDIAN
  endianflip(buf,8);
#endif
  return *(double *)buf;
}

void writeustring(ostream &file,string s)
// FIXME: if s contains a null character, it should be written as c0 a0
{
  file.write(s.data(),s.length());
  file.put(0);
}

string readustring(istream &file)
{
  int ch;
  string ret;
  do
  {
    ch=file.get();
    if (ch>0)
      ret+=(char)ch;
  } while (ch>0);
  return ret;
}
