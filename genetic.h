/******************************************************/
/*                                                    */
/* genetic.h - genetic algorithm                      */
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
#include <array>
#include "letters.h"

class LetterMap
{
public:
  LetterMap();
  LetterMap(std::array<BIT16,5> init);
  LetterMap(LetterMap &mother,LetterMap &father);
  void computeFitness();
  double fitness();
  void mutate();
  friend bool operator==(LetterMap &l,LetterMap &r);
private:
  std::array<BIT16,32> bitPatterns;
  double fit;
};
