/******************************************************/
/*                                                    */
/* hamming.h - Hamming codes                          */
/*                                                    */
/******************************************************/
/* Copyright 2017-2023 Pierre Abbat.
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

#include <vector>
#include <string>

int databits(int codebits);
std::vector<int> databits(std::vector<int> codebits);
int totaldatabits(std::vector<int> codebits);

class Hamming
{
protected:
  std::vector<signed char> code;
  /* signed char is used for belief propagation, where the soft bit ranges
   * from -127 for 1 to 127 for 0. -127 means 1 so that * can mean xor.
   * code can also contain hard bits, normally one per byte.
   */
public:
  std::vector<signed char> getCode();
  void setCode(std::vector<signed char> Code);
  void push_back(char byte);
  std::string dumpLetters();
  std::vector<int> belief();
  void propagate();
};
