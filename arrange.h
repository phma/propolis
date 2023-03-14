/******************************************************/
/*                                                    */
/* arrange.h - arrange letters in symbol              */
/*                                                    */
/******************************************************/
/* Copyright 2013-2023 Pierre Abbat.
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
#include "hamming.h"

int ndataletters(int n);
void listsizes();
void testfindsize();

class CodeMatrix
{
private:
  std::vector<Hamming> hammingBlocks;
  std::vector<int> hammingSizes;
  std::vector<char> metadata; // 6 or 7 letters, depending on size
  std::vector<char> data; // rearranged by criss-crossing
  std::vector<uint16_t> metaglyphs;
  std::vector<uint16_t> glyphs;
  int size,nLetters,nData,nDataCheck;
  /* Consider a size-3 symbol holding "1.618033988749894848204" in decimal
   * encoding. This is encoded as "_I_ZSJAA^\WM[^ZPFL". There must be a
   * check-count letter, so this takes at least 19 letters. The next space
   * available within size 3 is 20, so there is one check-padding letter.
   * The letters are arranged in the Hamming blocks as follows:
   * cc_cI_ZcSJAA^ cc\cWM[c^ZPF ccLc** where the first * is check-padding
   * and the second is check-count.
   * size=3; nLetters=31; nData=18; nDataCheck=20;
   */
public:
  int getSize();
  int getNLetters();
  int getNData();
  int getNDataCheck();
  double getRedundancy();
  int findSize(int n,double redundancy);
  void setDataCheck(std::string str,int encoding); // str should consist of letters @ through _; encoding should be 1-31
  void setData(std::string str,int encoding);
  void dump();
  void arrange(harray<char> &hletters);
  void unarrange(harray<uint16_t> &hglyphs);
};

extern CodeMatrix theMatrix;
void testbitctrot();
void testshuffle();
void testCheckLetters();
