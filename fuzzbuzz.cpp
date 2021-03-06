/******************************************************/
/*                                                    */
/* fuzzbuzz.cpp - main program for fuzzing with AFL   */
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

#include <boost/program_options.hpp>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "fileio.h"
#include "letters.h"
#include "propolis.h"

using namespace std;
namespace po=boost::program_options;

hvec a,b,q,r;

void initialize()
{
  fillpn();
  fillLetters(0,0,0,0);
  readinvletters();
}

void copyleft()
{
  cout<<"Propolis version "<<VERSION<<" © Pierre Abbat 2011-2021\nGPL v3 licensed\n";
}

int main(int argc,char **argv)
{
  int testflag=0,option_index=0,makedata=0;
  bool geneletters=false,doEcctest=false;
  int c,quality;
  double redundancy=0;
  int size=0;
  string text,infilename,outfilename;
  stringbuf filebuf;
  string redundancyStr,formatStr,patternStr;
  fstream infile;
  Header header;
  bool validCmd=true,helpFlag=false;
  po::options_description generic("Options");
  po::options_description hidden("Hidden options");
  po::options_description cmdline_options;
  po::positional_options_description p;
  po::variables_map vm;
  generic.add_options()
    ("format,f",po::value<string>(&formatStr)->default_value("ps"),"Output format")
    ("quality",po::value<int>(&quality)->default_value(1),"Quality of raster image (0-10)")
    ("pattern",po::value<string>(&patternStr),"Write a test pattern")
    ("writetables","Write decoding tables")
    ("geneletters","Optimize letters with genetic algorithm")
    ("ecctest","Test error correction")
    ("test","Run tests")
    ("help","Show options");
  hidden.add_options()
    ("input",po::value<string>(&infilename),"File to read");
  p.add("input",-1);
  initialize();
  cmdline_options.add(generic).add(hidden);
  debugletters=0;
  try
  {
    po::store(po::command_line_parser(argc,argv).options(cmdline_options)
	      .positional(p).run(),vm);
    po::notify(vm);
    if (vm.count("test"))
      testflag=1;
    if (vm.count("writetables"))
      makedata=1;
    if (vm.count("geneletters"))
      geneletters=true;
    if (vm.count("ecctest"))
      doEcctest=true;
    if (vm.count("help"))
      cout<<"Usage: propolis [options]\n"<<generic;
  }
  catch (exception &ex)
  {
    cerr<<ex.what()<<endl;
    validCmd=false;
  }
  if (validCmd)
  {
    if (quality<0)
      quality=0;
    if (quality>10)
      quality=10;
    if (makedata)
    {
      fillinvletters();
      writeinvletters();
    }
    if (infilename.size())
    {
      header=readHexArray(infilename,hbits);
      cout<<"Read "<<hbits.listPages().size()<<" pages\n";
      cout<<"Size "<<header.size<<" Bits "<<header.bits<<endl;
    }
    else
    {
      copyleft();
      //cout<<"size "<<size<<" redundancy "<<redundancy<<" text "<<text<<endl;
    }
  }
  return 0;
}
